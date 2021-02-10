#include "refersqlparser.h"

#include "ethoerror.h"
#include "iwsqlsyntax.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ReferSqlParser::ReferSqlParser():
	TableNames("ReferSqlParser::TableNames")
{
	Next=0;
	Parent=0;
	SubQueries=0;
	OutputUnique=false;

	TableNames.setFieldsNum(ID_TABLE_FIELDS_NUM);

	OutputExprs=0;
	OutputFields.setSortOrder(SORT_ORDER_NUM_INC);
	OutputFields.setDuplication(false);
	OutputFields.setCaseSensitivity(false);

	GroupByExprs=0;
	GroupByFields.setSortOrder(SORT_ORDER_NUM_INC);
	GroupByFields.setDuplication(false);
	GroupByFields.setCaseSensitivity(false);

	OrderByExprs=0;
	OrderByFields.setSortOrder(SORT_ORDER_NUM_INC);
	OrderByFields.setDuplication(false);
	OrderByFields.setCaseSensitivity(false);

	SqlType=SQL_SELECT;
}

ReferSqlParser::~ReferSqlParser(){
	reset();
}

void ReferSqlParser::reset(){
	Sql.reset();
	SqlSyntax.reset();
	if (Next){
		delete Next;
		Next=0;
	}
	Parent=0;
	TableNames.empty();
	if (SubQueries){
		delete SubQueries;
		SubQueries=0;
	}
	if (OutputExprs){
		deleteFieldExprs(OutputExprs);
		OutputExprs=0;
	}
	OutputFields.reset(); 
	OutputFields.setSortOrder(SORT_ORDER_NUM_INC);
	OutputFields.setDuplication(false);
	OutputFields.setCaseSensitivity(false);

	if (GroupByExprs){
		deleteFieldExprs(GroupByExprs);
		GroupByExprs=0;
	}
	GroupByFields.reset();
	GroupByFields.setSortOrder(SORT_ORDER_NUM_INC);
	GroupByFields.setDuplication(false);
	GroupByFields.setCaseSensitivity(false);

	if (OrderByExprs){
		deleteFieldExprs(OrderByExprs);
		OrderByExprs=0;
	}
	OrderByFields.reset();
	OrderByFields.setSortOrder(SORT_ORDER_NUM_INC);
	OrderByFields.setDuplication(false);
	OrderByFields.setCaseSensitivity(false);

	Condition.reset();
	Expression.reset();
	HavingCondition.reset();
	HavingExpression.reset();

	OutputUnique=false;
}
int ReferSqlParser::deleteFieldExprs(tExprCalc* fieldexprs){
	tExprCalc* temp;
	while (fieldexprs){
		temp=(tExprCalc*) fieldexprs->WalkNext;
		delete fieldexprs;
		fieldexprs=temp;
	}
	return 0;
}

int ReferSqlParser::setQuery(const char* sql){
	SqlSyntax.reset();
	SqlSyntax.setSentence(sql, 0, true);

	return parseQuery(&SqlSyntax);
}

