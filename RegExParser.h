#ifndef SIMPLE_REG_EX_H_2010_09_02
#define SIMPLE_REG_EX_H_2010_09_02

#include "qlsyntax.h"
#include "btree.h"
#include "referlink2.h"
#include "referset.h"

class ReferLinkHeap;
class RegExParser;
class RegExToken;

class RegExTokenBTree: public BTree{
public:
	int Type; 
	enum {typeTOKEN_TOKEN, typeTOKEN_STRING };
	RegExParser* RegParser;
	virtual int cmp(char* p1, char* p2);
	int matchTarget(RegExToken* target, RegExToken* p2, RegExToken** p2_end);

	RegExTokenBTree();
	~RegExTokenBTree();
	void reset();
};


class RegExToken {
public: 
	ReferData Name; 
	ReferLink2 Conditions;
	long Index; //index of token position
	ReferData TokenText; //point to actual text position
	ReferData Token; //extracted tag
	int TokenType;
	int IsNegativeToken;
	long RepeatNFrom; 
	long RepeatNTo; 

	RegExToken* Next; //deleted automatically, next token
	RegExToken* Prev; 
	RegExToken* SiblNext;//deleted automatically, token next | 
	RegExToken* SiblPrev;
	RegExToken* Child; //deleted automatically, token in ()
	RegExToken* Parent;
	RegExToken* ExtendNext; //delete automatically, extended token
	RegExToken* SubToken; //to keep tokens for <xxx/xxx/xxx>

	RegExToken* addNextToken(RegExToken* token);
	RegExToken* addSiblToken(RegExToken* token);
	RegExToken* addChildToken(RegExToken* token);
	RegExToken* setParentToken(RegExToken* token);
	RegExToken* addExtendedToken(RegExToken* token);

	int switchNextToSiblNext(); //move Next to SiblNext

	//Variables to keep matching information
	RegExToken* MatchedFrom; 
	RegExToken* MatchedToList; 
	RegExToken* MatchedTo; 
	RegExToken* MatchedSibl;
	long MatchedRepeatN; 
	int isMatchedInRange(long repeat);
	int clearMatched(); 
	int clearAllMatched(); //also clearMatched for all next, child, sibling, and ExtendNext links
	int pushMatchedToList(const char* info=0); 
	int popMatchedToList(const char* info=0); 
	int deleteMatchedToList(const char* info=0); 
	int takeMatchedToList(RegExToken* from);
	int getMatchedToListInfo(ReferData* info, const char* title=0, const char* title1=0); 
	int getTokenListInfo(ReferData* info, RegExToken* token_to);
	//int takeMatchedToList(RegExToken* from);

	//ReferLinkHeap NextTokens;
	enum {matchDEFAULT, matchTOKEN_SET, matchWORD_SET, matchSTRING_SET};
	int MatchType;
	RegExTokenBTree* MatchStringSet;
	int setupMatchStringTokenSet(RegExParser* parser);
	int setupMatchStringStrSet(RegExParser* parser, ReferLinkHeap* strset, int matchtype=0);

	enum {opNONE, opNAME, opEXPR };
	ReferLink2 OpOptions; 
public:
	RegExToken();
	~RegExToken();
	void reset(); 
}; 

class RegExSyntax: public QLSyntax{
public: 
	enum {
		synRegQL_TOKEN=100,
		synRegQL_WORD, // \w
		synRegQL_NONWORD,	//	\W
		synRegQL_WORDBOUND, // \b
		synRegQL_DIGIT,		//	\d
		synRegQL_NONDIGIT,		//	\D
		synRegQL_SPACE,		//	\s
		synRegQL_NONSPACE,		//	\S
		synRegQL_RANGE,			// a-z
		synRegQL_TAG,			// <xxx>
		synRegQL_OPTAG,			// &[...]
	}; 
	virtual int isSpace(char);
	virtual int isComment(char*, long* len);
	virtual void findNext();
	long parseNumber(); 

	int InOpMode; 
	RegExSyntax();
	virtual ~RegExSyntax();
	void reset();
}; 

class RegExKeeper {
public: 
	enum {ID_Name, ID_Expr, ID_RegEx, ID_Parser, ID_IsList, ID_FIELDS_NUM};
	ReferSet KeepSet;
	ReferData* findRegEx(ReferData* name, ReferData* expr); 
	int deleteRegEx(ReferData* name, ReferData* expr);
	ReferData* newRegEx(ReferData* name, ReferData* expr);
	ReferData* keepRegEx(ReferData* name, ReferData* expr, RegExToken* regex, RegExParser* parser, int islist);

	RegExKeeper(); 
	~RegExKeeper(); 
	void reset();
};

class RegExSearcher {
public:
	RegExParser* RegContext; //to keep context pointer
	RegExParser* RegParser; 
	RegExToken* RegToken; 
	RegExToken* TextStart;
	int IsRegList;
	int compile(const char* sentence, int is_list=false, int case_sensitive=true);
	int search(const char* text, ReferSet* results, int overlapping=false, int useindex=false, int group=false);
public:
	RegExSearcher(); 
	~RegExSearcher(); 
	void reset(); 
};

