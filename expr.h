#ifndef CLY_EXPR_H
#define CLY_EXPR_H

#include "log.h"
#include "qlsyntax.h"
#include "stroper.h"
#include "referdata.h"
#include "referlink.h"

#define Epsilon			(1.0e-6)
#define DateStandardFormat "YYYY/MM/DD HH24:MI:SS"
#ifdef DateStandardFormat 
#ifndef	DATEFORMAT_DEF
#define DATEFORMAT_DEF
	static char DateFormat[100]=DateStandardFormat;
#endif
#endif
#define SumAddLength	5
#define AddAddLength	0
#define FmtAddLength	30

#define dbSUCCESS		0

#if (!defined(ERRORLOG) || (ERRORLOG == 0))
#define dbMEMORYERR		-1
#define dbSYNTAXERR		-2
#define dbLOCKFAIL		-3
#define dbOPENFAIL		-4
#define dbFILEERR		-5
#define dbINDEXERR		-6
#define dbDATAERR		-7
#define dbFIELDNOTFOUND	-8
#define dbSQLNOTSET		-9
#define dbOVERFLOW		-10
#define dbTYPEERR		-11
#define dbFILEWRITE		-12
#define dbFILECLOSE		-13
#define dbFILECOMMIT	-14
#define dbDATEFORMAT		-15
#define dbBRACEERR			-16
#define dbNOTSUPPORT		-17

#else

#define dbMEMORYERR		(Log::add(ERRORLOGFILE,-1,"Memory error.",0))
#define dbSYNTAXERR		(Log::add(ERRORLOGFILE,-2,"Syntax error.",__LINE__))
#define dbLOCKFAIL		(Log::add(ERRORLOGFILE,-3,"Lock fail error.",__LINE__))
#define dbOPENFAIL		(Log::add(ERRORLOGFILE,-4,"File open fail.",__LINE__))
#define dbFILEERR		(Log::add(ERRORLOGFILE,-5,"File read error.",__LINE__))
#define dbINDEXERR		(Log::add(ERRORLOGFILE,-6,"Table define error.",__LINE__))
#define dbDATAERR		(Log::add(ERRORLOGFILE,-7,"Table data error.",__LINE__))
#define dbFIELDNOTFOUND		(Log::add(ERRORLOGFILE,-8,"Field not exist.",__LINE__))
#define dbSQLNOTSET		(Log::add(ERRORLOGFILE,-9,"Request not set.",__LINE__))
#define dbOVERFLOW		(Log::add(ERRORLOGFILE,-10,"Overflow.",__LINE__))
#define dbTYPEERR		(Log::add(ERRORLOGFILE,-11,"Type error.",__LINE__))
#define dbFILEWRITE		(Log::add(ERRORLOGFILE,-12,"File write error.",__LINE__))
#define dbFILECLOSE		(Log::add(ERRORLOGFILE,-13,"File close error.",__LINE__))
#define dbFILECOMMIT		(Log::add(ERRORLOGFILE,-14,"Data write back error.",__LINE__))
#define dbDATEFORMAT		(Log::add(ERRORLOGFILE,-15,"Date format error.",__LINE__))
#define dbBRACEERR		(Log::add(ERRORLOGFILE,-16,"Left-right parentheses not match.",__LINE__))
#define dbNOTSUPPORT		(Log::add(ERRORLOGFILE,-17,"Unsupported keywords.",__LINE__))

#endif

#define strCHAR			"CHAR"
#define strNUMBER		"NUMBER"
#define strDATE			"DATE"
#define strLONG			"LONG"
#define strINDEX		"INDEX"
#define strFIELDS		"FIELDS"


#ifndef NameLength
#define	NameLength		80
#endif
#define True			1
#define False			0

#define	MaxTmpLength		400
#define	LOCKTRY			200

#define	LongLength		4000
#define	DateLength		50
#define NumberDefLength	30
#define NumberDefPrecision	6

