#ifndef ALIGNMENT_H_CLY_2003_05_06
#define ALIGNMENT_H_CLY_2003_05_06

#include "referdata.h"
#include "referlink.h"

struct StrAlignmentNode{
	double cost;
	int prev;
};

//==GLOBAL== alignment: aligning the whole string
// this works only for text strings
// result is the alignment position
class StrAlignment {	
public:
	enum {algnMATCH, algnINSERT, algnDELETE, algnREPLACE};
	double IndelCost[4]; //for global alignment, typically = [0, 1, 1, 2]
						 //for local alignment, typically = [1, -0.5, -0.5, -1]
						 //change the cost to allow the frequency of mismatches
	int setIndelCost(double match, double insert, double del, double replace); 
	double MatchMismatchPenalty; //for each match->mismatch transition, this penalty is subtracted 
	
	int IsCaseSensitive;
	long (*CmpCode)(void* ch1);	//translate to code to expedite comparison
					//if set, result_tags will be the translated codes
	int (*CmpFun)(void* ch1, void* ch2); //using function to compare two pointer
			//CmpCode has more priority than CmpFun
	char (*DebugChar)(void* ch1);		//
	long (*Translate2Position)(void* ch1); //translate to original position

	//global string alignment
	int CompareStrings(char* str1, char* str2, double* cost=0, long* result_len=0, char** result_str1=0, char** result_str2=0);
						//	similarity = 1 - cost / (double)(2*result_len);
	//multiple string global alignment
	int computeConsensusStr(char** strs, int n_str, char** consen_str, double* cost);

	//global alignment of two tag sequences 
	int compareHyperTags(void**sourcetag, long source_count, void**strtag, long str_count, 
							double* cost=0, long* result_len=0, void*** result_tag1=0, void*** result_tag2=0, double** result_costs=0);

	//local alignment of two tag sequences
	int alignHyperTags(void**sourcetag, long source_count, void**strtag, long str_count, 
							ReferData* results, double* align_score=0, long*isource1=0, long*isource2=0,long*istr1=0,long*istr2=0);

	int dumpMatrix(const char* dump2file, const char*description, StrAlignmentNode* matrix, const char*col_str, const char*row_str, long cols, long rows);

	StrAlignment();
	~StrAlignment();
	void reset();
};

//==LOCAL== alignment, align a segment to the whole string;
// hash words or hypertext tags in a table and align exact words or tags
// results are multiple positions in a hyper text result
class HyperTagsAlignment{
	ReferLinkHeap STags;
	ReferLinkHeap ETags;
	void** SourceTag;
	long* SourcePos;
	long SourceCount;
	void** StrTag;
	long* StrPos;
	long StrCount;
public:
	enum{TYPE_TAG, TYPE_HYPER_CHAR, //hypertext input
		TYPE_WORD, TYPE_CHAR}; //plaintext input
	int AlignType;
	int IsCaseSensitive;
	int (*CmpFun)(void* ch1, void* ch2); 
	char (*DebugChar)(void* ch1); 

	enum {algnMATCH, algnINSERT, algnDELETE, algnREPLACE};
	double IndelScore[4];
		//algnMATCH: (will be added)
		//algnREPLACE: (will be subtracted)
		//algnINSERT: (will be subtracted) keep source, insert a space into str
		//algnDELETE: (will be subtracted) insert a space in source, or keep source and delete a str char

public:
	HyperTagsAlignment();
	~HyperTagsAlignment();
	void reset();

	int setAlignType(int type);

	int setSouceData(ReferData* source);
	int setStrData(ReferData* str);
	int alignHyperTagsText(ReferData* results, double* align_score=0, int whole_x=0);

	//local alignments
	int alignHyperTagsText(ReferData* source, ReferData* str, ReferData* results, double* align_score=0, int whole_x=0);
public:
	int builtTagsForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count);
	int alignHyperTagsText(void**sourcetag, long*sourcepos, long source_count, void**strtag, long*strpos, long str_count, 
							ReferData* results, double* align_score=0, long*isource1=0, long*isource2=0,long*istr1=0,long*istr2=0,
							int whole_x=false);
	static int builtHyperTagsForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count);
					//stag + etag
	static int builtHyperCharStrForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count);
					//stag + etag + text chars
	static int builtWordsForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count);
					//word + number - tags
	static int builtPlainCharStrForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count);
					//chars
	static int remallocTagsPos(void***strtag, long**strpos, long len, long newlen);
};

#endif
