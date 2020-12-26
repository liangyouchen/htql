#include "HtPageModel.h"
#include "htmlql.h"
#include "qhtql.h"
#include "alignment.h"
#include "htwrapper.h"
#include "referset.h"
#include "HtWrapperModels.h"
#include "HtTextAlign.h"
#include "docbase.h"
#include <math.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE
#endif

HtGlobalAlign::HtGlobalAlign(){
	ModelType=0;
	PagePos=0;
	StrPos=0;
	Cost=0;
	Similarity=0; 
}

HtGlobalAlign::~HtGlobalAlign(){
	reset(); 
}
void HtGlobalAlign::reset(){
	Page.reset(); 
	Str.reset(); 
	ModelType=0;

	resetAlignments();
}
void HtGlobalAlign::resetAlignments(){
	PagePattern.reset(); 
	StrPattern.reset(); 
	if (PagePos) {
		free(PagePos); 
		PagePos=0; 
	}
	if (StrPos){
		free(StrPos); 
		StrPos=0;
	}
	Cost=0;
	Similarity=0; 
	AlignedPage.reset(); 
	AlignedStr.reset();
}

int HtGlobalAlign::align(int model_type, int str_is_source){
	resetAlignments(); 

	ModelType=model_type;

	//convert page into tag patterns, model_type such as MODEL_HTML_PATTERN
	HtPageModel::mallocPatternSpace(Page.P, ModelType, &PagePattern.P, &PagePos);
	PagePattern.Size=Page.L; 
	PagePattern.L=strlen(PagePattern.P); 
	PagePattern.setToFree(true); 

	if (str_is_source){
		//convert model page into tag patterns
		HtPageModel::mallocPatternSpace(Str.P, ModelType, &StrPattern.P, &StrPos);
		StrPattern.Size=Str.L; 
		StrPattern.L=strlen(StrPattern.P); 
		StrPattern.setToFree(true); 
	}else{
		StrPattern.Set(Str.P, Str.L, false); 
	}

	//global alignment of model pattern to the page pattern
	StrAlignment align;
	align.CompareStrings(PagePattern.P, StrPattern.P, &Cost, &AlignedPage.L, &AlignedPage.P, &AlignedStr.P); 
	AlignedPage.Size=AlignedPage.L+1; 
	AlignedPage.setToFree(true); 

	AlignedStr.L=AlignedPage.L; 
	AlignedStr.Size=AlignedStr.L+1; 
	AlignedStr.setToFree(true); 

	Similarity=1-(double) Cost/(AlignedPage.L*2);
	return 0;
}


HtPageModel::HtPageModel() {
//	const char* pos_scores_fields[]={"source_pos", "score", "pattern_type", "htql", 0};
//	PosScores.setFieldsNum(ID_POS_SCORES_FIELDS, pos_scores_fields);
//	PosScores.newIndexField(0, ReferSetBTree::FLAG_REFER_L);
	PosScores=0;

	resetMainTags();
}
HtPageModel::~HtPageModel(){
	reset();
}
void HtPageModel::reset(){
	if (PosScores){
		free(PosScores);
		PosScores=0;
	}
	Page.reset();
//	PosScores.reset();

	resetMainTags();
}
int HtPageModel::resetMainTags(){
	MainBlankTags.empty(); 
	MainIgnoreTags.empty(); 
	MainDivTags.empty();

	const char* cMainBlankTags[]={"<P>", "</P>","<br>", "</br>", "<a>", "</a>", 
		"<b>", "</b>", "<strong>", "</strong>", "<i>", "</i>", "<em>", "</em>", "<font>", "</font>", 
		"<BLOCKQUOTE>", "</BLOCKQUOTE>", "<SUP>", "</SUP>", "<SUB>", "</SUB>", 
		"<h1>", "</h1>", "<h2>", "</h2>", "<h3>", "</h3>", "<h4>", "</h4>","<h5>", "</h5>",
		"<img>", "<td>", "</td>", "<span>", "</span>", 
		"<ul>","</ul>","<ol>","</ol>","<li>","</li>", "<dl>","</dl>", "<DT>","</DT>","<DD>","</DD>",
		0};
	const char* cMainIgnoreTags[]={"<textarea>", "<script>", "<style>", 
		"<head>", "<title>", "<meta>", "<option>", 
		0};
	const char* cMainDivTags[]={"<div>", "</div>", "<table>", "</table>","<body>", "</body>",0};
	const char* cMainSepTags[]={"<td>", "</td>", "<tr>", "</tr>", 0};
	
	int i;
	for (i=0; cMainBlankTags[i]; i++){
		MainBlankTags.add(cMainBlankTags[i], 0, 0); 
	}
	for (i=0; cMainIgnoreTags[i]; i++){
		MainIgnoreTags.add(cMainIgnoreTags[i], 0, 0); 
	}
	for (i=0; cMainDivTags[i]; i++){
		MainDivTags.add(cMainDivTags[i], 0, 0); 
	}
	for (i=0; cMainSepTags[i]; i++){
		MainSepTags.add(cMainSepTags[i], 0, 0); 
	}
	return 0;
}

int HtPageModel::remallocPosScores(double** scores, long len){
	if (*scores) free(*scores);
	*scores=(double*) malloc(sizeof(double)*len);
	if (!*scores){
		return -1; //memory error
	}
	memset(*scores, 0, sizeof(double)*len);
	return 0;
}
int HtPageModel::mallocPatternSpace(const char* source, int model_type, char** pattern, long** pattern_pos){
	long page_len=strlen(source);
	long i; 
	*pattern=(char*) malloc(sizeof(char)*(page_len+1));
	memset(*pattern, 0, sizeof(char)*(page_len+1));
	*pattern_pos=(long*)malloc(sizeof(long)*(page_len+1));
	memset(*pattern_pos, 0, sizeof(long)*(page_len+1));

	int (*constr_fun)(char*, char*, long*, long);
	switch (model_type){
	case MODEL_PLAIN_SOURCE: 
		for (i=0; i<page_len; i++){
			(*pattern)[i]=source[i];
			(*pattern_pos)[i]=i;
		}
		(*pattern_pos)[page_len]=page_len;
		return 0;
	case MODEL_PLAIN_PATTERN: 
		constr_fun=HtWrapper::buildPlainConStr;
		break;
	case MODEL_TEXT_PATTERN: 
		constr_fun=HtWrapper::buildTextConStr;
		break;
	case MODEL_HTML_PATTERN:
		constr_fun=HtWrapper::buildHyperConStr;
		break;
	};
	//HtWrapper::buildHyperConStr(page->P, page_pattern.P, page_pos, page_len);
	(*constr_fun)((char*) source, *pattern, *pattern_pos, page_len);
	(*pattern_pos)[strlen(*pattern)]=page_len;
	return 0;
}

