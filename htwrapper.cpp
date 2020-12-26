#include "htwrapper.h"
#include "qhtql.h"
#include "qhtmlql.h"
#include "alignment.h"
#include "htqlexpr.h"

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>



#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class HtWrapperTextConSyntax: public QLSyntax {
public:
	enum {synQL_KEY =100 , synQL_CURRENCY, synQL_DATE, synQL_TIME, synQL_SYMBOL };
	virtual void findNext();
};

void HtWrapperTextConSyntax::findNext(){
	if (Next>=Data.L){
		NextType = synQL_END;
		NextLen = 0;

	}else if (isAlpha(Sentence[Next]) ){
		while (Next+NextLen < Data.L && (isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen]))){
			NextLen++;
		}
		NextType=synQL_WORD;

		int i;
		for (i=0; i<NextLen; i++){
			if (!isupper(Sentence[Next+i])&&!isdigit(Sentence[Next+i])&&Sentence[Next+i]!='_') break;
		}
		if (i==NextLen) NextType=synQL_KEY;

	}else if (isDigit(Sentence[Next]) ){
		while (Next+NextLen < Data.L && (isDigit(Sentence[Next+NextLen]) || Sentence[Next+NextLen]=='.' )){
			NextLen++;
		}
		NextType=synQL_NUMBER;

		int i;
		if (Next+NextLen < Data.L && Sentence[Next+NextLen] == '/' && isdigit(Sentence[Next+NextLen+1]) ){
			i=2;
			while (Next+NextLen+i < Data.L && isdigit(Sentence[Next+NextLen+i]) ) i++;
			if (Next+NextLen+i < Data.L && Sentence[Next+NextLen+i] == '/' && isdigit(Sentence[Next+NextLen+i+1])){
				NextLen+=i+2;
				while (Next+NextLen < Data.L && isdigit(Sentence[Next+NextLen])) NextLen++;
			}
			NextType=synQL_DATE;
		}else if (Next+NextLen < Data.L && Sentence[Next+NextLen] == ':' && isdigit(Sentence[Next+NextLen+1]) ){
			int hour=0, min=0, sec=0;
			sscanf(Sentence+Next, "%d:%d", &hour, &min);
			if (hour<24 && min<60){
				NextLen +=2;
				while (Next+NextLen < Data.L && isdigit(Sentence[Next+NextLen]) ) NextLen++;
				if (Next+NextLen < Data.L && Sentence[Next+NextLen] == ':' && isdigit(Sentence[Next+NextLen+1])){
					NextLen+=2;
					while (Next+NextLen < Data.L && isdigit(Sentence[Next+NextLen])) NextLen++;
				}
				NextType=synQL_TIME;
			}
		}

	}else if (Sentence[Next] == '$' && isdigit(Sentence[Next+1]) ){
		while (Next+NextLen < Data.L && (isDigit(Sentence[Next+NextLen]) || Sentence[Next+NextLen]=='.' )){
			NextLen++;
		}
		NextType=synQL_CURRENCY;

	}else {
		NextType=synQL_SYMBOL;
	}

}


//================================================================


HtWrapper::HtWrapper(){
	IsWrapperAdjusted = 0;
	IsAdjustedByCandidateHtql=0;
	IsAdjustedByCandidatePattern=0;
	ValidationThreshold=0.8;
	ValidationFieldsSimil=0;
	ValidationFieldsNum=0;
	ValidationAllSimil=0.0;
	newWrapper("test");
}
HtWrapper::~HtWrapper(){
	reset();
}
void HtWrapper::reset(){
	WrapperName.reset();
	WrapperFileName.reset();
	Source.reset();
	newWrapper("test");
	IsWrapperAdjusted = 0;
	IsAdjustedByCandidateHtql=0;
	IsAdjustedByCandidatePattern=0;
	if (ValidationFieldsSimil) {
		delete[] ValidationFieldsSimil;
		ValidationFieldsSimil=0;
	}
	ValidationFieldsNum=0;
	ValidationAllSimil=0.0;
}

int HtWrapper::emptyWrapper(){
	Wrapper = "<Wrappers>\n</Wrappers>";
	return 0;
}

int HtWrapper::newWrapper(const char* wrapper_name){
	if (wrapper_name) WrapperName = wrapper_name;
	ReferData empty_wrapper;
	empty_wrapper = "<Wrapper Name=\"";
	empty_wrapper += WrapperName;
	empty_wrapper += "\" ViewType=\"Item";
	empty_wrapper += "\">\n\t<WrapperHtql Htql=\"\" Candidate=\"\">\n\t</WrapperHtql>\n";
	empty_wrapper += "\t<CandidateHtqls>\n\t</CandidateHtqls>\n";
	empty_wrapper += "\t<CandidatePatterns>\n\t</CandidatePatterns>\n";
	empty_wrapper += "</Wrapper>\n";
	
	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L);
	ql.setGlobalVariable("n", WrapperName.P);
	ql.setQuery("<Wrapper (Name=n)>");
	if (!ql.isEOF()){
		ql.setGlobalVariable("a", empty_wrapper.P);
		ql.setQuery("<Wrapper (Name=n)> &replace(a)");
	}else{
		ql.setQuery("<Wrappers>:tx");
		if (ql.isEOF()){
			Wrapper += empty_wrapper;
			return 0;
		}else{
			ql.setGlobalVariable("a", empty_wrapper.P);
			ql.setQuery("<Wrappers>:tx &insert_after(a)");
		}
	}
	Wrapper = ql.Parser->SourceData;
	return 0;
}