#ifndef ETDBTYPE_DEF
#define ETDBTYPE_DEF
typedef enum {dbCHAR,dbNUMBER,dbDATE,dbLONG,dbOBJECT}etDBTYPE;
#endif

#ifndef ETEXPR_DEF
#define ETEXPR_DEF
typedef enum {expUNKNOW,expBOOL,expCOMP,expCALC,expCONST,expNODE} etEXPR;
#endif

#define preEXP_CSTDATE		'#'
#define preEXP_DATE		'@'
#define preEXP_NUMBER	'#'
#define preEXP_CHAR		'$'
#define preEXP_PARA		'%'
#define preEXP_TYPE_PREFIX	"@#$%"
#define preEXP_WORD_PREFIX	"\"["

typedef enum {
	synEXP_UNKNOW, synEXP_END,
	//syntax keywords
	synEXP_WORD=100,synEXP_NUMBER,synEXP_CHAR, synEXP_DATE,
	synEXP_SEMICOLON,synEXP_COMMA,synEXP_DOT, synEXP_ARROW, 
	synEXP_RIGHTBRACE,
	//the following standard operations are supported
	synEXP_OR,synEXP_AND,synEXP_NOT,
	synEXP_EQ,synEXP_NE,synEXP_LE,synEXP_LT,synEXP_GE,synEXP_GT, 
	syn_REGEX_MATCH, syn_REGEX_SEARCH, syn_REGEX_MATCH_CASE, syn_REGEX_SEARCH_CASE, 
	synEXP_SUB,synEXP_ADD,synEXP_DIV,synEXP_ASTERISK, synEXP_NULL_OR, synEXP_MOD,
	synEXP_QUESTION, synEXP_COLON, synEXP_EXCLAIMER,
	//the following only synLIKE implemented
	synEXP_IN,synEXP_MATCHES,synEXP_LIKE, synEXP_IS, 
	synEXP_STRCAT,	
	//the following fuctions are supported
	synEXP_COUNT,synEXP_SUM, synEXP_NULL,
	synEXP_TO_NUMBER,synEXP_TO_CHAR,synEXP_TO_DATE,
	synEXP_SYSDATE,synEXP_ADD_MONTHS,synEXP_LAST_DAY,synEXP_MONTHS_BETWEEN,synEXP_NEXT_DAY,
	synEXP_LTRIM,synEXP_RTRIM, 
	//
	synEXP_LEFTBRACE,
} etEXP_SYNTAX;


class ExprSyntax: public QLSyntax{
public:
	virtual void findNext();
	virtual int KeyWord();
	virtual int takeSyntaxString(int type, long start, long len, ReferData* result);
};

class tExprField: public tStrOp{
public:
	char* Name;
	ReferData StringName;
	ReferData NamePrefix; //It includes '.' or '->'; Use ExprSyntax to parse for exact names
	long Length;
	int Precision;
	etDBTYPE Type;
	char * Value;
	ReferData StringValue;
	double DoubleValue;
	int BoolValue;
	void* FunValue;
	ReferData RefName;
	
	tExprField* WalkNext;

	tExprField();
	tExprField(char *name);
	~tExprField();
	void reset();
	int cpField(tExprField*);
	int setType(char *TypeStr);
	int newValue(char *Str, long Len, int copy=true);
	int newValue(long Len);
	int newDoubleValue(double val, int len, int precision);
	int newDateValue(time_t t);
	int deleteValue();
	int setName(char* name, long len, int copy=true);
	int setName(char* name);
	int getNameStartEnd(const char* name, long* i, long* len); 
} ;

class tExprCalc;

class tExprItem:public tExprField{
public:
	etEXPR Brand;
	etEXP_SYNTAX CalcOp;
	tExprField* Parent;
	tExprField* Left;
	tExprField* Right;
	int Passed;
	int IsNewGroup; 
	long Info;

	tExprItem();
	tExprItem(int);
	~tExprItem();
	void reset();
	int Calculate(tExprCalc* calc=0, int set_group_base=false);
	int NumCmp(tExprField *Num1, tExprField *Num2);
	int DateCmp(tExprField *Date1, tExprField *Date2);
	int checkCalcType();
	int checkNumber(int* is_digit, int* is_const); 
};