int HtPageModel::setModelPage(ReferData* page, int copy){
	Page.Set(page->P, page->L, copy);
	return remallocPosScores(&PosScores, page->L);
}
int HtPageModel::fillPosScores(double* pos_scores, long len, long from_pos, long to_pos, double score, int fill_type){
	if ((fill_type & FILL_FIRST) && !(fill_type & FILL_ALL) && from_pos<len){
		pos_scores[from_pos]+=score; //score are distributed to different matching positions
	}
	if ((fill_type & FILL_LAST) && !(fill_type & FILL_ALL) && to_pos<len){
		pos_scores[to_pos]+=score; //score are distributed to different matching positions
	}
	if (fill_type & FILL_ALL){
		for (long i=from_pos; i<=to_pos && i<len; i++){
			pos_scores[i]+=score; //score are distributed to different matching positions
		}
	}
	return 0;
}
int HtPageModel::findPageChangePositions(ReferData* page1, ReferData* page2, ReferLinkHeap* positions, int mode){  //Name.L: change_score; Value:change_text(P); Data: Offset
			//mode:0=html;1=text;2=char

	HtTextAlign align; 
	if (mode==0){
		align.setHtmlText(1, page1); 
		align.setHtmlText(2, page2); 
	}else if (mode==1){
		align.setPlainText(1, page1); 
		align.setPlainText(2, page2); 
	}else{
		align.setCharText(1, page1); 
		align.setCharText(2, page2); 
	}
	double cost=0; 
	long result_len=0; 
	ReferLink** source_align_links=0, **str_align_link=0; 
	align.align(1,2,&cost, &result_len, &source_align_links, &str_align_link); 	
	int cmpcode=0; 
	long i; 
	ReferLink* link;
#ifdef DEBUG_THIS_FILE
	ReferData debug_str;
	char debug_buf[128]; 
	long debug_count=0;
#endif
	for (i=0; i<result_len; ){
		cmpcode=align.cmpLinks(source_align_links[i], str_align_link[i]);
		if (cmpcode==3 || cmpcode==2){ //replace or link1 has data
			char* change_from=source_align_links[i]->Value.P;
			char* change_to=0; 
			long change_score=0, score=0; 
			while (i<result_len && (cmpcode==3 || cmpcode==2)){
				if (i<result_len && source_align_links[i]) 
					change_to=source_align_links[i]->Value.P+source_align_links[i]->Value.L;

				score=countMainTextLen(&source_align_links[i]->Value); 

				if (cmpcode==3) change_score+=score*100; //more score for replace
				else change_score+=score; //less score for insert

				i++; 
				if (i<result_len)
					cmpcode=align.cmpLinks(source_align_links[i], str_align_link[i]);

			}

			//add position from change_from to change_to
			link=positions->add((char*) 0, 0, change_from-page1->P); 
			link->Value.Set(change_from, change_to-change_from, false);
			link->Name.L=change_score;
#ifdef DEBUG_THIS_FILE
			debug_count++;
			sprintf(debug_buf, "%ld: %ld-%ld: ", debug_count, change_from-page1->P, change_to-page1->P);
			debug_str+=debug_buf;
			debug_str.Cat(change_from, change_to-change_from); 
			debug_str+="\r\n";
#endif 

		}else if (cmpcode==0 || cmpcode==1){ //matched or link2 has data	
			while (i<result_len && (cmpcode==0 || cmpcode==1)){
				i++; 
				if (i<result_len)
					cmpcode=align.cmpLinks(source_align_links[i], str_align_link[i]);
			}
		}
	}
	if (source_align_links) free(source_align_links); 
	if (str_align_link) free(str_align_link);

#ifdef DEBUG_THIS_FILE
		debug_str.saveFile("$findPageChangePositions.txt");
#endif 
	return 0;
}
int HtPageModel::findPageTagPositions(ReferData* page, const char** tags, ReferLinkHeap* positions){  //Name.L: change_score; Value:change_text(P); Data: Offset; 
	HTQLTagDataSyntax syntax;
	syntax.setSentence(page->P, &page->L, false);
	long last_tag=-1;
	long last_lag_level=0; 
	long last_lag_start=0; 
	ReferLink* link;
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG || syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			if (last_tag>=0){
				if (TagOperation::isTag(syntax.Sentence+syntax.Start, tags[last_tag])){
					if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG) last_lag_level++; 
					else{
						last_lag_level--; 
						if (last_lag_level==0){
							link=positions->add((char*) 0, 0, last_lag_start); 
							link->Value.Set(syntax.Sentence+last_lag_start, syntax.Start-last_lag_start, false);
						}
					}
				}
			}else{
				last_tag=TagOperation::isTags(syntax.Sentence+syntax.Start, tags);
				if (last_tag>=0){
					last_lag_level=1; 
					last_lag_start=syntax.Start; 
				}
			}
		}

		syntax.match();
	}
	return 0;
}
int HtPageModel::findPageWordsPositions(ReferData* page, ReferLinkHeap* words, ReferLinkHeap* positions){  //Name.L: change_score; Value:change_text(P); Data: Offset; 
					//find positions of a couple of words in the page
	const char* ignore_tags[]={"<script>", "<style>", "<meta>", 0};
	int is_valid_tag=0;
	ReferLink* link;
	HTQLTagDataSyntax syntax;
	syntax.setSentence(page->P, &page->L, false);
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG || syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isTags(syntax.Sentence+syntax.Start, ignore_tags)>=0 ){
				is_valid_tag=-1;
			}else{
				is_valid_tag=0;
			}
		}else if (syntax.Type==HTQLTagDataSyntax::synQL_DATA && is_valid_tag>=0){
			//count text segment
			PDocPlainDataSyntax wordtax; 
			ReferData word;
			wordtax.setSentence(syntax.Sentence+syntax.Start, &syntax.StartLen, false); 
			while (wordtax.Type!=QLSyntax::synQL_END){
				if (wordtax.Type==QLSyntax::synQL_WORD){
					word.Set(wordtax.Sentence+wordtax.Start, wordtax.StartLen, false); 
					if (words->findName(&word)){
						link=positions->add((char*) 0, 0, syntax.Start+wordtax.Start); 
						link->Value.Set(wordtax.Sentence+wordtax.Start, wordtax.StartLen, false);
					}
				}
				wordtax.match(); 
			}
		}else{ //ignore
			is_valid_tag=0;
		}

		syntax.match();
	}
	return 0;
}