int HtWrapper::setSourceData(char* source, long len, int copy){
	if (Source.setSourceData(source, len, copy)) return 0;
	return 1;
}
int HtWrapper::setSourceUrl(char* url){
	if (Source.setSourceUrl(url, 0)) return 0;
	return 1;
}

int HtWrapper::setSource(HTQL* htql, int copy){
	if (!htql->Parser->SourceData.isNULL()){
		Source.setSourceData(htql->Parser->SourceData.P, htql->Parser->SourceData.L, copy);
		if (!htql->Parser->SourceUrl.isNULL() ) 
			Source.setSourceUrl(htql->Parser->SourceUrl.P, htql->Parser->SourceUrl.L); 
		return 0;
	}else
		return 1;
}

int HtWrapper::setWrapperByBestView(const char* wrapper_name, int check_fields){
	newWrapper(wrapper_name);
	Source.setQuery("&find_repeat_views(5).<find_repeat_views>.<find_view>:htql, candidate, view_items");
	if (!Source.isEOF()){
		char* htql = Source.getValue("htql");
		char* candidate = Source.getValue("candidate");
		int i=updateWrapperHtql("View", htql, candidate);
		if (i!=0) return i;
		if (check_fields){
			setWrapperConFields();
			removeTrivialConFields();
		}
	}
	return 0;
}

int HtWrapper::setWrapperByCandidateView(const char* candidate_htql, const char* wrapper_name, int check_fields){
	newWrapper(wrapper_name);
	Source.setQuery(candidate_htql);
	Source.dotQuery("&find_best_views(5).<find_best_views>.<find_view>:htql, candidate, view_items");
	if (!Source.isEOF()){
		char* htql = Source.getValue("htql");
		char* candidate = Source.getValue("candidate");
		int i=updateWrapperHtql("View", htql, candidate_htql);
		if (i!=0) return i;
		if (check_fields){
			setWrapperConFields();
			removeTrivialConFields();
		}
	}
	return 0;
}

int HtWrapper::setWrapperByTargetItem(const char* candidate_htql, const char* wrapper_name, int check_fields){
	newWrapper(wrapper_name);
	int i=updateWrapperHtql("Item", 0, candidate_htql);
	if (i!=0) return i;
	if (check_fields){
		setWrapperConFields();
		removeTrivialConFields();
	}
	return 0;
}

int HtWrapper::replaceWrapperHtql(const char* htql, const char* candidate){
	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L);
	ql.setGlobalVariable("n", WrapperName.P);

	if (htql){
		ql.setGlobalVariable("a", htql);
		ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:Htql &replace(a)");
	}
	if (candidate){
		ql.setGlobalVariable("a", candidate);
		ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:Candidate &replace(a)");
	}
	Wrapper = ql.Parser->SourceData;
	return 0;
}

int HtWrapper::updateWrapperHtql(const char* view_type, const char* htql, const char* candidate){
	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L);
	ql.setGlobalVariable("n", WrapperName.P);

	ql.setGlobalVariable("a", view_type);
	ql.setQuery("<Wrapper (Name=n)>:ViewType &replace(a)");
	if (htql){
		ql.setGlobalVariable("a", htql);
		ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:Htql &replace(a)");
	}
	ql.setGlobalVariable("a", candidate);
	ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:Candidate &replace(a)");

	ReferData query;
	ReferData val;
	ReferData str;
	query = candidate;
	char buf[128];
	sprintf(buf, " &find_htql ./'\\n'/", HtmlQL::heuBEST_VIEW);
	query += buf;
	Source.setQuery(query.P);
	int i;
	for (i=0; i<10 && !Source.isEOF(); i++){
		//get candidate paths
		val=Source.getValue(1);
		if (!val.isNULL() && val.P[0] ){
			str = "\t<CandidateHtql Htql=\"";
			str += val;
			str += "\"></CandidateHtql>\n\t";

			ql.setGlobalVariable("a", str.P);
			ql.setQuery("<Wrapper (Name=n) >.<CandidateHtqls>:tx &insert_after(a)");
		}
		Source.moveNext();
	}

	if (!strcmp(view_type, "View") && htql){
		//get repeat patterns
		HtmlQL ql1;
		char* p, *p1;
		unsigned int len;

		HtqlExpression expr;
		expr.setExpr((char*) htql);
		expr.setExpr(expr.getSchemaPrefix(1));
		int num = expr.getPatternsNum();
		query = expr.getTagSelectionHeadTo(num-1);
		query += "&repeat_patterns .<pattern>.<pattern_words>1";

		Source.setQuery(query.P);
		int i=0;
		for (Source.moveFirst(); !Source.isEOF() && i<10; Source.moveNext(), i++ ){
			str = Source.getValue(1);
			if (str.isNULL() || !str.P[0] ) continue;

			val = "\t<CandidatePattern Pattern=\"";
			ql1.setSourceData(str.P, str.L, false);
			ql1.setQuery("<word>:tx");
			for (ql1.moveFirst(); !ql1.isEOF(); ql1.moveNext()){
				p=ql1.getValue(1);
				if (!p || !p[0]) continue;
				p1=TagOperation::getLabel(p, &len);
				str.Set(p, (p1+len)-p);
				if (p1){
					val+=str; //<tag 
					val+=">";
				}
			}
			val+="\" ></CandidatePattern>\n\t";

			ql.setGlobalVariable("a", val.P);
			ql.setQuery("<Wrapper (Name=n) >.<CandidatePatterns>:tx &insert_after(a)");
		}
	}

	if (!htql) {
		Source.moveFirst();
		htql = Source.getValue(1);
		if (htql){
			ql.setGlobalVariable("a", htql);
			ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:Htql &replace(a)");
		}
	}

	Wrapper = ql.Parser->SourceData;
	wrapSourceData(0, false);

	return 0;
}

