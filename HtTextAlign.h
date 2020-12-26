#ifndef HT_TEXT_ALIGN_H_CLY_2009_03_10
#define HT_TEXT_ALIGN_H_CLY_2009_03_10

#include "referlink.h" 
#include "tMatrix.h"

//piece-wise alignment of two similar text, with word & tag comparison.
class HtTextAlign {
public: 
	ReferLinkHeap TextWords; //Name: word; Data: count
	//ReferLinkHeap Text1; //Name.P: (ReferLink*)TextWords; Name.L:index; Value: value; Data: sorting order
	//ReferLinkHeap Text2; //Name.P: (ReferLink*)TextWords; Name.L:index; Value: value; Data: sorting order
	ReferLinkHeap TextN; //Data: ID; Name:title; Value.P:(ReferLinkHeap*)Text
										//{Text->Name.P: (ReferLink*)TextWords; Text->Name.L:index; Text->Value: value; Text->Data: sorting order}; 
	static int alignCmpFun(void* link_text1, void* link_text2); 
	static char alignGetDebugChar(void* link_text1);
	static long translateCmpCode(void* link_text1); 

	int cmpLinks(void* link_text1, void* link_text2); //0:match;1:insert;2:delete;31:replace;

	double MismatchedRatio; //target mismatched rate, default 1.0, allow 1 matched with 1 mismatched
	int setPlainText(long id, ReferData* text, long addoffset=0);	//word 
	int setHtmlText(long id, ReferData*html, long addoffset=0, int with_tag=true);		//word & tag
	int setCharText(long id, ReferData*str, long addoffset=0);		//char

	ReferLinkHeap* getTextN(long id); 
	int dropTextN(long id);
	ReferLink* addTextWord(long id, ReferData* word, ReferData* value, long data); 
	int align(long id1, long id2, double*cost, long* result_len, ReferLink*** source_align_links, ReferLink*** str_align_links, double* indel_cost=0); 

	tMatrix<long> Clusters; //ClusterN*2 matrix
	long ClusterN; 
	ReferLinkHeap ClustersCosts; 
	double getClustersCost(long id1, long id2); 
	long addToClusters(long id, int allow_dupplicate=true); 
	long isInCluster(long id); //get the closest index
	long getClosestNeighbor(long id);

	int setIdInfo(long id, const char*info, int copy=true); //set to TextN->Name
	int getClustersHtml(ReferData* page, long start_i=-1, long parent_id=-1, long level=0); 

	int alignMultiple(ReferLinkHeap* aligned_strs, long* aligned_len, double* cost, long start_i=-1, long parent_id=-1, long level=0);
				//aligned_strs: Data: (long) id; Value.P: (ReferLink**) align_links (need to be freed by caller)
				//((ReferLink*) aligned_links[i])->: Name.P: (ReferLink*) word; Name.L: seq# word; Value: row text; Data: text offset
				//also see ==> HtTextAlign::addTextWord()
	int mergeAlignments(ReferLinkHeap* left_align, long id_left, long left_len, ReferLinkHeap* right_align, long id_right, long right_len, ReferLinkHeap* results, long* result_len); 
	int freeAlignMemory(ReferLinkHeap* aligned_strs);
public:
	int insertClusterRow(long i, long id1, long id2, double cost=-1); 
	long estimateWindowSize(ReferLinkHeap* text1, ReferLinkHeap* text2, long max_size=0); 
	int moveNextLink(ReferLink** link, long* index, char* search_P, ReferLinkHeap* text); 
				//text: Text1 or Text2, search from text the next link with Name.P==search_P
				//index: the position of the link in sort order
	void** mallocLinkArray(ReferLinkHeap* text); 
public: 
	HtTextAlign();
	~HtTextAlign();
	void reset(); 
};

#endif