//========================================================================
long HtPageModel::countMainTextLen(ReferData* text){
	long len=0;
	int is_valid_tag=0;
	HTQLTagDataSyntax syntax;
	syntax.setSentence(text->P, &text->L, false);
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG || syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isTags(syntax.Sentence+syntax.Start, &MainBlankTags)>=0 ){
				is_valid_tag=1; //continue counting text segment
			}else if (TagOperation::isTags(syntax.Sentence+syntax.Start, &MainIgnoreTags)>=0 ){
				is_valid_tag=-1;
			}else{
				is_valid_tag=0;
			}
		}else if (syntax.Type==HTQLTagDataSyntax::synQL_DATA && is_valid_tag>=0){
			//count text segment
			len+=syntax.StartLen; 
		}else{ //ignore
			is_valid_tag=0;
		}

		syntax.match();
	}
	return len;
}


int HtPageModel::findPageMainTextFromPositions(ReferData* page, ReferLinkHeap* positions, ReferLinkHeap* mainpositions){  //Name.L: change_score; Value:change_text(P); Data: length; 
	//fill in gaps
	HtPageModel model; 
	ReferLink *prev=0, *main_first=0, *main_last=0, *link=0;
	long lastpos=0, last_offset=0, to_setlen=0, to_setpos=-1, to_setoffset=0;
	long last_score=0, to_setscore=0, score=0; 
	long space=-1;
#ifdef DEBUG_THIS_FILE
	ReferData debug_str;
	char debug_buf[128]; 
#endif
	for (link=(ReferLink*) positions->moveFirst(); link; link=(ReferLink*) positions->moveNext()){
		if (prev) space=(link->Value.P - prev->Value.P) - prev->Value.L;

#ifdef DEBUG_THIS_FILE
		sprintf(debug_buf, "SPACE: %ld>>POS: %ld (%ld)<<", space, link->Data, link->Value.L); 
		debug_str+=debug_buf; 
		debug_str+=link->Value;
		debug_str+="\r\n\r\n"; 
#endif

		if (space>=0 && ((prev && prev->Value.L>space) || link->Value.L>space)){ //with small gaps
			last_offset+=space+link->Value.L; 
			last_score+=link->Name.L;
		}else{ //with large gap
			lastpos=link->Value.P-page->P; 
			last_offset=link->Value.L;
			last_score=link->Name.L;
		}

		if (last_score>to_setscore){
			to_setpos=lastpos; 
			to_setoffset=last_offset; 
			to_setscore=last_score;
#ifdef DEBUG_THIS_FILE
			sprintf(debug_buf, "-\t-\t-\t-->FROM: %ld==>TO: %ld (%ld) *** SCORE:%ld\r\n\r\n", lastpos, lastpos+last_offset, last_offset, last_score); 
			debug_str+=debug_buf; 
#endif
		}
		prev=link;
	}
#ifdef DEBUG_THIS_FILE
	debug_str.saveFile("$findPageMainTextFromPositions.txt");
#endif

	//get the first text position
	int is_valid_tag=1, is_latertext=1;
	lastpos=-1; 
	last_offset=-1; 
	HTQLTagDataSyntax syntax;
	syntax.setSentence(page->P, &page->L, false);
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG || syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isTags(syntax.Sentence+syntax.Start, &model.MainBlankTags)>=0 ){
				if (lastpos<0 && syntax.Start<to_setpos) {
					lastpos=syntax.Start; //move start position
				}
				is_valid_tag=1; //continue counting text segment
				if (syntax.Start+syntax.StartLen > to_setpos+to_setoffset && is_latertext){
					last_offset=syntax.Start+syntax.StartLen; //move end position
				}
			}else if (TagOperation::isTags(syntax.Sentence+syntax.Start, &model.MainIgnoreTags)>=0 ){
				is_valid_tag=-1;
			}else{
				is_valid_tag=0;
				if (syntax.Start<to_setpos) lastpos=-1;
				if (syntax.Start+syntax.StartLen > to_setpos+to_setoffset) 
					is_latertext=0;
			}
		}else if (syntax.Type==HTQLTagDataSyntax::synQL_DATA && is_valid_tag>=0){
			//count text segment
			if (lastpos<0 && syntax.Start<to_setpos) {
				lastpos=syntax.Start; //move start position
			}
			if (syntax.Start+syntax.StartLen > to_setpos+to_setoffset && is_latertext){
				last_offset=syntax.Start+syntax.StartLen;  //move end position
			}
		}else{ //ignore
			if (is_valid_tag>0){
				is_valid_tag=0;
				if (syntax.Start<to_setpos) lastpos=-1;
				if (syntax.Start+syntax.StartLen > to_setpos+to_setoffset) 
					is_latertext=0;
			}
		}

		syntax.match();
	}

	//move start position
	if (lastpos>=0){ 
		to_setoffset+=to_setpos-lastpos;
		to_setpos=lastpos; 
	}
	//move end position
	if (last_offset>=0){
		to_setoffset=last_offset-to_setpos;
	}

	if (to_setpos>=0){
		link=mainpositions->add((char*) 0, 0, to_setoffset); 
		link->Value.Set(page->P+to_setpos, to_setoffset, false);
	}

	return 0;
}
int HtPageModel::findPageTagsFromPositions(ReferData* page, ReferLinkHeap* positions, ReferLinkHeap* newpositions){  //Name.L: change_score; Value:change_text(P); Data: Offset; 
	HtPageModel model; 
	ReferData last_tag; 
	int is_valid_tag=1, last_tag_can_add=0, is_last_text_added=0;
	HTQLTagDataSyntax syntax;
	syntax.setSentence(page->P, &page->L, false);
	ReferLink to_find;
	ReferLink* link; 
	const char* ignore_tags[]={"<textarea>", "<script>", "<style>", "<meta>", 0};
	ReferLink* position_link=0; 
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG || syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isTags(syntax.Sentence+syntax.Start, ignore_tags)>=0 ){
				is_valid_tag=-1;
			}else if (TagOperation::isTags(syntax.Sentence+syntax.Start, &model.MainBlankTags)>=0 ){
				is_valid_tag=1; //continue counting text segment
			}else{
				is_valid_tag=0;
			}
			last_tag_can_add=false;
			if (is_valid_tag>=0){
				int to_add_this=false;
				if (is_last_text_added && syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
					to_add_this=true; 
				}

				//check if there is any change in this segment
				if (!position_link || position_link->Data+position_link->Value.L < syntax.Start){
					to_find.Data=syntax.Start; 
					position_link=(ReferLink*) positions->moveFirstLarger((char*) &to_find); 
				}
				if (position_link && ( (position_link->Data<=syntax.Start+syntax.StartLen && position_link->Data+position_link->Value.L >= syntax.Start)
					|| (position_link->Data<=syntax.Start && position_link->Data+position_link->Value.L>syntax.Start)
					) ){//has change tag
					to_add_this=true; 
				}

				if (to_add_this){
					link=(ReferLink*) newpositions->add((char*) 0, 0, syntax.Start); 
					if (link) link->Value.Set(syntax.Sentence+syntax.Start, syntax.StartLen, false);
				}else if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG){
					last_tag_can_add=true;
				}
			}
			last_tag.Set(syntax.Sentence+syntax.Start, syntax.StartLen, false);
			is_last_text_added=false;
		}else if (syntax.Type==HTQLTagDataSyntax::synQL_DATA && is_valid_tag>=0){
			//check if there is any change in this segment
			if (!position_link || position_link->Data+position_link->Value.L < syntax.Start){
				to_find.Data=syntax.Start; 
				position_link=(ReferLink*) positions->moveFirstLarger((char*) &to_find); 
			}
			if (position_link && ( (position_link->Data<=syntax.Start+syntax.StartLen && position_link->Data+position_link->Value.L >= syntax.Start)
				|| (position_link->Data<=syntax.Start && position_link->Data+position_link->Value.L>syntax.Start)
				) ){//has change tag
				if (last_tag_can_add && last_tag.L){
					link=newpositions->add((char*) 0, 0, last_tag.P-syntax.Sentence); 
					if (link) link->Value.Set(last_tag.P, last_tag.L, false);
				}
				link=newpositions->add((char*) 0, 0, syntax.Start); 
				if (link) link->Value.Set(syntax.Sentence+syntax.Start, syntax.StartLen, false);
				is_last_text_added=true;
			}
		}else{ //ignore
			last_tag_can_add=false;
			if (is_valid_tag>0){
				is_valid_tag=0;
			}
		}

		syntax.match();
	}

	return 0;
}