int HtWrapper::readWrapper(const char* filename){
	WrapperFileName = (char*) filename;
	return Wrapper.readFile(filename);
}
int HtWrapper::saveWrapper(const char* filename){
	if (filename && filename[0]){
		WrapperFileName = filename;
	}
	if (!WrapperFileName.isNULL()){
		return Wrapper.saveFile(WrapperFileName.P);
	}
	return 1;
}

HTQL* HtWrapper::wrapSourceData(const char* wrapper_name, int auto_adjust){
	if (wrapper_name) {
		WrapperName = wrapper_name;
	}
	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L);

	if (WrapperName.isNULL() || !WrapperName.P[0]){
		ql.setQuery("<Wrapper>1:Name");
		if (!ql.isEOF()){
			WrapperName = ql.getValue(1);
		}
	}
	if (WrapperName.isNULL()) return 0;

	ql.setGlobalVariable("n", WrapperName.P);
	ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:Htql");
	if (ql.isEOF()){
		return 0;
	}
	ReferData htql;
	htql = ql.getValue(1);

	int ret=Source.setQuery(htql.P);
	if (!auto_adjust){
		if (ret==0) return &Source;
		else return 0;
	}else{
		int invalid=0;
		if (validateWrapper(&Source) >ValidationThreshold) return &Source;

		IsWrapperAdjusted = false;
		IsAdjustedByCandidateHtql=false;
		IsAdjustedByCandidatePattern=false;
		WrapperBackup = Wrapper;

		timeb t0, t1;
		ftime(&t0);
		if (adjustWithCandidateHtqls() ){
			ftime(&t1);
			printf("==>Adjusted with Cadidate Htql time ** %i,%03i msec\n",
				t1.millitm>=t0.millitm?(t1.time-t0.time):(t1.time-t0.time-1), 
				t1.millitm>=t0.millitm?(t1.millitm-t0.millitm):(1000+t1.millitm-t0.millitm));
			IsAdjustedByCandidateHtql = true;
			//printf("Wrapper adjusted with candidate Htqls\n");
		}else if (adjustWithCandidatePatterns() ){
			ftime(&t1);
			printf("==>Adjusted with Cadidate Patterns time ## %i,%03i msec\n",
				t1.millitm>=t0.millitm?(t1.time-t0.time):(t1.time-t0.time-1), 
				t1.millitm>=t0.millitm?(t1.millitm-t0.millitm):(1000+t1.millitm-t0.millitm));
			IsAdjustedByCandidatePattern = true;
			//printf("Wrapper adjusted with candidate Patterns\n");
		}else{
			//printf("Wrapper adjustment fail!\n");
		}

		if (this->IsWrapperAdjusted) {
/*			wrapSourceData(0, false);
			setWrapperConFields();
			removeTrivialConFields();
			ReferData result_wrapper;
			consolidateConWrapper(&WrapperBackup, &Wrapper, WrapperName.P, &result_wrapper);
			Wrapper = result_wrapper;
			wrapSourceData(wrapper_name, false);
			IsWrapperAdjusted = true; */
			return &Source;
		}
	}

	return 0;
}

double HtWrapper::validateWrapper(HTQL* qresult, const char* wrapper_name, double con_threshold, int* invalid_fields){
	if (ValidationFieldsSimil) {
		delete[] ValidationFieldsSimil;
		ValidationFieldsSimil=0;
	}
	ValidationFieldsNum=0;
	if (qresult->isEOF()) return 0.0;

	if (wrapper_name) {
		WrapperName = wrapper_name;
	}
	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L);

	if (WrapperName.isNULL() || !WrapperName.P[0]){
		ql.setQuery("<Wrapper>1:Name");
		if (!ql.isEOF()){
			WrapperName = ql.getValue(1);
		}
	}
	if (WrapperName.isNULL()) return 0.0;

	ql.setGlobalVariable("n", WrapperName.P);
	ql.setQuery("<Wrapper (Name=n)>.<ConsensusInfo>.<WrapperField>"
			"{hp=<HyperConsensus>:ConStr; hpc=<HyperConsensus>:Cost; hps=<HyperConsensus>:Support;"
			"tp=<TextConsensus>:ConStr; hpc=<TextConsensus>:Cost; hps=<TextConsensus>:Support;}");
	if (ql.isEOF()){
		return 1.0;
	}

	int valid_fields=0;
	int fields=qresult->getFieldsCount();
	ReferData hyper_cons;
	ReferData text_cons;
	ReferData hp;
	ReferData tp;
	double hyper_cost;
	double text_cost;
	StrAlignment align;
	double cost1, cost2;
	double S;

	ValidationFieldsSimil = new double[fields];
	ValidationFieldsNum=fields;

	for (int i=1; i<=fields && !ql.isEOF() ; i++){
		hp = ql.getValue("hp");
		tp = ql.getValue("tp");
		char* p=ql.getValue("hpc");
		if (p) sscanf(p, "%lf", &hyper_cost);
		else hyper_cost=0.0;
		p=ql.getValue("tpc");
		if (p) sscanf(p, "%lf", &text_cost);
		else text_cost=0.0;

		computeFieldHyperConStr(qresult, i, &hyper_cons, &hyper_cost, 10, 10);
		computeFieldTextConStr(qresult, i, &text_cons, &text_cost, 10, 10);
		
		align.CompareStrings(hyper_cons.P, hp.P, &cost1);
		align.CompareStrings(text_cons.P, tp.P, &cost2);
		if (hp.L+hyper_cons.L==0) S=1;
		else S= (1-cost1/(double)(hp.L+hyper_cons.L));
		if (tp.L+text_cons.L) S*=(1-cost2/(double)(tp.L+text_cons.L));
		//S= (1-cost1/(double)(hp.L+hyper_cons.L)) * (1-cost2/(double)(tp.L+text_cons.L));
		if (S>con_threshold){
			valid_fields++;
		}
		ql.moveNext();
		ValidationFieldsSimil[i-1]=S;
//		printf("%4.1lf", S);
	}

	int fields1=ql.getTuplesCount();
	if (fields1>fields) fields=fields1;
	S = valid_fields/(double)fields;
	if (invalid_fields){
		*invalid_fields = fields-valid_fields;
	}
	ValidationAllSimil=S;