int ReferSqlParser::parseQuery(IWSqlSyntax* syntax){
	ToSelectAll=0;
	int count=0;

	Sql.Set(syntax->Sentence+syntax->Start, syntax->Start, false); //Sql.Start: record the start position

	int err=0, err1=0;
	if (syntax->Type==IWSqlSyntax::synSQL_DELETE){
		syntax->match();
		SqlType=SQL_DELETE;

		if (syntax->Type==IWSqlSyntax::synSQL_FROM){
			syntax->match();

			err1=parseTablesList(syntax);
			if (err1) err=err1;
		}

		err1=parseWhere(syntax);
		if (err1) err=err1;

	}else if (syntax->Type==IWSqlSyntax::synSQL_UPDATE){
		syntax->match();
		SqlType=SQL_UPDATE;

		if (syntax->Type!=IWSqlSyntax::synSQL_SET){
			if (err1=parseTablesList(syntax)) err=err1;
		}

		if (err1=parseUpdateSetQuery(syntax, &OutputExprs)) err=err1;
		if (err1=indexFieldsInHeap(OutputExprs, &OutputFields, "Field")) err=err1;
		if (err1=parseWhere(syntax)) err=err1;

	}else if (syntax->Type==IWSqlSyntax::synSQL_INSERT){
		syntax->match();
		SqlType=SQL_INSERT;

		if (syntax->Type==IWSqlSyntax::synSQL_INTO){
			syntax->match();

			if (err1=parseTablesList(syntax)) err=err1;
		}

		if (syntax->Type==IWSqlSyntax::synQL_LBRACE){
			syntax->match(); 

			if (syntax->Type == IWSqlSyntax::synQL_STAR){
				ToSelectAll = true;
				syntax->match();
			}else{
				if (err1=parseFieldsList(syntax, &OutputExprs)) err=err1;
			}

			if (!syntax->match(IWSqlSyntax::synQL_RBRACE)) return dbSYNTAXERR;
		}

		if (err1=indexFieldsInHeap(OutputExprs, &OutputFields, "Field")) err=err1;

		if (syntax->Type!=IWSqlSyntax::synSQL_VALUES){ 
			err=ERR_ETHO_SQL_INSERT_NOVALUE;
		}else {
			syntax->match(); 
		}

		if (syntax->Type==IWSqlSyntax::synQL_LBRACE){
			syntax->match(); 

			if (err1=parseInsertValues(syntax)) err=err1;

			if (syntax->Type==IWSqlSyntax::synQL_RBRACE){
				syntax->match(); 
			}else{
				err=dbSYNTAXERR;
			}
		}
	}else{ // IWSqlSyntax::synSQL_SELECT or others such as ORDER BY
		if (syntax->Type==IWSqlSyntax::synSQL_SELECT){
			syntax->match();
			SqlType=SQL_SELECT;

			if (syntax->Type==IWSqlSyntax::synSQL_UNIQUE) {
				syntax->match();
				OutputUnique=true;
			}

			if (syntax->Type == IWSqlSyntax::synQL_STAR){
				ToSelectAll = true;
				syntax->match();
			}else{
				if (err1=parseFieldsList(syntax, &OutputExprs)) err=err1;
			}
		}

		if (syntax->Type==IWSqlSyntax::synSQL_FROM){
			syntax->match();

			if (err1=parseTablesList(syntax)) err=err1;
		}

		if (err1=indexFieldsInHeap(OutputExprs, &OutputFields, "Field")) err=err1;
		if (err1=parseWhere(syntax)) err=err1;

	} 

	Sql.L=syntax->Start-Sql.L;
	Sql.Seperate();

	return err;
}
int ReferSqlParser::parseUpdateSetQuery(IWSqlSyntax* syntax, tExprCalc **expr_list){
	if (syntax->Type!=IWSqlSyntax::synSQL_SET) return 0;
	syntax->match(); 

	int j;
	tExprCalc **NewExpression;
	NewExpression=expr_list;
	ReferData fieldsentence;
	while (*NewExpression!=NULL) NewExpression=(tExprCalc**)&(*NewExpression)->WalkNext;
	do{
		ReferData name; 
		name.Set(syntax->Sentence+syntax->Start, syntax->StartLen, false);
		syntax->match();
		
		if (!syntax->match(IWSqlSyntax::synQL_EQ)) return ERR_ETHO_SQL_UPDATE_NOASSIGN;

		*NewExpression = new tExprCalc;
		// (*NewExpression)->useContext(&Expression); //changed because the parser Expression may be deleted
		if (Expression.Context != &Expression){
			(*NewExpression)->useContext(Expression.Context);
		}
		if ((j=parseExpr(*NewExpression, syntax, &fieldsentence, synEXP_RIGHTBRACE))!=dbSUCCESS) return j;
		if (fieldsentence.L==0){ //no any word, syntax error
			delete (*NewExpression);
			*NewExpression=0;
			return dbSYNTAXERR; //return syntax error, do later
		}
		(*NewExpression)->ExprSentence->setSentence(fieldsentence.P,&fieldsentence.L);
		//(*NewExpression)->setExpression(fieldsentence.P, &fieldsentence.L, true);
		(*NewExpression)->setName(name.P, name.L, true);

		if (syntax->Type!=QLSyntax::synQL_COMMA) break;
		syntax->match();
		NewExpression=(tExprCalc**)&(*NewExpression)->WalkNext;
	}while (1);

	return dbSUCCESS;	
}
int ReferSqlParser::parseInsertValues(IWSqlSyntax* syntax){
	int err=0, err1=0;
	tExprCalc* value_expr=0;
	if (err1=parseFieldsList(syntax, &value_expr)) err=err1;

	if (!err){
		tExprCalc* field1=OutputExprs; 
		tExprCalc* field2=value_expr; 
		while (field1 && field2){
			field2->setName(field1->StringName.P, field1->StringName.L, true); 
			field1=(tExprCalc*) field1->WalkNext; 
			field2=(tExprCalc*) field2->WalkNext;
		}
		if (OutputExprs) {
			deleteFieldExprs(OutputExprs);
		}
		OutputExprs=value_expr;
	}

	OutputFields.empty();
	if (err1=indexFieldsInHeap(OutputExprs, &OutputFields, "Field")) err=err1;

	return err;
}
int ReferSqlParser::parseTablesList(IWSqlSyntax* syntax){
	int err=0;
	ReferData tname, dsname;
	ReferData* tuple=0;
	while (syntax->Type != IWSqlSyntax::synSQL_WHERE && syntax->Type != IWSqlSyntax::synSQL_SET){
		dsname="";
		if (syntax->Type == IWSqlSyntax::synQL_WORD){
			if (syntax->NextType == IWSqlSyntax::synQL_DOT){
				dsname.Set(syntax->Sentence+syntax->Start, syntax->StartLen, 1);
				syntax->match();
				syntax->match();
				if (syntax->Type != IWSqlSyntax::synQL_WORD) return ERR_ETHO_SQL_SEL_TABLE_NOTSUPPORT;
			}
			tname.Set(syntax->Sentence+syntax->Start, syntax->StartLen, 1);
			tuple=TableNames.newTuple(); 
			tuple[ID_TABLE_NAME]=tname;
			tuple[ID_TABLE_DSNAME]=dsname;
			tuple[ID_TABLE_ASNAME]=tname;
		}else if (syntax->Type==QLSyntax::synQL_LBRACE){
			//sub query
			syntax->match();

			ReferSqlParser* subquery=new ReferSqlParser;
			subquery->parseQuery(syntax);

			subquery->Next=SubQueries;
			SubQueries=subquery;

			if (!syntax->match(QLSyntax::synQL_RBRACE)) return ERR_ETHO_SQL_SEL_TABLE_NOTSUPPORT;
		}else{
			return ERR_ETHO_SQL_SEL_TABLE_NOTSUPPORT;
		}
		if (syntax->Type==IWSqlSyntax::synSQL_AS){
			syntax->match();
			tuple[ID_TABLE_ASNAME].Set(syntax->Sentence+syntax->Start, syntax->StartLen, true);
			syntax->match();
		}

		syntax->match();
		if (syntax->Type != IWSqlSyntax::synQL_COMMA) break;
		syntax->match();
	}
	return err;
}

