
#include "HtWrapperModels.h"
#include "htmlql.h"
#include "htwrapper.h"
#include "htqlexpr.h"
#include "alignment.h"
#include "htmlbuf.h"
#include "expr.h"
#include "tsoper.h"
#include "HtPageModel.h"
#include <math.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE
#endif





HtWrapperModels::HtWrapperModels(): Models("HtWrapperModels::Models"), Pages("HtWrapperModels::Pages") {
	setupInitialModels();
	MaxLocalModels=3;
}

HtWrapperModels::~HtWrapperModels(){
	reset();
}

void HtWrapperModels::reset(){
	Models.reset();
	Pages.reset();
}

int HtWrapperModels::setupInitialModels(){
	const char* model_fields[]={"ModelID", "ModelType", "PatternType", "Pattern", "Options", 0};
	Models.setFieldsNum(ID_MODEL_FIELDS_NUM, model_fields);
	const long model_idx[]={ID_MODEL_TYPE, ID_PATTERN_TYPE};
	const long model_flags[]={ReferSetBTree::FLAG_CHAR|ReferSetBTree::FLAG_CASE_INSENSITIVE, 
		ReferSetBTree::FLAG_CHAR|ReferSetBTree::FLAG_CASE_INSENSITIVE};
	Models.newIndex("ModelPattern", 2, model_idx, model_flags);
	Models.newIndex("ModelType", 1, model_idx, model_flags);
	
	const char* page_fields[]={"ModelID", "PageSource", "PageUrl", "SavedFile", 0};
	Pages.setFieldsNum(ID_PAGE_FIELDS_NUM, page_fields);
	const long model_idx1[]={ID_MODEL_ID};
	const long page_flags[]={ReferSetBTree::FLAG_INTEGER};
	Pages.newIndex("ModelID", 2, model_idx1, page_flags);

	return 0;
}
ReferData* HtWrapperModels::addNewPage(ReferData* url, ReferData*page, ReferData* model_id){
	Pages.useIndexName("ModelID");
	ReferData* tuple;
	if (!model_id->P){
		if (Pages.TupleCount==0 && Models.TupleCount>0){
			ReferSet existing_ids;
			Models.executeSQL("select ModelID where ModelType='Local' group by ModelID order by ModelID integer inc", &existing_ids, 0);
			for (ReferData* tuple1=existing_ids.moveFirst(); tuple1; tuple1=existing_ids.moveNext()){
				tuple=Pages.newTuple();
				tuple[ID_MODEL_ID].Set(tuple1->P, tuple1->L, true);
			}
		}
		long modelid=0;
		for (tuple=Pages.moveLast(); tuple; tuple=Pages.movePrevious()){
			if (tuple[ID_MODEL_ID].P && tStrOp::isDigit(tuple[ID_MODEL_ID].P[0]) ){
				sscanf(tuple[ID_MODEL_ID].P, "%ld", &modelid);
				break;
			}
		}
		model_id->setLong(modelid+1);
	}
	tuple=Pages.newTuple();
	tuple[ID_MODEL_ID].Set(model_id->P, model_id->L, true);
	tuple[ID_MODEL_ID].L=strlen(tuple[ID_MODEL_ID].P);
	tuple[ID_PAGE_SOURCE].Set(page->P, page->L,true);
	tuple[ID_PAGE_URL].Set(url->P, url->L,true);

	*model_id=tuple[ID_MODEL_ID];
	return tuple;
}

int HtWrapperModels::addNewModels(ReferData*model_id, ReferData* htql, long pattern_type){
	ReferData *tuple;
	tuple=Pages.findFieldString("ModelID", model_id);
	if (!tuple) return -1;
	
	ReferSet temp_set;
	Models.executeSQL("select ModelID where ModelType='Local' group by ModelID", &temp_set);
	if (temp_set.TupleCount>MaxLocalModels-1){
		dropWorseModel(model_id, htql);
	}

	HtmlQL ql;
	ql.setSourceData(tuple[ID_PAGE_SOURCE].P, tuple[ID_PAGE_SOURCE].L,false);
	ql.setSourceUrl(tuple[ID_PAGE_URL].P, tuple[ID_PAGE_URL].L);

	ql.setQuery(htql->P);
	if (pattern_type & PAT_HTQL) {
		ql.setQuery(htql->P);
		createLocalModelHtql(model_id, &ql, htql);
	}
	if (pattern_type & PAT_HTQL_SCHEMA){
		ql.setQuery(htql->P);
		createLocalModelHtqlSchema(model_id, htql);
	}
	if (pattern_type & PAT_TAG_PATTERN) {
		ql.setQuery(htql->P);
		createLocalModelTagPattern(model_id, &ql);
	}
	if (pattern_type & PAT_TEXT_PATTERN){
		ql.setQuery(htql->P);
		createLocalModelTextPattern(model_id, &ql);
	}
	if (pattern_type & PAT_HTQL_NEXT){
		ql.setQuery(htql->P);
		createLocalModelHtqlNext(model_id, &ql, htql);
	}
	if (pattern_type & PAT_TEXT){
		ql.setQuery(htql->P);
		createLocalModelText(model_id, &ql);
	}

	return 0;
}