//	printf("*V - %4.1lf\n", S);
	return S;
}

int HtWrapper::adjustWithCandidateHtqls(){
	IsWrapperAdjusted = false;

	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L, true);
	ql.setGlobalVariable("n", WrapperName.P);

	int view_type = 0;
	ql.setQuery("<Wrapper (Name=n)>: ViewType");
	if (!ql.isEOF()){
		char* p=ql.getValue(1);
		if (p && !strcmp(p, "View"))
			view_type =1;
	}

	ql.setQuery("<Wrapper (Name=n)>.<CandidateHtqls>.<CandidateHtql>:Htql");
	ReferLinkHeap candidates;
	candidates.setSortOrder(SORT_ORDER_NUM_INC);
	int i=0;
	ReferData key;
	while (!ql.isEOF()){
		key=ql.getValue(1);
		candidates.add(&key, 0, i++);
		//candidates.set(i++, ql.getValue(1), "");
		ql.moveNext();
	}
	
	ReferData candidate;
	ReferData htql;
	ReferData old_wrapper;
	old_wrapper=Wrapper;

	double S;
	BestWrapperAdjustedScore=0;
	BestWrapperAdjusted.reset();

	if (view_type ==0){//Item
		for (ReferLink* ts=(ReferLink*) candidates.moveFirst(); ts; ts=(ReferLink*) candidates.moveNext() ){
			if (!ts->Name.L) continue;
			Source.setQuery(ts->Name.P);
			if (validateWrapper(&Source) >ValidationThreshold) {
				candidate = ts->Name;
				break;
			}
		}
	}else{//View
		ReferLinkHeap rank_view;
		HtmlQLParser::rankPathHtqlsToViews(&Source, &candidates, &rank_view, 5, 10);
		int view_can=0;
		for (ReferLink* ts=(ReferLink*) rank_view.moveFirst(); ts; ts=(ReferLink*) rank_view.moveNext() ){
			if (++view_can>5) break;
			if (!ts->Name.L) continue;
			Source.setQuery(ts->Name.P);
			if (validateWrapper(&Source) >ValidationThreshold) {
				IsWrapperAdjusted=true;
				replaceWrapperHtql(ts->Name.P, ts->Value.P);
				return IsWrapperAdjusted;
			}else if (!Source.isEOF()){
				replaceWrapperHtql(ts->Name.P, ts->Value.P);
				setWrapperConFields(10);
				removeTrivialConFields();
				ReferData result_wrapper;
				consolidateConWrapper(&old_wrapper, &Wrapper, WrapperName.P, &result_wrapper);
				Wrapper = result_wrapper;
				wrapSourceData(0, false);
				if ((S=validateWrapper(&Source)) > ValidationThreshold){
					IsWrapperAdjusted=true;
					return IsWrapperAdjusted;
				}else{
					if (S>BestWrapperAdjustedScore) {
						BestWrapperAdjustedScore = S;
						BestWrapperAdjusted=Wrapper;
					}
					Wrapper = old_wrapper;
				}
			}
		}
	}
	
	return IsWrapperAdjusted;
}