class RegExParser{
public: 
	int CaseSensitive; 
	int Overlap;
	inline int matchCmpChar(char ch1, char ch2);

	int searchRegExText(const char* text, const char* regexpr, ReferLinkHeap* results); 
	int searchRegExTextSet(const char* text, const char* regexpr, ReferSet* results, int as_text=false); 
	int matchRegExText(const char* text, const char* regexpr); //return boolean;
	int matchRegExSubToken(RegExToken* text, RegExToken* regexpr); //return boolean;

	virtual RegExSyntax* newRegExSyntax(); 
	int parseRegEx(RegExSyntax* syntax, RegExToken* first, int is_seq_mode=true, long stop_token=QLSyntax::synQL_END); 
	virtual int parseRegExToken(RegExSyntax* syntax, RegExToken* token); //for regular expression syntax parsing
	virtual int setRegExTokenRange(RegExSyntax* syntax, RegExToken* token); //for regular expression syntax parsing
	virtual int parseRegExOpOptions(RegExSyntax* syntax, RegExToken** token, int is_seq_mode=true);
	virtual int parseRegExOpOptionName(RegExSyntax* syntax, RegExToken** token, int is_seq_mode=true);
	virtual int parseRegExOpOptionCondition(RegExSyntax* syntax, RegExToken** token, int is_seq_mode=true);
	virtual int parseRegExOpOptionSet(RegExSyntax* syntax, RegExToken** token, int is_seq_mode=true, int matchtype=0);
	virtual int createRegExSubToken(RegExToken* regexpr); 

	virtual int tokenizeText(const char* text, RegExToken* start, long limit=-1); //be sure to add a BEGIN and END tag
	virtual int isMatchedRegToken(RegExToken* text_token, RegExToken* reg_token); //return true or false
	virtual int matchedRegTokenText(RegExToken* text_token, RegExToken* reg_token); //return true or false
	virtual int getTokenText(RegExToken* text, RegExToken* text_to, ReferData* result);

	int searchRegExTokensSet(RegExToken* text, RegExToken* regex, ReferSet* results, int as_text=false, int withgroup=true); //Name.P: token_start, Value.P: token_end (exclude), Data: index
	int searchRegExTokens(RegExToken* text, RegExToken* regex, ReferLinkHeap* results); //Name.P: token_start, Value.P: token_end (exclude), Data: index
	int searchRegEx(RegExToken* text, RegExToken* regex, RegExToken** text_matched=0, RegExToken** matched_to=0); //to_match: to match from start to end
	int doMatchRegExSequence(RegExToken* text,RegExToken* regex, RegExToken** matched_to); //return err
	int doMatchRegExRepeat(RegExToken* text,RegExToken* regex, RegExToken** matched_to); //return err
	int handleMultipleNegative(RegExToken* reg_start, int has_sibling, int repeat, RegExToken* repeat_text_to, RegExToken** text_to, RegExToken** matched_sibl, int* err);
	RegExToken* newToken(RegExSyntax* syntax, RegExToken* token, int is_seq_mode);
	int searchRegExGroups(RegExToken* regex, ReferLinkHeap* groups);
	int isConditionMatched(tExprCalc* expr, RegExToken* reg_start);
	int setExprVariables(tExprCalc* expr, RegExToken* reg_start);

	RegExParser* Context;
	ReferLinkHeap* (*getNameHeap)(RegExParser* parser, void* p, const char* name); 
	void* GetNameHeapPara; 
	ReferLinkNamedHeap ParserNameHeaps;
	ReferLinkHeap* findContextNameHeap(const char* name); 
	ReferLinkHeap* getContextNameHeap(const char* name); 
	static long addTokens2Heap(RegExToken* sibl_token, ReferLinkHeap* heap);

public: 
	RegExParser(); 
	~RegExParser();
	void reset(); 
}; 

class TokenRegExSyntax: public RegExSyntax{
public:
	virtual int isSpace(char);
};

class TokenRegExParser: public RegExParser{
public:
	virtual RegExSyntax* newRegExSyntax(); 
	virtual int parseRegExToken(RegExSyntax* syntax, RegExToken* token); //for regular expression syntax parsing
	virtual int tokenizeText(const char* text, RegExToken* start, long limit=-1); //be sure to add a BEGIN and END tag
	virtual int matchedRegTokenText(RegExToken* text_token, RegExToken* reg_token); //return true or false
};

class ListRegExParser: public RegExParser{
	virtual RegExSyntax* newRegExSyntax(); 
	virtual int parseRegExToken(RegExSyntax* syntax, RegExToken* token); //for regular expression syntax parsing
	virtual int tokenizeText(const char* text, RegExToken* start, long limit=-1); //be sure to add a BEGIN and END tag
	virtual int matchedRegTokenText(RegExToken* text_token, RegExToken* reg_token); //return true or false
};

#endif

