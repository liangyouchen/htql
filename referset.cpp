#include "referset.h"
#include "stroper.h"
#include "expr.h"
#include "refersqlparser.h"
#include "iwsqlsyntax.h"
#include "htmlql.h"
#include "qhtql.h"

#if (!defined(NOIWEB) && !defined(NOIWIRB))
#include "iwdriverinterface.h"
#endif


#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#define DEBUG_THIS_FILE
#endif


ReferSet::ReferSet(const char* name){
	FieldNames=0;
	FieldsNum=0;
	TupleCount=0;
	DataCursor=0;
	QueryTuple=0;
	RowNum=0;
	if (name) ReferSetName=name;
}
ReferSet::~ReferSet(){
	reset();
}
void ReferSet::reset(){
	empty();

	ReferLink* link;
	for (link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
		ReferSetBTree* index=(ReferSetBTree*) link->Value.P;
		if (index) delete index;
		link->Value.P=0;
	}
	IndexNameList.reset();
	UniqueIndexList.reset();
	
	if (FieldNames) {
		free(FieldNames);
		FieldNames=0;
	}
	FieldNameIndex.reset();
	FieldsNum=0;
	RowNum=0;
	if (QueryTuple) {
		delete[] QueryTuple;
		QueryTuple=0;
	}
}
int ReferSet::empty(){
	ReferLink* link;
	for (link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
		ReferSetBTree* index=(ReferSetBTree*) link->Value.P;
		if (index) index->resetBTree();
	}

	for (link=DataHead.Next; link && link!=(ReferLink*) &DataHead; link=link->Next){
		ReferData* tuple=(ReferData*) link->Value.P;
		if (link->Value.L && tuple) delete[] tuple;
		link->Value.P=0;
	}
	DataHead.reset();

	TupleCount=0;
	DataCursor=0;
	RowNum=0;
	UseIndex.reset();
	return 0;
}

int ReferSet::setFieldsNum(long fieldsnum, const char**fieldnames){
	reset(); 

	FieldsNum=fieldsnum;
	if (FieldsNum==0 && fieldnames){
		for (long i=0; fieldnames[i]; i++){
			FieldsNum++;
		}
	}
	if (FieldsNum>0){
		FieldNames=(ReferData**) malloc(sizeof(ReferData*)*FieldsNum);
		memset(FieldNames, 0, sizeof(ReferData*)*FieldsNum);
	}
	if (fieldnames){
		for (long i=0; i<FieldsNum; i++){
#ifdef _WINDOWS
			ASSERT(fieldnames[i]);
#endif
			setFieldName(i, fieldnames[i]);
		}
	}
	if (QueryTuple) {
		delete [] QueryTuple; 
		QueryTuple=0;
	}
	return 0;
}
int ReferSet::setFieldName(long fieldindex0, const char* fieldname){
	if (fieldindex0>=FieldsNum){
		ReferData** newnames=(ReferData**) malloc(sizeof(ReferData*)*(fieldindex0+1));
		memset(newnames, 0, sizeof(ReferData*)*(fieldindex0+1));

		if (FieldNames){
			memcpy(newnames, FieldNames, sizeof(ReferData*)*FieldsNum);
			free(FieldNames);
		}
		FieldNames=newnames;

		FieldsNum=fieldindex0+1;
		if (QueryTuple) {
			delete [] QueryTuple; 
			QueryTuple=0;
		}
		resizeAllTuples(fieldindex0, fieldindex0+1);
		rebuildAllIndex();
	}
	if (fieldname){
		ReferData name;
		name.Set((char*) fieldname, fieldname?strlen(fieldname):0, false);
		ReferLink* link=FieldNameIndex.findName(&name);
		if (link){
			link->Data=fieldindex0;
		}else{
			link=FieldNameIndex.add(&name, 0, fieldindex0);
		}
		FieldNames[fieldindex0]=&link->Name;
	}

	return 0;
}
ReferData* ReferSet::getQueryTuple(ReferLinkHeap* name_values, int caller_delete){
	long i;

	if (!QueryTuple) QueryTuple=new ReferData[FieldsNum];
	for (i=0; i<FieldsNum; i++) QueryTuple[i].reset(); 

	if (name_values){
		for (ReferLink* link=name_values->getReferLinkHead(); link; link=link->Next){
			i=getFieldIndex(&link->Name);
			if (i>=0) QueryTuple[i].Set(link->Value.P, link->Value.L,link->Value.Type);
		}
	}

	ReferData* query_tuple=QueryTuple;
	if (caller_delete) {
		QueryTuple=0; 
	}

	return query_tuple;
}
ReferData* ReferSet::borrowTuple(ReferSet* from, int force_borrow){
	//force_borrow=0:by reference; =1:borrow tuple; =2:new and copy tuple
	if (!from->DataCursor || from==this) return 0;
	ReferData* tuple=0;

	if (force_borrow==0){ //=0: by reference; 
		tuple=newTuple((ReferData*) from->DataCursor->Value.P, false); //the new results are referencing to this dataset

	}else if (force_borrow==1){ //=1:borrow tuple;
		tuple=newTuple((ReferData*) from->DataCursor->Value.P, from->DataCursor->Value.L);
		from->DataCursor->Value.L=0;

	}else{ //=2:new and copy tuple
		ReferData* tuple0=(ReferData*) from->DataCursor->Value.P; 
		tuple=newTuple();
		for (long i=0; i<FieldsNum; i++) tuple[i].Set(tuple0[i].P, tuple0[i].L, tuple0[i].Type);

	}
	return tuple;
}