double HtWrapperModels::validateWithModel(ReferData*page, ReferData* htql, ReferData* model_id, long pattern_type){
	double score;
	double highest=0;
	if (pattern_type & PAT_HTQL){
		score=validateModelHtql(page, htql, model_id);
		if (score>highest) highest=score;
	}
	if (pattern_type & PAT_HTQL_SCHEMA){
		score=validateModelHtqlSchema(page, htql, model_id);
		if (score>highest) highest=score;
	}
	if (pattern_type & PAT_TAG_PATTERN){
		score=validateModelTagPattern(page, htql, model_id);
		if (score>highest) highest=score;
	}
	if (pattern_type & PAT_TEXT_PATTERN){
		score=validateModelTextPattern(page, htql, model_id);
		if (score>highest) highest=score;
	}
	if (pattern_type & PAT_TEXT){
		score=validateModelText(page, htql, model_id);
		if (score>highest) highest=score;
	}
	return highest;
}
long HtWrapperModels::findBetterMatchPos(ReferData*page, ReferData* htql, ReferData* model_id, long pattern_type, double* score){
	HtPageModel model;
	int err=model.setModelPage(page, false);
	if (err<0) return err;

	int page_model=0;
	if (pattern_type & PAT_HTQL) page_model|=HtPageModel::MODEL_HTQL;
	if (pattern_type & PAT_TAG_PATTERN) page_model|=HtPageModel::MODEL_HTML_PATTERN;
	if (pattern_type & PAT_TEXT_PATTERN) page_model|=HtPageModel::MODEL_TEXT_PATTERN;
	if (pattern_type & PAT_TEXT) page_model|=HtPageModel::MODEL_PLAIN_PATTERN;
	model.addWrapperModelScore(this, model_id, page_model, 1.0);

	ReferLinkHeap results;
	model.recombineScores();
	model.findBestPos(&results);
	
	HtPageModel model1;
	err=model1.setModelPage(page, false);
	if (err<0) return err;
	model1.addModelScore(htql, false, HtPageModel::MODEL_HTQL, 2, HtPageModel::FILL_ALL);

	ReferLink* link=(ReferLink*) results.moveFirst(); 
	long best_pos=-1;
	if (link) {
		best_pos=link->Value.L;
		if (score) *score=(double) link->Data/1.0e5;
	}else{
		if (score) *score=0;
	}
#ifdef DEBUG_THIS_FILE
	long print_count=0;
	while (link){
		TRACE("BETTER-MATCH: pos=%ld, score=%ld\n", link->Value.L, link->Data);
		link=(ReferLink*) results.moveNext(); 
		if (print_count++>10) break; 
	}
#endif
	if (best_pos>=0 && model1.PosScores[best_pos]<1)
		return best_pos;
	else
		return -1;
}
int HtWrapperModels::selectAlternativeHtql(ReferData*page, long target_pos, ReferData* model_id, ReferData* selected_htql){
	ReferData* tuple;
	if (model_id && model_id->L){
		//select from a specific model
		ReferSet patternhtql;
		tExprCalc context;
		context.setVariable("model_id", model_id->P);
		Models.executeSQL("select Pattern where ModelID=model_id and ModelType='Local' and PatternType='HtqlPrefix'", &patternhtql, &context);
		
		HtmlQL ql;
		ql.setSourceData(page->P, page->L, false);

		//test valid and invalid htqls
		long from, to;
		ReferLinkHeap valid, invalid;
		for (tuple=patternhtql.moveFirst(); tuple; tuple=patternhtql.moveNext()){
			ql.setQuery(tuple->P);
			ql.dotQuery("&position .<position>:from, to, length");
			
			int is_valid=0;
			if (!ql.isEOF()){
				from=atoi(ql.getValue(1));
				to=atoi(ql.getValue(2));
				if (from<=target_pos && to>target_pos){
					valid.add(tuple, 0, 0);
					is_valid=1;
				}
			}
			if (!is_valid){
				invalid.add(tuple, 0, 0);
			}
		}

		//drop invalid htql
		if (valid.Total>0) {
			ReferLink* link;
			for (link=(ReferLink*) invalid.moveFirst(); link; link=(ReferLink*) invalid.moveNext()){
				context.setVariable("p", link->Name.P);
				Models.executeSQL("delete where Pattern=p and ModelID=model_id and ModelType='Local' and PatternType='HtqlPrefix'", 0, &context);
			}

			//set the selected htql
			link=(ReferLink*) valid.moveFirst();
			if (selected_htql){
				selected_htql->Set(link->Name.P, link->Name.L, true);
			}
			return 1;
		}
		return 0;
	}else{
		//select from all models
		ReferSet model_ids;
		Models.executeSQL("select ModelID where ModelType='Local' and PatternType='HtqlPrefix' group by ModelID", &model_ids);
		ReferData selected;
		int valid_count=0;
		for (tuple=model_ids.moveFirst(); tuple; tuple=model_ids.moveNext()){
			if (selectAlternativeHtql(page, target_pos, tuple, &selected)){
				if (selected_htql && !selected_htql->L) {
					selected_htql->Set(selected.P, selected.L, true);
				}
				if (model_id && !model_id->L){
					model_id->Set(tuple->P, tuple->L, true);
				}
				valid_count++;
			}
		}
		return valid_count;
	}
}

