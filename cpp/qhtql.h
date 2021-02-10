#ifndef QUERY_HTQL_H
#define QUERY_HTQL_H

#include "log.h"
#include "expr.h"
#include "referdata.h"
#include "htmlbuf.h"
#include "htqlsyntax.h"
#include <stdio.h>

#define TAGINDEX_MAX	5
class ReferLink;

class HTQLTag{
public:
	enum{tagPLAIN, tagHTML, tagXML }; 
	int TagType;  // enum{tagPLAIN, tagHTML, tagXML }; 
	ReferData S;
	ReferData E;
	unsigned long SourceOffset;

	HTQLTag* copyTag(HTQLTag* From, int newMem=false);

	HTQLTag();
	~HTQLTag();
	void reset();
};

class HTQLItem: public HTQLTag{
public:
	enum {itemSET, itemSCHEMA, itemSCHEMA_DEF, itemSCHEMA_REF };
	int ItemType; //enum {itemSET, itemSCHEMA };
	HTQLItem* NextItem; //links the item set
	HTQLItem* NextField; //links the Fields
	HTQLItem* ParentItem; //links the parent
	ReferData FieldName;

	ReferData Data;
	int isRefNextItem;

	HTQLItem* copyItemData(HTQLItem* item);
	HTQLItem* getSetItem(); //return NULL for itemSCHEMA
	HTQLItem* getFirstSetItem(); //return the first item if there is no Set item
	static int printHtqlItem(HTQLItem* item, FILE* fw = stdout);
	static int dropField(HTQLItem** item, int index1); //index from 1;
	static int mergeField(HTQLItem** item, int index1); //index from 1;
	static int insertField(HTQLItem** item, int index1, HTQLItem* parent); //index from 1;
	static int setFieldData(HTQLItem** item, int index1, char*p, long len, int copy, HTQLItem* parent); //index from 1; copy=true
	static int setFieldName(HTQLItem** item, int index1, char*name, HTQLItem* parent); //index from 1;

	HTQLItem(HTQLItem* parent);
	~HTQLItem();
	void reset();
};
//======= How the results are stored? 
//
//     SSSSSS---------FFFDDD--------FFFDDD   
//        |
//     FFFFFF---------RRRRRR--------DDDDDD
//        |             |
//        |           SSSS--FFDD--FFDD
//        |             |
//        |           FFFF--DDDD--DDDD  
//        |
//        |
//     FFFFFF---------DDDDDD--------DDDDDD   
//
// ==> SSSS: itemSHEMA_DEF -- schema definition
// ==> DDDD: itemSET -- field data 
// ==> FFFF: itemSCHEMA -- point to a record
// ==> RRRR: itemSCHEMA_REF -- point to a relation
// ==> FFDD: itemSET -- field name

class HTQLScope: public HTQLTag{
public:
	int SerialNo;
	HTQLScope* NextTag;
	HTQLScope* PreviousTag;
	HTQLScope* EnclosedTag;

	HTQLScope();
	~HTQLScope();
	void reset();
};


class TagOptions{
public: 
	enum{matNULL, matSTAR, matAT, matMATCH};

	int Sen;
	int PlainIncl;
	int HyperIncl;
	int Iran;
	int Sep;
	int Recur;
	int NoEmbedding;
	int NoIFrame;
	int WildMatching;

	void operator = (TagOptions& option);
	TagOptions();
	~TagOptions();
	void reset();
};

class tRange{
public:
	int From;
	int To;
	int Per;
	static int NONE;
	tRange();
	~tRange();
};

class HTQLFunction{
public:
	int (*FunPrepare)(char* data, void* call_from, ReferData* tx);
	int (*FunComplete)(char* data, void* call_from, ReferData* tx);
	int (*FunItem)(char* data, void* call_from, ReferData* tx);
	int (*FunItemSetPrepare)(char* data, void* call_from, ReferData* tx);
	int (*FunItemSetComplete)(char* data, void* call_from, ReferData* tx);
	int (*FunSchemPrepare)(char* data, void* call_from, ReferData* tx);
	int (*FunSchemComplete)(char* data, void* call_from, ReferData* tx);
	void (*FunFinalRelease)(HTQLFunction* fun);
	ReferData Name;
	ReferData Description;
	void* FunData;

	HTQLFunction();
	~HTQLFunction();
	void reset();
};

class HTQLTagSelection{
public:
	enum{optUNKNOW, optSEN, optINSEN, optINCL, optEXCL, optRECUR, optNORECUR, optIRAN, optORAN, optSEP, optEMBED, optNOEMBED, optIFRAME, optNOIFRAME,optMATCH};
	enum{opUNKNOW, opCONDITION, opTRANSFORM};
	unsigned long SourceOffset;
	ReferData Data; // data based for selection
	ReferData Sentence;
	//int parse();
	int parseSentence();
	int parseData();
	HTQLItem* Results;
	HTQLItem* ParentItem;