long ReferSet::filterByCondition(ReferData* condition, ReferSet* results, tExprCalc* context, int to_delete, int force_borrow){
	condition->Seperate();
	if (condition && condition->P) {
		tStrOp::convertSpaceSpecial(condition->P);
		condition->L=strlen(condition->P);
	}
	return filterByCondition(condition->P, results, context, to_delete, force_borrow);
}
long ReferSet::filterByCondition(const char* condition, ReferSet* results, tExprCalc* context, int to_delete, int force_borrow){
	//to_delete: 0=select the tuples satisfying the conditions from this to results
	//			 1=select all tuples to results, and delete the ones satisfying the condition from results
	//			 2=move the tuples satisfying the condition from this to results
	//force_borrow=0:by reference; =1:borrow tuple; =2:new and copy tuple
	//	force_borrow is only valid when results!=this

	if (!results) results=this;
	commitTuple();

	tExprCalc expr;
	if (context) expr.useContext(context, true);
	expr.setExpression((char*) condition);
	int err=expr.parse(synEXP_RIGHTBRACE);
	if (err<0) return err;

	ReferLinkHeap assign_names;
	tExprCalc::checkAssignValueExpression(context, expr.Root, &assign_names);
	ReferSetBTree* index=chooseIndex(&assign_names);

	ReferData* query_tuple=getQueryTuple(&assign_names);
	if (!index) query_tuple=0;
	
	long count=0;
	long rownum=0;
	ReferLink2* link, *link1;
	ReferData* tuple;
	if (to_delete==1){ //select all tuples to results, and delete the ones satisfying the condition from results
		ReferSetBTree to_remove;
		BTreeRecord remove_record;
		//copy all to results;
		//and add all tuple entries to to_remove
		if (index){
			to_remove.allocateIndex(index->IndexNum, index->IndexFields, index->IndexFlags);
		}
		for (link=(ReferLink2*) DataHead.Next; link && link!=&DataHead; link=(ReferLink2*) link->Next){
			if (results!=this){
				results->borrowTuple(this, force_borrow);
				to_remove.insert((char*) results->DataCursor);
			}else{
				to_remove.insert((char*) link);
			}
		}
		remove_record.tree=&to_remove;

		//remove the true ones from results
		ReferLink2 query_link;
		query_link.Value.P=(char*) query_tuple;
		link1=index?(&query_link):0;

		rownum=0;
		for (link=(ReferLink2*) remove_record.moveFirst((char*) link1); link; link=(ReferLink2*) remove_record.moveNext((char*) link1) ){
			setExprVariables((ReferData*) link->Value.P, &expr, ++rownum); 
			expr.calculate();
			if (expr.getBoolean()){
				results->dropTuple(link);
				count++;
			}
		}
	}else if (to_delete==0 && results!=this){ //select the tuples satisfying the conditions from this to results
		//move tuples satisfying the condition to the result
		useIndex(index);
		rownum=0;
		for (tuple=moveFirst(query_tuple); tuple; tuple=moveNext(query_tuple) ){
			setExprVariables(tuple, &expr, ++rownum);
			expr.calculate();
			if (expr.getBoolean()){
				results->borrowTuple(this, force_borrow);
				count++;
			}
		}
	}else if ((to_delete==0 && results==this) //to_delete==0: select the tuples satisfying the conditions
		|| (to_delete==2 && results==this) ){ //to_delete==2: move the tuples satisfying the condition from this to results
		//borrow tuples satisfying the condition to temp_results first
		ReferSet temp_results;
		useIndex(index);
		rownum=0;
		for (tuple=moveFirst(query_tuple); tuple; tuple=moveNext(query_tuple) ){
			setExprVariables(tuple, &expr, ++rownum);
			expr.calculate();
			if (expr.getBoolean()){
				temp_results.borrowTuple(this);
				count++;
			}
		}
		//empty the dataset and move tuples back
		empty();
		for (tuple=temp_results.moveFirst(); tuple; tuple=temp_results.moveNext()){
			borrowTuple(&temp_results);
		}
	}else{ //(to_delete==2 && results!=this): move the tuples satisfying the condition from this to results
		//borrow tuples satisfying the condition to results, and keep a record of entries to remove
		ReferSetBTree to_remove;
		BTreeRecord remove_record;
		if (index){
			to_remove.allocateIndex(index->IndexNum, index->IndexFields, index->IndexFlags);
		}
		
		useIndex(index);
		rownum=0;
		for (tuple=moveFirst(query_tuple); tuple; tuple=moveNext(query_tuple) ){
			setExprVariables(tuple, &expr, ++rownum);
			expr.calculate();
			if (expr.getBoolean()){
				results->borrowTuple(this, force_borrow);
				to_remove.insert((char*) DataCursor);
				count++;
			}
		}
		remove_record.tree=&to_remove;

		//remove tuples from this
		rownum=0;
		for (link=(ReferLink2*) remove_record.moveFirst(); link; link=(ReferLink2*) remove_record.moveNext() ){
			dropTuple(link);
		}
	}


	if (results==this){
		DataCursor=0;
		RowNum=0;
	}
	return count;
}
long ReferSet::parseUpdateQuery(ReferSqlParser* parser, tExprCalc* context){
	//check if any field is in the index
	int is_indexed=0;
	for (ReferLink* link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
		ReferSetBTree* index=(ReferSetBTree*) link->Value.P;
		for (ReferLink* link1=(ReferLink*)parser->OutputFields.getReferLinkHead(); link1; link1=link1->Next){
			long field=getFieldIndex(&link1->Name);
			if (index->findIndexField(field)>=0) {
				is_indexed=1; 
				break;
			}
		}
	}


	long count=0;
	if (parser->Condition.L){
		//filter tuples to condition_dataset
		int force_borrow=1; //field not in index, copy by reference
		int to_delete=2; //move tuples to condition_dataset

		ReferSet condition_dataset; 
		condition_dataset.copyFields(this);
		count=filterByCondition(&parser->Condition, &condition_dataset, context, to_delete, force_borrow);

		condition_dataset.projectFields(&parser->OutputFields, this, context, 0);
	}else{
		projectFields(&parser->OutputFields, this, context, 0);

		rebuildAllIndex();

		count=TupleCount;
	}

	return count;
}
long ReferSet::parseInsertQuery(ReferSqlParser* parser, tExprCalc* context){
	projectFields(&parser->OutputFields, this, context, 0, 1);
	return 1; 
}
long ReferSet::executeSQL(ReferData* sql, ReferSet* results, tExprCalc* context){
	return executeSQL(sql->P, results, context);
}
long ReferSet::executeSQL(const char* sql, ReferSet* results, tExprCalc* context){
	ReferSqlParser parser;
	parser.Expression.useContext(context, true);
	parser.setQuery(sql);

	if (parser.SqlType==ReferSqlParser::SQL_DELETE){
		return filterByCondition(&parser.Condition, results, context, true);
	}else if (parser.SqlType==ReferSqlParser::SQL_INSERT){
		return parseInsertQuery(&parser, context); 
	}else if (parser.SqlType==ReferSqlParser::SQL_UPDATE){
		return parseUpdateQuery(&parser, context); 
	}

	ReferSet result_dataset; 
	int force_borrow=0; //borrow to result_dataset;
	if (!results || results==this){
		results=&result_dataset;
		force_borrow=1;
	}
	long i;
	if (parser.ToSelectAll || !parser.OutputExprs){
		if (results==&result_dataset) result_dataset.copyFields(this);
	}else{
		i=0;
		for (tExprCalc* calc=parser.OutputExprs; calc; calc=(tExprCalc*) calc->WalkNext){
			results->setFieldName(i, calc->Name);
			i++;
		}
	}

	ReferSet condition_dataset; //temp_result: in this or condition_dataset;
	ReferSet* temp_results=this;
	long count=0;
	if (parser.Condition.L){
		condition_dataset.copyFields(this);
		count=temp_results->filterByCondition(&parser.Condition, &condition_dataset, context, false, force_borrow);
		temp_results=&condition_dataset;
	}

	ReferSet groupby_dataset; //only work as an index
	if (parser.GroupByFields.Total){
		//set group by fields
		groupby_dataset.copyFields(this);
		groupby_dataset.newIndex("groupby", parser.GroupByExprs);
		groupby_dataset.useIndexName("groupby");
	}
	
	ReferSet proj_dataset;  //temp_result: in this, condition_dataset(maybe borrowed), or proj_dataset (always new tuple);
	if ((parser.OutputFields.Total && !parser.ToSelectAll) || parser.GroupByFields.Total>0){
		temp_results->projectFields(&parser.OutputFields, &proj_dataset, context, parser.GroupByFields.Total?&groupby_dataset:0);
		temp_results=&proj_dataset;
		count=proj_dataset.TupleCount; 
	}

	if (parser.HavingCondition.L){ 
		count=temp_results->filterByCondition(&parser.HavingCondition, &proj_dataset, context, false, force_borrow); //should this force_borrow alway be true?? check later
		temp_results=&proj_dataset;
	}


	ReferData* tuple;
	if (parser.OrderByFields.Total){
		if (temp_results != &proj_dataset){
			proj_dataset.copyFields(temp_results);
		}

		if (context){
			for (tExprCalc* field=parser.OrderByExprs; field; field=(tExprCalc*) field->WalkNext){
				field->Context=(tExprCalc*) context; 
			}
		}
		proj_dataset.newIndex("orderby", parser.OrderByExprs);

		if (temp_results!=&proj_dataset){
			for (tuple=temp_results->moveFirst(); tuple; tuple=temp_results->moveNext()){
				if (force_borrow==1){
					proj_dataset.borrowTuple(temp_results); //need to set free tuple flag
				}else{
					proj_dataset.newTuple(tuple, false); //need to set free tuple flag
				}
			}
		}

		temp_results=&proj_dataset;
	}

	if (!results->FieldsNum){
		results->copyStruct(temp_results);
	}else if (results!=temp_results){
		results->copyIndices(temp_results);
	}
	for (tuple=temp_results->moveFirst(); tuple; tuple=temp_results->moveNext()){
		if (temp_results==&proj_dataset || force_borrow==1){
			results->borrowTuple(temp_results); //need to set free tuple flag
		}else{
			results->newTuple(tuple, false);
		}
	}
	if (results==&result_dataset){
		reset();
		copyFields(results);
		for (tuple=results->moveFirst(); tuple; tuple=results->moveNext()){
			borrowTuple(results);
		}
	}
	if (parser.OrderByFields.Total){
		results->newIndex("orderby", parser.OrderByExprs);
	}
	
	return count;
}