int HtWrapperModels::dropWorseModel(ReferData*page, ReferData* htql){
	ReferSet temp_set;
	Models.executeSQL("select ModelID where ModelType='Local' group by ModelID", &temp_set);
	double *scores=new double[temp_set.TupleCount];
	ReferData* tuple;
	long count=0;
	for (temp_set.moveFirst(); !temp_set.isEOF(); temp_set.moveNext()){
		tuple=temp_set.getTuple();

		scores[count]=validateWithModel(page, htql, &tuple[ID_MODEL_ID]);

		count++;
	}

	long min_i=0;
	double min_score=tTsOp::getMin(scores, temp_set.TupleCount, &min_i);
	temp_set.moveFirst();
	for (count=0; count<min_i; count++) temp_set.moveNext();
	tuple=temp_set.getTuple();

	tExprCalc context;
	context.setVariable("pid", tuple[ID_MODEL_ID].P, tuple[ID_MODEL_ID].L, false);
	Models.filterByCondition("ModelType='Local' and ModelID=pid", 0, &context, true);
	
	delete[] scores;

	return 0;
}

int HtWrapperModels::createLocalModelTagPattern(ReferData*model_id, HTQL* ql){
	ReferData pattern;
	getConsensusTagPattern(ql, &pattern);

	ReferData* tuple=Models.newTuple();
	tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
	tuple[ID_MODEL_TYPE]="Local";
	tuple[ID_PATTERN_TYPE]="TagPattern";
	tuple[ID_PATTERN]=pattern;

	return 0;
}
int HtWrapperModels::createLocalModelTextPattern(ReferData*model_id, HTQL* ql){
	ReferData pattern;
	getConsensusTextPattern(ql, &pattern);

	ReferData* tuple=Models.newTuple();
	tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
	tuple[ID_MODEL_TYPE]="Local";
	tuple[ID_PATTERN_TYPE]="TextPattern";
	tuple[ID_PATTERN]=pattern;

	return 0;
}

int HtWrapperModels::createLocalModelText(ReferData*model_id, HTQL* ql){
	ReferData pattern;
	getConsensusText(ql, &pattern);

	ReferData* tuple=Models.newTuple();
	tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
	tuple[ID_MODEL_TYPE]="Local";
	tuple[ID_PATTERN_TYPE]="Text";
	tuple[ID_PATTERN]=pattern;

	return 0;
}

int HtWrapperModels::createLocalModelHtqlNext(ReferData*model_id, HTQL* ql, ReferData* htql){
	HtqlExpression htqlexpr;
	htqlexpr.setExpr(htql->P, htql->L);
	int num=htqlexpr.getTagSelectionsNum();

	ReferData query;
	int i;
	for (i=num; i>0; i--){
		query=htqlexpr.replaceTagSelectionRange(i, 0, 0);
		ql->setQuery(query.P);
		if (ql->getTuplesCount()>1) break;
	}
	ReferData* tuple=0;
	if (i>0){//simple replace the range
		tuple=Models.newTuple();
		tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
		tuple[ID_MODEL_TYPE]="Local";
		tuple[ID_PATTERN_TYPE]="HtqlNext";
		tuple[ID_PATTERN]=query;
	}else{//more complex, search from the html tags, do it later
		tuple=Models.newTuple();
		tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
		tuple[ID_MODEL_TYPE]="Local";
		tuple[ID_PATTERN_TYPE]="HtqlNext";
		tuple[ID_PATTERN]=*htql;
	}
	return 0;
}