int ReferSqlParser::parseWhere(IWSqlSyntax* syntax){
	int err=0, err1=0;
	if (syntax->Type == IWSqlSyntax::synSQL_WHERE) syntax->match(); //WHERE can be optional? 

	if (syntax->Type != IWSqlSyntax::synSQL_GROUP && syntax->Type!=IWSqlSyntax::synSQL_GROUPBY && syntax->Type!=IWSqlSyntax::synSQL_HAVING
		&& syntax->Type!=IWSqlSyntax::synSQL_ORDER
		){
		err1=parseExpr(&Expression, syntax, &Condition, synEXP_RIGHTBRACE);
		if (err1) err=err1;
	}

	Expression.buildVarsList();

	if ((syntax->Type==IWSqlSyntax::synSQL_GROUP && syntax->NextType==IWSqlSyntax::synSQL_BY) || syntax->Type==IWSqlSyntax::synSQL_GROUPBY){
		if (syntax->Type==IWSqlSyntax::synSQL_GROUPBY){
			syntax->match();
		}else{
			syntax->match(); syntax->match();
		}
		err1=parseFieldsList(syntax, &GroupByExprs);
		if (err1) err=err1;
		err1=indexFieldsInHeap(GroupByExprs, &GroupByFields, "Groupby");
		if (err1) err=err1;
	}

	if (syntax->Type==IWSqlSyntax::synSQL_HAVING){
		syntax->match();
		
		err1=parseExpr(&HavingExpression, syntax, &HavingCondition, synEXP_RIGHTBRACE);
		if (err1) err=err1;
	}

	if (syntax->Type==IWSqlSyntax::synSQL_ORDER && syntax->NextType==IWSqlSyntax::synSQL_BY){
		syntax->match(); syntax->match();
		err1=parseFieldsList(syntax, &OrderByExprs);
		if (err1) err=err1;
		err1=indexFieldsInHeap(OrderByExprs, &OrderByFields, "Orderby");
		if (err1) err=err1;
	}
	
	return err;
}
int ReferSqlParser::indexFieldsInHeap(tExprCalc* fields, ReferLinkHeap* heap, char* default_name){
	int fieldindex=heap->Total;
	char buf[128];
	ReferData tmp;
	for (tExprCalc* calc=fields; calc; calc=(tExprCalc*) calc->WalkNext){
		if (!calc->StringName.L){
			sprintf(buf, "%s%d", default_name?default_name:"Fields", fieldindex+1);
			calc->setName(buf, strlen(buf), strlen(buf));
		}
		tmp = calc->ExprSentence->Sentence; //include the field expression, as name and its modifiers such as DESC, ASC, NUMBER ...
		heap->add(&calc->StringName, &tmp, fieldindex++);
	}
	return 0;
}