int ReferSet::projectFields(ReferLinkHeap* fieldmap, ReferSet* projset, tExprCalc* context, ReferSet* groupby_index, int sel_ins_upd){
	if (!projset) projset=this;

	tExprCalc* exprlist=new tExprCalc[fieldmap->Total];
	long* fieldindex=new long[fieldmap->Total];

	//set default field names
	ReferLink* link, *link1;
	long count=0;
	char buf[128];
	for (link=(ReferLink*) fieldmap->moveFirst(); link; link=(ReferLink*) fieldmap->moveNext() ){
		if (link->Name.L==0){
			sprintf(buf, "Field%d", count+1);
			link->Name=buf;
		}
		count++;
	}


	//set projset fields
	count=0;
	if (projset->FieldsNum==0){
		projset->setFieldsNum(fieldmap->Total);
		for (link=(ReferLink*) fieldmap->moveFirst(); link; link=(ReferLink*) fieldmap->moveNext() ){
			projset->setFieldName(count, link->Name.P);
			fieldindex[count]=count;
			count++;
		}
	}else{
		long oldfields=projset->FieldsNum;

		for (link=(ReferLink*) fieldmap->moveFirst(); link; link=(ReferLink*) fieldmap->moveNext() ){
			fieldindex[count]=projset->getFieldIndex(&link->Name);
			if (fieldindex[count]<0){
				fieldindex[count]=projset->FieldsNum;
				projset->setFieldName(projset->FieldsNum, link->Name.P);
			}
			count++;
		}

		if (oldfields<projset->FieldsNum){
			projset->resizeAllTuples(oldfields, projset->FieldsNum);
		}
	}
	

	//parse expressions
	count=0;
	for (link=(ReferLink*) fieldmap->moveFirst(); link; link=(ReferLink*) fieldmap->moveNext() ){
		if (!link->Value.L) link->Value=link->Name;
		link->Value.Seperate();
		if (link->Value.P) {
			tStrOp::convertSpaceSpecial(link->Value.P);
			link->Value.L=strlen(link->Value.P);
		}

		if (context) exprlist[count].useContext(context, false); //cannot clear context variables because their are used by multiple expressions
		exprlist[count].setExpression(link->Value.P);
		exprlist[count].parse(synEXP_RIGHTBRACE);

		count++;
	}

	ReferData* tuple;
	ReferSet* data_cursor=this;
	ReferSetBTree* index=0;
	if (groupby_index){
		for (link1=(ReferLink2*) DataHead.Next; link1 && link1!=&DataHead; link1=(ReferLink2*) link1->Next){
			tuple=(ReferData*) link1->Value.P;
			groupby_index->newTuple(tuple, false);
		}
		index=groupby_index->getIndex();
		data_cursor=groupby_index;
	}

	//compute data
	ReferData* proj_tuple=0;
	ReferLink* last_grouptuple=0;
	int new_group;
	for (tuple=data_cursor->moveFirst(); tuple || sel_ins_upd==1; tuple=data_cursor->moveNext()){
		new_group=1;
		
		if (groupby_index && index){
			if (last_grouptuple && index->cmp((char*) last_grouptuple, (char*) data_cursor->DataCursor)==0){
				new_group=0;
			}else{
				last_grouptuple=data_cursor->DataCursor;
			}
		}

		if (new_group){
			if (projset!=this || sel_ins_upd==1){
				proj_tuple=projset->newTuple();
			}else{
				proj_tuple=tuple;
			}
		}

		count=0;
		long rownum=0;
		for (link=(ReferLink*) fieldmap->moveFirst(); link; link=(ReferLink*) fieldmap->moveNext() ){
			//new_group
			setExprVariables(tuple, &exprlist[count], ++rownum);
			exprlist[count].calculate(new_group);

			if (exprlist[count].Root){
				proj_tuple[fieldindex[count]].Set(exprlist[count].Root->StringValue.P, exprlist[count].Root->StringValue.L, true); 
			}else{
				proj_tuple[fieldindex[count]]="";
			}
			count++;
		}

		if (sel_ins_upd==1) break; //insert only one tuple
	}

	delete[] fieldindex;
	delete[] exprlist;

	return 0;
}
ReferSetBTree* ReferSet::chooseIndex(ReferLinkHeap* names){
	if (names->Total==0) return 0;

	int max_fields=0;
	ReferSetBTree* chosen_index=0;

	ReferSetBTree* index;
	ReferLink* link;
	long i=0;
	for (link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
		index=(ReferSetBTree*) link->Value.P;
		if (index && (index->isUniqueKey() || index->IndexNum>max_fields)){
			int is_index=true;
			for (i=0; i<index->IndexNum; i++){
				if (!names->findName(FieldNames[index->IndexFields[i]])) is_index=false;
			}
			if (is_index){
				max_fields=index->IndexNum;
				chosen_index=index;
				if (index->isUniqueKey()) break;
			}
		}
	}

	return chosen_index;
}

int ReferSet::dropTuple(ReferLink2* tuple, int delete_data){
	if (!tuple) {
		tuple=DataCursor;
		DataCursor=tuple->Prev;
	}

	if (tuple){
		#ifdef DEBUG_THIS_FILE
			TRACE("*ReferSet==>dropTuple: %s=>%lx\n", ReferSetName.P, tuple);
		#endif

		dropfromAllIndex(tuple);
		dropOneTupleNoIndex(tuple, delete_data);

		return 1;
	}
	return 0;
}
int ReferSet::dropOneTupleNoIndex(ReferLink2* tuple, int delete_data){
	if ((delete_data && tuple->Value.L) || (!tuple->Value.L && delete_data>=2)){  //tuple->Value->L: to free tuple or not
		delete[] (ReferData*) tuple->Value.P;
	}
	tuple->Value.P=0;
	tuple->Value.L=0;
	if (DataCursor==tuple) 
		DataCursor=(ReferLink2*) (tuple->Next);
	DataHead.remove(tuple);
	TupleCount--;
	return 0;
}

/*ReferLink* ReferSet::setVariable(const char* name, const char* value, int copy){
	ReferLink* link=Variables.findName(name);
	if (!link) link=Variables.add(name, 0, 0);
	link->Value.Set((char*) value, strlen(value), copy);
	return link;
}*/
int ReferSet::setExprVariables(ReferData* tuple, tExprCalc* expr, long rownum){
	for (int i=0; i<expr->FieldsNum; i++){
		tExprField* field=expr->FieldsList[i];
		int fieldindex=getFieldIndex(field->StringName.P);
		if (fieldindex>=0){
			field->newValue(tuple[fieldindex].P, tuple[fieldindex].L, false);
		}else if (!field->StringName.Cmp("rownum",6,false)){
			field->newDoubleValue(rownum, NumberDefPrecision, 0);
		}else {
			/*ReferLink* link=Variables.findName(&field->StringName);
			if (link){
				field->newValue(link->Value.P, link->Value.L, link->Value.Type);
			}else{
				//field->newValue(10); //use whatever value
			}*/
		}
	}
	return 0;
}

long ReferSet::getFieldIndex(const char* fieldname){
	ReferLink* link=FieldNameIndex.findName(fieldname);
	return link?link->Data:-1;
}
long ReferSet::getFieldIndex(ReferData* fieldname){
	ReferLink* link=FieldNameIndex.findName(fieldname);
	return link?link->Data:-1;
}

ReferData* ReferSet::newTuple(ReferData* tuple, int free_tuple){
#ifdef _WINDOWS
	ASSERT(FieldsNum>0 || tuple);
#endif
	if (FieldsNum<=0 && !tuple) return 0;

	if (!tuple) tuple=new ReferData[FieldsNum];
	if (!tuple) return 0;

	commitTuple();

	DataCursor=DataHead.insert();
	DataCursor->Value.Set((char*) tuple, free_tuple, false);
	DataCursor->Data=REC_NEWTUPLE;

	TupleCount++;

	return tuple;
}
int ReferSet::setFieldValue(const char* fieldname, const char* fieldvalue, int copy){ 
	ReferData name, value;
	name.Set((char*) fieldname, fieldname?strlen(fieldname):0, false);
	value.Set((char*) fieldvalue, fieldvalue?strlen(fieldvalue):0, false);
	return setFieldValue(&name, &value, copy);
}

int ReferSet::setFieldValue(ReferData* fieldname, ReferData* fieldvalue, int copy){
	if (!DataCursor || DataCursor==&DataHead) return false;
	ReferData* tuple=(ReferData*) DataCursor->Value.P;
	long i=getFieldIndex(fieldname);
	if (i>=0){
		tuple[i].Set(fieldvalue->P, fieldvalue->L, copy);
		return true;
	}
	return false;
}
int ReferSet::getFieldValue(long fieldindex0, ReferData* fieldvalue){
	fieldvalue->reset();
	ReferData* tuple=getTuple();
	if (!tuple) return -1; 

	fieldvalue->Set(tuple[fieldindex0].P, tuple[fieldindex0].L, false);
	return 0;
}
int ReferSet::getFieldValue(const char* fieldname, ReferData* fieldvalue){
	fieldvalue->reset();
	long fieldindex=getFieldIndex(fieldname);
	if (fieldindex<0) return -1;
	return getFieldValue(fieldindex, fieldvalue);
}
ReferData* ReferSet::getField(long fieldindex0){
	ReferData* tuple=getTuple();
	if (!tuple || fieldindex0>FieldsNum || fieldindex0<0) return 0; 
	return tuple+fieldindex0;
}
ReferData* ReferSet::getField(const char* fieldname){
	return getField(getFieldIndex(fieldname));
}
ReferData* ReferSet::getField(ReferData* fieldname){
	return getField(getFieldIndex(fieldname));
}