int HtWrapperModels::createLocalModelHtql(ReferData*model_id, HTQL* ql, ReferData* htql){
	HtqlExpression htqlexpr;
	htqlexpr.setExpr(htql->P, htql->L);
	ReferData prefix;
	prefix=htqlexpr.getSchemaPrefix(1);
	char buf[128];

	ReferData* tuple=0;
	int heuristic[]={HtmlQL::heuBEST, HtmlQL::heuBEST_VIEW, -1};
	for (int h=0; heuristic[h]>=0; h++){
		ql->setQuery(prefix.P);
		sprintf(buf, "&find_htql(%d) ./'\\n'/", heuristic[h]); //HtmlQL::heuBEST_VIEW, HtmlQL::heuBEST
		ql->dotQuery(buf);
		int i=0;
		ReferData query;
		for (ql->moveFirst(); !ql->isEOF(); ql->moveNext()){
			query=ql->getValue(1);
			if (query.L){
				tuple=Models.newTuple();
				tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
				tuple[ID_MODEL_TYPE]="Local";
				tuple[ID_PATTERN_TYPE]="HtqlPrefix";
				tuple[ID_PATTERN]=query;
			}
			if (++i>=2) break;
		}
	}

	tuple=Models.newTuple();
	tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
	tuple[ID_MODEL_TYPE]="Local";
	tuple[ID_PATTERN_TYPE]="SelectedHTQL";
	tuple[ID_PATTERN]=htql->P;

	return 0;
}
int HtWrapperModels::createLocalModelHtqlSchema(ReferData*model_id, ReferData* htql){
	HtqlExpression htqlexpr;
	ReferData prefix, schema;
	htqlexpr.setExpr(htql->P, htql->L);
	schema=htqlexpr.getSchema(1);

	ReferData* tuple;
	if (schema.L){
		tuple=Models.newTuple();
		tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
		tuple[ID_MODEL_TYPE]="Local";
		tuple[ID_PATTERN_TYPE]="HtqlSchema";
		tuple[ID_PATTERN]=schema;
	}

	return 0;
}

double HtWrapperModels::validateModelHtql(ReferData*page, ReferData* htql, ReferData* model_id){
	HtmlQL ql;
	ql.setSourceData(page->P, page->L, false);
	ql.setQuery(htql->P);

	if (ql.isEOF()) return 0;
	return 0.5;
}
double HtWrapperModels::validateModelHtqlSchema(ReferData*page, ReferData* htql, ReferData* model_id){
	HtqlExpression htqlexpr;
	ReferData prefix, schema;
	htqlexpr.setExpr(htql->P, htql->L);
	prefix=htqlexpr.getSchemaPrefix(1);

	ReferData query; query="select Pattern where ModelType='Local' and PatternType='HtqlSchema'";
	if (model_id && model_id->L) {query+=" and ModelID='"; query+=*model_id; query+="'";}	
	ReferSet local_patterns;
	Models.executeSQL(&query, &local_patterns);

	HtmlQL ql;
	ql.setSourceData(page->P, page->L, false);
	ql.setQuery(prefix.P);
	double score=0.5;
	for (local_patterns.moveFirst(); !local_patterns.isEOF(); local_patterns.moveNext()){
		ReferData* tuple=local_patterns.getTuple();
		ql.setQuery(prefix.P);
		ql.dotQuery(tuple[0].P);
		if (!ql.isEOF()) score=1;//now simplest set the highest score, improve later
	}

	return score;
}
double HtWrapperModels::validateModelTagPattern(ReferData*page, ReferData* htql, ReferData* model_id){
	HtmlQL ql;
	ql.setSourceData(page->P, page->L, false);
	ql.setQuery(htql->P);

	if (ql.isEOF()) return 0;
	ReferData pattern;
	getConsensusTagPattern(&ql, &pattern);

	ReferData query; query="select Pattern where ModelType='Local' and PatternType='TagPattern'";
	if (model_id && model_id->L) {query+=" and ModelID='"; query+=*model_id; query+="'";}
	return scoreConsensusPattern(&pattern, query.P);
}
double HtWrapperModels::validateModelTextPattern(ReferData*page, ReferData* htql, ReferData* model_id){
	HtmlQL ql;
	ql.setSourceData(page->P, page->L, false);
	ql.setQuery(htql->P);

	if (ql.isEOF()) return 0;
	ReferData pattern;
	getConsensusTextPattern(&ql, &pattern);

	ReferData query; query="select Pattern where ModelType='Local' and PatternType='TextPattern'";
	if (model_id && model_id->L) {query+=" and ModelID='"; query+=*model_id; query+="'";}
	return scoreConsensusPattern(&pattern, query.P);
}
double HtWrapperModels::validateModelText(ReferData*page, ReferData* htql, ReferData* model_id){
	HtmlQL ql;
	ql.setSourceData(page->P, page->L, false);
	ql.setQuery(htql->P);

	if (ql.isEOF()) return 0;
	ReferData pattern;
	getConsensusText(&ql, &pattern);

	ReferData query; query="select Pattern where ModelType='Local' and PatternType='Text'";
	if (model_id && model_id->L) {query+=" and ModelID='"; query+=*model_id; query+="'";}
	return scoreConsensusPattern(&pattern, query.P);
}