int HtWrapper::adjustWithCandidatePatterns(){
	IsWrapperAdjusted = false;
//printf("adjusting wrapper with candidate pattern...\n");
	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L, true);
	ql.setGlobalVariable("n", WrapperName.P);

	ReferData candidate;
	ReferData htql;
	ReferData old_wrapper;
	old_wrapper=Wrapper;

	char* p;
	int view_type = 0;
	ql.setQuery("<Wrapper (Name=n)>{ViewType=:ViewType; Htql=<WrapperHtql>:Htql } ");
	if (!ql.isEOF()){
		p=ql.getValue(1);
		if (p && !strcmp(p, "View"))
			view_type =1;
		htql = ql.getValue(2);
	}

	if (view_type==0) //item 
		return 0;

	HtqlExpression expr;
	expr.setExpr(htql.P, htql.L);
	int fields=expr.getSchemaFieldsNum(1);
	ql.setQuery("<Wrapper (Name=n)>.<CandidatePatterns>.<CandidatePattern>:Pattern");
	ReferData str;
	ReferData results;
	HtmlQL ql1;
	int i=0;
	double S;
	BestWrapperAdjustedScore=0;
	BestWrapperAdjusted.reset();
	char buf[256];
	int pat_can=0;

	HyperTagsAlignment hyperalign;
	hyperalign.setSouceData(&Source.Parser->SourceData);

	for (ql.moveFirst();  !ql.isEOF(); ql.moveNext() ){
		if (++pat_can >2) break;
		str = ql.getValue(1);
//printf("adjusting wrapper with candidate pattern...\n%s\n", str.P);

		hyperalign.setStrData(&str);
		hyperalign.alignHyperTagsText(&results);
		//HtmlQLParser::alignHyperTagsText(&Source.Parser->SourceData, &str, &results);

		ql1.setSourceData(results.P, results.L, false);
		ql1.setQuery("<max_positions>.<position>: source_pos, from_pos");
		htql.reset();
		candidate.reset();
		for (i=0; i<3 && !ql1.isEOF(); i++, ql1.moveNext() ){ //only get the first i positions 
			for (int k=1; k<=2; k++){
				p=ql1.getValue(k); //source_pos, from_pos
				sprintf(buf, "&offset(%s) &find_best_views(5,10) .<find_view>:htql, candidate, view_items", p);
				Source.setQuery(buf);
				if (!Source.isEOF()){
					htql = Source.getValue(1);
					candidate = Source.getValue(2);
				}

				Source.setQuery(htql.P);
				if (validateWrapper(&Source) >ValidationThreshold) {
					IsWrapperAdjusted=true;
					replaceWrapperHtql(htql.P, candidate.P);
					return IsWrapperAdjusted;
				}else if (!Source.isEOF()){
					replaceWrapperHtql(htql.P, candidate.P);
					setWrapperConFields(10);
					removeTrivialConFields();
					ReferData result_wrapper;
					consolidateConWrapper(&old_wrapper, &Wrapper, WrapperName.P, &result_wrapper);
					Wrapper = result_wrapper;
					wrapSourceData(0, false);
					if ((S=validateWrapper(&Source)) > ValidationThreshold){
						IsWrapperAdjusted=true;
						return IsWrapperAdjusted;
					}else{
						if (S>BestWrapperAdjustedScore) {
							BestWrapperAdjustedScore = S;
							BestWrapperAdjusted=Wrapper;
						}
						Wrapper = old_wrapper;
					}
				}
			}
		}
		//if (!Source.isEOF() && Source.getFieldsCount() >=fields) break;
	}

	return IsWrapperAdjusted;
}

int HtWrapper::removeTrivialConFields(){
	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L);
	ql.setGlobalVariable("n", WrapperName.P);

	ql.setQuery("<Wrapper (Name=n)>.<ConsensusInfo>.<WrapperField> {max=<LengthConsensus>:MaxLen; hyper=<HyperConsensus>:ConStr; text=<TextConsensus>:ConStr} ");

	long fields=ql.getTuplesCount();
	int* to_delete= new int[fields];
	
	int i=0;
	while (!ql.isEOF()){
		char* max=ql.getValue(1);
		char* hyper1=ql.getValue(2);
		char* text=ql.getValue(3);
		int trivial=false;
		if ((max&&!max[0]) || (hyper1&&!hyper1[0]) ) trivial=true;
		else if (text&&!text[0] && (!hyper1 || !strchr(hyper1,dhyconS_TAG_IMAGE)) ) trivial=true;
		
		to_delete[i]=trivial;
		if (++i>=fields) break;
		ql.moveNext();
	}

	ReferData expr;
	ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:Htql");
	if (!ql.isEOF()){
		expr = ql.getValue(1);
	}
	HtqlExpression htexpr;
	htexpr.setExpr(expr.P, expr.L);

	char buf[256];
	for (i=fields; i>=1; i--) if (to_delete[i-1]) {
		expr = htexpr.deleteSchemaField(1, i);
		htexpr.setExpr(expr.P, expr.L);
		sprintf(buf, "<Wrapper (Name=n)>.<ConsensusInfo>.<WrapperField>%i &delete", i);
		ql.setQuery(buf);
	}
	ql.setGlobalVariable("a", expr.P);
	ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:Htql &replace(a)");

	Wrapper = ql.Parser->SourceData;
	delete [] to_delete;

	Source.setQuery(expr.P);
	return 0;
}