ReferData* ReferSet::getField(ReferData*tuple, const char* fieldname){
	long fieldindex=getFieldIndex(fieldname);
	if (fieldindex<0) return 0;
	return tuple+fieldindex;
}
ReferData* ReferSet::getField(ReferData*tuple, ReferData* fieldname){
	return getField(tuple, fieldname->P);
}
int ReferSet::checkUniqueIndex(ReferLink* tuple){
	if (UniqueIndexList.Total==0) return 0;
	ReferLink* link;
	ReferSetBTree* index;
	int count=0;
	for (link=(ReferLink*) UniqueIndexList.moveFirst(); link; link=(ReferLink*) UniqueIndexList.moveNext()){
		index=(ReferSetBTree*) link->Value.P;
		if (index && index->find((char*) tuple, false)){
			count++;
		}
	}
	return count;
}
int ReferSet::addtoAllIndex(ReferLink* tuple){
	ReferLink* link;
	ReferSetBTree* index;
	long count=0;
	for (link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
		index=(ReferSetBTree*) link->Value.P;
		if (index){
			#ifdef DEBUG_THIS_FILE
				FILE* f;
				char buf[256];
				sprintf(buf, "d:\\printTree+%d.txt", count);
				f=fopen(buf, "w+");
				long newtotal=index->printTree(f, index->root, "",1,1);
				fclose(f);
				long total=index->total;
				if (newtotal!=index->total){
#ifdef _WINDOWS
					ASSERT(0);
#endif
				}
			#endif

			int inserted=index->insert((char*) tuple);

			#ifdef DEBUG_THIS_FILE
				sprintf(buf, "d:\\printTree+%d_.txt", count);
				f=fopen(buf, "w+");
				newtotal=index->printTree(f, index->root, "",1,1);
				fclose(f);
				if (total+1!=index->total || newtotal!=index->total){
					ASSERT(0);
				}
			#endif

#ifdef _WINDOWS
			ASSERT(inserted==1 && index->total==(unsigned long) TupleCount);
#endif

			count++;
		}
	}
	return count;
}
int ReferSet::dropfromAllIndex(ReferLink* tuple){
	ReferLink* link;
	ReferSetBTree* index;
	int count=0;
	for (link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
		index=(ReferSetBTree*) link->Value.P;
		if (index){
			#ifdef DEBUG_THIS_FILE
				FILE* f;
				char buf[256];
				sprintf(buf, "d:\\printTree-%d.txt", count);
				f=fopen(buf, "w+");
				long newtotal=index->printTree(f, index->root, "",1,1);
				fclose(f);
				long total=index->total;
				if (newtotal!=index->total){
					ASSERT(0);
				}
			#endif

			int dropped=index->remove((char*) tuple, 1);

			#ifdef DEBUG_THIS_FILE
				sprintf(buf, "d:\\printTree-%d_.txt", count);
				f=fopen(buf, "w+");
				newtotal=index->printTree(f, index->root, "",1,1);
				fclose(f);
				if (dropped!=1 || newtotal!=total-1 || newtotal!=index->total){
					ASSERT(0);
				}else{
					TRACE("%lx deleted from index: %s\n", tuple, index->IndexName.P);
				}
			#endif

#ifdef _WINDOWS
			ASSERT(dropped==1 && index->total==(unsigned long) (TupleCount-1));
#endif

			count++;
		}
	}
	return count;
}
void ReferSet::resetIndexCursor(){
	commitTuple();
	DataCursor=0;
	UseIndex.reset();
}
ReferData* ReferSet::findFieldString(const char* fieldname, ReferData* fieldvalue){
	ReferLink* link=0;
	ReferData index_name; index_name=fieldname;
	ReferSetBTree* index=0;

	resetIndexCursor();

	//check if it is a field name
	long field_index=getFieldIndex(fieldname);
	if (field_index>=0) { //it is a field name  
		//check if index exists
		if ((link=IndexNameList.findName(&index_name)) ){
				index=(ReferSetBTree*) link->Value.P;
				if (index && index->IndexNum==1 && index->IndexFields[0]==field_index ){
					UseIndex.tree=index;
				}
		}else{
			int selected=0; 
			for (link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
				index=(ReferSetBTree*) link->Value.P;
				//index include only this field
				if (index && index->IndexNum==1 && index->IndexFields[0]==field_index && index->isStringFlag(index->IndexFlags[0]) ){
					UseIndex.tree=index;
					selected=1; 
					break;
				}
			}
			if (!selected){
				for (link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
					index=(ReferSetBTree*) link->Value.P;
					//index include this field as the first one
					if (index && index->IndexNum>=1 && index->IndexFields[0]==field_index && index->isStringFlag(index->IndexFlags[0]) ){
						UseIndex.tree=index;
						selected=1; 
						break;
					}
				}
			}
		}
	}else{ //not field name, may be index name
		if ((link=IndexNameList.findName(&index_name)) ){
			index=(ReferSetBTree*) link->Value.P;
			if (index && index->IndexNum==1){
				field_index=index->IndexFields[0];
				fieldname=FieldNames[field_index]->P;
				UseIndex.tree=index;
			}
		}
	}
	if (field_index<0) return 0;

	//if no existing index, create a new one
	if (!UseIndex.tree){
		while (IndexNameList.findName(&index_name)){
			index_name+="$"; //from previous search, we know the there is no index for the field
		}
		long fieldslist[]={field_index};
		long fieldsflag[]={0};
		UseIndex.tree=newIndex(index_name.P, 1, fieldslist, fieldsflag);
	}

	//tuple for search
	if (!QueryTuple) QueryTuple=new ReferData[FieldsNum];
	QueryTuple[field_index].Set(fieldvalue->P, fieldvalue->L, false);

	return moveFirst(QueryTuple);
}

ReferSetBTree* ReferSet::newIndex(const char* indexname, int indexfieldsnum, const long* fieldslist, const long* fieldsflag, int is_unique, tExprCalc* sortexprs){
	commitTuple();

	ReferData name;
	name.Set((char*) indexname, indexname?strlen(indexname):0, false);
	ReferLink* link=IndexNameList.findName(&name);
	if (link) { //existing index
		dropIndex(indexname); //drop it anyway, so that default field index can be replaced by user index
	}

	ReferSetBTree *index=new ReferSetBTree;
	index->IndexName=indexname;
	index->TypeOfP=ReferSetBTree::TYPE_REFLINK;
	index->NoDuplicate=is_unique;

	index->allocateIndex(indexfieldsnum, fieldslist, fieldsflag);
	if (sortexprs) 
		index->setSortExpressions(sortexprs, this); 
	index->sortReferSet(this, is_unique);

	link=IndexNameList.add(&name, 0, 0);
	link->Value.Set((char*) index, 0, false);

	if (is_unique){
		link=UniqueIndexList.add(&name, 0, 0);
		link->Value.Set((char*) index, 0, false);
	}

	return useIndex(index);
}