int HtPageModel::addModelScore(ReferData* model, int model_is_source, int model_type, double model_score, int fill_type, int to_scale){
	return addModelScore(&Page, PosScores, model, model_is_source, model_type, model_score, fill_type, to_scale); 
}
int HtPageModel::addModelScore(ReferData* page, double* pos_scores, ReferData* model, int model_is_source, int model_type, double model_score, int fill_type, int to_scale){
	int err=0;
	if (model_type==MODEL_HTQL){ //model is an HTQL query
		//****QUERY****
		//model is HTQL, add to extracted positions
		HtmlQL ql;
		ql.setSourceData(page->P, page->L, false);
		ql.setQuery(model->P);
		ql.dotQuery("&position .<position>:from, to, length");
		char* p;
		long count=ql.getTuplesCount();
		for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
			p=ql.getValue(1);
			long from=p?atoi(p):0;
			p=ql.getValue(2);
			long to=p?atoi(p):page->L;
			fillPosScores(pos_scores, page->L, from, to-1, to_scale?(model_score/count):model_score, fill_type); 

#ifdef DEBUG_THIS_FILE
			ReferData seg;
			seg.Set(page->P+from, to-from, true);
			printf("*Model: %x, from %ld to %ld: %s\n", model_type, from, to, seg.P);
#endif
		}
		return 0;
	}else if (model_type==MODEL_SCRIPT_TAGS){
		const char* script_tags[]={"<script>", "<style>", "<meta>", 
			"</script>", "</style>", "</meta>", 0};

		ReferLinkHeap positions; 
		positions.setSortOrder(SORT_ORDER_NUM_INC);
		findPageTagPositions(page, script_tags, &positions); 

		for (ReferLink* link=(ReferLink*) positions.moveFirst(); link; link=(ReferLink*) positions.moveNext()){
			fillPosScores(pos_scores, page->L, link->Data, link->Data+link->Value.L-1, to_scale?(model_score/positions.Total):model_score, fill_type); 			
		}

	}else if (model_type==MODEL_MAIN_TEXT_CHANGE){
		//global alignment using HtTextAlign
		//see also PathNavigationFunctions::functionComparePages()
		ReferLinkHeap positions; 
		positions.setSortOrder(SORT_ORDER_NUM_INC);
		findPageChangePositions(page, model, &positions,0);  //Name.L: change_score; Value:change_text(P); Data: Offset

		ReferLinkHeap mainpositions; 
		mainpositions.setSortOrder(SORT_ORDER_NUM_DEC);
		findPageMainTextFromPositions(page, &positions, &mainpositions); 
		
		for (ReferLink* link=(ReferLink*) mainpositions.moveFirst(); link; link=(ReferLink*) mainpositions.moveNext()){
			long to_setpos=link->Value.P-page->P; 
			long to_setoffset=link->Value.L; 
			fillPosScores(pos_scores, page->L, to_setpos, to_setpos+to_setoffset-1, to_scale?(model_score*to_setoffset/page->L):model_score, fill_type); 
		}

	}else if (model_type==MODEL_MAIN_TAG_CHANGE) {
		//see also PathNavigationFunctions::functionComparePages()
		ReferLinkHeap positions; 
		positions.setSortOrder(SORT_ORDER_NUM_INC);
		findPageChangePositions(page, model, &positions,1);  //Name.L: change_score; Value:change_text(P); Data: Offset

		ReferLinkHeap newpositions; 
		newpositions.setSortOrder(SORT_ORDER_NUM_INC);
		findPageTagsFromPositions(page, &positions, &newpositions); 
		
#ifdef DEBUG_THIS_FILE
		ReferData debug_str; 
		char debug_buf[128]; 
		long debug_count=0;
#endif 
		for (ReferLink* link=(ReferLink*) newpositions.moveFirst(); link; link=(ReferLink*) newpositions.moveNext()){
			fillPosScores(pos_scores, page->L, link->Data, link->Data+link->Value.L-1, to_scale?(model_score/newpositions.Total):model_score, fill_type); 			
#ifdef DEBUG_THIS_FILE
			debug_count++;
			sprintf(debug_buf, "%ld: %ld-%ld: ", debug_count, link->Data, link->Data+link->Value.L-1);
			debug_str+=debug_buf;
			debug_str.Cat(page->P+link->Data, link->Value.L); 
			debug_str+="\r\n";
#endif 
		}
#ifdef DEBUG_THIS_FILE
		debug_str.saveFile("$MODEL_MAIN_TAG_CHANGE.txt");
#endif 

			
	}else if (model_type==MODEL_MAIN_DATE){//by header location
		long section_start=0, section_end=page->L;
		int level=0; 
		getHtmlMainSection(page, &section_start, &section_end, &level);
		ReferLinkHeap dates;
		dates.setSortOrder(SORT_ORDER_NUM_INC);
		TagOperation::searchHtmlDateStr(page->P, page->L, &dates, true);
		ReferLink* link=0, *link1=0; 
		ReferLinkHeap ranking;
		ranking.setSortOrder(SORT_ORDER_NUM_INC);
		for (link=(ReferLink*) dates.moveFirst(); link; link=(ReferLink*) dates.moveNext()){
			if (link->Data>=section_start && link->Data < section_end){
				link1=(ReferLink*) ranking.add(&link->Name, 0, (link->Data - section_start)*1000/ (section_end - section_start + 1) );
			}else if (link->Data<section_start){
				link1=(ReferLink*) ranking.add(&link->Name, 0, 1000 + (section_start-link->Data)*1000/ (section_start+1) );
			}else{
				link1=(ReferLink*) ranking.add(&link->Name, 0, 2000 + (link->Data - section_end)*1000/ (page->L - section_end +1) );
			}
			if (link1){
				link1->Value.P = (char*) link; 
			}
		}
		link1=(ReferLink*) ranking.moveFirst(); 
		if (link1){
			link = (ReferLink*) link1->Value.P; 
			fillPosScores(pos_scores, page->L, link->Data, link->Data+link->Name.L-1, to_scale?(model_score*1.0):model_score, fill_type); 
		}
	}else if (model_type==MODEL_MAIN_LARGEST_TEXT){//by content length
		ReferLinkHeap positions; 
		rankHtmlMainText(page, &positions); 
		ReferLink* link=(ReferLink*) positions.moveFirst(); 
		if (link){
			long fill_from=link->Name.L, fill_to=link->Name.L+link->Value.L-1;
			fillPosScores(pos_scores, page->L, fill_from, fill_to, to_scale?(model_score*link->Data/page->L):model_score, fill_type); 
		}

	}else if (model_type==MODEL_MAIN_TEXT){//by content length, and header location, and date
		long section_start=0, section_end=page->L;
		int level=0;
		getHtmlMainSection(page, &section_start, &section_end, &level);


		ReferLinkHeap positions; 
		rankHtmlMainText(page, &positions); 

		ReferLink* link=(ReferLink*) positions.moveFirst(); 

		
		
		//the text section is before the main header section, and header has high confidence
		if (link && link->Name.L + link->Value.L < section_start && level>75){
			while (link && link->Name.L + link->Value.L < section_start){
				link=(ReferLink*) positions.moveNext(); 
			}
			if (!link || link->Value.L<10) link=(ReferLink*) positions.moveFirst(); 
		}
		//the text sections is after the header section, check if it is too long
		if (link && section_start && link->Name.L > section_end) {
			while (!positions.isEOF() ){
				//check if there is any higher level h tag in between
				int too_long=false;
				HtmlQL ql;
				ql.setSourceData(page->P+section_end, link->Name.L - section_end, true); 
				ql.setQuery("<h1 sep>.<h2 sep>.<h3 sep>");
				char* p=ql.moveFirst();
				for (p=ql.moveNext(); !ql.isEOF(); p=ql.moveNext()){
					if (p[2]<=page->P[section_start+2] ){
						too_long=true;
						break;
					}
				}
				if (too_long && (level > 85)){ //high confidence header, move text 
					link=(ReferLink*) positions.moveNext(); 
				}else break; 
			}
			if (!link || link->Value.L<10) {
				link=(ReferLink*) positions.moveFirst(); 
				link->Name.L=section_start; 
				link->Value.L=section_end - section_start; 
			}
		}
		if (link){
			long fill_from=link->Name.L, fill_to=link->Name.L+link->Value.L-1;

			if (section_start<fill_from) fill_from=section_start;
			
			fillPosScores(pos_scores, page->L, fill_from, fill_to, to_scale?(model_score*link->Data/page->L):model_score, fill_type); 
		}

	}else if (model_type==MODEL_PAGE_CHANGE){ //older code, may not work
		//***GLOBAL ALIGNMENT***
		//model is another page, add to segments that are different to the current page
		//		align to page by their tag-patterns (only matching tag-patterns are considered)
		//		then compare each matching tag by text-patterns

		// global alignment of model pattern to the page pattern
		//		may be slow for large page; if model is not source, the code may not work, 
		//		use HtTextAlign instead?
		HtGlobalAlign align; 
		align.Page.Set(page->P, page->L, false); 
		align.Str.Set(model->P, model->L, false);
		align.align(MODEL_HTML_PATTERN, model_is_source); 
		
		long page_pos1=0, str_pos2=0; 
		long last_page_pos1=-1, last_str_pos2=-1, last_i=-1; 
		for (long i=0; i<align.AlignedStr.L; i++){
			if (align.AlignedPage.P[i]==align.AlignedStr.P[i]){
				if (last_i>=0){ //from last_i to i is a match, do text alignment and add score
					//do text alignment of the matching segments
					StrAlignment align_str;
					ReferData str1, str2; 
					long len1=align.PagePos[page_pos1]-align.PagePos[last_page_pos1];  
					long len2=align.StrPos[str_pos2]-align.StrPos[last_str_pos2]; 
					if (len1>256) len1=256; 
					if (len2>256) len2=256; 
					//need to remove HTML tags, do later ...
					HtGlobalAlign align2; 
					align2.Page.Set(page->P+align.PagePos[last_page_pos1], len1, true); 
					align2.Str.Set(model->P+align.StrPos[last_str_pos2], len2, true); 
					align2.align(MODEL_PLAIN_PATTERN, true); 
					
					if (align2.Cost>1e-3){
						fillPosScores(pos_scores, page->L, align.PagePos[last_page_pos1], align.PagePos[page_pos1]-1, to_scale?(model_score*(1-align2.Similarity)):model_score, fill_type); 

#ifdef DEBUG_THIS_FILE
						ReferData seg;
						seg.Set(page->P+align.PagePos[last_page_pos1], align.PagePos[page_pos1]-align.PagePos[last_page_pos1], true);
						printf("*Model: %x, from %ld to %ld: %s\n", model_type, align.PagePos[last_page_pos1], align.PagePos[page_pos1], seg.P);
#endif
					}
				}
				last_i=i;
				last_page_pos1=page_pos1;
				last_str_pos2=str_pos2;
			}
			if (align.AlignedPage.P[i]!='-') page_pos1++; 
			if (align.AlignedStr.P[i]!='-') str_pos2++;
		}
	}else{ //MODEL_PLAIN_SOURCE, MODEL_PLAIN_PATTERN, MODEL_TEXT_PATTERN, MODEL_HTML_PATTERN
		//****LOCAL ALIGNMENTS****
		//fill best matching positions with model_score
		char* page_pattern=0;
		long* page_pos=0;
		char* model_pattern=0;
		long* model_pos=0;

		//convert page into tag patterns
		mallocPatternSpace(page->P, model_type, &page_pattern, &page_pos);
		//convert model page into tag patterns
		if (model_is_source)
			mallocPatternSpace(model->P, model_type, &model_pattern, &model_pos);
		else
			model_pattern=model->P;

		//local alignment of model pattern to the page pattern
		HyperTagsAlignment align;
		ReferData ppage_pattern, pmodel_pattern, best_pos;
		ppage_pattern.Set(page_pattern, strlen(page_pattern), false);
		pmodel_pattern.Set(model_pattern, strlen(model_pattern), false);
		align.setAlignType(HyperTagsAlignment::TYPE_CHAR);
		align.setSouceData(&ppage_pattern);
		align.setStrData(&pmodel_pattern);
		if ((err=align.alignHyperTagsText(&best_pos))>=0){  //ppage_pattern+best_pos.*; pmodel_pattern+best_pos.*
#ifdef DEBUG_THIS_FILE
			char* page_pos_test=strstr(ppage_pattern.P,"Next");
#endif

			//get best matching positions
			HtmlQL patternql;
			patternql.setSourceData(best_pos.P, best_pos.L, false);
			patternql.setQuery("<max_positions>.<position>:source_pos, str_pos, cost, from_pos");
			long count=patternql.getTuplesCount();
			for (patternql.moveFirst(); !patternql.isEOF(); patternql.moveNext()){
				long to=atoi(patternql.getValue(1));
				//to++; 
				long from=atoi(patternql.getValue(4));
				if (from<to) from++;
				fillPosScores(pos_scores, page->L, page_pos[from], page_pos[to], to_scale?(model_score/count):model_score, fill_type); 

#ifdef DEBUG_THIS_FILE
				ReferData seg;
				seg.Set(page->P+page_pos[from], page_pos[to]-page_pos[from], true);
				printf("*Model: %x, from %ld to %ld: %s\n", model_type, page_pos[from], page_pos[to], seg.P);
#endif
			}
		}

		if (page_pattern) free(page_pattern);
		if (page_pos) free(page_pos);
		if (model_pattern && model_is_source) free(model_pattern);
		if (model_pos) free(model_pos);
	}
	return err;
}
int HtPageModel::rankHtmlMainText(ReferData* page, ReferLinkHeap* results){
	//ranked based on text segment size
	long best_len=0, newlen=0, last_len=0;

	int is_valid_tag=1;
	char* tag=0, *lasttag=page->P, *best_tag=0, *best_lasttag=0;
	long lastpos=0, last_offset=0, to_setlen=0, to_setpos=0, to_setoffset=0;
	HTQLTagDataSyntax syntax;
	syntax.setSentence(page->P, &page->L, false);
	results->setSortOrder(SORT_ORDER_NUM_DEC);
	results->setDuplication(true);
	ReferLink* link; 
#ifdef DEBUG_THIS_FILE
	ReferData debug_str;
	char debug_buf[128]; 
	long debug_count=0;
#endif
	//is_valid_tag>0: continue counting; =0: reset counting; <0: ignore text
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG || syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isTags(syntax.Sentence+syntax.Start, &MainBlankTags)>=0 ){
				is_valid_tag=1; //continue counting text segment
			}else if (TagOperation::isTags(syntax.Sentence+syntax.Start, &MainIgnoreTags)>=0 ){
				is_valid_tag=-1;
			}else if (TagOperation::isTags(syntax.Sentence+syntax.Start, &MainSepTags)>=0 ){
				is_valid_tag=0;
			}else if (TagOperation::isTags(syntax.Sentence+syntax.Start, &MainDivTags)>=0  ){
				to_setlen=last_len; //add to positions
				to_setpos=lastpos; 
				to_setoffset=last_offset;

				last_len=0; //new text segment from here, but do not count
				lastpos=syntax.Start;
				is_valid_tag=0;
			}else{ //continue counting text segment
				is_valid_tag=1;
			}
		}else if (syntax.Type==HTQLTagDataSyntax::synQL_DATA && is_valid_tag>=0){
			//count text segment
			newlen=syntax.StartLen;
			if (is_valid_tag>0 ){ //count text
				newlen+=last_len;
			}else {	//reset counting
				to_setlen=last_len; 
				to_setpos=lastpos; 
				to_setoffset=last_offset;

				lastpos=syntax.Start;
			}
			last_offset=syntax.Start+syntax.StartLen-lastpos;
			last_len=newlen;
		}else{ //ignore
			if (is_valid_tag>0)
				is_valid_tag=0;
		}

		if (to_setlen){
			link=results->add((char*) 0, 0, to_setlen); //score
			link->Value.Set(page->P+to_setpos, to_setoffset, false);
			link->Name.L=to_setpos; 

#ifdef DEBUG_THIS_FILE
			debug_count++;
			sprintf(debug_buf, "%ld: %ld-%ld (%ld/%ld): ", debug_count, to_setpos, to_setpos+to_setoffset, to_setlen, to_setoffset);
			debug_str+=debug_buf;
			debug_str.Cat(page->P+to_setpos, to_setoffset); 
			debug_str+="\r\n";
#endif 

			to_setlen=0;
			to_setpos=0;
			to_setoffset=0;
		}

		syntax.match();
	}