int ReferSqlParser::parseFieldsList(IWSqlSyntax* syntax, tExprCalc** expr_list){
	int j;
	tExprCalc **NewExpression;
	NewExpression=expr_list;
	ReferData fieldsentence;
	while (*NewExpression!=NULL) NewExpression=(tExprCalc**)&(*NewExpression)->WalkNext;
	do{
		*NewExpression = new tExprCalc;
		// (*NewExpression)->useContext(&Expression); //changed because the parser Expression may be deleted
		if (Expression.Context != &Expression){
			(*NewExpression)->useContext(Expression.Context);
		}
		if ((j=parseExpr(*NewExpression, syntax, &fieldsentence, synEXP_RIGHTBRACE))!=dbSUCCESS) return j;
		if (fieldsentence.L==0){ //no any word, syntax error
			delete (*NewExpression);
			*NewExpression=0;
			return dbSYNTAXERR; //return syntax error, do later
		}
		(*NewExpression)->ExprSentence->setSentence(fieldsentence.P,&fieldsentence.L);
		//(*NewExpression)->setExpression(fieldsentence.P, &fieldsentence.L, true);

		if (syntax->Type==IWSqlSyntax::synSQL_AS){
			syntax->match();
			(*NewExpression)->setName(syntax->Sentence+syntax->Start, syntax->StartLen, true);
			syntax->match();
		}

		long start=syntax->Start;
		while (syntax->Type==IWSqlSyntax::synSQL_DESC || syntax->Type==IWSqlSyntax::synSQL_ASC
			|| syntax->Type==IWSqlSyntax::synSQL_NUMBER || syntax->Type==IWSqlSyntax::synSQL_INTEGER 
			|| syntax->Type==IWSqlSyntax::synSQL_CHAR || syntax->Type==IWSqlSyntax::synSQL_LONG 
			|| syntax->Type==IWSqlSyntax::synSQL_DATE
			){
			syntax->match(); //put all modifiers in the (*NewExpression)->StringValue
		}
		if (syntax->Start!=start){
			(*NewExpression)->StringValue.Set(syntax->Sentence+start, syntax->Start-start, true);
		}

		if (syntax->Type!=QLSyntax::synQL_COMMA) break;
		syntax->match();
		NewExpression=(tExprCalc**)&(*NewExpression)->WalkNext;
	}while (1);
	return dbSUCCESS;
}