ReferSetBTree* ReferSet::newIndex(const char* indexname, const char** fieldsname, const long* fieldsflag, int is_unique){
	int indexfieldsnum;
	for (indexfieldsnum=0; fieldsname[indexfieldsnum]; indexfieldsnum++);
	long* fieldslist=new long[indexfieldsnum];

	int i;
	for (i=0; i<indexfieldsnum; i++){
		fieldslist[i]=getFieldIndex(fieldsname[i]);
#ifdef _WINDOWS
		ASSERT(fieldslist[i]>=0);
#endif
		if (fieldslist[i]<0) { //error
			delete[] fieldslist;
			return 0;
		}
	}
	ReferSetBTree* tree=newIndex(indexname, indexfieldsnum, fieldslist, fieldsflag, is_unique);

	delete [] fieldslist;
	return tree;
}
ReferSetBTree* ReferSet::newIndexField(const long fieldindex0, const long fieldflag, int is_unique){
	if (fieldindex0>=FieldsNum) return 0;
	return newIndex(FieldNames[fieldindex0]?FieldNames[fieldindex0]->P:"Index"
		, 1, &fieldindex0, &fieldflag, is_unique);
}
ReferSetBTree* ReferSet::newIndex(const char*index_name, tExprCalc* fields_walklist, int is_unique){
	tExprCalc* field;
	int orderby_num=0; 
	for (field=fields_walklist; field; field=(tExprCalc*) field->WalkNext) 
		orderby_num++;
#ifdef _WINDOWS
	ASSERT(orderby_num>0);
#endif

	long* index_fields=new long[orderby_num];
	long* index_flags=new long[orderby_num];
	long field_index=0;
	for (field=fields_walklist; field; field=(tExprCalc*)field->WalkNext){
		long ifield=0; 
		if (field->Root->Brand==expNODE){
			ifield=getFieldIndex(&field->StringName);
#ifdef _WINDOWS
			ASSERT(ifield>=0);
#endif
		}else{
			ifield=-field_index-1;
		}

		index_fields[field_index]=ifield;
		index_flags[field_index]=0; //set asc or desc later

		if (field->StringValue.L){ //sorting flags
			IWSqlSyntax sort_syntax;
			sort_syntax.setSentence(field->StringValue.P, &field->StringValue.L, false);
			while (sort_syntax.Type != QLSyntax::synQL_UNKNOW && sort_syntax.Type!=QLSyntax::synQL_END){
				switch (sort_syntax.Type) {
				case IWSqlSyntax::synSQL_DESC: 
					index_flags[field_index] |= ReferSetBTree::FLAG_DESCENDING;
					break;
				case IWSqlSyntax::synSQL_NUMBER: 
					index_flags[field_index] |= ReferSetBTree::FLAG_FLOAT;
					break;
				case IWSqlSyntax::synSQL_INTEGER: 
					index_flags[field_index] |= ReferSetBTree::FLAG_INTEGER;
					break;
				case IWSqlSyntax::synSQL_DATE: 
					//same as CHAR, so no need to do anything now
					//index_flags[field_index] |= ReferSetBTree::FLAG_DATE;
					break;
				default: 
					break;
				}
				sort_syntax.match();
			}
		}
		field_index++;
	}

	ReferSetBTree* index=newIndex(index_name, orderby_num, index_fields, index_flags, is_unique, fields_walklist);

	delete[] index_fields;
	delete[] index_flags;
	return index;
}

ReferSetBTree* ReferSet::getIndex(const char* indexname){
	if (indexname){
		ReferData name;
		name.Set((char*) indexname, indexname?strlen(indexname):0, false);
		ReferLink* link=IndexNameList.findName(&name);
		if (!link) return 0;

		ReferSetBTree* index=(ReferSetBTree*) link->Value.P;

		commitTuple();

		return index;
	}else{
		return (ReferSetBTree*) UseIndex.tree;
	}
}
ReferSetBTree* ReferSet::useIndexName(const char* indexname){
	commitTuple();
	UseIndex.reset();

	ReferSetBTree* index=0;
	if (indexname) {
		index=getIndex(indexname);
		UseIndex.tree=index;
	}
	return index;
}
ReferSetBTree* ReferSet::useIndex(ReferSetBTree* index){
	commitTuple();
	if (UseIndex.tree!=index){
		UseIndex.reset();
		UseIndex.tree=index;
	}
	return index;
}
int ReferSet::dropIndex(const char* indexname){
	ReferLink* link=IndexNameList.findName(indexname);
	if (link){
		ReferSetBTree* tree=(ReferSetBTree*) link->Value.P;
		if (tree) delete tree;
		link->Value.reset();
		IndexNameList.remove(link);
		return 1;
	}
	return 0;
}

int ReferSet::commitTuple(){
	if (DataCursor && DataCursor->Data==REC_NEWTUPLE){
		#ifdef DEBUG_THIS_FILE
			TRACE("*ReferSet==>newTuple: %x: %s=>%lx\n", this, ReferSetName.P, DataCursor);
		#endif
		if (checkUniqueIndex(DataCursor)==0){
			addtoAllIndex(DataCursor);
			DataCursor->Data=REC_TUPLE;
			return 1;
		}else{
			dropOneTupleNoIndex(DataCursor, true);
			DataCursor=0;
		}
	}
	return 0;
}
int ReferSet::resizeAllTuples(long original_fieldsnum, long new_fieldsnum){
	ReferLink2* link=0;
	ReferData* tuple, *newtuple;
	long i;
	for (link=(ReferLink2*) DataHead.Next; link && link!=&DataHead; link=(ReferLink2*) link->Next){
		tuple=(ReferData*) link->Value.P;

		//if (link->Data){  //REC_NEWTUPLE
			newtuple=new ReferData[new_fieldsnum];
			for (i=0; i<original_fieldsnum && i<new_fieldsnum; i++){
				newtuple[i].Set(tuple[i].P, tuple[i].L, false);
				if (tuple[i].Type){
					newtuple[i].setToFree(true);
					tuple[i].setToFree(false);
				}
			}
			delete[] tuple;
			link->Value.P=(char*) newtuple;
		//}
	}
	return 0;
}
int ReferSet::copyStruct(ReferSet* from){
	reset();

	copyFields(from); 
	copyIndices(from);

	return 0;
}
int ReferSet::copyFields(ReferSet* from){ //fields
	setFieldsNum(from->FieldsNum);
	long i;
	for (i=0; i<from->FieldsNum; i++){
		setFieldName(i, from->FieldNames[i]?from->FieldNames[i]->P:0);
	}
	return 0;
}
int ReferSet::copyIndices(ReferSet* from){ //index
	ReferLink* link;
	ReferSetBTree* index, *newindex, *useindex;
	useindex=(ReferSetBTree*) from->UseIndex.tree; //what is index from is using
	for (link=(ReferLink*) from->IndexNameList.moveFirst(); link; link=(ReferLink*) from->IndexNameList.moveNext()){
		index=(ReferSetBTree*) link->Value.P;
		if (index){
			newindex=newIndex(link->Name.P, index->IndexNum, index->IndexFields, index->IndexFlags);
			//also copy SortExpressions;
			if (index->SortExpressions) newindex->setSortExpressions(index->SortExpressions, this); 

			if (useindex==index){
				UseIndex.tree=newindex;
			}
		}
	}
	return 0;
}
ReferData* ReferSet::moveFirst(ReferData* tuple){
	commitTuple();

	if (UseIndex.tree){
		ReferLink2 link;
		link.Value.P=(char*) tuple;
		DataCursor=(ReferLink2*) UseIndex.moveFirst(tuple?(char*) &link:0);
		if (DataCursor) {
			RowNum=1; return (ReferData*) DataCursor->Value.P;
		}else{
			RowNum=0; return 0;
		}
	}else{
		DataCursor=(ReferLink2*) DataHead.Next;
		if (tuple){
			while (DataCursor && DataCursor!=&DataHead && DataCursor->Value.P!=(char*) tuple){
				DataCursor=(ReferLink2*) DataCursor->Next;
			}
		}
		if (isEOF()){
			RowNum=0; return 0;
		}else{
			RowNum=1; return (ReferData*) DataCursor->Value.P;
		}
	}
}
ReferData* ReferSet::moveNext(ReferData* tuple){
	commitTuple();

	if (UseIndex.tree){
		DataCursor=(ReferLink2*) UseIndex.moveNext();
		ReferLink2 link;
		link.Value.P=(char*) tuple;
		if (tuple && UseIndex.tree->cmp((char*) &link, (char*) DataCursor)) DataCursor=0;
		if (DataCursor) {
			RowNum++; return (ReferData*) DataCursor->Value.P;
		}else{
			RowNum=0; return 0;
		}
	}else{
		if (DataCursor){
			DataCursor=(ReferLink2*) DataCursor->Next;
		}
		if (tuple && DataCursor->Value.P != (char*) tuple) DataCursor=0;
		if (isEOF()){
			RowNum=0; return 0;
		}else{
			RowNum++; return (ReferData*) DataCursor->Value.P;
		}
	}
}
ReferData* ReferSet::moveLast(ReferData* tuple){
	commitTuple();

	if (UseIndex.tree){
		ReferLink2 link;
		link.Value.P=(char*) tuple;
		DataCursor= (ReferLink2*) UseIndex.moveLast(tuple?(char*) &link:0);
		if (DataCursor) {
			RowNum=-1; return (ReferData*) DataCursor->Value.P;
		}else{
			RowNum=0; return 0;
		}
	}else{
		DataCursor=DataHead.Prev;
		if (tuple){
			while (DataCursor && DataCursor!=&DataHead && DataCursor->Value.P!=(char*) tuple){
				DataCursor=(ReferLink2*) DataCursor->Prev;
			}
		}
		if (isEOF()){
			RowNum=0; return 0;
		}else{
			RowNum=-1; return (ReferData*) DataCursor->Value.P;
		}
	}
}
ReferData* ReferSet::movePrevious(){
	if (UseIndex.tree){
		DataCursor= (ReferLink2*) UseIndex.moveNext();
		if (DataCursor) {
			RowNum--; return (ReferData*) DataCursor->Value.P;
		}else{
			RowNum=0; return 0;
		}
	}else{
		commitTuple();
		if (DataCursor){
			DataCursor=(ReferLink2*) DataCursor->Prev;
		}
		if (isEOF()){
			RowNum=0; return 0;
		}else{
			RowNum--; return (ReferData*) DataCursor->Value.P;
		}
	}
}
int ReferSet::isEOF(){
	if (UseIndex.tree){
		return UseIndex.isEOF();
	}else{
		return (!DataCursor || DataCursor==&DataHead);
	}
}
int ReferSet::isBOF(){
	if (UseIndex.tree){
		return UseIndex.isBOF();
	}else{
		return (!DataCursor || DataCursor==&DataHead);
	}
}
ReferData* ReferSet::getTuple(){
	if (DataCursor && DataCursor!=&DataHead){
		return (ReferData*) DataCursor->Value.P;
	}
	return 0;
}

