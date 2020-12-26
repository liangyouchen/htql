//LASTMODIFY CLY19991125

#ifndef FILEDB_H
#define FILEDB_H 

#include "log.h"
#include "recfile.h"
#include "stroper.h"

#include "log.h"

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

//#define DEBUG "debug"
//ERRORLOG=1 log error code, =2 log error sentence, =3 log all sentence
//#define ERRORLOG 		2
//#define ERRORLOGFILE		"txtdb.log"
//#define CASEINSENSITIVE "case insensive for table name and column"
//#define	ZeroFileNotUnlock
#define CreateRowid "Create rowid when creating new tables"

#define InfSuf			".def"
#define DatSuf			".sav"
#define TmpSuf			".tmp"
#define InfFieldSep		":"
#define InfRecordSep	"\n"
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

#ifdef unix
#define DirSepa			'/'
#else
#define DirSepa			'\\'
#endif

#define dbSUCCESS		0
#define dbCREATESUCCESS		1
#define dbDROPSUCCESS		2
#define dbNOTFOUND		100

#if (!defined(ERRORLOG) || (ERRORLOG == 0))
#define dbMEMORYERR		-1
#define dbSYNTAXERR		-2
#define dbLOCKFAIL		-3
#define dbOPENFAIL		-4
#define dbFILEERR		-5
#define dbINDEXERR		-6
#define dbDATAERR		-7
#define dbFIELDNOTFOUND		-8
#define dbSQLNOTSET		-9
#define dbOVERFLOW		-10
#define dbTYPEERR		-11
#define dbFILEWRITE		-12
#define dbFILECLOSE		-13
#define dbFILECOMMIT		-14
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
#define dbNOTSUPPORT		(Log::add(ERRORLOGFILE,-17,"Not supprted keywords.",__LINE__))
#endif

#define strCHAR			"CHAR"
#define strNUMBER		"NUMBER"
#define strDATE			"DATE"
#define strLONG			"LONG"
#define strINDEX		"INDEX"
#define strFIELDS		"FIELDS"

typedef enum {dbOUT,dbSUM,dbAVERAGE} etDBAction ;

#ifndef ETDBTYPE_DEF
#define ETDBTYPE_DEF
typedef enum {dbCHAR,dbNUMBER,dbDATE,dbLONG}etDBTYPE;
#endif

typedef enum {
	synUNKNOW,
	//the following sql keywords are valid
	synCREATE,synDROP,synTABLE,
	synSELECT,synDELETE,synINSERT,synUPDATE,
	synFROM, synINTO,synVALUES,synSET,synWHERE,
	//syntax keywords
	synWORD,synNUMBER,synCHAR,
	synSEMICOLON,synCOMMA,synDOT,
	synRIGHTBRACE,
	//the following standard operations are supported
	synOR,synAND,synNOT,
	synEQ,synNE,synLE,synLT,synGE,synGT,
	synSUB,synADD,synDIV,synASTERISK,
	//the following only synLIKE implemented
	synIN,synMATCHES,synLIKE, synIS,
	//the following fuctions are supported
	synCOUNT,synSUM, synNULL,
	synTO_NUMBER,synTO_CHAR,synTO_DATE,
	synSYSDATE,synADD_MONTHS,synLAST_DAY,synMONTHS_BETWEEN,synNEXT_DAY,
	synLTRIM,synRTRIM,synSUBSTR,
	//
	synLEFTBRACE,
} etSYNTAX;

#ifndef ETEXPR_DEF
#define ETEXPR_DEF
typedef enum {expUNKNOW,expBOOL,expCOMP,expCALC,expCONST,expNODE} etEXPR;
#endif

typedef enum {walkSTOP,walkNEXT,walkRECUR} etWALK;

class tField: public tStrOp{
public:
	char Name[NameLength];
	long Length;
	int Precision;
	etDBTYPE Type;
	char * Value;
	double DoubleValue;
	
	int IsMalloc;
	tField* WalkNext;

	tField();
	tField(char *);
	~tField();
	int cpField(tField*);
	int setType(char *TypeStr);
	int newValue(char *Str, long Len);
	int newValue(long Len);
	int expandValue(long new_len);
	int deleteValue();
} ;

class tIndex{
public:
	char Name[NameLength];
	tField* Field;

	tIndex();
	tIndex(char*);
} ;

class tTable;

class tExpression:public tField{
public:
	etEXPR Brand;
	etSYNTAX CalcOp;
	tField* Parent;
	tField* Left;
	tField* Right;
	tTable* Table;
	int BoolValue;
	int Passed;

	tExpression();
	tExpression(etSYNTAX);
	~tExpression();
	int Calculate();
	int NumCmp(tField *Num1, tField *Num2);
	int DateCmp(tField *Date1, tField *Date2);
	int checkCalcType();
};