double HtWrapperModels::scoreConsensusPattern(ReferData* pattern, const char* pattern_select_sql){
	//compare pattern to local model patterns
	ReferSet local_patterns;
	Models.executeSQL(pattern_select_sql, &local_patterns);

	HTQL ql1, ql2;
	ql1.setSourceData(pattern->P, pattern->L, false);
	ql1.setQuery("<ConsensusPatterns>.<ConsensusPattern>:tx");
	long npattern=ql1.getTuplesCount();
	double* scores=new double[npattern]; 
	long min_npattern=100; //the shortest model pattern

	StrAlignment align;
	long i;
	double score;
	for (local_patterns.moveFirst(); !local_patterns.isEOF(); local_patterns.moveNext()){
		ReferData* tuple=local_patterns.getTuple();
		ql2.setSourceData(tuple[0].P, tuple[0].L, false);
		ql2.setQuery("<ConsensusPatterns>.<ConsensusPattern>:tx");
		ql1.moveFirst();
		//calculate score for each model pattern
		i=0;
		for (ql2.moveFirst(); !ql2.isEOF(); ql2.moveNext()){
			double cost=0;
			long len=0;
			score=0;
			if (!ql1.isEOF()){
				char* p1=ql1.getValue(1);
				char* p2=ql2.getValue(1);
				align.CompareStrings(p1, p2, &cost, &len);
				score=1-(double) cost/(len*2);
			}

			if (i<npattern && scores[i]<score) scores[i]=score; //use the higher score

			ql1.moveNext(); 
			i++;
		}

		if (i<min_npattern) min_npattern=i; 
	}

	score=1;
	for (i=0; i<npattern; i++){
		if (scores[i]<score) score=scores[i]; //use the lower overall score
	}
	if (min_npattern>npattern)
		score=score*npattern/min_npattern;

	if (scores) delete[] scores;
	return score;
}