int ReferSet::rebuildAllIndex(){
	ReferLink* link;
	for (link=(ReferLink*) IndexNameList.moveFirst(); link; link=(ReferLink*) IndexNameList.moveNext()){
		ReferSetBTree* index=(ReferSetBTree*) link->Value.P;
		index->resetBTree();
	}
	for (link=DataHead.Next; link && link!=(ReferLink*) &DataHead; link=link->Next){
		addtoAllIndex(link);
	}
	return 0;
}
int ReferSet::getCSVData(ReferData* results, const char* sql, int save_csv_header, const char* field_sep, const char* record_sep, const char*quotation){
	const char* default_sep="\t";
	const char* default_record_sep="\r\n"; 
	if (!field_sep) field_sep=default_sep;
	if (!record_sep) record_sep=default_record_sep;
	//use "" if not to use default_sep

	ReferSet temp_results;
	ReferSet* r=this;
	if (sql && *sql){
		executeSQL(sql, &temp_results, 0);
		r=&temp_results;
	}

	results->Malloc(sizeof(char)*FieldsNum*r->TupleCount*50); //estimate the space
	*results="";

	long i;
	if (save_csv_header){
		for (i=0; i<r->FieldsNum; i++){
			if (i) *results+=field_sep;
			if (r->FieldNames[i] && r->FieldNames[i]->P) {
				if (quotation) *results+=quotation; 
				*results+=r->FieldNames[i]->P;
				if (quotation) *results+=quotation; 
			}
		}
		*results+=record_sep;
	}
	ReferData* tuple;
	char buf[64];
	for (r->moveFirst(); !r->isEOF(); r->moveNext()){
		tuple=r->getTuple();
		for (i=0; i<r->FieldsNum; i++){
			if (i>0) *results+=field_sep;
			if (quotation) *results+=quotation; 
			if (!tuple[i].Type && tuple[i].L){
				sprintf(buf, "%ld", tuple[i].L);
				*results+=buf; 
			}else{
				*results+=tuple[i]; 
			}
			if (quotation) *results+=quotation; 
		}
		*results+=record_sep;
	}
	return 0;
}
int ReferSet::getXMLData(ReferData* results, int to_encode_html, const char* sql, const char* record_tag, const char* field_tag){
	if (!record_tag || !record_tag[0]) record_tag="VariableRecord";
	if (!field_tag) field_tag="VariableData";

	int fieldname_as_tag=false; 
	if (!field_tag[0]) fieldname_as_tag=true;

	ReferSet temp_results;
	ReferSet* r=this;
	if (sql && *sql){
		executeSQL(sql, &temp_results, 0);
		r=&temp_results;
	}

	results->Malloc(sizeof(char)*FieldsNum*r->TupleCount*50); //estimate the space

	//set constances
	char fmt[64];
	tStrOp::DateToChar(time(0), "YYYY/MM/DD HH24:MI:SS", fmt);
	ReferData recordhead; recordhead="<"; recordhead+=record_tag; recordhead+=" Name=\""; recordhead+=ReferSetName; recordhead+="\" Date=\""; recordhead+=fmt; recordhead+="\">\r\n";
	ReferData recordtail; recordtail="</"; recordtail+=record_tag; recordtail+=">\r\n";
	ReferData fieldhead; fieldhead="\t<"; fieldhead+=field_tag; fieldhead+=" Name=\""; //continue from field name
	ReferData fieldtail; fieldtail="</"; fieldtail+=field_tag; fieldtail+=">\r\n";

	ReferData original_xml; original_xml="_OriginalXML_";
	long original_xml_field=r->getFieldIndex(&original_xml);

	//construct records
	long i;
	ReferData*tuple;
	char buf[64];
	ReferData encoded;
	for (r->moveFirst(); !r->isEOF(); r->moveNext()){
		tuple=r->getTuple();
		if (original_xml_field>=0 && tuple[original_xml_field].L){ //use original XML
			*results+=tuple[original_xml_field]; 
			*results+="\r\n";
		}else{ //construct xml record
			*results+=recordhead;
			for (i=0; i<r->FieldsNum; i++){
				if (i==original_xml_field) continue; 

				//construct start-tag
				if (fieldname_as_tag){
					*results+="<"; 
					*results+=(FieldNames&&FieldNames[i])?FieldNames[i]->P:""; 
					*results+=">";
				}else{
					*results+=fieldhead; 
					*results+=(FieldNames&&FieldNames[i])?FieldNames[i]->P:""; 
					*results+="\">";
				}

				//add data
				if (!tuple[i].Type && tuple[i].L){
					sprintf(buf, "%ld", tuple[i].L);
					*results+=buf; 
				}else{
					if (to_encode_html) {
						tStrOp::encodeHtml(tuple[i].P, &encoded);
						*results+=encoded;
					}else{
						*results+=tuple[i]; 
					}
				}

				//construct start-tag
				if (fieldname_as_tag){
					*results+="<"; 
					*results+=(FieldNames&&FieldNames[i])?FieldNames[i]->P:""; 
					*results+=">";
				}else{
					*results+=fieldtail;
				}
			}
			*results+=recordtail;
		}
	}
	return 0;
}
long ReferSet::loadXMLData(ReferData* data, int to_decode_html, const char* with_name, const char* record_tag, const char* field_tag){
	//get the first two tags
	ReferData tag1, tag2;
	unsigned int len;
	char* p;
	ReferLinkHeap tagnames; //use when field_tag is an empty string
	tagnames.setSortOrder(SORT_ORDER_NUM_INC); 
	int fieldname_as_tag=false;
	if (field_tag && !field_tag[0]){ //field_tag is an empty string
		fieldname_as_tag=true;
	}
	if (!record_tag || !record_tag[0] || !field_tag || !field_tag[0]){ //get tag names from data
		HTQLTagDataSyntax DataSyntax;
		DataSyntax.setSentence(data->P, &data->L, false);
		long fields_count=0; 
		int in_field=false; 
		while (DataSyntax.Type != QLSyntax::synQL_END){
			if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
				p=TagOperation::getLabel(DataSyntax.Sentence+DataSyntax.Start, &len);
				if (!tag1.L) {
					tag1.Set(p, len, true); //record tag
				}else{
					if (fieldname_as_tag){
						if (!in_field){
							tag2.Set(p, len, true); //field tag
							tagnames.add(&tag2, 0, ++fields_count);
							in_field=true; 
						}
					}else{
						tag2.Set(p, len, true); //field tag
						break; //only get two tags
					}
				}
			}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
				if (fieldname_as_tag){
					p=TagOperation::getLabel(DataSyntax.Sentence+DataSyntax.Start, &len);
					if (in_field){
						if (!tag2.Cmp(p, len, false)) in_field=false;
					}else{
						if (!tag1.Cmp(p, len, false)) break;
					}
				}
			}
			DataSyntax.match();
		}
		if (!record_tag || !record_tag[0]) record_tag=tag1.P;
		if (!field_tag) field_tag=tag2.P;
	}
	//no tags in the data, use default tag names
	if (!record_tag || !record_tag[0]) record_tag="VariableRecord";
	if (!field_tag) field_tag="VariableData";


	HtmlQL ql1, ql2;
	ql1.setSourceData(data->P, data->L, false);

	ReferData record_query; record_query="<"; record_query+=record_tag; 
	if (with_name){
		record_query+=" (Name='"; record_query+=with_name; record_query+="')";
	}
	record_query+=">:tx, Name, ht";
	ReferData field_query; field_query="<"; field_query+=field_tag; field_query+=">:Name, tx";
	if (fieldname_as_tag){
		field_query="<* >:tn, tx";
	}

	ql1.setQuery(record_query.P);
	char *name, *value;
	long field_index;
	long count=0;
	ReferData original_xml; original_xml="_OriginalXML_";
	ReferData decoded;
	for (ql1.moveFirst(); !ql1.isEOF(); ql1.moveNext()){
		if (!ReferSetName.L){
			name=ql1.getValue(2);
			if (name && name[0]) ReferSetName=name;
		}
		
		p=ql1.getValue(1);
		if (!p || !p[0]) continue; 
		
		ql2.setSourceData(p, strlen(p), false);
		ql2.setQuery(field_query.P);
		if (FieldsNum==0){
			setFieldsNum(ql2.getTuplesCount()+1);
			field_index=0;
			for (ql2.moveFirst(); !ql2.isEOF(); ql2.moveNext()){
				name=ql2.getValue(1); 
				value=ql2.getValue(2);
				setFieldName(field_index++, name);
			}
			setFieldName(field_index++, original_xml.P);
		}

		ReferData* tuple=newTuple();
		field_index=0;
		for (ql2.moveFirst(); !ql2.isEOF(); ql2.moveNext()){
			name=ql2.getValue(1); 
			value=ql2.getValue(2);
			
			if (to_decode_html){
				decoded=value;
				tStrOp::decodeHtml(decoded.P); 
			}else{
				decoded.Set(value, value?strlen(value):0, false); 
			}

			if (name && name[0]) setFieldValue(name, decoded.P);
			else if (field_index<FieldsNum) tuple[field_index]=decoded.P;
			//else there is error, maybe additional < in the text.  fix this later!! ...

			field_index++;
		}
		setFieldValue(original_xml.P, ql1.getValue(3) );

		if (commitTuple()>0) count++;
	}
	return count;
}
long ReferSet::loadCSVData(ReferData* data, int first_line_isname, const char*line_sep, const char* field_sep, int quoted_field){
	ReferData htql;
	int fieldsnum=tStrOp::getCSVHtqlExpr(data->P, &htql, first_line_isname, field_sep, line_sep, quoted_field);
	HtmlQL ql;
	ql.setSourceData(data->P, data->L, false);
	ql.setQuery(htql.P);
	return loadHTQLData(&ql);
}
long ReferSet::loadHTQLData(HTQL* ql){
	int fields=ql->getFieldsCount();
	if (fields<=0) return 0;
	int i;

	ReferData* names=new ReferData[fields]; //get field names from HTQL
	for (i=0; i<fields; i++){
		names[i]=ql->getFieldName(i+1);
		if (!names[i].L) {
			names[i].Malloc(128); 
			sprintf(names[i].P, "Column%d", i+1);
			names[i].L=strlen(names[i].P); 
		}
	}
	if (FieldsNum==0){ //set field names if this is an empty set
		setFieldsNum(fields);
		for (i=0; i<fields; i++){
			setFieldName(i, names[i].P);
		}
	}
	for (i=0; i<fields; i++){ //check if field names are exist in this 
		if (names[i].L>0 && getFieldIndex(&names[i])<0){
			names[i].reset();
		}
	}

	long count=0;
	char *p;
	ReferData* tuple;
	for (ql->moveFirst(); !ql->isEOF(); ql->moveNext()){
		//check if there is data
		int has_data=false; 
		for (i=0; i<fields; i++){
			p=ql->getValue(i+1);
			if (p && *p){
				has_data=true;
				break;
			}
		}
		if (!has_data) continue;

		tuple=newTuple();
		for (i=0; i<fields; i++){
			p=ql->getValue(i+1);

			if (names[i].L){
				this->setFieldValue(names[i].P, p);
			}else if (i<FieldsNum) {
				tuple[i]=p;
			}
		}
		if (commitTuple()>0) count++;
#ifdef _DEBUG
		if (count%1000==0){
			TRACE("ReferSet::loadHTQLData(): %ld\n", count);
		}
#endif
	}
	delete [] names;
	return count;
}