int HtWrapper::consolidateConWrapper(ReferData* original_wrapper, ReferData* current_wrapper, char* wrapper_name, ReferData* result_wrapper, double** evals, int* fieldsnum){
	HtmlQL ql0, ql1;
	original_wrapper->saveFile("c:\\w1.txt");
	current_wrapper->saveFile("c:\\w2.txt");
	ql0.setSourceData(original_wrapper->P, original_wrapper->L, false);
	ql1.setSourceData(current_wrapper->P, current_wrapper->L, false);

	ReferData name;
	char buf[256];
	if (wrapper_name){
		name="<Wrapper (name=n)>.";
		ql0.setGlobalVariable("n", wrapper_name);
		ql1.setGlobalVariable("n", wrapper_name);
	}else{
		name="<Wrapper>1.";
	}
	
	ReferData s0, s1;
	s0=name;
	s0+="<ConsensusInfo>.<WrapperField>{field=:FieldName; "
		"max=<LengthConsensus>:MaxLen; min=<LengthConsensus>:MinLen; "
		"support=<LengthConsensus>:Support;"
		"hycon=<HyperConsensus>:ConStr; hycost=<HyperConsensus>:Cost; "
		"txcon=<TextConsensus>:ConStr; txcost=<TextConsensus>:Cost;}";
	ql0.setQuery(s0.P);
	ql1.setQuery(s0.P);

	int fields1=ql0.getTuplesCount();
	int fields2=ql1.getTuplesCount();
	int* map12=new int[fields1];
	int* map21=new int[fields2];
	double* map1_cost=new double[fields1];
	int i, j;
	for (i=0; i<fields1; i++) map12[i]=0;	
	for (i=0; i<fields2; i++) map21[i]=0;
	for (i=0; i<fields1; i++) map1_cost[i]=0;	

	StrAlignment align;
	double cost1, cost2;
	for (i=0, ql0.moveFirst(); !ql0.isEOF(); ql0.moveNext(), i++){
		double max_score=0;
		int max_j=0;
		char* hycon0=ql0.getValue("hycon");
		char* txcon0=ql0.getValue("txcon");
		for (j=0, ql1.moveFirst(); !ql1.isEOF(); ql1.moveNext(), j++){
			if (map21[j]) continue;
			char* hycon1=ql1.getValue("hycon");
			char* txcon1=ql1.getValue("txcon");

			double score;
			if (!strcmp(hycon0, hycon1) && !strcmp(txcon0, txcon1)) {
				score=strlen(hycon0)+strlen(txcon0)+1.0;
			}else score=0;
			if (score-max_score>0.01){
				max_score=score;
				max_j=j;
				if (max_score >= 1.0) break;
			}
		}
		if (max_score >=1.0 ){
			map12[i] = max_j+1;
			map21[max_j] = i+1;
			map1_cost[i]=max_score;
		}
	}
	for (i=0, ql0.moveFirst(); !ql0.isEOF(); ql0.moveNext(), i++){
		if (map12[i]) continue;
		double max_score=0;
		int max_j=0;
		char* hycon0=ql0.getValue("hycon");
		char* txcon0=ql0.getValue("txcon");
		for (j=0, ql1.moveFirst(); !ql1.isEOF(); ql1.moveNext(), j++){
			if (map21[j]) continue;
			char* hycon1=ql1.getValue("hycon");
			char* txcon1=ql1.getValue("txcon");

			double score, score1, score2;
			if (!strcmp(hycon0, hycon1)) score1=1.0;
			else {
				align.CompareStrings(hycon0, hycon1, &cost1);
				score1=1-cost1/(double)(strlen(hycon0)+strlen(hycon1));
			}
			if (!strcmp(txcon0, txcon1)) score2=1.0;
			else {
				align.CompareStrings(txcon0, txcon1, &cost2);
				score2=1-cost2/(double)(strlen(txcon0)+strlen(txcon1));
			}
			score=score1*score2;
			if (score > max_score){
				max_score=score;
				max_j=j;
				if (max_score > 0.99) break;
			}
		}
		if (max_score > 0.3){
			map12[i] = max_j+1;
			map21[max_j] = i+1;
			map1_cost[i]=max_score;
		}
	}

	HtqlExpression expr0, expr1;
	sprintf(buf, "%s<WrapperHtql>:Htql", name.P);
	ql0.setQuery(buf);
	expr0.setExpr(ql0.getValue(1));
	ql1.setQuery(buf);
	expr1.setExpr(ql1.getValue(1));

	ReferData val;
	val=expr1.getSchemaPrefix(1);
	val += "{";
	for (i=0; i<fields1; i++){
		if (map12[i]){
			if (i) val+=";\n";
			val += expr0.getSchemaFieldName(1, i+1);
			val += "=";
			val += expr1.getSchemaFieldHtql(1, map12[i]);
		}else{
			if (i) val+=";\n";
			val += expr0.getSchemaField(1, i+1);
		}
	//	printf("%5.1lf", map1_cost[i]);
	}
	//printf("*C\n");

	long start, len;
	expr1.getSchema(1, &start, &len);
	val += "\n}";
	val += expr1.Expression.P + start+len;

	HtmlQL ql2;
	ql2.setSourceData(current_wrapper->P, current_wrapper->L, false);
	ql2.setGlobalVariable("n", wrapper_name);

	ql2.setGlobalVariable("a", val.P);
	sprintf(buf, "%s<WrapperHtql>:Htql &replace(a)", name.P);
	ql2.setQuery(buf);
	sprintf(buf, "%s<ConsensusInfo>:tx &delete", name.P);
	ql2.setQuery(buf);

	for (i=1; i<=fields1; i++){
		if (map12[i-1]){
			sprintf(buf, "%s<ConsensusInfo>.<WrapperField>%d", name.P, map12[i-1]);
			ql1.setQuery(buf);
			ql2.setGlobalVariable("a", ql1.getValue(1));
			sprintf(buf, "%s<ConsensusInfo>:tx &insert_after(a)", name.P);
			ql2.setQuery(buf);
			ql2.setGlobalVariable("a", expr0.getSchemaFieldName(1, i));
			sprintf(buf, "%s<ConsensusInfo>.<WrapperField>%d:FieldName &replace(a)", name.P, i);
			ql2.setQuery(buf);
		}else{
			sprintf(buf, "%s<ConsensusInfo>.<WrapperField>%d", name.P, i);
			ql0.setQuery(buf);
			ql2.setGlobalVariable("a", ql0.getValue(1));
			sprintf(buf, "%s<ConsensusInfo>:tx &insert_after(a)", name.P);
			ql2.setQuery(buf);
		}
	}
	*result_wrapper=ql2.Parser->SourceData;

	delete[] map12;
	delete[] map21;

	if (evals) *evals=map1_cost;
	else delete[] map1_cost;
	if (fieldsnum) *fieldsnum=fields1;
	return 0;
}