int HtWrapperModels::getConsensusTagPattern(HTQL* ql, ReferData* pattern){
	int fields=ql->getFieldsCount();
	*pattern="<ConsensusPatterns>";
	ReferData str;
	double cost;
	int max_comp_tuple=20;
	int max_comp_len=10;
	char buf[256];
	for (int i=1; i<=fields; i++){
		HtWrapper::computeFieldHyperConStr(ql, i, &str, &cost, max_comp_len, max_comp_tuple);
		sprintf(buf, "\n\t<ConsensusPattern Cost=\"%lf\">", cost);
		*pattern += buf;
		*pattern += str;
		*pattern += "</ConsensusPattern>";
	}
	*pattern+="\n</ConsensusPatterns>";
	return 0;
}
int HtWrapperModels::getConsensusTextPattern(HTQL* ql, ReferData* pattern){
	int fields=ql->getFieldsCount();
	*pattern="<ConsensusPatterns>";
	ReferData str;
	double cost;
	int max_comp_tuple=20;
	long max_comp_len=10;
	char buf[256];
	for (int i=1; i<=fields; i++){
		HtWrapper::computeFieldTextConStr(ql, i, &str, &cost, max_comp_len, max_comp_tuple);
		sprintf(buf, "\n\t<ConsensusPattern Cost=\"%lf\">", cost);
		*pattern += buf;
		*pattern += str;
		*pattern += "</ConsensusPattern>";
	}
	*pattern+="\n</ConsensusPatterns>";
	return 0;
}
int HtWrapperModels::getConsensusText(HTQL* ql, ReferData* pattern){
	int fields=ql->getFieldsCount();
	*pattern="<ConsensusPatterns>";
	ReferData str;
	int max_comp_tuple=20;
	int max_comp_len=10;
	char buf[256];
	HtmlQL ql1;
	for (int i=1; i<=fields; i++){
		sprintf(buf, "\n\t<ConsensusPattern Cost=\"%lf\">", 1);
		*pattern += buf;
		str=ql->getValue(i);
		ql1.setSourceData(str.P, str.L, false);
		ql1.setQuery("&txstr(100)");
		*pattern += ql1.getValue(i);
		*pattern += "</ConsensusPattern>";
	}
	*pattern+="\n</ConsensusPatterns>";
	return 0;
}
int HtWrapperModels::relocateWithModel(ReferData* page, ReferData* htql, ReferData* results){
	ReferLinkHeap pos_results;
	pos_results.setSortOrder(SORT_ORDER_NUM_INC);
	pos_results.setDuplication(true);
	ReferLink* link;

	//get the best matching position from different models
	ReferData positions;
	long count=locateModelPattern(page, HtWrapper::buildHyperConStr, "select Pattern where ModelType='Local' and PatternType='TagPattern'", &positions);
	if (count>0) {
		link=pos_results.add(0, &positions, count);
		link->Name="TagPattern";
	}
	positions.reset();
	count=locateModelPattern(page, HtWrapper::buildTextConStr, "select Pattern where ModelType='Local' and PatternType='TextPattern'", &positions);
	if (count>0) {
		link=pos_results.add(0, &positions, count);
		link->Name="TextPattern";
	}
	positions.reset();
	count=locateModelPattern(page, HtWrapper::buildPlainConStr, "select Pattern where ModelType='Local' and PatternType='Text'", &positions);
	if (count>0) {
		link=pos_results.add(0, &positions, count);
		link->Name="Text";
	}

	//rank the scores for each matching position
	HtmlQL ql;
	count=0;
	ReferSet ranking;
	const char* fields[]={"source_pos", "score", "pattern_type", "htql", 0};
	ranking.setFieldsNum(4, fields);
	ranking.newIndexField(0, ReferSetBTree::FLAG_REFER_L);
	ReferData* tuple, *query;
	double score;

	for (link=(ReferLink*) pos_results.moveFirst(); link; link=(ReferLink*) pos_results.moveNext()){
		ql.setSourceData(link->Value.P, link->Value.L, false);
		ql.setQuery("<max_positions>.<position>:source_pos, str_pos, cost, from_pos");

		long pos_count=ql.getTuplesCount();//the more pos_count, the less score it will contribute to the position

		for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
			query=ranking.getQueryTuple(0);
			query[0].L=atoi(ql.getValue("source_pos")); 

			tuple=ranking.moveFirst(query);
			if (!tuple){
				tuple=ranking.newTuple();
				tuple[0].L=query[0].L;	//ranking[0]: source_pos
				tuple[2]=link->Name;	//ranking[2]: pattern_type
			}
			score=tuple[1].getDouble();	
			//each matching position contribute 1/pos_count scores
			tuple[1].setDouble(score+((double)1)/pos_count);	//ranking[1]: score
		}
		count++;
		if (count>=2) break; //use 2 best models
	}

	//modify scores by their neighboring scores
	ReferData* last_tuple=0;
	double last_score=0, newscore=0;

	for (ranking.moveFirst(); !ranking.isEOF(); ranking.moveNext()){
		tuple=ranking.getTuple();
		score=tuple[1].getDouble();
		if (last_tuple){
			newscore=exp(log(score)-(tuple[0].L-last_tuple[0].L)*log(1.1)); //score/(1.1)^distance
			last_tuple[1].setDouble(last_tuple[1].getDouble()+newscore);

			newscore=exp(log(last_score)-(tuple[0].L-last_tuple[0].L)*log(1.1)); //last_score/(1.1)^distance
			tuple[1].setDouble(tuple[1].getDouble()+newscore);
		}

		last_tuple=tuple;
		last_score=score;
	}

	//sort ranking by scores
	ranking.newIndexField(1, ReferSetBTree::FLAG_FLOAT|ReferSetBTree::FLAG_DESCENDING);

	//reconstruct htql, get the last tag and the tail
	ReferData last_tag;
	ReferData tail;
	HtqlExpressionSyntax expr;
	expr.setSentence(htql->P, &htql->L, false);
	char* p; 
	unsigned int len;
	int is_last=0;
	while (expr.Type != QLSyntax::synQL_END){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG){
			p=TagOperation::getLabel(expr.Sentence+expr.Start, &len);
			last_tag.Set(p, len,true);
			is_last=1;
		}else if ( expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG 
			|| expr.Type == HtqlExpressionSyntax::synHTEXP_SCHEMA
			|| expr.Type == HtqlExpressionSyntax::synHTEXP_REDUCTION
			|| expr.Type == HtqlExpressionSyntax::synHTEXP_FUNCTION
			) {
			if (is_last) tail=expr.Sentence+expr.Start;
			is_last=false;
		}
		expr.match();
	}

	//find htql from each ranked positions
	ql.setSourceData(page->P, page->L, false);
	char buf[128];

	//construct results
	*results+="<position_htqls>";
	count=0;
	for (ranking.moveFirst(); !ranking.isEOF(); ranking.moveNext()){
		tuple=ranking.getTuple();

		//find htql from each ranked positions
		sprintf(buf, "%ld", tuple[0].L);
		ql.setGlobalVariable("off", buf);
		if (last_tag.L){
			ql.setGlobalVariable("tag", last_tag.P);
			ql.setQuery("&offset(off) &tag_parent(tag) ");
			p=ql.getValue(1);
			if (!p || !p[0]) continue;
			ql.dotQuery("&find_htql ./'\n'/");
		}else{
			ql.setQuery("&offset(off) &find_htql ./'\n'/");
		}
		//put found htql in tuple[3]
		tuple[3]=ql.getValue(1);
		if (tail.L) tuple[3]+=tail;

		//construct results
		sprintf(buf, "<position_htql source_pos=\"%ld\" score=\"%s\">", 
			tuple[0].L, tuple[1].P);
		*results+=buf;
		*results+=tuple[3];
		*results+="</position_htql>\n";

		if (++count>=10) break;
	}
	*results+= "</position_htqls>";

	return 0;
}