#if (!defined(NOIWEB) && !defined(NOIWIRB))
long ReferSet::loadDBData(IWDriverInterface* iw){
	int fields=iw->getFieldCount();
	if (fields<=0) return 0;
	int i;

	ReferData* names=new ReferData[fields]; //get field names from DB
	for (i=0; i<fields; i++){
		iw->getFieldName(i, &names[i]);
	}
	if (FieldsNum==0){ //set field names if this is an empty set
		setFieldsNum(fields);
		for (i=0; i<fields; i++){
			setFieldName(i, names[i].P);
		}
	}
	for (i=0; i<fields; i++){ //check if field names are exist in this 
		if (names[i].L>0 && getFieldIndex(&names[i])<0){
			names[i].reset();
		}
	}

	long count=0;
	ReferData value;
	ReferData* tuple;
	for (iw->moveFirst(); !iw->isEOF(); iw->moveNext()){
		tuple=newTuple();
		for (i=0; i<fields; i++){
			iw->getFieldValue(i, &value);
			if (names[i].L){
				this->setFieldValue(&names[i], &value);
			}else if (i<FieldsNum) {
				tuple[i]=value;
			}
		}
		if (commitTuple()>0) count++;
#ifdef _DEBUG
		if (count%1000==0){
			TRACE("ReferSet::loadDBData(): %ld\n", count);
		}
#endif
	}
	delete [] names;
	return count;
}
#endif

ReferSetBTree::ReferSetBTree(){
	IndexFields=0;
	IndexNum=0;
	IndexFlags=0;
	TypeOfP=TYPE_REFLINK;
	NoDuplicate=false;

	SortExpressionsIndex=0;
	SortExpressions=0;
	SortExpressionsNum=0;
	SortSet=0;
}
ReferSetBTree::~ReferSetBTree(){
	reset();
}
void ReferSetBTree::reset(){
	BTree::reset();
	if (IndexFields){
		delete[] IndexFields; 
		IndexFields=0;
	}
	if (IndexFlags){
		delete[] IndexFlags;
		IndexFlags=0;
	}
	TypeOfP=TYPE_REFLINK;
	NoDuplicate=false;

	resetSortExpressions();
}
void ReferSetBTree::resetBTree(){
	BTree::reset();	
	NoDuplicate=false;
}
int ReferSetBTree::resetSortExpressions(){
	if (SortExpressionsIndex) {
		delete[] SortExpressionsIndex;
		SortExpressionsIndex=0;
	}
	if (SortExpressions){
		tExprCalc* expr=SortExpressions; 
		tExprCalc* expr1=(tExprCalc*) expr->WalkNext; 
		while (expr){
			delete expr; 
			expr=expr1; 
			expr1=expr? (tExprCalc*) expr->WalkNext:0;
		}
		SortExpressions=0;
	}
	SortExpressionsNum=0;
	SortSet=0;
	return 0;
}
int ReferSetBTree::isStringFlag(long flag){
	return !(flag&(FLAG_INTEGER|FLAG_FLOAT|FLAG_REFER_L|FLAG_REFER_P));
}
int ReferSetBTree::findIndexField(long field0){
	for (int i=0; i<IndexNum; i++){
		if (IndexFields[i]==field0) return i;
	}
	return -1;
}

int ReferSetBTree::sortReferSet(ReferSet* set, int indexfieldsnum, const long* fieldslist, const long* fieldsflag, int remove_duplicate, tExprCalc* sortexprs){
	set->commitTuple();

	TypeOfP=ReferSetBTree::TYPE_REFDATA;
	NoDuplicate=remove_duplicate;

	allocateIndex(indexfieldsnum, fieldslist, fieldsflag);
	if (sortexprs) 
		setSortExpressions(sortexprs, set); 
	sortReferSet(set, remove_duplicate);

	return 0;
}
int ReferSetBTree::sortReferSet(ReferSet* set, const char** fieldsname, const long* fieldsflag, int remove_duplicate){
	int indexfieldsnum;
	for (indexfieldsnum=0; fieldsname[indexfieldsnum]; indexfieldsnum++);
	long* fieldslist=new long[indexfieldsnum];

	int i;
	for (i=0; i<indexfieldsnum; i++){
		fieldslist[i]=set->getFieldIndex(fieldsname[i]);
#ifdef _WINDOWS
		ASSERT(fieldslist[i]>=0);
#endif
		if (fieldslist[i]<0) { //error
			delete[] fieldslist;
			return -1;
		}
	}
	int ret=sortReferSet(set, indexfieldsnum, fieldslist, fieldsflag, remove_duplicate);

	delete [] fieldslist;
	return ret;
}
int ReferSetBTree::sortReferSetField(ReferSet* set, const long fieldindex0, const long fieldflag, int remove_duplicate){
	if (fieldindex0>=set->FieldsNum) return 0;
	return sortReferSet(set, 1, &fieldindex0, &fieldflag, remove_duplicate);
}