class RegExParser; 

class tExprCalc:public tExprField{
public:
	tExprItem* Root;
	tExprItem* FirstStep;
	tExprItem* CurrentStep;
	ExprSyntax* ExprSentence;

	int IsNameCaseSensitive;
	int HaveGroupFunction;

	//use context of functions, fields, and constances
	tExprCalc* Context;
	int useContext(tExprCalc* context_calc, int clear_contextvars=false);
	ReferLink* findFunctionName(ReferData* func_name, ReferData* class_name=0, tExprItem*item=0, tExprCalc* calc=0); //may contain prefix
	ReferLink* findVariableName(ReferData* var_name); //may contain prefix

	RegExParser* RegExContext; 
	tExprCalc* FunctionContext; //additional function context

	ReferLinkHeap Variables;  //constant variables; use Context->Variables;
	ReferLink* setVariable(const char*name, const char* value, long len=0, int copy=true);

	tExprField* Fields;	//Unkown fields, a link list; use Context->FieldsList
	int FieldsNum;
	tExprField** FieldsList; //an array pointing to fields

	ReferData AssignName; //To assign computed expr to this variable name

	ReferLinkHeap UnkownFunctions; //Unkown functions; Name: function name; Value: prefix
								   //register to the external functions in Context directly, or set the Data a function pointer
	/*tExprField* Functions; //Unkown functions, a link list; use Context->FunctionsList
	tExprField** FunctionsList; //an array pointing to Functions
	int FunctionsNum;
	*/

	ReferLinkHeap InternalFunctions; //existing functions
	ReferLink* addInternalFunction(const char* fun_name, int (*fun)(tExprCalc*, tExprItem*, void*), void* p);
	ReferLinkHeap RegisteredFunctions; //internal functions; use Context->registerFunction();
	ReferLink* registerFunction(const char* fun_name, int (*fun)(tExprCalc*, tExprItem*, void*), void* p);
	long addDllFunctions(ReferLink* loadedfuncs, long para);
	static int getFuncParaTotal(tExprItem*item);
	static int getFuncParaIndex(tExprItem*item);
	static char* getFuncParaValue(tExprItem*item, int index1);
	static long getFuncParaLong(tExprItem*item, int index1);
	static double getFuncParaDouble(tExprItem*item, int index1);
	static tExprItem* getFuncParaItem(tExprItem*item, int index1);
	static char* getFuncName(tExprItem*item); 
	int getPrefixName(const char* prefix, int index0, ReferData* name);

	ReferLinkHeap ObjectFunctions; // Name: class_name, Value.P: func para, Data: ReferLink* (*func)(tExprCalc*calc, ReferData*func_name, ReferData*class_name, void*p)
	ReferLink* findObjectFunction(ReferData* func_name, ReferData* class_name, tExprItem*item, tExprCalc* calc); //may contain prefix
	ReferLink* registerObjectFunctionClass(const char* class_name, ReferLink* (*fun)(tExprCalc*, tExprItem*, ReferData*, ReferData*, void*), void* p);


	tExprCalc(tExprCalc* context=0);
	~tExprCalc();
	void reset();
	int setExpression(char* Sentence, long *Len=0);
	int parse(etEXP_SYNTAX PreOp=synEXP_LEFTBRACE, int built_varlist=true); //synEXP_LEFTBRACE: single item in (); synEXP_RIGHTBRACE: whole expression
	int buildVarsList();	//built FunctionsList and FieldsList in !Context;
	int clearContextVars(); //clear Functions and Fields in !Context;
	int takeContextVars(tExprCalc* from);
	int cmpFieldName(const char* field_name, const char* test_name);
	int setField(int Index, char* Value, long Length=-1, int copy=true); //not in Context?
	int setField(char* Name, char* Value, long Length=-1, int copy=true); //in Context
	tExprField* getField(char* Name); //in Context
	int calculate(int initial_group=false);
	char* getString();
	int getBoolean();
	double getDouble();
	long getLong();