long HtWrapperModels::locateModelPattern(ReferData* page, int (*constr_fun)(char*, char*, long*, long), const char* model_sql, ReferData* results){
	//convert page into tag patterns
	long page_len=strlen(page->P);
	ReferData page_pattern;
	page_pattern.Malloc(sizeof(char)*(page_len+1));
	long* page_pos=(long*)malloc(sizeof(long)*(page_len+1));
	memset(page_pos, 0, sizeof(long)*(page_len+1));
	//HtWrapper::buildHyperConStr(page->P, page_pattern.P, page_pos, page_len);
	(*constr_fun)(page->P, page_pattern.P, page_pos, page_len);
	page_pattern.L=strlen(page_pattern.P);
	
	//local alignment of model pattern to the page pattern
	HyperTagsAlignment align;
	align.setAlignType(HyperTagsAlignment::TYPE_CHAR);
	align.setSouceData(&page_pattern);
	
	ReferSet local_patterns;
	ReferData* tuple;
	HtmlQL patternql;
	ReferData pattern_results;
	//Models.executeSQL("select Pattern where ModelType='Local' and PatternType='TagPattern'", &local_patterns, 0);
	Models.executeSQL(model_sql, &local_patterns, 0);
	for (local_patterns.moveFirst(); !local_patterns.isEOF(); local_patterns.moveNext()){
		tuple=local_patterns.getTuple();
		patternql.setSourceData(tuple[0].P, tuple[0].L,false);
		patternql.setQuery("<ConsensusPatterns>.<ConsensusPattern>:tx");
		int fields=patternql.getTuplesCount();
		ReferData record_pattern;
		for (patternql.moveFirst();  !patternql.isEOF(); patternql.moveNext()){
			if (record_pattern.L) record_pattern+="^"; //concatenate pattern fields by a ^ symbol
			record_pattern+=patternql.getValue(1);
		}

		ReferData best_pos;
		align.setStrData(&record_pattern);
		align.alignHyperTagsText(&best_pos);
		pattern_results+=best_pos;
	}
	
	patternql.setSourceData(pattern_results.P, pattern_results.L, false);
	patternql.setQuery("<max_positions>.<position>:source_pos, str_pos, cost, from_pos");
	*results+="<max_positions>";
	char buf[128];
	long count=0;
	for (patternql.moveFirst(); !patternql.isEOF(); patternql.moveNext()){
		long pos=atoi(patternql.getValue(1));
		sprintf(buf, "<position source_pos=\"%ld\" str_pos=\"%s\" cost=\"%s\" from_pos=\"%s\" />", 
			page_pos[pos], patternql.getValue(2), patternql.getValue(3), patternql.getValue(4));
		*results+=buf;
		count++;
	}
	*results+= "</max_positions>";

	free(page_pos);
	return count;
}