#ifdef DEBUG_THIS_FILE
	debug_str.saveFile("$MODEL_MAIN_TEXT.txt");
#endif 
	return 0;
}

int HtPageModel::getHtmlMainSection(ReferData* page, long* section_start, long* section_end, int* level){
	//based on similarity to title, and <h> tags
	HTQL ql, ql1;
	ql.setSourceData(page->P, page->L, false);
	ql.setQuery("<title>:tx &trim()"); 
	ReferData title; 
	title=ql.getValue(1); 
	ql.setQuery("<h1 sep>.<h2 sep>.<h3 sep>"); 
	*section_start=0, *section_end=page->L; 
	char* p=0;
	int first_h='9', last_h=0;
	//find top level header tag
	for (p=ql.moveFirst(); !ql.isEOF(); p=ql.moveNext()){
		if (p[2]>='0' && p[2]<first_h) first_h=p[2];
	}
	char buf[128];
	*level=0;
	//find header similar to title
	if (title.L){
		for (p=ql.moveFirst(); !ql.isEOF(); p=ql.moveNext()){
			if (p[2]>'0' && p[2]<'9') {
				char* p1=ql.getValue(1); 
				ql1.setSourceData(p1, strlen(p1), false); 
				sprintf(buf, "<h%c>:tx &trim()", p[2]); 
				ql1.setQuery(buf); 
				char* p2=ql1.getValue(1); 
				if (p2 && *p2 && (tStrOp::strNstr(title.P, p2, false) || tStrOp::strNstr(p2, title.P, false) ) ){
					*level=100;
					break;
				}
			}
		}
	}

	//use header date information
/*	if (ql.isEOF()){
		ReferLinkHeap dates;
		dates.setSortOrder(SORT_ORDER_NUM_INC);
		TagOperation::searchHtmlDateStr(page->P, page->L, &dates, true);
		long pos=0, off=0;
		ReferLink* link;
		long min_distance=page->L; 
		for (p=ql.moveFirst(); !ql.isEOF(); p=ql.moveNext()){
			ql.getFieldOffset(1, &pos, &off);
			for (link=dates.moveFirst(); link; link=dates.moveNext()){
				if (abs(link->Data - pos) < min_distance) min_distance = abs(link->Data - pos);
			}
		}
	}
*/

	//find top level header tag, for sections with at least some <p>
	if (ql.isEOF()){
		for (p=ql.moveFirst(); !ql.isEOF(); p=ql.moveNext()){
			if (p[2]==first_h) {
				char* p1=ql.getValue(1); 
				ql1.setSourceData(p1, strlen(p1), false); 
				ql1.setQuery("<p> &tx //"); 
				char* p2=ql1.getValue(1); 
				if (p2 && strlen(p2)>20 ){
					*level=90;
					break;
				}
			}
		}
	}
	//find other level header tag, if top level has no <p> 
	if (ql.isEOF()){
		for (p=ql.moveFirst(); !ql.isEOF(); p=ql.moveNext()){
			char* p1=ql.getValue(1); 
			ql1.setSourceData(p1, strlen(p1), false); 
			ql1.setQuery("<p>&tx //"); 
			char* p2=ql1.getValue(1); 
			if (p2 && strlen(p2)>20 ){
				*level=80;
				break;
			}
		}
	}
	//use the top level tag anyway
	if (ql.isEOF()){
		for (p=ql.moveFirst(); !ql.isEOF(); p=ql.moveNext()){
			if (p[2]==first_h){
				*level=70;
				break;
			}
		}
	}

	//set section start and end positions
	if (!ql.isEOF()){
		first_h=p[2];
		last_h=p[2];
		*section_start=ql.getFieldOffset(1);
		while (!ql.isEOF()){
			p=ql.moveNext();
			if (!ql.isEOF() && p){
				if (p[2]<first_h){
					if (*level<85){
						*section_start=ql.getFieldOffset(1);
						first_h=p[2];
						last_h=p[2];
					}
				}else{
					*section_end=ql.getFieldOffset(1);
					if (p[2]<=last_h) break;
					last_h=p[2];
				}
			}
		}
	}
	//incorporate all <p> tags? 
	ql.setQuery("<p>"); 
	long offset=0;
	int p_count=0; 
	for (p=ql.moveFirst(); !ql.isEOF(); p=ql.moveNext()){
		offset=ql.getFieldOffset(1);
		if (offset > *section_start){
			if (offset > *section_end){ 
				*section_end = offset + 3;
				break;
			}else{
				if (++p_count>=3){
					*section_end = offset + 3;
					break;
				}
			}
		}
	}

	return 0;
}

