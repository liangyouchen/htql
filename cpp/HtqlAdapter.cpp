#include "HtqlAdapter.h" 
#include "htmlql.h"
#include "qhtmlql.h"
#include "HtPageModel.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE
#endif



HtqlAdapter::HtqlAdapter(){
	Pages.setSortOrder(SORT_ORDER_NUM_INC);
	Pages.setDuplication(false); 
	PageID=0; 
	HtqlOption=LEAVE_SINGLE_ITEM; 
}

HtqlAdapter::~HtqlAdapter(){
	reset(); 
}

void HtqlAdapter::reset(){
	Pages.empty();
	PageID=0;
	Htql.reset();
	HtqlOption=LEAVE_SINGLE_ITEM; 
}

int HtqlAdapter::addAlignScores(long id1, long id2, int* page_scores){
	ReferLink* link=Pages.findData(id1);

	double cost=0.0; 
	long result_len=0;
	ReferLink** source_align_links=0, ** str_align_links=0; 
	Aligns.align(id1, id2, &cost, &result_len, &source_align_links, &str_align_links); 
	if (cost<0.01) return 0; 

	int cmpcode=0; 
	long pos; 
	for (long i=0; i<result_len; i++){
		cmpcode=Aligns.cmpLinks(source_align_links[i], str_align_links[i]);

		if (cmpcode==0){  //match
			for (pos=0; pos<source_align_links[i]->Value.L; pos++){
				page_scores[source_align_links[i]->Value.P-link->Value.P+pos]++; 
			}
		}else if (cmpcode==3){ //replace
		}else if (cmpcode==2){ //link1 has data
		}else if (cmpcode==1){ //link2 has data	
		}
	}
	return 1;
}
long HtqlAdapter::addPage(const char* page, const char* htql){
	long id=1; 
	ReferLink* link=(ReferLink*) Pages.moveLast(); 
	if (link) id=link->Data+1; 
	link=Pages.add(htql, page, id); 
	
	//add this page to history
	Aligns.setHtmlText(id, &link->Value, 0, true); 

	return id;
}
long HtqlAdapter::dropPage(long id){
	ReferLink* link=Pages.findData(id); 
	if (link){
		Pages.remove(link); 
	}
	Aligns.dropTextN(id); 
	return 0;
}
int* HtqlAdapter::mallocPageMark(long id, int* maxlevel){
	//this function also delete duplicated pages
	ReferLink* link=Pages.findData(id); 
	if (!link) return 0;

	//learn htql from the current page, optimizing for common tags
	long len=link->Value.L; 
	int *page_scores=(int*) malloc(sizeof(int)*len); 
	memset(page_scores, 0, sizeof(int)*len); 

	ReferLinkHeap duplicated; //duplicated page the same as id
	duplicated.setSortOrder(SORT_ORDER_NUM_INC);

	//add matching score for all pages, so don't need to sort
	ReferLink* link1; 
	int count=0; 
	for (link1=(ReferLink*) Pages.moveFirst(); link1; link1=(ReferLink*) Pages.moveNext()){
		if (link1->Data != link->Data){
			if (addAlignScores(link->Data, link1->Data, page_scores)>0) 
				count++; 
			else
				duplicated.add((char*) 0, (char*) 0, link1->Data); 
		}
	}

	//delete duplicated pages
	for (link1=duplicated.getReferLinkHead(); link1; link1=link1->Next){
		dropPage(link1->Data); 
	}

	if (maxlevel) *maxlevel=count;
	
	return page_scores; 
}


int HtqlAdapter::adaptPage(const char* page, const char* htql){
	long id=addPage(page, htql);
	ReferLink* link=Pages.findData(id); 
	int marklevel=0; 
	int* page_scores=mallocPageMark(id, &marklevel); 
	
	//learn htql
	HtmlQL ql; 
	ql.setSourceData(link->Value.P, link->Value.L, false); 
	ql.setQuery(htql); 
	long offset;
	ql.getFieldOffset(1, &offset); 

	ReferLinkHeap result;
	while (!result.Total && marklevel>=0){
		HtmlQLParser::findHtql(&link->Value, offset, &result, HtmlQL::heuBEST, HtqlOption, page_scores, marklevel);
		marklevel--;
	}

	link=(ReferLink*) result.moveFirst();
	Htql=link->Name; 

	free(page_scores);
	return 0;
}

int HtqlAdapter::findMinChanges(const char* page, ReferData* mainchange){ //Name.L: change_score; Value:change_text(P); Data: Offset;
			//Find change positions from the closest page
	ReferLink* link1;
	for (link1=(ReferLink*) Pages.moveFirst(); link1; link1=(ReferLink*) Pages.moveNext()){
		if (!Aligns.isInCluster(link1->Data)){
			Aligns.addToClusters(link1->Data);
		}
	}

	//duplcated page
	long id=addPage(page, 0);
	long id1=-1;
	ReferLink* link=Pages.findData(id); 
	id1=Aligns.addToClusters(id, false);
	if (id1<0){
		dropPage(id);
		id=-id1;
		link=Pages.findData(id); 
	}

	id1=Aligns.getClosestNeighbor(id); 
	if (id1<0) return 0;
	link1=Pages.findData(id1); 

	ReferData option; option="mainchange";
	HtPageModel model;
	model.getHtmlMainText(&link->Value, &option, &link1->Value, mainchange); 

	/* // ReferLinkHeap* positions
	long len=link->Value.L; 
	int *page_scores=(int*) malloc(sizeof(int)*len); 
	memset(page_scores, 0, sizeof(int)*len); 

	addAlignScores(id, id1, page_scores);

	if (positions->Total==0){
		positions->setSortOrder(SORT_ORDER_VAL_NUM_INC);
	}

	long i, start=-1; 
	ReferLink* link2; 
	for (i=0; i<len; i++){
		if (!page_scores[i]){
			if (start<0) start=i; 
		}else{
			if (start>=0){
				//found from start to i-1; 
				link2=(ReferLink*) positions->add((char*) 0, (char*) 0, start);
				link2->Value.Set(link->Value.P+start, i-start, false);
				link2->Name.L=i-start;

				start=-1;
			}
		}
	}
	
	if (page_scores) free(page_scores);
	*/
	return 0;
}