int HtWrapper::setWrapperConFields(int max_comp_tuple){
	HtmlQL ql;
	ql.setSourceData(Wrapper.P, Wrapper.L);
	ql.setGlobalVariable("n", WrapperName.P);

	int view_type = 0;
	ql.setQuery("<Wrapper (Name=n)>.<ConsensusInfo>");
	if (ql.isEOF()){
		ql.setGlobalVariable("a", "\n\t<ConsensusInfo></ConsensusInfo>");
		ql.setQuery("<Wrapper (Name=n)>.<WrapperHtql>:et &insert_after(a)");
	}

	int fields = Source.getFieldsCount();

	int i;
	ReferData coninfo;
	coninfo = "<ConsensusInfo>";
	ReferData field_info;
	ReferData str;
	long maxlen, minlen, averagelen, tuples;
	double cost;
	char buf[256];
	for (i=1; i<=fields; i++){
		sprintf(buf, "\n\t\t<WrapperField FieldName=\"");
		field_info = buf;
		field_info += Source.getFieldName(i);
		field_info += "\">";

		computeFieldLengthStatistic(&Source, i, &maxlen, &minlen, &averagelen, &tuples, max_comp_tuple);
		sprintf(buf, "\n\t\t\t<LengthConsensus MaxLen=\"%ld\" MinLen=\"%ld\" AverageLen=\"%d\" Support=\"%ld\"/>",
			maxlen, minlen, averagelen, tuples);
		field_info += buf;

		computeFieldHyperConStr(&Source, i, &str, &cost, 10, max_comp_tuple);
		sprintf(buf, "\n\t\t\t<HyperConsensus ConStr=\"%s\" Cost=\"%lf\" Support=\"%ld\"/>",
			str.P, cost, tuples);
		field_info += buf;

		computeFieldTextConStr(&Source, i, &str, &cost, 10, max_comp_tuple);
		sprintf(buf, "\n\t\t\t<TextConsensus ConStr=\"%s\" Cost=\"%lf\" Support=\"%ld\"/>",
			str.P, cost, tuples);
		field_info += buf;

		field_info +="\n\t\t</WrapperField>";

		coninfo += field_info;
	}
	coninfo += "\n\t</ConsensusInfo>";

	ql.setGlobalVariable("a", coninfo.P);
	ql.setQuery("<Wrapper (Name=n)>.<ConsensusInfo> &replace(a)");

	Wrapper = ql.Parser->SourceData.P;

	return 0;
}

int HtWrapper::computeFieldHyperConStr(HTQL* htql, int field_index1, ReferData* consen_str, double* cost, long max_comp_len, int max_comp_tuple){
	return computeFieldConStr(htql, field_index1, consen_str, cost, buildHyperConStr, max_comp_len, max_comp_tuple);
}
int HtWrapper::computeFieldTextConStr(HTQL* htql, int field_index1, ReferData* consen_str, double* cost, long max_comp_len, int max_comp_tuple){
	return computeFieldConStr(htql, field_index1, consen_str, cost, buildTextConStr, max_comp_len, max_comp_tuple);
}

int HtWrapper::computeFieldConStr(HTQL* ql, int field_index1, ReferData* consen_str, double* cost, int (*constr_fun)(char*, char*, long*, long), long max_comp_len, int max_comp_tuple){

	char** syntax_strs=0;
	long total_tuple = ql->getTuplesCount();
	if (max_comp_tuple && total_tuple > max_comp_tuple) total_tuple = max_comp_tuple;
	syntax_strs = (char**) malloc(sizeof(char*)*total_tuple);
	memset(syntax_strs, 0, sizeof(char*)*total_tuple);
	int i;
	for (i=0; i<total_tuple; i++){
		syntax_strs[i] = (char*) malloc(sizeof(char) * (max_comp_len+1));
		memset(syntax_strs[i], 0, sizeof(char) * (max_comp_len+1));
	}

	ql->moveFirst();
	int cur_tuple=0;
	while (!ql->isEOF()){
		char* p=ql->getValue(field_index1);

		if (p) {
			(*constr_fun)(p, syntax_strs[cur_tuple], 0, max_comp_len);
		}

		cur_tuple++;
		if (cur_tuple >= total_tuple) break;
		ql->moveNext();
	}

	char* consen_str1=0;
	double cost1=0;

	StrAlignment align;
	align.computeConsensusStr(syntax_strs, cur_tuple, &consen_str1, &cost1);

	if (consen_str){
		consen_str->Set(consen_str1, consen_str1?strlen(consen_str1):0, false);
		consen_str->setToFree(true);
	}else if (consen_str1) {
		free(&consen_str1);
	}
	if (cost){
		*cost = cost1/(double)cur_tuple;
	}

	for (i=0; i<total_tuple; i++){
		free(syntax_strs[i]);
	}
	free(syntax_strs);

	return 0;
}