int HtPageModel::addWrapperModelScore(HtWrapperModels* model, ReferData* model_id, int model_type, double model_score, int fill_type){
	return addWrapperModelScore(&Page, model, PosScores, model_id, model_type, model_score, fill_type);
}
int HtPageModel::addWrapperModelScore(ReferData* page, HtWrapperModels* model, double* pos_scores, ReferData* model_id, int model_type, double model_score, int fill_type){
	int type_list[]={MODEL_HTQL, MODEL_PLAIN_PATTERN, MODEL_TEXT_PATTERN, MODEL_HTML_PATTERN};
	char* pattern_list[]={"HtqlPrefix", "Text", "TextPattern", "TagPattern", 0};

	ReferData query;
	ReferData* tuple;
	HtmlQL patternql;
	for (int i=0; pattern_list[i]; i++){
		if (model_type & type_list[i]){
			query="select Pattern where ModelType='Local' and PatternType='";
			query+=pattern_list[i]; 
			query+="'"; 
			if (model_id && model_id->L){ 
				query+=" and ModelID='"; query+=*model_id; query+="'"; 
			}

			ReferSet patterns;
			model->Models.executeSQL(&query, &patterns);
			for (tuple=patterns.moveFirst(); tuple; tuple=patterns.moveNext()){
				patternql.setSourceData(tuple->P, tuple->L, false);
				patternql.setQuery("<ConsensusPatterns>.<ConsensusPattern>:tx");
				int fields=patternql.getTuplesCount();
				ReferData record_pattern;
				for (patternql.moveFirst();  !patternql.isEOF(); patternql.moveNext()){
					if (record_pattern.L) record_pattern+="^"; //concatenate pattern fields by a ^ symbol
					record_pattern+=patternql.getValue(1);
				}

				if (record_pattern.L){
					addModelScore(page, pos_scores, &record_pattern, false, type_list[i], 1.0/patterns.TupleCount, fill_type);
				}
			}
		}
	}
	return 0;
}