class tTable:public tRecFile{
public:
	char FullPath[PathLength];
	char Name[NameLength];
	int IndexNum;
	tIndex* Indexes;
	int FieldNum;
	tField *Fields;
	char Buffer[PathLength];
	tTable* NextTable;
	tExpression* ControlNode;
	int DataOpen;
	etSYNTAX Action;
	tRecFile *TmpFile;
	int RecordCount;

	tTable();
	tTable(char* setName);
	~tTable();
	int setFile(char *setName);
	int openData(etSYNTAX Action);
	int closeData();
	int getRecord();
	int openTmp();
//	int saveToTmp(char *Str=NULL); 
	int saveToTmp();
	int commitTmp();

protected:
	int newIndexes(int);
	int setIndexes();
	int readTable();
} ;

class tSyntax{
public:
	char *Sentence;
	long Start;
	long StartLen;
	int StartPrecision;
	etSYNTAX Type;
	long Next;
	long NextLen;
	int NextPrecision;
	etSYNTAX NextType;

	tSyntax();
	~tSyntax();
	int setSentence(char *);
	int match(int);
	etSYNTAX match();

	etSYNTAX KeyWord();
	int isSpace(char);
	int isAlpha(char);
	int isDigit(char);
};

/////////////////////[ tSQLRequest ]//////////////////////////////////
// the tSQLRequest is a class for general SQL request includeing:
//		SELECT, INSERT, DELETE,UPDATE, CREATE TABLE, DROP TABLE )
// Use it by call of next two examples:
//**Example1
//**		tSQLRequest *s;
//**		s=new tSQLRequest("database path");
//**		int i=s->doSQL("...SQL Sentence...");
//**		if (i<0) { ...error...;}
//**Example2
//**		tSQLRequest *s;
//**		s=new tSQLRequest("database path");
//**		int i=s->setRequest("...SQL Sentence...");
//**		while (i==0){
//**			i=s->doRequest();
//**			printf("%s", s->Value);
//**			if (i<0) { ...error...;}
//**		};
// ///////////////////////////////////////////////////////////////////
class tSQLRequest:public tField{
public:
	char OutFdSep[NameLength];	// separator fields when printing out value
	char OutRdSep[NameLength];	// separator recored when printing out value
	int OutFieldNum;		//count of selected fields for printing out.
	tExpression* OutFields;	//Out Fields connected as: OutFields->WalkNext->WalkNext->...
	int TableNum;			//tables number
	tTable* Tables;			//tables connected as: Tables->NextTable->NextTable->...
	int Flag;				//values: dbSUCCESS, dbNOTFOUND, dbFILEERR, ...
	tSyntax *Sql;			
	etSYNTAX Action;		//values: synSELECT, synDELETE, synINSERT, synUPDATE
	int IsSubRequest;		//values: True, False
	int GroupAction;		//values: 0:no group, 1:sum/count, 

	etWALK WalkStatus;
	tExpression* Root;
	tExpression* FirstStep;
	tExpression* CurrentStep;
	tExpression* FirstNodeTab;
	tExpression* LastNodeTab;
	tTable* CurrentTable;

	char* SelectResults;
public:
	char DataBase[PathLength];	// database full path

	tSQLRequest(char *SetDatabase=NULL);
	~tSQLRequest();
	int setDatabase(char *SetDatabase);
	long doSQL(char * S, char **Result);
	long doSQL(char * S);
	int setRequest(char *S);
	int doRequest();

protected:
	void initData();
	void clearData();
	int doOneRequest();
	int setCurrentStep(int Next=False);
	int doCommit();
	int parseSql();
	int parseSelect();
	int parseDelete();
	int parseUpdate();
	int parseInsert();
	int parseCreate();
	int parseDrop();
	int printOut();
	int setRowid(tField* Field);
	int parseExpr(etSYNTAX PreOp, tExpression **Result,tExpression* SetParent=NULL);
	int parseField(tExpression **Result);
	int parseTableList();
	int parseFieldList();
	int addAllFieldsOut(tTable*);
	int setInsertValue();
	int setUpdateValue();
	int spotTable(char *TableName, tExpression* SpotField);
	int spotNode(tExpression *WalkStart);
	int PostWalk(tExpression*WalkStart, tField ***PreWalk);
};
/*
class tSQLCreate{
public:
	tSQLCreate(char* Path=NULL);
	tSyntax *Sql;
	tRecFile WriteFile;
	int setPath(char *Path);
	int parseCreate();
	char TableName[NameLength];
};
*/
class Frmem{
	char* SavedPointer;
public:
	Frmem();
	~Frmem();
	void setP(char *);
};

#endif