	HTQLTagSelSyntax Syntax;
	HTQLTag Tag; // tag for selection
	HTQLTag LastTag; 
	ReferLink* Vars; // variables for conditions
	tStack* ReferVars;
	TagOptions Options;
	ReferLink* Operations;
	tRange* Indx; //Indx[IndxTop]
	int IndxMax; 
	int IndxNum; 
	int setIndxValue(int index0, int from, int to, int per);
	int parseIndxRange1(int index0, int from0_to1_per2); 
	HTQLScope* TagScope;
	HTQLScope* CurrentTagScope;
	int IsReversedTag;

	static int parseSpecial(ReferData* d);
	int parseTagScope();
	int checkIndxRange(long index1, int& inRange, int& inGroup, long total_tuple=0);

	int parseHTMLScope(int include_comment=0);
	static int linkUnclosedTag(char*Source, long length, HTQLScope* start, char* tagname);
	static int linkUnclosedTag(char*Source, long length, HTQLScope* start, char* tagname, char*parent_tagname);
	static int linkUnclosedTableTag(char*Source, long length, HTQLScope* start);
	static int isMatchedHtmlTag(HTQLScope* curr_tag, ReferData* tag_name, int iswildmatching=false); 
	static HTQLScope* searchNextTag(HTQLScope* curr_tag, ReferData* tag_name, int is_html=true, int case_sensitive=false, int noembedding=0, ReferData* last_tagname=0, int noiframe=true, int iswildmatching=false);
	static HTQLScope* searchNextTagNoRecur(HTQLScope* curr_tag, ReferData* tag_name, int tag_type=HTQLTag::tagHTML, int noembedding=0, ReferData* last_tagname=0, int noiframe=true, int iswildmatching=false);
			//noembedding: search the immediate child (tags within last_tagname is ignored
			//noiframe: ignore <iframe> and <noscript> tags that are not explained by IE
	static HTQLScope* searchNextPlainTagNoRecur(HTQLScope* curr_tag, ReferData* tag_name, int tag_type=HTQLTag::tagHTML, TagOptions* options=0, ReferData* last_tagname=0);
	static HTQLScope* searchNextMatchingAt(HTQLScope* curr_tag, ReferData* tag_name);

	int	setVariableValues(HTQLScope* CurrTag);
	static HTQLScope*	nextTag(HTQLScope* CurrTag);
	static HTQLScope*	prevTag(HTQLScope* CurrTag, int keep_last=false);
protected:
	int linkPairTag(HTQLScope* PreviousTag, int isPlain);
	int parsePLAINScope();
	HTQLScope* movetoNextTag();
	int parseAttributes();
	int parsePlainTag();
	int parseHyperTag();
	int parseFilters();
	int getOption(char* name, int len);
	int setTagVal(HTQLScope* CurrTag, HTQLItem* res, int Incl);
	int reverseResultTags();
	int sepResultTags();
	static int dumpHTMLScope(HTQLScope* start, const char* filename);

public:
	HTQLTagSelection();
	~HTQLTagSelection();
	void reset();
};

class HTQLParser{
public:
	enum{ htqlUNKNOW, htqlTAG_SELECTION, htqlHTQL_PARSER };
	ReferData Sentence;
	ReferData SourceData;
	ReferData SourceUrl;
	ReferData CurrentSourceUrl;
	HTQLItem* Data;
	TagOptions Options;
	int QueryPatternNum;
	char* setData(char* data, long len, int copy=false);
	char* resetData();
	char* setSentence(char* sentence, long len, int copy=false);
	char* setSourceUrl(char* url, long len);
	char* getSourceUrl();
	int dotSentence(char* sentence, long len, int copy=false);
	int parse();
	HTQLItem* moveFirst();
	HTQLItem* moveNext();
	int isEOF();
	HTQLItem* movePrev();
	HTQLItem* moveLast();
	int isBOF();
	HTQLItem* getFieldItem(int index1=0);
	char* getField(int index1=0);
	char* getField(const char* name);
	int getFieldsCount();
	long getTuplesCount();
	char* getFieldName(int index1=0);
	int setGlobalVariable(char* name, char* value);
	void resetGlobalVariables();

	HTQLItem* getDataItem(long row, long col); //row, col: index from 1;
	int dropDataItem(long row, long col); //row, col: index from 1;
	int insertDataItem(long row, long col); //row, col: index from 1;
	int setDataItemData(long row, long col, char* p, long len, int copy=true); //row, col: index from 1;
	int setDataItemName(long row, long col, char* name); //row, col: index from 1;
	int dropCurrentRecordItem(long col); //row, col: index from 1;
	int dropCurrentRecord(); //row, col: index from 1;
	int insertCurrentRecordItem(long col); //row, col: index from 1;
	int mergeCurrentRecordItem(long col); //row, col: index from 1;
	int setCurrentRecordItemData(long col, char* p, long len, int copy=true); //row, col: index from 1;
	int setCurrentRecordItemName(long col, char* name); //row, col: index from 1;
	//HTQLItem* getSetItem(HTQLItem* item);

	HTQLSyntax Syntax;
	ReferLink* TagSelections;
	int IsMidResult;