int HtPageModel::recombineScores(){
	return recombineScores(PosScores, Page.L);
}
int HtPageModel::recombineScores(double* pos_scores, long len){
	long last_pos=-1;
	double newscore=0;
	double last_score=0, score=0;
	for (long i=0; i<len; i++){
		score=pos_scores[i];
		if (score>1e-30){
			//each scored position has a new score:  
			//	 x(k[i]) + x(k[i-1])/(1.1)^distance(left) + x(k[i+1])/(1.1)^distance(right)
			if (last_pos>=0){
				newscore=exp(log(score)-(i-last_pos)*100/len*log(1.1)); //score/(1.1)^(distance/(len/100))
				pos_scores[last_pos]+=newscore;

				newscore=exp(log(last_score)-(i-last_pos)*100/len*log(1.1)); //last_score/(1.1)^(distance/(len/100))
				pos_scores[i]+=newscore;
			}

			last_pos=i;
			last_score=score;
		}
	}
	return 0;
}
int HtPageModel::findBestPos(ReferLinkHeap* results){
	return findBestPos(PosScores, Page.L, results);
}

int HtPageModel::findBestPos(double* pos_scores, long len, ReferLinkHeap* results){
	results->setSortOrder(SORT_ORDER_NUM_DEC);
	results->setDuplication(true);
	ReferLink* link;
	for (long i=0; i<len; i++){
		if (pos_scores[i]>1e-10){
			link=results->add((const char*) 0, 0, (long) (pos_scores[i]*1e5));
			link->Value.L=i;
		}
	}
	return 0;
}
long HtPageModel::getScoreText(double threshold_score, ReferData* result){
	result->reset(); 
	long from_pos=-1; 
	long first_pos=-1; 
	for (long i=0; i<=Page.L; i++){
		if (PosScores[i]>=threshold_score && from_pos<0){
			from_pos=i; 
			if (first_pos<0) first_pos=from_pos;
		}else if (PosScores[i]<threshold_score && from_pos>=0){
			result->Cat(Page.P+from_pos, i-from_pos); 
			from_pos=-1; 
		}
	}
	if (from_pos>=0){
		if (first_pos<0) first_pos=from_pos;
		if (!result->L){
			//referencing the starting position
			result->Set(Page.P+from_pos, Page.L-from_pos,false); 
		}else{
			result->Cat(Page.P+from_pos, Page.L-from_pos); 
		}
		from_pos=-1; 
	}
	return first_pos;
}