int HtWrapperModels::createGlobalModelFromLocal(ReferData* model_id){
	tExprCalc context;
	ReferSet prefix_rank; //Pattern, PatternCount
	ReferData* page_tuple=Pages.findFieldString("ModelID", model_id);

	//determine global HtqlPrefix
	Models.useIndexName("ModelPattern");
	if (page_tuple && page_tuple[ID_PAGE_SOURCE].L){
		//rank all HtqlPrefix by their frequency
		//just count the number it can wrap now ...
		context.setVariable("source", page_tuple[ID_PAGE_SOURCE].P, page_tuple[ID_PAGE_SOURCE].L, false);
		context.setVariable("url", page_tuple[ID_PAGE_URL].P, page_tuple[ID_PAGE_URL].L, false);
		Models.executeSQL("select Pattern, count(Pattern) as PatternCount where ModelType='Local' and PatternType='HtqlPrefix' "
			"and htql(source, url, Pattern) is not null group by Pattern order by PatternCount desc integer"
			, &prefix_rank, &context);
	}else{
		//merge the most common HtqlPrefix into global
		Models.executeSQL("select Pattern, count(Pattern) as PatternCount where ModelType='Local' and PatternType='HtqlPrefix' "
			"group by Pattern order by PatternCount desc integer"
			, &prefix_rank, &context);
	}
	//drop existing HtqlPrefix first
	Models.executeSQL("delete where ModelType='Global' and PatternType='HtqlPrefix'");

	//insert new global HtqlPrefix
	long count=0;
	ReferData best_prefix;
	ReferData* pattern_tuple=0;
	for (prefix_rank.moveFirst(); !prefix_rank.isEOF(); prefix_rank.moveNext()){
		pattern_tuple=prefix_rank.getTuple();

		ReferData* tuple=Models.newTuple();
		tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
		tuple[ID_MODEL_TYPE]="Global";
		tuple[ID_PATTERN_TYPE]="HtqlPrefix";
		tuple[ID_PATTERN]=pattern_tuple[0];

		if (count==0) best_prefix=tuple[ID_PATTERN];

		if (++count>=10) break;
	}

	//===============================
	//determine global HtqlSchema
	ReferSet schema_rank; //Pattern, PatternCount
	Models.useIndexName("ModelPattern");
	if (page_tuple && page_tuple[ID_PAGE_SOURCE].L){
		//rank all HtqlSchema that have the best match in the page
		//just count the number it appears for now ...
		context.setVariable("source", page_tuple[ID_PAGE_SOURCE].P, page_tuple[ID_PAGE_SOURCE].L, false);
		context.setVariable("url", page_tuple[ID_PAGE_URL].P, page_tuple[ID_PAGE_URL].L, false);
		context.setVariable("prefix", best_prefix.P, best_prefix.L, false);
		Models.executeSQL("select Pattern, count(Pattern) as PatternCount where ModelType='Local' and PatternType='HtqlSchema' "
			"and htql(source, url, prefix+Pattern) is not null group by Pattern order by PatternCount desc integer"
			, &schema_rank, &context);
	}else{
		//merge the most common HtqlSchema into global
		Models.executeSQL("select Pattern, count(Pattern) as PatternCount where ModelType='Local' and PatternType='HtqlSchema' "
			"group by Pattern order by PatternCount desc integer"
			, &prefix_rank, &context);
	}
	//drop existing HtqlSchema first
	Models.executeSQL("delete where ModelType='Global' and PatternType='HtqlSchema'");

	//get the top schema
	ReferData best_schema;
	pattern_tuple=schema_rank.moveFirst();
	if (pattern_tuple){
		best_schema=pattern_tuple[0];

		ReferData* tuple=Models.newTuple();
		tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
		tuple[ID_MODEL_TYPE]="Global";
		tuple[ID_PATTERN_TYPE]="HtqlSchema";
		tuple[ID_PATTERN]=best_schema.P;
	}


	//global SelectedHTQL as the top counted HtqlPrefix plus HtqlSchema
	if (best_prefix.L && best_schema.L){
		//drop existing HtqlSchema first
		Models.executeSQL("delete where ModelType='Global' and PatternType='SelectedHTQL'");

		//insert a new HtqlSchema
		ReferData* tuple=Models.newTuple();
		tuple[ID_MODEL_ID].Set(model_id->P, model_id->L,true);
		tuple[ID_MODEL_TYPE]="Global";
		tuple[ID_PATTERN_TYPE]="SelectedHTQL";
		tuple[ID_PATTERN]=best_prefix;
		tuple[ID_PATTERN]+=best_schema;
	}

	return 0;
}