	int parseExpr(etEXP_SYNTAX PreOp, tExprItem **Result,tExprItem* SetParent=NULL);
	int parseField(tExprItem **Result);
	int addNewField(tExprItem *new_field);
	int spotNode(tExprItem *WalkStart);
	int PostWalk(tExprItem*WalkStart, tExprField ***PreWalk);
	static int getExprErrorMsg(int err, ReferData* errormsg); 

	enum {walkPOST_ORDER, walkPRIOR_ORDER, walkMID_ORDER };
	int checkBranch(void* p, tExprItem* CheckStart, int (*CheckFunc)(void* p, tExprItem* CheckItem), int WalkOrder=walkPOST_ORDER);
	static int reformulateExpression(tExprItem* expr_item, int (*node_transform)(tExprItem*, ReferData*, void*), ReferData* condition, void* p);
	static int checkAssignValueExpression(tExprCalc* expr, tExprItem* expr_item, ReferLinkHeap* assign_names);
	static int checkConstValueExpression(tExprCalc* expr, tExprItem* expr_item, int* is_const);

public:
	static int functionDllFunction(tExprCalc*calc, tExprItem*item, void* p); //ReferLink*p; p->Name:name; p->Value.P:func, p->Value.L:n para
	//group functions
	static int functionCount(tExprCalc*calc, tExprItem*item, void* p);	//count(*), count(a,b,c...);
	static int functionSum(tExprCalc*calc, tExprItem*item, void* p);	//Sum(field), Sum(a,b,c...);
	static int functionMax(tExprCalc*calc, tExprItem*item, void* p);	//Max(field), Max(a,b,c...);
	static int functionMin(tExprCalc*calc, tExprItem*item, void* p);	//Min(field), Min(a,b,c...);
	//string functions
	static int functionSubstr(tExprCalc*calc, tExprItem*item, void* p);	//substr(str, [pos0], [len])
	static int functionLtrim(tExprCalc*calc, tExprItem*item, void* p);	//ltrim(str, [pattern])
	static int functionRtrim(tExprCalc*calc, tExprItem*item, void* p);	//rtrim(str, [pattern])
	static int functionStrcat(tExprCalc*calc, tExprItem*item, void* p);	//strcat(str1, str2)
	static int functionStrcmp(tExprCalc*calc, tExprItem*item, void* p);	//strcat(str1, str2)
	static int functionStrcmpi(tExprCalc*calc, tExprItem*item, void* p);	//strcat(str1, str2)
	static int functionStrlen(tExprCalc*calc, tExprItem*item, void* p); //strlen(str)
	static int functionStrFind(tExprCalc*calc, tExprItem*item, void* p); //strlen(str)
	static int functionReplace(tExprCalc*calc, tExprItem*item, void* p); //replace(str, 'text', 'new')
	static int functionLower(tExprCalc*calc, tExprItem*item, void* p); //lower(str)
	static int functionUpper(tExprCalc*calc, tExprItem*item, void* p); //upper(str)
	static int functionFormat(tExprCalc*calc, tExprItem*item, void* p); //format(fmt, ...)
	static int functionGetEmails(tExprCalc*calc, tExprItem*item, void* p); //get_email(text, fmt)
	static int functionPackNull(tExprCalc*calc, tExprItem*item, void* p); //packnull(text)
	//boolean functions
	static int functionIsChar(tExprCalc*calc, tExprItem*item, void* p);	//ischar(str)
	static int functionIsNumber(tExprCalc*calc, tExprItem*item, void* p);	//isnumber(str)
	static int functionIsBlank(tExprCalc*calc, tExprItem*item, void* p);	//isblank(str)
	static int functionIsPhoneNumber(tExprCalc*calc, tExprItem*item, void* p);	//isphonenumber(str)
	//date functions
	static time_t getDateFromCharType(tExprField*item);
	static int functionTime(tExprCalc*calc, tExprItem*item, void* p);	//time(t)
	static int functionDate(tExprCalc*calc, tExprItem*item, void* p);	//date(d)
	static int functionToDate(tExprCalc*calc, tExprItem*item, void* p);	//to_date(str, fmt)
	static int functionToNumber(tExprCalc*calc, tExprItem*item, void* p);	//to_number(str)
	static int functionToChar(tExprCalc*calc, tExprItem*item, void* p);	//to_char(date, fmt)
	static int functionMonthsBetween(tExprCalc*calc, tExprItem*item, void* p);	//months_between(date1, date2)
	static int functionAddMonths(tExprCalc*calc, tExprItem*item, void* p);	//add_months(date, n)
	static int functionLastDay(tExprCalc*calc, tExprItem*item, void* p);	//last_day(date)
	static int functionNextDay(tExprCalc*calc, tExprItem*item, void* p);	//next_day(date, wday)
	static int functionDateDiff(tExprCalc*calc, tExprItem*item, void* p);	//DateDiff(interval, date1, date2); //, [firstdayofweek], [firstweekofyear])
	//encode and decode functions
	static int functionHtmlEncode(tExprCalc*calc, tExprItem*item, void* p);	//html_encode(str)
	static int functionHtmlDecode(tExprCalc*calc, tExprItem*item, void* p);	//html_decode(str)
	static int functionUrlEncode(tExprCalc*calc, tExprItem*item, void* p);	//url_encode(str)
	static int functionUrlDecode(tExprCalc*calc, tExprItem*item, void* p);	//url_decode(str)
	static int functionUTF8Encode(tExprCalc*calc, tExprItem*item, void* p); //utf8_encode(text)
	//numeric function
	static int functionRand(tExprCalc*calc, tExprItem*item, void* p);	//rand()
	static int functionSRand(tExprCalc*calc, tExprItem*item, void* p);	//srand()
	static int functionRound(tExprCalc*calc, tExprItem*item, void* p);	//round(f)
	static int functionFloor(tExprCalc*calc, tExprItem*item, void* p);	//floor(f)
	static int functionCeil(tExprCalc*calc, tExprItem*item, void* p);	//ceil(f)
	static int functionAbs(tExprCalc*calc, tExprItem*item, void* p);	//abs(f)
	//file functions?
	static int functionReadFile(tExprCalc*calc, tExprItem*item, void* p);
					//ReadFile(FileName);
	static int functionSaveFile(tExprCalc*calc, tExprItem*item, void* p);
					//SaveFile(TextData, FileName);
	static int functionRenameFile(tExprCalc*calc, tExprItem*item, void* p);
					//RenameFile(FileName1, FileName2);
	static int functionDeleteFile(tExprCalc*calc, tExprItem*item, void* p);
					//DeleteFile(FileName);
	static int functionMkDir(tExprCalc*calc, tExprItem*item, void* p);
					//Mkdir(DirName);
	static int functionAppendFile(tExprCalc*calc, tExprItem*item, void* p);
					//AppendFile(TextData, FileName);
	static int functionGetFilePath(tExprCalc*calc, tExprItem*item, void* p);	
					//GetFilePath(filename)
	static int functionGetFileName(tExprCalc*calc, tExprItem*item, void* p);	
					//GetFileName(filename)
	static int functionTempPath(tExprCalc*calc, tExprItem*item, void* p);	
					//TempPath()
	
	//other functions
	static int functionTextWords(tExprCalc*calc, tExprItem*item, void* p); //text_words(str); count the number of words
	static int functionStringComp(tExprCalc*calc, tExprItem*item, void* p); //string_comp(str1, str2, comp_type, option); compare two strings
	static int functionHtql(tExprCalc*calc, tExprItem*item, void* p); //htql(source, query, url)
	static int functionMatchLocalScore(tExprCalc*calc, tExprItem*item, void* p); //match_local_score(str1, str2, match_type)
	static int functionGetNumber(tExprCalc*calc, tExprItem*item, void* p); //get_number(str, index1)
};


#endif