long HtPageModel::getHtmlMainText(ReferData* page, ReferData* option, ReferData* page2, ReferData* maintext){
	int err=setModelPage(page, false);
	if (err<0) {
		maintext->reset(); return err;
	}
	long offset=0; 
	ReferData text;
	if (option && !option->Cmp("mainchange", strlen("mainchange"), false)  && page2 && page2->L){
		addModelScore(page2, true, HtPageModel::MODEL_MAIN_TEXT_CHANGE, 1, HtPageModel::FILL_ALL,false);
		addModelScore(page, true, HtPageModel::MODEL_SCRIPT_TAGS, -1, HtPageModel::FILL_ALL,false);
		offset=getScoreText(0.5, &text); 
		repairHtmlText(&text, maintext); 
	}else if (option && !option->Cmp("changetag", strlen("changetag"), false)){
		addModelScore(page2, true, HtPageModel::MODEL_MAIN_TAG_CHANGE, 1, HtPageModel::FILL_ALL, false);
		offset=getScoreText(0.5, maintext); 
	}else if (option && !option->Cmp("changeimg", strlen("changeimg"), false)){
		addModelScore(page2, true, HtPageModel::MODEL_MAIN_TAG_CHANGE, 1, HtPageModel::FILL_ALL, false);
		//model.getScoreText(0.5, maintext); 
		ReferData query; query="<img>:st"; 
		addModelScore(&query, true, HtPageModel::MODEL_HTQL, 1, HtPageModel::FILL_ALL, false);
		offset=getScoreText(1.5, maintext); 
	}else if (option && !option->Cmp("date", strlen("date"), false)){
		addModelScore(page, true, HtPageModel::MODEL_MAIN_DATE, 1, HtPageModel::FILL_ALL,false);
		offset=getScoreText(0.5, maintext); 
	}else if (option && !option->Cmp("textlen", strlen("textlen"), false)){
		addModelScore(page, true, HtPageModel::MODEL_MAIN_LARGEST_TEXT, 1, HtPageModel::FILL_ALL,false);
		offset=getScoreText(0.5, maintext); 
	}else{
		addModelScore(page, true, HtPageModel::MODEL_MAIN_TEXT, 1, HtPageModel::FILL_ALL,false);
		offset=getScoreText(0.5, &text); 
		repairHtmlText(&text, maintext); 
	}

	return offset;
}
int HtPageModel::repairHtmlText(ReferData* text, ReferData* result){
	result->reset(); 
	if (!text || !text->L) return 0; 

	ReferLinkHeap start_tags;
	start_tags.setCaseSensitivity(false);

	ReferData added_start, added_end, new_tag;
	const char* to_match_tags[]={"div", "table", "tr", "p", 0}; 
	ReferData tag;
	long i=0;
	ReferLink* link=0;
	char*p=0; 
	size_t len=0;

	HTQLTagDataSyntax syntax;
	syntax.setSentence(text->P, &text->L, false);
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG){
			p=TagOperation::getLabel((const char*) (syntax.Sentence+syntax.Start), (unsigned int*) &len); 
			tag.Set(p, len, false);
			link=start_tags.findName(&tag); 
			if (!link){
				link=start_tags.add(&tag, 0, 1); 
			}else{
				link->Data++; 
			}
		}else if (syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			p=TagOperation::getLabel(syntax.Sentence+syntax.Start, (unsigned int*) &len); 
			tag.Set(p, len, false);
			link=start_tags.findName(&tag); 
			if (!link){
				link=start_tags.add(&tag, 0, 0); 
			}
			if (link->Data>0){
				link->Data--;
			}else{
				new_tag="<"; new_tag+=tag; new_tag+=">"; new_tag+=added_start; 
				added_start=new_tag; 
			}
		}
		syntax.match();
	}
	for (link=start_tags.getReferLinkHead(); link; link=link->Next){
		if (link->Data>0){
			i=tStrOp::strMcmp(link->Name.P, to_match_tags, false); 
			if (i>=0){
				while (link->Data>0){
					added_end+="</"; added_end+=link->Name; added_end+=">";
					link->Data--;
				}
			}
		}
	}
	*result=added_start; *result+=*text; *result+=added_end;
	
	return 0;
}