int ReferSetBTree::allocateIndex(int indexnum, const long*fields_list, const long*flags_list){
	if (IndexFields){
		delete[] IndexFields; 
		IndexFields=0;
	}
	if (IndexFlags){
		delete[] IndexFlags;
		IndexFlags=0;
	}
	IndexNum=indexnum;
	IndexFields=new long[indexnum];
	IndexFlags=new long[indexnum];
	if (fields_list) memcpy(IndexFields, fields_list, sizeof(long)*indexnum);
	if (flags_list) memcpy(IndexFlags, flags_list, sizeof(long)*indexnum);
	return 0;
}
int ReferSetBTree::sortReferSet(ReferSet* set, int remove_duplicate){
	int is_inserted=0;
	for (ReferLink2* link=(ReferLink2*) set->DataHead.Next; link && link!=(ReferLink2*) &set->DataHead; ){
		is_inserted=insert((char*) link);
		//if (TypeOfP==TYPE_REFLINK){
		//	is_inserted=insert((char*) link);
		//}else{
		//	is_inserted=insert(link->Value.P);
		//}

		//for unique set
		if (!is_inserted && remove_duplicate){ //duplicated, remove from set
			ReferLink2* link1=(ReferLink2*) link->Next;
			set->dropTuple(link, true);
			link=link1;
		}else{
			link=(ReferLink2*) link->Next;
		}
	}
	return 0;
}
int ReferSetBTree::setSortExpressions(tExprCalc* sortexprs, ReferSet* sortset){ 
	resetSortExpressions();

	SortExpressionsNum=0;
	tExprCalc* expr; 
	for (expr=sortexprs; expr; expr=(tExprCalc*) expr->WalkNext){
		SortExpressionsNum++;
	}

	SortExpressionsIndex=new tExprCalc*[SortExpressionsNum]; 
	SortExpressionsNum=0;
	for (expr=sortexprs; expr; expr=(tExprCalc*) expr->WalkNext){
		//suppose the original expression sentence is still in expr->ExprSentence->Sentence
		SortExpressionsIndex[SortExpressionsNum]=new tExprCalc;
		SortExpressionsIndex[SortExpressionsNum]->setExpression(expr->ExprSentence->Sentence); 
		if (expr->Context!=expr) 
			SortExpressionsIndex[SortExpressionsNum]->useContext(expr->Context);
		SortExpressionsIndex[SortExpressionsNum]->parse(synEXP_RIGHTBRACE); 
		SortExpressionsIndex[SortExpressionsNum]->setName(expr->Name); //also copy name set by ReferSqlParser::parseFieldsList()
		SortExpressionsNum++;
	}

	SortExpressions=SortExpressionsIndex[0]; 
	SortSet=sortset;

	//set WalkNext, so that it can be copied by setSortExpressions()
	for (int i=0; i<SortExpressionsNum; i++){
		SortExpressionsIndex[i]->WalkNext=(i<SortExpressionsNum-1)?SortExpressionsIndex[i+1]:0;
	}

	return SortExpressionsNum;
}

int ReferSetBTree::cmp(char* p1, char* p2){
	if (IndexNum==0){
		return (p1-p2>0)?1:((p1-p2<0)?-1:0);
	}
	ReferData* tuple1=0, *tuple2=0;

	if (TypeOfP==TYPE_REFLINK){
		tuple1=p1?(ReferData*) (((ReferLink*)p1)->Value.P):0;
		tuple2=p2?(ReferData*) (((ReferLink*)p2)->Value.P):0;
	}else{
		tuple1=(ReferData*) p1;
		tuple2=(ReferData*) p2;
	}

	ReferData* data1, *data2;
	ReferData value1, value2;
	tExprCalc* expr;
	int rank=0;
	if (!tuple1 && !tuple2) rank=0; 
	else if (!tuple1 && tuple2) rank=-1;
	else if (tuple1 && !tuple2) rank=1;
	else{
		int i;
		for (i=0; i<IndexNum; i++){
			if (IndexFields[i]>=0){
				//regular field
				data1=&tuple1[IndexFields[i]];
				data2=&tuple2[IndexFields[i]];
			}else{
				//compute sort expression
				expr=SortExpressionsIndex[-IndexFields[i]-1];
				SortSet->setExprVariables(tuple1, expr, 0); //rownum is 0
				expr->calculate(); 
				value1.Set(expr->StringValue.P, expr->StringValue.L, true); 
				SortSet->setExprVariables(tuple2, expr, 0); //rownum is 0
				expr->calculate(); 
				value2.Set(expr->StringValue.P, expr->StringValue.L, true); 
				data1=&value1; 
				data2=&value2; 
			}

			int c=0;
			if (IndexFlags[i] & FLAG_CHAR){ //string
				c=data1->Cmp(data2, !(IndexFlags[i]&FLAG_CASE_INSENSITIVE));
			}else if (IndexFlags[i] & FLAG_INTEGER){ //integer
				long d1=0, d2=0; 
				if (data1->P) sscanf(data1->P, "%ld", &d1);
				if (data2->P) sscanf(data2->P, "%ld", &d2);
				c=(d1>d2)?1:((d1==d2)?0:-1);
			}else if (IndexFlags[i] & FLAG_FLOAT){ //double
				double d1=0, d2=0; 
				if (data1->P) sscanf(data1->P, "%lf", &d1);
				if (data2->P) sscanf(data2->P, "%lf", &d2);
				c=(d1>d2)?1:((d1==d2)?0:-1);
			}else if (IndexFlags[i] & FLAG_REFER_P){ //Pointer
				long l=data1->P-data2->P;
				c=(l>0)?1:((l==0)?0:-1);
			}else if (IndexFlags[i] & FLAG_REFER_L){ //long
				long l=data1->L-data2->L;
				c=(l>0)?1:((l==0)?0:-1);
			}else{ //default: string
				c=data1->Cmp(data2, !(IndexFlags[i]&FLAG_CASE_INSENSITIVE));
			}
			if (c) {
				if (IndexFlags[i]&FLAG_DESCENDING)
					rank=-c;
				else
					rank=c;
				break;
			}
		}
	}
	return rank;
}
int ReferSetBTree::insert(char* p){
	if (TypeOfP==TYPE_REFLINK){
		return BTree::insert(p);
	}else{
		return BTree::insert(((ReferLink2*)p)->Value.P);
	}
}

ReferSetHeap::ReferSetHeap(const long fields_num, const long key_field0, const long key_field_flag): ReferSet("ReferSetHeap"){
	KeyField=0;
	KeyIndex=0;
	setFieldsNum(fields_num);
	setKeyField(key_field0, key_field_flag);
}

ReferSetHeap::~ReferSetHeap(){
	reset();
}

void ReferSetHeap::reset(){
	ReferSet::reset();
	KeyField=0;
	KeyIndex=0;
}
int ReferSetHeap::setKeyField(const long key_field0, const long key_field_flag){
	if (key_field0>=FieldsNum){
		setFieldsNum(key_field0+1);
	}

	KeyField=key_field0;
	KeyIndex=newIndex("ReferSetHeap", 1, &key_field0, &key_field_flag);
	UseIndex.tree=KeyIndex;
	return 0;
}

ReferData* ReferSetHeap::findKey(ReferData* key){
	UseIndex.tree=KeyIndex;
	
	//tuple for search
	if (QueryTuple){
		delete[] QueryTuple;
		QueryTuple=0;
	}
	QueryTuple=new ReferData[FieldsNum];
	QueryTuple[KeyField].Set(key->P, key->L, false);

	return moveFirst(QueryTuple);
}

ReferData* ReferSetHeap::addkey(ReferData* key){
	ReferData* tuple=newTuple();
	tuple[KeyField].Set(key->P, key->L, key->Type);//may cause error for string type?

	return tuple;
}