int ReferSqlParser::parseExpr(tExprCalc* calc, IWSqlSyntax* syntax, ReferData* condition, etEXP_SYNTAX pre_op){
	calc->setExpression(syntax->Sentence + syntax->Start);
	calc->parse(pre_op);
	if (condition){
		condition->Set(calc->ExprSentence->Sentence, calc->ExprSentence->Start, true);
	}
	syntax->Next = syntax->Start;
	syntax->NextLen = calc->ExprSentence->Start;
	syntax->NextType = QLSyntax::synQL_UNKNOW;
	syntax->match();
	syntax->match();
	return 0;
}
int ReferSqlParser::getSqlErrorMsg(int err, ReferData* errormsg){
	switch (err){
	case ERR_ETHO_SQL_NOSELECT: *errormsg+="SQL no SELECT."; break;
	case ERR_ETHO_SQL_NOFROM: *errormsg+="SQL no FROM."; break;
	case ERR_ETHO_SQL_NOWHERE: *errormsg+="SQL no WHERE."; break;
	case ERR_ETHO_SQL_SEL_FIELD_NOTSUPPORT: *errormsg+="SQL select field not supported."; break;
	case ERR_ETHO_SQL_SEL_TABLE_NOTSUPPORT: *errormsg+="SQL select table not supported."; break;
	case ERR_ETHO_SQL_EXECUTION_ERR: *errormsg+="SQL execution error."; break;
	case ERR_ETHO_FIELD_INDEX_EXCEED: *errormsg+="SQL index field error."; break;
	case ERR_ETHO_FIELD_NAME_NOTFOUND: *errormsg+="SQL field name not found."; break;
	case ERR_ETHO_CONFIG_FILE_OPEN_FAIL: *errormsg+="SQL configuration file not found."; break;
	case ERR_ETHO_CONFIG_DATA_SOURCE_FAIL: *errormsg+="SQL configuration data source not found."; break;
	case ERR_ETHO_DATABASE_OPEN_FAIL: *errormsg+="SQL database open failed."; break;
	case ERR_ETHO_DATABASE_NULL_CONNECTSTR: *errormsg+="SQL database no connection string."; break;
	case ERR_ETHO_DATABASE_UNKNOWN_SOURCE: *errormsg+="SQL database unkown data source."; break;
	case ERR_ETHO_DATABASE_OPEN_NOURL: *errormsg+="SQL database no URL open failure."; break;
	case ERR_ETHO_DATABASE_OPEN_NOFORM: *errormsg+="SQL database no FORM open failure."; break;
	case ERR_ETHO_RECORDSET_OPEN_FAIL: *errormsg+="SQL recordset open failure."; break;
	case ERR_ETHO_RECORDSET_MOVENEXT_ERR: *errormsg+="SQL recordset move next failure."; break;
	case ERR_ETHO_RECORDSET_MOVEPREV_ERR: *errormsg+="SQL recordset move previous failure."; break;
	case ERR_ETHO_RECORDSET_DELREC_ERR: *errormsg+="SQL recordset delete record failure."; break;
	case ERR_ETHO_RECORDSET_GETFIELD_ERR: *errormsg+="SQL recordset get field failure."; break;
	case ERR_ETHO_RECORDSET_GETFIELDNAME_ERR: *errormsg+="SQL recordset get field name failure."; break;
	case ERR_ETHO_RECORDSET_GETFIELDTYPE_ERR: *errormsg+="SQL recordset get field type failure."; break;
	case ERR_ETHO_RECORDSET_NOTOPEN: *errormsg+="SQL recordset not openned."; break;
	case ERR_ETHO_TABLE_DROP_FAIL: *errormsg+="SQL drop table failure."; break;
	case ERR_ETHO_TABLE_CREATE_FAIL: *errormsg+="SQL create table failure."; break;
	case ERR_ETHO_TABLE_NO_METAALL: *errormsg+="SQL table no meta all."; break;
	case ERR_ETHO_TABLE_NO_METAFIELD: *errormsg+="SQL table no meta field."; break;
	case ERR_ETHO_INTERFACE_NOTSUPPORT: *errormsg+="SQL interface not supported."; break;
	case ERR_ETHO_SESSION_CREATION_FAIL: *errormsg+="SQL session creation failed."; break;
	case ERR_ETHO_SESSION_NOTFOUND: *errormsg+="SQL session not found."; break;
	case ERR_ETHO_SESSION_NOTOPEN: *errormsg+="SQL session not openned."; break;
	case ERR_ETHO_SCRIPT_NOVARIABLE: *errormsg+="SQL script no variable."; break;
	case ERR_ETHO_SQL_INSERT_NOVALUE: *errormsg+="SQL insert no VALUES."; break;
	case ERR_ETHO_SQL_UPDATE_NOASSIGN: *errormsg+="SQL update no =."; break;
	default: *errormsg+="SQL error, "; 
		tExprCalc::getExprErrorMsg(err, errormsg);
		break;
	}
	return 0;
}