int HtWrapper::buildHyperConStr(char* p, char* syntax_buf, long*pos_buf, long max_len){
	long len = strlen(p);
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(p, &len, false);

	int IsSkip=false;
	int cur_pos=0;
	long dscore=0;
	long tscore=0;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA && !IsSkip){
			if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
			syntax_buf[cur_pos++]=dhyconPLAIN;
			dscore+= DataSyntax.StartLen;
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
			if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "Script")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconS_TAG;
				IsSkip = true;
			}else if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "A")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconS_TAG_A;
			}else if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "B")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconS_TAG_B;
			}else if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "FONT")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconS_TAG_FONT;
			}else if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "IMAGE")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconS_TAG_IMAGE;
			}else if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "IMG")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconS_TAG_IMAGE;
			}else{
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconS_TAG;
			}
			tscore+= DataSyntax.StartLen;
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "Script")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconE_TAG;
				IsSkip = false;
			}else if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "A")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconE_TAG_A;
			}else if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "B")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconE_TAG_B;
			}else if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "FONT")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconE_TAG_FONT;
			}else if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "IMAGE")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconE_TAG_IMAGE;
			}else if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "IMG")) {
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconE_TAG_IMAGE;
			}else{
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]=dhyconE_TAG;
			}
			tscore+= DataSyntax.StartLen;
		}

		if (cur_pos>=max_len) break;
		DataSyntax.match();
	}
	if (pos_buf) pos_buf[cur_pos]=len;
	syntax_buf[cur_pos]=0;
	return cur_pos;
}

int HtWrapper::buildTextConStr(char* p, char* syntax_buf, long*pos_buf, long max_len){
	long len = strlen(p);
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(p, &len, false);
	HtWrapperTextConSyntax TextSyntax;

	int IsSkip=false;
	int cur_pos=0;
	long dscore=0;
	long tscore=0;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA && !IsSkip){
			TextSyntax.setSentence(DataSyntax.Sentence+DataSyntax.Start, &DataSyntax.StartLen, false);
			while (TextSyntax.Type != QLSyntax::synQL_END){
				switch (TextSyntax.Type){
				case QLSyntax::synQL_WORD: 
					if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+TextSyntax.Start;
					syntax_buf[cur_pos++] = dtxconWORD;
					break;
				case QLSyntax::synQL_NUMBER: 
					if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+TextSyntax.Start;
					syntax_buf[cur_pos++] = dtxconNUMBER;
					break;
				case HtWrapperTextConSyntax::synQL_DATE: 
					if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+TextSyntax.Start;
					syntax_buf[cur_pos++] = dtxconDATE;
					break;
				case HtWrapperTextConSyntax::synQL_TIME: 
					if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+TextSyntax.Start;
					syntax_buf[cur_pos++] = dtxconTIME;
					break;
				case HtWrapperTextConSyntax::synQL_CURRENCY: 
					if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+TextSyntax.Start;
					syntax_buf[cur_pos++] = dtxconCURRENCY;
					break;
				case HtWrapperTextConSyntax::synQL_KEY: 
					if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+TextSyntax.Start;
					syntax_buf[cur_pos++] = dtxconKEY;
					break;
				case HtWrapperTextConSyntax::synQL_SYMBOL: 
					if (strchr(dtxconSYMBOLS, TextSyntax.Sentence[TextSyntax.Start])) {
						if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+TextSyntax.Start;
						syntax_buf[cur_pos++] = TextSyntax.Sentence[TextSyntax.Start];
					}
					break;
				default:
					break;

				}
				if (cur_pos>=max_len) break;
				TextSyntax.match();
			}
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
			if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "Script")) {
				IsSkip = true;
			}
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "Script")) {
				IsSkip = false;
			}
		}

		if (cur_pos>=max_len) break;
		DataSyntax.match();
	}
	if (pos_buf) pos_buf[cur_pos]=len;
	syntax_buf[cur_pos]=0;
	return cur_pos;
}
int HtWrapper::buildPlainConStr(char* p, char* syntax_buf, long*pos_buf, long max_len){
	long len = strlen(p);
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(p, &len, false);

	int IsSkip=false;
	int cur_pos=0;
	long dscore=0;
	long tscore=0;
	long i;
	int in_tag=0;
	ReferData temp_text;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA && !IsSkip){
			temp_text.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, true); 
			tStrOp::decodeHtml(&temp_text);
			int space=1;
			for (i=0; i<temp_text.L; i++){
				if (tStrOp::isSpace(temp_text.P[i])){
					if (!space) {
						if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+i;
						syntax_buf[cur_pos++]=' ';
						dscore++;
					}
					space=1;
				}else{
					space=0;

					if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start+i;
					syntax_buf[cur_pos++]=temp_text.P[i];
					dscore++;
				}
			}
			in_tag=0;
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG || DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (!in_tag){
				if (pos_buf) pos_buf[cur_pos]=DataSyntax.Start;
				syntax_buf[cur_pos++]='~';
				dscore++;
			}
			in_tag=1;
		}else{
			in_tag=0;
		}

		if (cur_pos>=max_len) break;
		DataSyntax.match();
	}
	if (pos_buf) pos_buf[cur_pos]=len;
	syntax_buf[cur_pos]=0;
	return cur_pos;
}

int HtWrapper::computeFieldLengthStatistic(HTQL* htql, int field_index1, long* max_len, long* min_len, long* average_len, long* tuple_count, int max_comp_tuple){
	long i=0;
	char* p=0;
	long len;
	long len_sum=0;
	long len_min=100000;
	long len_max=0;
	for (htql->moveFirst(); !htql->isEOF(); htql->moveNext()){
		p = htql->getValue(field_index1);
		if (!p) len=0;
		else len=strlen(p);

		len_sum+=len;
		if (len < len_min) len_min=len;
		if (len > len_max) len_max=len;

		i++;

		if (max_comp_tuple && i>=max_comp_tuple) break;
	}
	if (max_len) *max_len = len_max;
	if (min_len) *min_len = len_min;
	if (average_len) {
		if (i) *average_len = len_sum/i;
		else *average_len = len_sum;
	}
	if (tuple_count) *tuple_count=i;

	return 0;
}






