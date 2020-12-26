#ifndef HT_PAGE_MODEL_H_CLY_2007_02_03
#define HT_PAGE_MODEL_H_CLY_2007_02_03

#include "referdata.h"
#include "referlink.h"

class ReferLinkHeap;
class HtWrapperModels;

class HtGlobalAlign{
public:
	ReferData Page;
	ReferData Str; 
	int align(int model_type, int str_is_source=true); 

	int ModelType;
	ReferData PagePattern;
	long* PagePos; 
	ReferData StrPattern; 
	long* StrPos; 

	double Cost; 
	double Similarity; 
	ReferData AlignedPage; 
	ReferData AlignedStr; 

	HtGlobalAlign(); 
	~HtGlobalAlign(); 
	void reset();
	void resetAlignments();
};

class HtPageModel{
public:
	enum{MODEL_HTQL, //query
		MODEL_PLAIN_SOURCE, MODEL_PLAIN_PATTERN, MODEL_TEXT_PATTERN, MODEL_HTML_PATTERN, //local alignments
		MODEL_PAGE_CHANGE,	//global alignment, find changes of matching tag-patterns
		MODEL_SCRIPT_TAGS, //script, style and meta tags
		MODEL_MAIN_LARGEST_TEXT,	//by content length
		MODEL_MAIN_TEXT,	//by content length, and header location
		MODEL_MAIN_DATE,	//by header location
		MODEL_MAIN_TEXT_CHANGE, //global alignment using HtTextAlign, progressive segmented alginment
		MODEL_MAIN_TAG_CHANGE	//changes including enclosed tags
	};
	enum{FILL_FIRST=0x01, FILL_LAST=0x02, FILL_ALL=0x04};

	//score each position of the page by stepwise adding patterns
	ReferData Page; 
	double* PosScores; 
	
	int setModelPage(ReferData* page, int copy); //set the HTML page source
	int addModelScore(ReferData* model, int model_is_source, int model_type, double model_score, int fill_type=FILL_FIRST, int to_scale=true); //add scores by a pattern
			//distribute model_score to different page positions
			//model_type:	MODEL_HTQL:model is HTQL, add to extracted positions
			//				MODEL_PAGE_CHANGE: model is another page, add to segments that are different to the current page
			//				others: if model_is_source, model is HTML source
			//						else !model_is_source, model is a pattern
			//				to fill best matching positions with model_score
	int addWrapperModelScore(HtWrapperModels* model, ReferData* model_id, int model_type, double model_score, int fill_type=FILL_FIRST); 
		//add model scores from multiple model patterns in HtWrapperModels* model
	int recombineScores();	//modify scores by their neighboring scores
		//each scored position has a score:  x(k[i]) + x(k[i-1])/(1.1)^distance(left) + x(k[i+1])/(1.1)^distance(right)
	int findBestPos(ReferLinkHeap* results); //Value.L: position; Data: score*1e10
		//sort all positions based on their score
	long getScoreText(double threshold_score, ReferData* result);

	ReferLinkHeap MainDivTags;
	ReferLinkHeap MainIgnoreTags;
	ReferLinkHeap MainBlankTags;
	ReferLinkHeap MainSepTags;
	int resetMainTags();

	//same as the above definitions, as static functions
	int addModelScore(ReferData* page, double* pos_scores, ReferData* model, int model_is_source, int model_type, double model_score, int fill_type=FILL_FIRST, int to_scale=true); 
	int addWrapperModelScore(ReferData* page, HtWrapperModels* model, double* pos_scores, ReferData* model_id, int model_type, double model_score, int fill_type=FILL_FIRST); 
	static int findBestPos(double* pos_scores, long len, ReferLinkHeap* results); //results: Data:score*1e10, Value.L: index of position
	static int recombineScores(double* pos_scores, long len); //modify neighboring scores
	static int fillPosScores(double* pos_scores, long len, long from_pos, long to_pos, double score, int fill_type); 
	int findPageChangePositions(ReferData* page1, ReferData* page2, ReferLinkHeap* positions, int mode);  //Name.L: change_score; Value:change_text(P); Data: Offset; mode:0=html;1=text;2=char
	static int findPageTagsFromPositions(ReferData* page, ReferLinkHeap* positions, ReferLinkHeap* newpositions);  //Name.L: change_score; Value:change_text(P); Data: Offset; 
	static int findPageMainTextFromPositions(ReferData* page, ReferLinkHeap* positions, ReferLinkHeap* mainpositions);  //Name.L: change_score; Value:change_text(P); Data: length; 
					//find the main text for a set of positions 
	static int findPageTagPositions(ReferData* page, const char** tags, ReferLinkHeap* positions);  //Name.L: change_score; Value:change_text(P); Data: Offset; 
					//find positions of a couple of tags
	static int findPageWordsPositions(ReferData* page, ReferLinkHeap* words, ReferLinkHeap* positions);  //Name.L: change_score; Value:change_text(P); Data: Offset; 
					//find positions of a couple of words in the page, words must be single words
	long getHtmlMainText(ReferData* page, ReferData* option, ReferData* page2, ReferData* maintext); 
	long countMainTextLen(ReferData* text); 
	static int repairHtmlText(ReferData* text, ReferData* result);
	static int getHtmlMainSection(ReferData* page, long* section_start, long* section_end, int *level);
	int rankHtmlMainText(ReferData* page, ReferLinkHeap* results);
public:
	HtPageModel();
	~HtPageModel();
	void reset();

public:
	static int remallocPosScores(double** scores, long len); //it will free scores, if any
	static int mallocPatternSpace(const char* source, int model_type, char** pattern, long** pattern_pos);
};

#endif

