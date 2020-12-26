#ifndef CLY_HTQL_ADAPTER_2010_04_10
#define CLY_HTQL_ADAPTER_2010_04_10

#include "referlink.h"
#include "HtTextAlign.h"

class HtqlAdapter{
public: 
	ReferLinkHeap Pages; //Data: id; Name: htql, Value: page; 
	ReferData Htql;
	long PageID; 
	long HtqlOption; 

	long addPage(const char* page, const char* htql); //only add page to the history
	long dropPage(long id); //delete the page from history

	int* mallocPageMark(long id, int* maxlevel);  
			//Compare all pages with id, and return matching count at each position
			//this function also delete duplicated pages

	int adaptPage(const char* page, const char* htql);
			//Adapt page htql based on matching positions, result is set to Htql

	int findMinChanges(const char* page, ReferData* mainchange); //Name.L: change_score; Value:change_text(P); Data: Offset;
			//Find change positions from the closest page

	HtTextAlign Aligns;
	int addAlignScores(long id1, long id2, int* page_scores); //return 0 if id1 matches id2; otherwise return 1
			//page_scores need to be malloced by caller, same as id1 length
public: 
	HtqlAdapter(); 
	~HtqlAdapter(); 
	void reset(); 
};

#endif