	int printResultData(const char* filename);
	int printResultData(FILE* fw);
	long formatHtmlResult(const char* filename);
 	long formatHtmlResult(FILE* fw);
	int saveSourceData(const char* filename);
	int saveSourceData(FILE* fw);

	/* 
		HTQL function should take char* and void* inputs and returns a char* output.
		The function should release the output memory (if any) itself.
		HTQL will copy the result output to its own space.
	*/
	static int functionHelp(char* data, void* call_from, ReferData* tx);
	static int functionAbout(char* data, void* call_from, ReferData* tx);
	static int functionTX(char* data, void* call_from, ReferData* tx);
	static int functionTxStr(char* data, void* call_from, ReferData* tx);
	static int functionSaveSourceFile(char* data, void* call_from, ReferData* tx);
	static int functionCharReplace(char*p, void*call_from, ReferData* tx);
	static int toUpper(char*p, void* call_from, ReferData* tx);
	static int toLower(char*p, void* call_from, ReferData* tx);
	static int toHtmlPrintable(char*p, void* call_from, ReferData* tx);
	static int functionDecodeHtml(char*p, void* call_from, ReferData* tx);
	static int functionGetUrl(char* tag, void* call_from, ReferData* tx); //does not translate to absolute url, use &get_url &url to get it.
	static int functionEncodeUrl(char*p, void* call_from, ReferData* tx);
	static int functionDecodeUrl(char*p, void* call_from, ReferData* tx);
	static int functionUrl(char* url, void* call_from, ReferData* tx);
	static int functionUrlAll(char* p, void* call_from, ReferData* tx);
	static int functionUrlAllOld(char* p, void* call_from, ReferData* tx);
	static int functionSkipHeader(char*p, void* call_from, ReferData* tx);
	static int functionRemoveHeader(char*p, void* call_from, ReferData* tx);
	static int functionAfter(char*p, void* call_from, ReferData* tx);
	static int functionBefore(char*p, void* call_from, ReferData* tx);
	static int functionTrim(char*p, void* call_from, ReferData* tx);
	static int functionGetNumber(char*p, void* call_from, ReferData* tx);
	static int functionGetDate(char*p, void* call_from, ReferData* tx);
	static int functionGetCsvField(char*p, void* call_from, ReferData* tx);
	static int functionSetSource(char*p, void* call_from, ReferData* tx);
	static int functionTupleCount(char*p, void* call_from, ReferData* tx);//&tuple_count()
	static int functionExprFunction(char*p, void* call_from, ReferData* tx);//any other functions
	static int functionDllFunction(char*p, void* call_from, ReferData* tx);
	tExprCalc ExprContext; //use ExprContext.Context
	int addHtqlFunction(char* fun_name, void* fun, char* description, HTQLFunction** htql_fun=0);
	int addDllFunctions(ReferLink* loadedfuncs, long para);
	int resetHtqlFunctions();
	int resetHtqlFunctionParameters();
	int calculateFunctionParameters(HTQLItem* schema_def, HTQLItem* schema_data);
	HTQLFunction* getHtqlFunction(ReferData* fun_name);
	char* executeFunction(void* fun, HTQLItem* data);
	HTQLItem* FunctionCurrentItem;
	HTQLFunction* CurrentFunction;
	ReferLink* FunctionParameters;
	tStack GlobalVariables;
	static int mergeUrl(char* base, char* url, ReferData* tx);

	int copyRow(HTQLItem* row, HTQLItem** to_addr);
	int copyRowSetItem(HTQLItem* row, HTQLItem** to_addr);
	long copyAllRowsSetItem(HTQLItem* row, HTQLItem** to_addr);

protected:
	ReferData BaseUrl;
	HTQLTag LastTag;
	HTQLItem* Results;
	HTQLItem* MidResults;
	HTQLItem* CurrentRecord;
	ReferLinkHeap HtqlFunctions;

	int parseSentence();
	int parseTagSelection();
	int reverseResultTags();
	int parseSchema();
	int parseSchemaReduction();
	int parseSchemaConstruct();
	int parseSchemaCondition();
	int parseSchemaTransform();
	int parseString();

	int addHtmlQLFunctions();
	int parseFunctions();
	int applyFunctionField(HTQLItem* data,void* fun );
	int applyFunctionItem(HTQLItem* data,void* fun );
	static int readHtqlParameterLong(ReferLink* parameters, int index1, long* val);
	static ReferLink* readHtqlParameter(ReferLink* parameters, int index1, ReferData* val);

	int setExprParam(HTQLItem* schema_def, HTQLItem* schema_data, tExprCalc* expr, HTQLItem** firstfield=0);

	int	reduceDimension(HTQLItem* data, int dimension, char* pat, int pat_len);
	int	reduceDimField(HTQLItem* data, char* pat, int pat_len);
	int	reduceDimItem(HTQLItem* data, char* pat, int pat_len);

	int switchResults();

public:
	HTQLParser();
	virtual ~HTQLParser();
	virtual void reset();
};

#include "htql.h"

#endif


