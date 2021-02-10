// LASTMODIFY CLY19991213
#include "filedb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <malloc.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "stroper.h"
#ifdef unix
#include <unistd.h>
#endif

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


tField::tField(){
	Name[0]='\0';
	Length=0;
	Precision=0;
	Type=dbCHAR;
	Value=NULL;
	DoubleValue=0.0;
	IsMalloc=False;
	WalkNext=NULL;
}
tField::tField(char *setName){
	strcpy(Name,setName);
	Length=0;
	Precision=0;
	Type=dbCHAR;
	Value=NULL;
	DoubleValue=0.0;
	IsMalloc=False;
	WalkNext=NULL;
}

tField::~tField(){
	deleteValue();
}

int tField::cpField(tField* from){
	strcpy(Name,from->Name);
	Length=from->Length;
	Precision=from->Precision;
	Type=from->Type;
	DoubleValue=0.0;
	return dbSUCCESS;
}

int tField::setType(char *TypeStr){
	int i;

	if (strcmp(TypeStr,strLONG)==0){
		Length=LongLength;
		Precision=0;
		Type=dbLONG;
		newValue(Length);
		return dbSUCCESS;
	}
	if (strcmp(TypeStr,strDATE)==0){
		Length=DateLength;
		Precision=0;
		Type=dbDATE;
		newValue(Length);
		return dbSUCCESS;
	}
	if (strncmp(TypeStr,strNUMBER,6)==0){
		Length=0;
		Precision=0;
		i=sscanf(TypeStr+7,"%d,%d",&Length,&Precision);
		if ((i!=1)&&(i!=2)) return dbINDEXERR;
		if (i==1) Precision=0;
		Type=dbNUMBER;
		newValue(Length);
		return dbSUCCESS;
	}
	if (strncmp(TypeStr,strCHAR,4)==0){
		Length=0;
		i=sscanf(TypeStr+5,"%d",&Length);
		if (i!=1) return dbINDEXERR;
		Precision=0;
		Type=dbCHAR;
		newValue(Length);
		return dbSUCCESS;
	}

	return dbINDEXERR;
}

int tField::newValue(char *Str, long Len){
	deleteValue();
	if ((Value=(char *)malloc(sizeof(char)*(Len+1)))==NULL) return dbMEMORYERR;
	IsMalloc=True;
	strncpy(Value,Str,Len);
	Value[Len]='\0';
	Length=Len;
	return dbSUCCESS;
}

int tField::newValue(long Len){
	deleteValue();
	if ((Value=(char *)malloc(sizeof(char)*(Len+1)))==NULL) return dbMEMORYERR;
	IsMalloc=True;
	Value[0]='\0';
	Length=Len;
	return dbSUCCESS;
}

int tField::expandValue(long new_len){
	char* v=Value;
	int m=IsMalloc;
	long l=Length;
	IsMalloc=False;
	newValue(new_len);
	if (v) memcpy(Value, v, l+1);
	if (m) free(v);
	return dbSUCCESS;
}

int tField::deleteValue(){
	if (IsMalloc){
		IsMalloc=False;
		if (Value) free(Value);
		Value=NULL;
	}
	return dbSUCCESS;
}

tIndex::tIndex(char *setName){
	strcpy(Name,setName);
	Field=NULL;
}

tTable::tTable(){
	FullPath[0]='\0';
	Name[0]='\0';
	IndexNum=0;
	Indexes=NULL;
	FieldNum=0;
	Fields=NULL;
	Buffer[0]='\0';
	NextTable=NULL;
	ControlNode=NULL;
	DataOpen=False;
	Action=synUNKNOW;
	TmpFile=NULL;
	RecordCount=0;

	DoReadLock=False;
	DoWriteLock=True;
}

tTable::tTable(char *setName){
	FullPath[0]='\0';
	Name[0]='\0';
	IndexNum=0;
	Indexes=NULL;
	FieldNum=0;
	Fields=NULL;
	Buffer[0]='\0';
	NextTable=NULL;
	ControlNode=NULL;
	DataOpen=False;
	Action=synUNKNOW;
	TmpFile=NULL;
	RecordCount=0;

	DoReadLock=False;
	DoWriteLock=True;

	setFile(setName);
}

tTable::~tTable(){
	if (Indexes!=NULL) free(Indexes) ;
	tField *NowField=Fields,*NextField;
	while (NowField!=NULL) {
		NextField=NowField->WalkNext;
		delete NowField;
		NowField=NextField;
	}
	if (NextTable!=NULL) delete NextTable;
	NextTable=NULL;
	if (TmpFile) delete TmpFile;
	TmpFile=NULL;
}

int tTable::openData(etSYNTAX Action){
	if (DataOpen) return dbOPENFAIL;
	char tmp[PathLength];
	int i;
	strcpy(tmp,FullPath);
	strcat(tmp,DatSuf);
	switch (Action) {
	case synSELECT:
		if ((i=openRead(tmp))!=dbSUCCESS) return i;
		break;
	case synDELETE:
	case synUPDATE:
	case synINSERT:
		if ((i=openWrite(tmp))!=dbSUCCESS) return i;
		break;
	default:
		return dbOPENFAIL;
	}
	DataOpen=True;
	return dbSUCCESS;
}

int tTable::closeData(){
	int i=closeFile();
	DataOpen=False;
	return i;
}

int tTable::getRecord(){
	int i;
	if (!DataOpen){
		if ((i=openData(Action))!=dbSUCCESS) return i;
		RecordCount=0;
	}
	if (EndFile){
		if ((i=reOpen())!=dbSUCCESS) return i;
		RecordCount=0;
	}
	tField *NowField;
	if ((i=readRecord())<0) return  i;
	for (i=0,NowField=Fields;(i<FieldNum) && (NowField!=NULL) && !EndFile;i++,NowField=NowField->WalkNext){
//		if (EndFile) break;
		if (getRecordField(i)) {
			if (NowField->Type == dbLONG && NowField->Length<CurrentField.DataLen){
				NowField->newValue(CurrentField.DataLen);
			}
			memcpy(NowField->Value,CurrentField.Data, CurrentField.DataLen);
			NowField->Value[CurrentField.DataLen]='\0';
		}
	}
	RecordCount++;
#ifdef DEBUG
	Log::add(ERRORLOGFILE,0,Fields->Value,__LINE__);
#endif
	if (i<FieldNum || EndFile) return dbSUCCESS;
	return dbSUCCESS;
}

int tTable::openTmp(){
	int i;
	if (!TmpFile) {
		TmpFile=new tRecFile;
		strcpy(Buffer,FullPath);
		strcat(Buffer,TmpSuf);
		TmpFile->openWrite(Buffer);
		if ((i=TmpFile->startWrite())!=dbSUCCESS) return i;
	}
	return dbSUCCESS;
}

int tTable::saveToTmp(){ 
	int i,j;
	if (!TmpFile) {
		if ((i=openTmp())!=dbSUCCESS) return i;
	}
	if ((j=TmpFile->writeRecord(CurrentRecord.Data, CurrentRecord.DataLen))!=dbSUCCESS) return j;
	return dbSUCCESS;
}

int tTable::commitTmp(){
	int i;
	if (!TmpFile && (i=openTmp())!=dbSUCCESS) return i;
	
	if ((i=startWrite())!=dbSUCCESS) return i;
	if (TmpFile){
		if (TmpFile->FileHandle>=0) 
			if (close(TmpFile->FileHandle)!=0) return dbFILECLOSE;
		if (FileHandle>=0) if (close(FileHandle)!=0) return dbFILECLOSE;
		FileHandle=-1;
		tRecFile::UnlinkFile(OpFileName);
		if (rename(TmpFile->OpFileName,OpFileName)!=0){
			usleep(5000);
			tRecFile::UnlinkFile(OpFileName);
			if (rename(TmpFile->OpFileName,OpFileName)!=0)
				return dbFILECOMMIT;
		}
		if ((TmpFile->FileHandle=open(TmpFile->OpFileName, O_WRONLY|O_CREAT|O_EXCL,0666))<0) return dbOPENFAIL;
		if ((i=TmpFile->closeFile())!=dbSUCCESS) return i;
	}
	return dbSUCCESS;
}

int tTable::newIndexes(int Num){
	IndexNum=Num;
	if (Indexes!=NULL) free(Indexes);
	if ((Indexes=(tIndex *)malloc(sizeof(tIndex)* IndexNum))==NULL) return dbMEMORYERR;
	return dbSUCCESS;
}

int tTable::setIndexes(){
	tField* NowField;
	int i,j;
	for (i=0;i<IndexNum;i++){
		for (j=0,NowField=Fields;(j<FieldNum) && (NowField!=NULL);j++,NowField=NowField->WalkNext){
			if (strcmp((Indexes+i)->Name,NowField->Name)==0){
				(Indexes+i)->Field=NowField;
				break;
			}
		}
		if (j==FieldNum||NowField==NULL) return dbINDEXERR;
	}
	return dbSUCCESS;
}

int tTable::setFile(char* setName){
	//set table fullpath and table name
	int i;
	strcpy(FullPath,setName);
//19990818MODIFY for directory with character '.'
	for (i=strlen(FullPath)-1; (i>=0) &&(*(FullPath+i)!='.');i--){
		if (FullPath[i] == '\\' || FullPath[i] == '/'){
			i=-1;
			break;
		}
	}
	if (i>=0) *(FullPath+i)='\0';
	for (i=strlen(FullPath)-1; i>=0 && FullPath[i]!='\\' && FullPath[i]!='/';i--);
	strcpy(Name,FullPath+1+i);

	//open table discript file and set tTable structure
	char tmp[PathLength];
	strcpy( tmp,FullPath);
	strcat( tmp,InfSuf);

	if ((i=openRead(tmp))!=dbSUCCESS) return i;
	strcpy(RecordSep, InfRecordSep);
	strcpy(FieldSep,InfFieldSep);
	if ((i=readTable())!=dbSUCCESS) return i;
	strcpy(RecordSep, RECORD_SEP);
	strcpy(FieldSep,FIELD_SEP);
	if ((i=closeFile())!=dbSUCCESS) return i;
	return dbSUCCESS;
}

int tTable::readTable(){
	char TypeTmp[PathLength];
	int i,j;

	// build Indexes
	
	while (readRecord()>0){
		if (!getRecordField(0)) return dbINDEXERR;
		if (strcmp(CurrentField.Data,strFIELDS)==0 ) break;
		if (strcmp(CurrentField.Data,strINDEX)==0) {
			if (getRecordField(1)) 
			IndexNum=tStrOp::countChar(CurrentField.Data,',')+1;
			if ((i=newIndexes(IndexNum))!=dbSUCCESS) return i;
			for (i=0;i<IndexNum;i++)
				tStrOp::SubString(CurrentField.Data,',',i,(Indexes+i)->Name);
		} else
			return dbINDEXERR;
	}

	if (!CurrentField.Data||strcmp(CurrentField.Data,strFIELDS)!=0) return dbINDEXERR;
	//build Fields
	if (Fields) delete Fields;
	Fields=NULL;
	tField **NewField=&Fields;
	while (readRecord()>0){
		if (!getRecordField(0)) return dbINDEXERR;
		(*NewField)=new tField;
		strcpy((*NewField)->Name,CurrentField.Data);
#ifdef CASEINSENSITIVE
		for (char* c=(*NewField)->Name; *c!='\0';c++) *c=toupper(*c);
#endif
		if (!getRecordField(1)) return dbINDEXERR;
		strcpy(TypeTmp,CurrentField.Data);
		if ((*NewField)->Name[0]=='\0') break;
		if ((j=(*NewField)->setType(TypeTmp))!=dbSUCCESS) return j;
		FieldNum++;
		NewField=&(*NewField)->WalkNext;
	}
	//set Indexes
	if ((i=setIndexes())!=dbSUCCESS) return i;

	return dbSUCCESS;
}

tSyntax::tSyntax(){
	Sentence=NULL;
	StartLen=0;
	Start=0;
	StartPrecision=0;
	Type=synUNKNOW;
	Next=0;
	NextLen=0;
	NextPrecision=0;
	NextType=synUNKNOW;
}

tSyntax::~tSyntax(){
	if (Sentence!=NULL) free(Sentence);
}

int tSyntax::setSentence(char* S){
	int i=strlen(S)+9;
	if ((Sentence=(char *)malloc(sizeof(char)* i))==NULL)return dbMEMORYERR;
	strcpy(Sentence,S);
#if (ERRORLOG > 2)
	Log::add(ERRORLOGFILE,0,Sentence,__LINE__);
#endif
	strcat(Sentence,";;");
	match(); match();
	return dbSUCCESS;
}

int tSyntax::match(int ExpectType){
	if (Type!=ExpectType)
		return dbSYNTAXERR;
	match();
	return dbSUCCESS;
}

etSYNTAX tSyntax::match(){
	etSYNTAX PreviousType=Type;
	Start=Next;
	StartLen=NextLen;
	StartPrecision=NextPrecision;
	Type=NextType;
	int i;

	Next+=NextLen;
	while (isSpace(Sentence[Next])) Next++;
	NextPrecision=0;
	NextLen=1;
	if (isAlpha(Sentence[Next])){
#ifdef CASEINSENSITIVE
		Sentence[Next]=toupper(Sentence[Next]);
#endif
		while (isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen])){
#ifdef CASEINSENSITIVE
			Sentence[Next+NextLen]=toupper(Sentence[Next+NextLen]);
#endif
			NextLen++;
		}
		NextType=synWORD;
		NextType=KeyWord();
	}else if (isDigit(Sentence[Next])){
		while (isDigit(Sentence[Next+NextLen])||(Sentence[Next+NextLen])=='.') NextLen++;
		NextType=synNUMBER;
	}else{
		switch (Sentence[Next]){
//MODIFY 19991006 for double "'"; 
		case '\'':
			while (1){
				if (Sentence[Next+NextLen]!='\'' && Sentence[Next+NextLen]!='\0') NextLen++;
				else if (Sentence[Next+NextLen]=='\'' && Sentence[Next+NextLen+1]=='\''){
					for (i=Next+NextLen+1; Sentence[i]!='\0'; i++) Sentence[i]=Sentence[i+1];
					NextLen++;
				}
				else break;
			}
			if (Sentence[Next+NextLen]=='\'') {
				NextLen++;
				NextType=synCHAR;
			}else NextType=synUNKNOW;
			break;
		case '<':
			switch (Sentence[Next+NextLen]){
			case '>':
				NextLen++;
				NextType=synNE;
				break;
			case '=':
				NextLen++;
				NextType=synLE;
				break;
			default:
				NextType=synLT;
			}
			break;
		case '>':
			if (Sentence[Next+NextLen]=='='){
				NextLen++;
				NextType=synGE;
			}else NextType=synGT;
			break;
		case '=':
			if (Sentence[Next+NextLen]=='=') NextLen++;
			NextType=synEQ;
			break;
		case '.':
			if (isDigit(Sentence[Next+NextLen])){
				NextLen++;
				NextPrecision++;
				while (isDigit(Sentence[Next+NextLen])) NextLen++;
				NextType=synNUMBER;
			}else{
				NextType=synDOT;
			}
			break;
		case '+':
			NextType=synADD;
			break;
		case '-':
			NextType=synSUB;
			break;
		case '(':
			NextType=synLEFTBRACE;
			break;
		case ')':
			NextType=synRIGHTBRACE;
			break;
		case ',':
			NextType=synCOMMA;
			break;
		case '/':
			NextType=synDIV;
			break;
		case '*':
			NextType=synASTERISK;
			break;
		case ';':
			NextType=synSEMICOLON;
			break;
		default:
			NextType=synUNKNOW;
			NextLen=0;
			break;
		}
	}
	return PreviousType;
}

etSYNTAX tSyntax::KeyWord(){
	if (NextType!=synWORD) return NextType;
	if ((NextLen==6)&&(strncmp(Sentence+Next,"SELECT",NextLen)==0
		||strncmp(Sentence+Next,"select",NextLen)==0)) return synSELECT;
	if ((NextLen==6)&&(strncmp(Sentence+Next,"DELETE",NextLen)==0
		||strncmp(Sentence+Next,"delete",NextLen)==0)) return synDELETE;
	if ((NextLen==6)&&(strncmp(Sentence+Next,"INSERT",NextLen)==0
		||strncmp(Sentence+Next,"insert",NextLen)==0)) return synINSERT;
	if ((NextLen==6)&&(strncmp(Sentence+Next,"UPDATE",NextLen)==0
		||strncmp(Sentence+Next,"update",NextLen)==0)) return synUPDATE;
	if ((NextLen==4)&&(strncmp(Sentence+Next,"FROM",NextLen)==0
		||strncmp(Sentence+Next,"from",NextLen)==0)) return synFROM;
	if ((NextLen==4)&&(strncmp(Sentence+Next,"INTO",NextLen)==0
		||strncmp(Sentence+Next,"into",NextLen)==0)) return synINTO;
	if ((NextLen==6)&&(strncmp(Sentence+Next,"VALUES",NextLen)==0
		||strncmp(Sentence+Next,"values",NextLen)==0)) return synVALUES;
	if ((NextLen==3)&&(strncmp(Sentence+Next,"SET",NextLen)==0
		||strncmp(Sentence+Next,"set",NextLen)==0)) return synSET;
	if ((NextLen==5)&&(strncmp(Sentence+Next,"WHERE",NextLen)==0
		||strncmp(Sentence+Next,"where",NextLen)==0)) return synWHERE;
	if ((NextLen==3)&&(strncmp(Sentence+Next,"AND",NextLen)==0
		||strncmp(Sentence+Next,"and",NextLen)==0)) return synAND;
	if ((NextLen==2)&&(strncmp(Sentence+Next,"OR",NextLen)==0
		||strncmp(Sentence+Next,"or",NextLen)==0)) return synOR;
	if ((NextLen==3)&&(strncmp(Sentence+Next,"NOT",NextLen)==0
		||strncmp(Sentence+Next,"not",NextLen)==0)) return synNOT;
	if ((NextLen==2)&&(strncmp(Sentence+Next,"IN",NextLen)==0
		||strncmp(Sentence+Next,"in",NextLen)==0)) return synIN;
	if ((NextLen==7)&&(strncmp(Sentence+Next,"MATCHES",NextLen)==0
		||strncmp(Sentence+Next,"matches",NextLen)==0)) return synMATCHES;
	if ((NextLen==4)&&(strncmp(Sentence+Next,"LIKE",NextLen)==0
		||strncmp(Sentence+Next,"like",NextLen)==0)) return synLIKE;
	if ((NextLen==2)&&(strncmp(Sentence+Next,"IS",NextLen)==0
		||strncmp(Sentence+Next,"is",NextLen)==0)) return synIS;
	if ((NextLen==6)&&(strncmp(Sentence+Next,"CREATE",NextLen)==0
		||strncmp(Sentence+Next,"create",NextLen)==0)) return synCREATE;
	if ((NextLen==4)&&(strncmp(Sentence+Next,"DROP",NextLen)==0
		||strncmp(Sentence+Next,"drop",NextLen)==0)) return synDROP;
	if ((NextLen==5)&&(strncmp(Sentence+Next,"TABLE",NextLen)==0
		||strncmp(Sentence+Next,"table",NextLen)==0)) return synTABLE;
	if ((NextLen==3)&&(strncmp(Sentence+Next,"SUM",NextLen)==0
		||strncmp(Sentence+Next,"sum",NextLen)==0)) return synSUM;
	if ((NextLen==5)&&(strncmp(Sentence+Next,"COUNT",NextLen)==0
		||strncmp(Sentence+Next,"count",NextLen)==0)) return synCOUNT;
	if ((NextLen==5)&&(strncmp(Sentence+Next,"LTRIM",NextLen)==0
		||strncmp(Sentence+Next,"ltrim",NextLen)==0)) return synLTRIM;
	if ((NextLen==5)&&(strncmp(Sentence+Next,"RTRIM",NextLen)==0
		||strncmp(Sentence+Next,"rtrim",NextLen)==0)) return synRTRIM;
	if ((NextLen==6)&&(strncmp(Sentence+Next,"SUBSTR",NextLen)==0
		||strncmp(Sentence+Next,"substr",NextLen)==0)) return synSUBSTR;
	if ((NextLen==9)&&(strncmp(Sentence+Next,"TO_NUMBER",NextLen)==0
		||strncmp(Sentence+Next,"to_number",NextLen)==0)) return synTO_NUMBER;
	if ((NextLen==4)&&(strncmp(Sentence+Next,"NULL",NextLen)==0
		||strncmp(Sentence+Next,"null",NextLen)==0)) return synNULL;
	if ((NextLen==7)&&(strncmp(Sentence+Next,"SYSDATE",NextLen)==0
		||strncmp(Sentence+Next,"sysdate",NextLen)==0)) return synSYSDATE;
	if ((NextLen==7)&&(strncmp(Sentence+Next,"TO_CHAR",NextLen)==0
		||strncmp(Sentence+Next,"to_char",NextLen)==0)) return synTO_CHAR;
	if ((NextLen==7)&&(strncmp(Sentence+Next,"TO_DATE",NextLen)==0
		||strncmp(Sentence+Next,"to_date",NextLen)==0)) return synTO_DATE;
	if ((NextLen==10)&&(strncmp(Sentence+Next,"ADD_MONTHS",NextLen)==0
		||strncmp(Sentence+Next,"add_months",NextLen)==0)) return synADD_MONTHS;
	if ((NextLen==8)&&(strncmp(Sentence+Next,"LAST_DAY",NextLen)==0
		||strncmp(Sentence+Next,"last_day",NextLen)==0)) return synLAST_DAY;
	if ((NextLen==14)&&(strncmp(Sentence+Next,"MONTHS_BTWEEN",NextLen)==0
		||strncmp(Sentence+Next,"months_between",NextLen)==0)) return synMONTHS_BETWEEN;
	if ((NextLen==8)&&(strncmp(Sentence+Next,"NEXT_DAY",NextLen)==0
		||strncmp(Sentence+Next,"next_day",NextLen)==0)) return synNEXT_DAY;
	return synWORD;
}

int tSyntax::isSpace(char c){
	return (c==' '||c=='\t'||c=='\n');
}
int tSyntax::isAlpha(char c){
	return ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c=='_'));
}
int tSyntax::isDigit(char c){
	return ((c>='0')&&(c<='9'));
}

tExpression::tExpression(){
	Brand=expUNKNOW;
	CalcOp=synUNKNOW;
	Parent=Left=Right=NULL;
	Table=NULL;
	BoolValue=True;
	Passed=False;
}

tExpression::tExpression(etSYNTAX SetCalOp){
	Brand=expUNKNOW;
	CalcOp=SetCalOp;
	Parent=Left=Right=NULL;
	Table=NULL;
	BoolValue=True;
	Passed=False;

	if (CalcOp==synAND||CalcOp==synOR||CalcOp==synNOT||CalcOp==synLIKE || CalcOp==synIS) 
		Brand=expBOOL;
	else if (CalcOp==synEQ||CalcOp==synLT||CalcOp==synLE||CalcOp==synGT||CalcOp==synGE||CalcOp==synNE) 
		Brand=expCOMP;
	else if (CalcOp==synADD||CalcOp==synSUB||CalcOp==synASTERISK||CalcOp==synDIV)
		Brand=expCALC;
	else if (CalcOp==synNUMBER) {
		Brand=expCONST;
		Type=dbNUMBER;
	}else if (CalcOp==synCHAR) {
		Brand=expCONST;
		Type=dbCHAR;
	}
}

tExpression::~tExpression(){
	if (Brand!=expNODE){
		if (Left!=NULL) delete (tExpression*)Left;
		if (Right!=NULL) delete (tExpression*) Right;
	}
}

int tExpression::Calculate(){
	long i,n;
	time_t now=0;
	tm* pt;
	char* str;
	double Double;

	switch (Brand){
	case expNODE:
		/* there is no need to do these sentence, or there must be bugs.
		deleteValue();
		*/
		Value=Left->Value;
		if (Left->Type==dbNUMBER && !strcmp(Value,"")){
			strcpy(Value,"0");
		}
		if (Left->Type==dbLONG) {
			Type=dbLONG;
			Length=Left->Length;
		}
		//set value
	case expCONST:
		switch (Type) {
		case dbNUMBER:
			DoubleValue=0.0;
			sscanf(Value, "%lf", &DoubleValue);
			break;
		case dbDATE:
#ifdef DateStandardFormat
			DateToLong(Value,DateFormat,&now);
			DoubleValue=now;
#else
			DoubleValue=0.0;
			sscanf(Value, "%lf", &DoubleValue);
#endif
		default:
			break;
		}
		break;
	case expBOOL:
		switch (CalcOp){
		case synNOT:
			BoolValue=!(((tExpression *)Left)->BoolValue);
			break;
		case synOR:
			BoolValue=(((tExpression *)Left)->BoolValue||((tExpression*)Right)->BoolValue);
			break;
		case synAND:
			BoolValue=(((tExpression *)Left)->BoolValue&&((tExpression *)Right)->BoolValue);
			break;
		case synLIKE:
			BoolValue=StringLike(((tExpression *)Left)->Value,((tExpression *)Right)->Value);
			break;
		case synIS:
			BoolValue=!strcmp(((tExpression *)Left)->Value,((tExpression *)Right)->Value);
			break;
		default:
			BoolValue=False;
		}
		break;
	case expCOMP:
		switch (Left->Type){
		case dbDATE:
			if (Right->Type!=dbDATE){
				BoolValue=False;
				break;
			}
			switch (CalcOp){
			case synEQ:
				BoolValue=(DateCmp(Left,Right)==0);
				break;
			case synNE:
				BoolValue=(DateCmp(Left,Right)!=0);
				break;
			case synLT:
				BoolValue=(DateCmp(Left,Right)<0);
				break;
			case synGT:
				BoolValue=(DateCmp(Left,Right)>0);
				break;
			case synLE:
				BoolValue=(DateCmp(Left,Right)<=0);
				break;
			case synGE:
				BoolValue=(DateCmp(Left,Right)>=0);
				break;
			default:
				BoolValue=False;
				break;
			}
			break;
		case dbCHAR:
		case dbLONG:
			switch (CalcOp){
			case synEQ:
				BoolValue=(strcmp(Left->Value,Right->Value)==0);
				break;
			case synNE:
				BoolValue=(strcmp(Left->Value,Right->Value)!=0);
				break;
			case synLT:
				BoolValue=(strcmp(Left->Value,Right->Value)<0);
				break;
			case synGT:
				BoolValue=(strcmp(Left->Value,Right->Value)>0);
				break;
			case synLE:
				BoolValue=(strcmp(Left->Value,Right->Value)<=0);
				break;
			case synGE:
				BoolValue=(strcmp(Left->Value,Right->Value)>=0);
				break;
			default:
				BoolValue=False;
				break;
			}
			break;
		case dbNUMBER:
			switch (CalcOp){
			case synEQ:
				BoolValue=(NumCmp(Left,Right)==0);
				break;
			case synNE:
				BoolValue=(NumCmp(Left,Right)!=0);
				break;
			case synLT:
				BoolValue=(NumCmp(Left,Right)<0);
				break;
			case synGT:
				BoolValue=(NumCmp(Left,Right)>0);
				break;
			case synLE:
				BoolValue=(NumCmp(Left,Right)<=0);
				break;
			case synGE:
				BoolValue=(NumCmp(Left,Right)>=0);
				break;
			default:
				BoolValue=False;
				break;
			}
			break;
		default:
			BoolValue=False;
			break;
		}
		break;
	case expCALC:
		switch (CalcOp){
		case synADD:
			if ( Left->Type==dbNUMBER && !Right ) {
				DoubleValue=Left->DoubleValue;
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else if (!Left ||  !Right) {
				return dbTYPEERR;
			}else if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
				DoubleValue=Left->DoubleValue+Right->DoubleValue;
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else if ( ( Left->Type==dbCHAR || Right->Type==dbCHAR || Left->Type==dbLONG || Right->Type==dbLONG )){
				sprintf(Value,"%s%s",Left->Value,Right->Value);
			}else if ( ( Left->Type==dbDATE && Right->Type==dbNUMBER ) || ( Right->Type==dbDATE && Left->Type==dbNUMBER ) ){
				DoubleValue=Left->DoubleValue+Right->DoubleValue;
#ifdef DateStandardFormat
				now=(time_t)(DoubleValue);
				i=DateToChar(now,DateFormat,Value);
#else
				i=StringPrintf(Value,DoubleValue,Length,Precision);
#endif
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synSUB:
			if ( Left->Type==dbNUMBER && !Right) {
				DoubleValue=-Left->DoubleValue;
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else if (!Left ||  !Right) {
				return dbTYPEERR;
			}else if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
				DoubleValue=Left->DoubleValue-Right->DoubleValue;
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else if ( Left->Type==dbDATE && Right->Type==dbNUMBER ){
				DoubleValue=Left->DoubleValue-Right->DoubleValue;
#ifdef DateStandardFormat
				now=(time_t)(DoubleValue);
				i=DateToChar(now,DateFormat,Value);
#else
				i=StringPrintf(Value,DoubleValue,Length,Precision);
#endif
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synASTERISK:
			if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
				DoubleValue=Left->DoubleValue * Right->DoubleValue;
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synDIV:
			if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
				DoubleValue=Left->DoubleValue / Right->DoubleValue;
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synSUM:
			if ( Left->Type==dbNUMBER ) {
				DoubleValue+=Left->DoubleValue;
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synLTRIM:
			if ( (Left->Type==dbCHAR||Left->Type==dbLONG) && !Right) {
				str=Left->Value;
				while (*str!='\0' && *str==' ')
					str++;
				strcpy(Value,str);
			}else if ( (Left->Type==dbCHAR||Left->Type==dbLONG) && Right && (Right->Type==dbCHAR||Right->Type==dbLONG) ) {
				str=Left->Value;
				while (*str!='\0' && strchr(Right->Value,*str) )
					str++;
				strcpy(Value,str);
			}else{
				return dbTYPEERR;
			}
			break;
		case synRTRIM:
			if ( (Left->Type==dbCHAR||Left->Type==dbLONG) && !Right) {
				strcpy(Value,"");
				str=Left->Value+strlen(Left->Value)-1;
				while (str>=Left->Value && *str==' ')
					str--;
				if (str>=Left->Value){
					strncpy(Value,Left->Value,str+1-Left->Value);
					Value[str+1-Left->Value]='\0';
				}
			}else if ( (Left->Type==dbCHAR||Left->Type==dbLONG) && Right && (Right->Type==dbCHAR||Right->Type==dbLONG) ) {
				strcpy(Value,"");
				str=Left->Value+strlen(Left->Value)-1;
				while (str>=Left->Value && strchr(Right->Value,*str) )
					str--;
				if (str>=Left->Value){
					strncpy(Value,Left->Value,str+1-Left->Value);
					Value[str+1-Left->Value]='\0';
				}
			}else{
				return dbTYPEERR;
			}
			break;
		case synSUBSTR:
			if ( (Left->Type==dbCHAR||Left->Type==dbLONG) && Right->Type==dbNUMBER) {
				if (Precision==0){
					n=(long)(Right->DoubleValue);
					if (n>(long)strlen(Left->Value)) n=strlen(Left->Value);
					strcpy(Value,Left->Value+n-1);
				}else{
					strcpy(Value,"");
					n=(long)(Right->DoubleValue);
					strncpy(Value,Left->Value,n);
					Value[n]='\0';
				}
			}else{
				return dbTYPEERR;
			}
			break;
		case synTO_NUMBER:
			if ( Left->Type==dbCHAR ||Left->Type==dbLONG) {
				DoubleValue=0;
				sscanf(Left->Value,"%lf",&DoubleValue);
				Length=strlen(Left->Value)+1;
				Double=DoubleValue-(long)DoubleValue;
				Precision=0;
				while (Precision<Length-1 && Double>Epsilon){
					Precision++;
					Double = Double*10-long(Double*10);
				}
				if ((i=newValue(Length+1))!=dbSUCCESS) return i;
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synCOUNT:
			DoubleValue+=1;
			i=StringPrintf(Value,DoubleValue,Length,Precision);
			if (i!=0) return i;
			break;
		case synNULL:
			break;
		case synSYSDATE:
			if (strcmp(Value,"")) break;
			DoubleValue=time(NULL);
			//LongValue=(long)DoubleValue;
			//strcpy(Value,asctime(localtime(&LongValue)));
#ifdef DateStandardFormat
			now=(time_t) (DoubleValue);
			i=DateToChar(now,DateFormat,Value);
#else
			i=StringPrintf(Value,DoubleValue,Length,Precision);
#endif
			if (i!=0) return i;
			break;
		case synTO_CHAR:
			if ( (Left->Type==dbNUMBER||Left->Type==dbDATE) && (Right->Type==dbCHAR||Right->Type==dbLONG) ) {
				str=(char*)malloc(sizeof(char)*(strlen(Right->Value)+100));
				if (!str) return dbMEMORYERR;
				now=(time_t) (Left->DoubleValue);
				if ((i=DateToChar(now,Right->Value,str))!=dbSUCCESS){
					free (str);
					return i;
				}
				Length=strlen(str)+1;
				if ((i=newValue(Length))!=dbSUCCESS){
					free (str);
					return i;
				}
				strcpy(Value,str);
				free(str);
			}else{
				return dbTYPEERR;
			}
			break;
		case synTO_DATE:
			if ( (Left->Type==dbCHAR||Left->Type==dbLONG) && (Right->Type==dbCHAR || Right->Type==dbLONG) ) {
				now=0;
				if ((i=DateToLong(Left->Value,Right->Value,&now))!=dbSUCCESS) return i;
				DoubleValue=now;
#ifdef DateStandardFormat
				i=DateToChar(now,DateFormat,Value);
#else
				i=StringPrintf(Value,DoubleValue,Length,Precision);
#endif
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synADD_MONTHS:
			if ( Left->Type==dbDATE && Right->Type==dbNUMBER ) {
				now=(time_t) (Left->DoubleValue);
				pt=localtime(&now);
				pt->tm_mon +=(long)(Right->DoubleValue);
				pt->tm_year +=pt->tm_mon/12;
				pt->tm_mon %=12;
				if (pt->tm_mon <=0){
					pt->tm_mon+=12;
					pt->tm_year--;
				}
				now=mktime(pt);
				DoubleValue=now;
#ifdef DateStandardFormat
				i=DateToChar(now,DateFormat,Value);
#else
				i=StringPrintf(Value,DoubleValue,Length,Precision);
#endif
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synLAST_DAY:
			if ( Left->Type==dbDATE) {
				now=(time_t) (Left->DoubleValue);
				pt=localtime(&now);
				n=MonthDays(pt->tm_year,pt->tm_mon);
				pt->tm_mday=n;
				now=mktime(pt);
				DoubleValue=now;
#ifdef DateStandardFormat
				i=DateToChar(now,DateFormat,Value);
#else
				i=StringPrintf(Value,DoubleValue,Length,Precision);
#endif
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synNEXT_DAY:
			if ( Left->Type==dbDATE && (Right->Type==dbCHAR || Right->Type==dbLONG)) {
				now=(time_t) (Left->DoubleValue);
				pt=localtime(&now);
				for (i=0;i<7;i++){
					if (strcmp(Right->Value,S_DAY[i])==0) 
						break;
				}
				if (i<7){
					i-=pt->tm_wday;
					if (i<=0) i+=7;
					pt->tm_mday+=i;
					n=MonthDays(pt->tm_year,pt->tm_mon);
					if (pt->tm_mday>n){
						pt->tm_mday-=n;
						pt->tm_mon++;
						if (pt->tm_mon==12){
							pt->tm_mon=0;
							pt->tm_year++;
						}
					}
				}
				now=mktime(pt);
				DoubleValue=now;
#ifdef DateStandardFormat
				i=DateToChar(now,DateFormat,Value);
#else
				i=StringPrintf(Value,DoubleValue,Length,Precision);
#endif
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		case synMONTHS_BETWEEN:
			if ( Left->Type==dbDATE && Right->Type==dbDATE) {
				now=(time_t) (Right->DoubleValue);
				pt=localtime(&now);
				n=pt->tm_year;
				i=pt->tm_mon;
				now=(time_t) (Left->DoubleValue);
				pt=localtime(&now);
				n-=pt->tm_year;
				i-=pt->tm_mon;
				DoubleValue=n*12+i;

				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
			}else{
				return dbTYPEERR;
			}
			break;
		default:
			break;
		}
	default:
		break;
	}
	Passed=True;
	return dbSUCCESS;
}

int tExpression::NumCmp(tField *Num1, tField *Num2){
	if (Num1->DoubleValue-Num2->DoubleValue >Epsilon) return 1;
	if (Num1->DoubleValue-Num2->DoubleValue <-Epsilon) return -1;
	return 0;
}

int tExpression::DateCmp(tField *Date1, tField* Date2){
	if (Date1->DoubleValue-Date2->DoubleValue >Epsilon) return 1;
	if (Date1->DoubleValue-Date2->DoubleValue <-Epsilon) return -1;
	return 0;
}

int tExpression::checkCalcType(){
	int i;
	switch (CalcOp){
	case synNOT:
		Brand=expBOOL;
		break;
	case synADD:
		Brand=expCALC;
		if ( Left->Type==dbNUMBER && !Right) {
			Length=Left->Length+1;
			Type=dbNUMBER;
			Precision=Left->Precision;
			if ( (i=newValue(Length) )!=dbSUCCESS )
				return i;
		}else if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
			Length=(Left->Length > Right->Length)?Left->Length:Right->Length;
			Length+=AddAddLength;
			Type=dbNUMBER;
			Precision=(Left->Precision > Right->Precision)?Left->Precision : Right->Precision;
			if ( (i=newValue(Length) )!=dbSUCCESS )
				return i;
		}else if ( !Left || !Right){
			return dbSYNTAXERR;
		}else if ( Left->Type==dbCHAR || Right->Type==dbCHAR || Left->Type==dbLONG || Right->Type==dbLONG ){
			Length=Left->Length+Right->Length;
			Length+=AddAddLength;
			Type=dbCHAR;
			Precision=0;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else if ( Left->Type==dbDATE && Right->Type==dbNUMBER ){
			Length=Left->Length;
			Type=dbDATE;
			Precision=0;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else if ( Right->Type==dbDATE && Left->Type==dbNUMBER ){
			Length=Right->Length;
			Type=dbDATE;
			Precision=0;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else{
			return dbTYPEERR;
		}
		break;
	case synSUB:
		Brand=expCALC;
		if ( Left->Type==dbNUMBER && !Right ) {
			Length=Left->Length+1;
			Type=dbNUMBER;
			Precision=Left->Precision;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else if ( !Left || !Right){
			return dbSYNTAXERR;
		}else if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
			Length=(Left->Length > Right->Length)?Left->Length:Right->Length;
			Length+=AddAddLength;
			Type=dbNUMBER;
			Precision=(Left->Precision > Right->Precision)?Left->Precision:Right->Precision;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else if ( Left->Type==dbDATE && Right->Type==dbNUMBER ){
			Length=Left->Length;
			Type=dbDATE;
			Precision=0;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else if ( Left->Type==dbDATE && Right->Type==dbDATE ){
			Length=DateLength;
			Type=dbNUMBER;
			Precision=0;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else{
			return dbTYPEERR;
		}
		break;
	case synASTERISK:
		Brand=expCALC;
		if (Left == NULL || Right == NULL ) 
			return dbSYNTAXERR;
		if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
			Length=Left->Length + Right->Length;
			Length+=AddAddLength;
			Type=dbNUMBER;
			Precision=Left->Precision + Right->Precision;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else{
			return dbTYPEERR;
		}
		break;
	case synDIV:
		Brand=expCALC;
		if (Left == NULL || Right == NULL ) 
			return dbSYNTAXERR;
		if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
			Length=Left->Length + Right->Length;
			Length+=AddAddLength;
			Type=dbNUMBER;
			Precision=Left->Precision + Right->Precision;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else{
			return dbTYPEERR;
		}
		break;
	case synSUM:
		Brand=expCALC;
		Type=dbNUMBER;
		Length=Left->Length;
		Length+=SumAddLength;
		Precision=Left->Precision;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
		break;
	case synTO_NUMBER:
	case synMONTHS_BETWEEN:
		Brand=expCALC;
		Type=dbNUMBER;
		Length=NumberDefLength;
		Precision=0;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
		break;
	case synCOUNT:
		Brand=expCALC;
		Type=dbNUMBER;
		Length=1+SumAddLength;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
		break;
	case synLTRIM:
	case synRTRIM:
	case synSUBSTR:
		Brand=expCALC;
		Type=dbCHAR;
		Length=Left->Length;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
		break;
	case synTO_CHAR:
		Brand=expCALC;
		Type=dbCHAR;
		Length=strlen(Right->Value)+FmtAddLength;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
		break;
	case synSYSDATE:
	case synTO_DATE:
	case synADD_MONTHS:
	case synLAST_DAY:
	case synNEXT_DAY:
		Brand=expCALC;
		Type=dbDATE;
		Length=DateLength;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
		break;
	case synNULL:
		Brand=expBOOL;
		Length=1;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
	default:
		break;
	}
	return dbSUCCESS;
}

tSQLRequest::tSQLRequest(char *SetDatabase){
	//signal(SIGINT,SIG_IGN);
	//signal(SIGTERM,SIG_IGN);
#ifdef unix
	//signal(SIGPIPE,SIG_IGN);
#endif
	setDatabase(SetDatabase);
	initData();
}


tSQLRequest::~tSQLRequest(){
	clearData();
#ifdef unix
	//signal(SIGPIPE,SIG_DFL);
#endif
	//signal(SIGTERM,SIG_DFL);
	//signal(SIGINT,SIG_DFL);
}

int tSQLRequest::setDatabase(char *SetDatabase){
	DataBase[0]='\0';
	if (SetDatabase)
		strcpy(DataBase, SetDatabase);
//19990818MODIFY for database with no '\\' or '/' (DirSepa) at the end
	int i=strlen(DataBase);
	for (int j=0; j<i ;j++){
		if (DataBase[j] == '\\' || DataBase[j] == '/')
			DataBase[j]=DirSepa;
	}
	if (i>0 && DataBase[i-1]!='\\' && DataBase[i-1]!='/'){
		DataBase[i++]=DirSepa;
		DataBase[i]='\0';
	}

	Flag=dbSUCCESS;
	return Flag;
}

void tSQLRequest::initData(){
	Length=0;
	Precision=0;
	Type=dbCHAR;
	Value=NULL;
	IsMalloc=False;
	WalkNext=NULL;

	strcpy(OutFdSep,"_F_D_");
	strcpy(OutRdSep,"_R_D_");
	TableNum=0;
	Tables=NULL;
	OutFieldNum=0;
	OutFields=NULL;
	Flag=dbSUCCESS;
	Sql=NULL;
	Action=synUNKNOW;
	IsSubRequest=True;
	GroupAction=0;
	
	WalkStatus=walkSTOP;
	Root=FirstStep=CurrentStep=FirstNodeTab=LastNodeTab=NULL;
	CurrentTable=NULL;
	SelectResults=NULL;
}

void tSQLRequest::clearData(){
#if (ERRORLOG > 1)
	if (Flag <0 && Sql){
		Log::add(ERRORLOGFILE,0,Sql->Sentence,__LINE__);
		Log::add(ERRORLOGFILE,Flag,Sql->Sentence+Sql->Start,__LINE__);
	}
#endif
	tExpression* e,*e1;
	deleteValue();
	if (!IsSubRequest){
		if (Sql) delete Sql;
		Sql=NULL;
		if (Tables) delete Tables;
		Tables=NULL;
	}
	if (Root) delete Root;
	Root=NULL;
	e=OutFields;
	while (e!=NULL){
		e1=(tExpression*)e->WalkNext;
		delete e;
		e=e1;
	}
	OutFields=NULL;
	if (SelectResults) delete SelectResults;
	SelectResults=NULL;
}

long tSQLRequest::doSQL(char *Sentence,char **Result){
	long Count=doSQL(Sentence);
	*Result=SelectResults;
	return Count;

}

long tSQLRequest::doSQL(char *Sentence){
	long Count=0;
	int i;
	char *buf;
	long buf_size=0;
	long buf_len=0;
	if ((i=setRequest(Sentence))<0) return i;
	while (Flag==dbSUCCESS){
		if (doRequest()==dbSUCCESS) {
			Count++;
			if (Action == synSELECT && Value){
				while ((long)strlen(Value)+buf_len>buf_size-1){
					buf_size += BufferSize;
					//buf=(char*)realloc(SelectResults,sizeof(char)*(buf_size+1));
					buf=(char*)malloc(sizeof(char)*(buf_size+1));
					if (!buf) return dbMEMORYERR;
					if (SelectResults){
						memcpy(buf,SelectResults,buf_len+1);
						free(SelectResults);
						SelectResults=buf;
					}else{
						buf[0]='\0';
						SelectResults=buf;
					}
				}
				strcat(SelectResults,Value);
				buf_len+=strlen(Value);
			}
		}
		if (Flag<0) return Flag;
#ifdef DEBUG
		Log::add(ERRORLOGFILE,Count,Value,__LINE__);
#endif
	}
	return Count;
}

int tSQLRequest::setRequest(char *S){
	if (Tables||Root||Sql){
		clearData();
		initData();
	}

	Sql=new tSyntax;
	if ((Flag=Sql->setSentence(S))==dbSUCCESS) IsSubRequest=False;
	if (Flag==dbSUCCESS) Flag=parseSql();
	return Flag;
}

int tSQLRequest::doRequest(){
	int i;
	if (Flag!=dbSUCCESS) return Flag;
	if (!Sql){
		Flag=dbSQLNOTSET;
		return Flag;
	}
	Flag=doOneRequest();
	if (GroupAction==1){//for SUM/COUNT
		while (Flag==dbSUCCESS){
			Flag=doOneRequest();
		}
	}
	if (Flag!=dbSUCCESS) if ((i=doCommit())!=dbSUCCESS) return i;
	if (GroupAction==1) {
		if (Flag==dbSUCCESS|| Flag==dbNOTFOUND) 
			return dbSUCCESS;
	}
	return Flag;
}

int tSQLRequest::doOneRequest(){
	int i,j;
	tField* tmpField;

	if (WalkStatus==walkSTOP){
		CurrentStep=FirstStep;
		if (FirstNodeTab)
            CurrentTable=FirstNodeTab->Table;
		else
			CurrentTable=Tables;
		WalkStatus=walkRECUR;
	}

	while (WalkStatus==walkRECUR){
		if ((Action==synDELETE || Action==synUPDATE ) && CurrentTable==Tables){
//19990325 modify for Root==NULL;
			if ( Root!=NULL && !Root->BoolValue)   // the initial Root->BoolValue is True
				if ((i=Tables->saveToTmp())!=dbSUCCESS) return i;
		}

		if ((i=CurrentTable->getRecord())!=dbSUCCESS) return i;
		if (Action==synINSERT){
			if (!CurrentTable->EndFile){
				if ((i=Tables->saveToTmp())!=dbSUCCESS) return i;
				continue;
			}
			else {
				if (Tables && Tables->Fields){
					for (tmpField=Tables->Fields; tmpField; tmpField=tmpField->WalkNext){
						if (tmpField->Value)
							tmpField->Value[0]='\0';
					}
				}
				if ((i=printOut())!=dbSUCCESS) return i;

				Flag=dbNOTFOUND;
				WalkStatus=walkSTOP;
				return Flag;
			}
		}

		if (CurrentTable->EndFile){
			if ((i=setCurrentStep(True))!=dbSUCCESS) return i;
			WalkStatus=walkRECUR;
			continue;
		}
//19990325 modify for CurrentStep==NULL;
		WalkStatus=walkNEXT;
		while(WalkStatus==walkNEXT){
			if (CurrentStep!=NULL){
				if ((i=CurrentStep->Calculate())!=dbSUCCESS) return i;
			}
			if (CurrentStep==NULL || CurrentStep->Parent==NULL || FirstStep==NULL ){
				switch (Action){
				case synSELECT:
				case synDELETE:
					j=(CurrentStep==NULL || CurrentStep->BoolValue);
					if ( j ) if ((i=printOut())!=dbSUCCESS) return i;
					setCurrentStep(False);
					/*
					if (LastNodeTab){
						CurrentStep=LastNodeTab;
						CurrentTable=LastNodeTab->Table;
					}else {
						CurrentStep=FirstStep;
						CurrentTable=Tables;
					}
					*/
					WalkStatus=walkRECUR;
					if ( j ) return dbSUCCESS;
					break;

				case synUPDATE:
					if (CurrentStep==NULL || CurrentStep->BoolValue==True) {
						if ((i=printOut())!=dbSUCCESS) return i;

						CurrentStep=FirstStep;
						CurrentTable=Tables;
						WalkStatus=walkRECUR;
						return dbSUCCESS;
					}else {
						setCurrentStep(False);
						/*
						if (LastNodeTab){
							CurrentStep=LastNodeTab;
							CurrentTable=LastNodeTab->Table;
						}else {
							CurrentStep=FirstStep;
							CurrentTable=Tables;
						}
						*/
						WalkStatus=walkRECUR;
					}
					break;

				default:
					break;
				}
			}else{
				if (((tExpression*)(CurrentStep->Parent))->Brand==expBOOL){
					switch (((tExpression*)(CurrentStep->Parent))->CalcOp){
					case synAND:
						if (!CurrentStep->BoolValue) 
							CurrentStep=(tExpression *)CurrentStep->Parent;
						else
							CurrentStep=(tExpression *)CurrentStep->WalkNext;
						break;
					case synOR:
						if (CurrentStep->BoolValue) 
							CurrentStep=(tExpression *)CurrentStep->Parent;
						else
							CurrentStep=(tExpression *)CurrentStep->WalkNext;
						break;
					default:
						CurrentStep=(tExpression *)CurrentStep->WalkNext;
						break;
					}
				}else{
					CurrentStep=(tExpression *)CurrentStep->WalkNext;
				}
			}
		}
	}
	return dbSUCCESS;
}

int tSQLRequest::setCurrentStep(int Next){
	int i;
	if (Next){
			if (CurrentStep) CurrentStep->Passed=False;
			while (CurrentStep && !CurrentStep->Passed){
				CurrentStep=(tExpression *)CurrentStep->Right;
			}
			if (!CurrentStep){
				Flag=dbNOTFOUND;
				WalkStatus=walkSTOP;
				return Flag;
			}
			if (CurrentStep->Table){
				if ((i=CurrentTable->getRecord())!=dbSUCCESS) return i;
				CurrentTable=CurrentStep->Table;
			}
	}else{
		CurrentStep=LastNodeTab;
		while (CurrentStep && !CurrentStep->Passed){
			CurrentStep=(tExpression*)CurrentStep->Right;
		}
		if (CurrentStep)
			CurrentTable=CurrentStep->Table;
		else{
			CurrentStep=FirstStep;
			CurrentTable=Tables;
		}
	}
	return dbSUCCESS;
}

int tSQLRequest::doCommit(){
	int i=dbSUCCESS;
	if (Flag==dbNOTFOUND){
		if (Action==synDELETE || Action==synINSERT|| Action==synUPDATE) i=Tables->commitTmp();
		if (Tables->TmpFile && i==dbSUCCESS){
			Tables->TmpFile->closeFile();
			delete Tables->TmpFile;
			Tables->TmpFile=NULL;
		}
		if (Tables&&i==dbSUCCESS) i=Tables->closeData();
		if (i!=dbSUCCESS) Flag=i;
	}else if (Flag<0){
		if (Tables->TmpFile){
			Tables->TmpFile->closeFile();
			delete Tables->TmpFile;
			Tables->TmpFile=NULL;
		}
		if (Tables) i=Tables->closeData();
	}
	return i;
}

int tSQLRequest::parseSql(){
	int i;
	switch (Sql->Type){
	case synSELECT:
#ifdef DEBUG
		Log::add(ERRORLOGFILE,0,"parse select",__LINE__);
#endif
		Action=synSELECT;
		if ((i=parseSelect())!=dbSUCCESS) return i;
		break;
	case synDELETE:
#ifdef DEBUG
		Log::add(ERRORLOGFILE,0,"parse delete",__LINE__);
#endif
		Action=synDELETE;
		if ((i=parseDelete())!=dbSUCCESS) return i;
		break;
	case synINSERT:
#ifdef DEBUG
		Log::add(ERRORLOGFILE,0,"parse insert",__LINE__);
#endif
		Action=synINSERT;
		if ((i=parseInsert())!=dbSUCCESS) return i;

		break;
	case synUPDATE:
#ifdef DEBUG
		Log::add(ERRORLOGFILE,0,"parse update",__LINE__);
#endif
		Action=synUPDATE;
		if ((i=parseUpdate())!=dbSUCCESS) return i;
		break;
	case synCREATE:
#ifdef DEBUG
		Log::add(ERRORLOGFILE,0,"parse create",__LINE__);
#endif
		Action=synCREATE;
		do {
/*			CreateSql=new tSQLCreate(DataBase);
			CreateSql->Sql=Sql;
			if ((i=CreateSql->parseCreate())!=dbSUCCESS){
				delete CreateSql;
				return i;
			}
			delete CreateSql;
			CreateSql=NULL;
			*/
			if ((i=parseCreate())!=dbSUCCESS) return i;
			Sql->match();
			if ((i=Sql->match(synSEMICOLON))!=dbSUCCESS) return i;
		} while (Sql->Type==synCREATE);
		return dbCREATESUCCESS;
	case synDROP:
#ifdef DEBUG
		Log::add(ERRORLOGFILE,0,"parse drop",__LINE__);
#endif
		Action=synDROP;
		if ((i=parseDrop())!=dbSUCCESS) return i;
		return dbDROPSUCCESS;
	default:
		return dbSYNTAXERR;
		;
	}
	return dbSUCCESS;
}

int tSQLRequest::parseSelect(){
	int i,j,len;
	tExpression *WalkFrom;
	tField** PreWalk;
	int AllFields=False;

	if ((i=Sql->match(synSELECT))!=dbSUCCESS) return i;
	if (Sql->Type==synASTERISK){
		AllFields=True;
		Sql->match();
	}
	else {
		AllFields=False;
		if ((i=parseFieldList())!=dbSUCCESS) return i;
	}

	if ((i=Sql->match(synFROM))!=dbSUCCESS) return i;
	if ((i=parseTableList())!=dbSUCCESS) return i;
	
	if (Sql->Type==synWHERE) {
		if ((i=Sql->match(synWHERE))!=dbSUCCESS) return i;
		if ((i=parseExpr(synSEMICOLON,&Root))!=dbSUCCESS) return i;
	}
	if (Sql->Type!=synSEMICOLON) return dbSYNTAXERR;

	if (AllFields){
		if ((i=addAllFieldsOut(Tables))!=dbSUCCESS) return i;
	}

	WalkFrom=OutFields;
	len=strlen(OutFdSep);
	j=strlen(OutRdSep)+1;
	while (WalkFrom){
		if ((i=spotNode(WalkFrom))!=dbSUCCESS) return i;
		j+=WalkFrom->Length+len;
		WalkFrom=(tExpression *)WalkFrom->WalkNext;
	}
	if ((i=newValue(j))!=dbSUCCESS) return i;

	PreWalk= (tField **)&FirstStep;
//19990325 modify for Root==NULL
	if (Root!=NULL && (i=PostWalk(Root,&PreWalk))!=dbSUCCESS) return i;

	return dbSUCCESS;
}

int tSQLRequest::parseDelete(){
	int i,j,len;
	tExpression *WalkFrom;
	tField** PreWalk;

	if ((i=Sql->match(synDELETE))!=dbSUCCESS) return i;
	if ((i=Sql->match(synFROM))!=dbSUCCESS) return i;
	if ((i=parseTableList())!=dbSUCCESS) return i;
	if ((i=addAllFieldsOut(Tables))!=dbSUCCESS) return i;

	WalkFrom=OutFields;
	len=strlen(OutFdSep);
	j=strlen(OutRdSep)+1;
	while (WalkFrom){
		j+=WalkFrom->Length+len;
		WalkFrom=(tExpression *)WalkFrom->WalkNext;
	}
	if ((i=newValue(j))!=dbSUCCESS) return i;

	if (Sql->Type==synWHERE){
		if ((i=Sql->match(synWHERE))!=dbSUCCESS) return i;
		if ((i=parseExpr(synSEMICOLON,&Root))!=dbSUCCESS) return i;
	}
	if (Sql->Type!=synSEMICOLON) return dbSYNTAXERR;
	PreWalk= (tField **)&FirstStep;
//19990325 modify for Root==NULL
	if (Root!=NULL && (i=PostWalk(Root,&PreWalk))!=dbSUCCESS) return i;

	if (FirstNodeTab) FirstNodeTab->Right=NULL;

	return dbSUCCESS;
}


int tSQLRequest::parseInsert(){
	int i,j,len;
	tExpression *WalkFrom;

	if ((i=Sql->match(synINSERT))!=dbSUCCESS) return i;
	if ((i=Sql->match(synINTO))!=dbSUCCESS) return i;
	if ((i=parseTableList())!=dbSUCCESS) return i;

	if (Sql->Type==synLEFTBRACE){
		Sql->match();
		if ((i=parseFieldList())!=dbSUCCESS) return i;
		if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
	}else 
		if ((i=addAllFieldsOut(Tables))!=dbSUCCESS) return i;
	FirstNodeTab=OutFields;
	OutFields=NULL;

	if ((i=Sql->match(synVALUES))!=dbSUCCESS) return i;
	if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;

	if ((i=parseFieldList())!=dbSUCCESS) return i;
	FirstStep=OutFields;
	OutFields=NULL;
	if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
	if (Sql->Type!=synSEMICOLON) return dbSYNTAXERR;

	if ((i=addAllFieldsOut(Tables))!=dbSUCCESS) return i;

	if ((i=setInsertValue())!=dbSUCCESS) return i;

	WalkFrom=OutFields;
	len=strlen(OutFdSep);
	j=strlen(OutRdSep)+1;
	while (WalkFrom){
		j+=WalkFrom->Length+len;
		WalkFrom=(tExpression *)WalkFrom->WalkNext;
	}
	if ((i=newValue(j))!=dbSUCCESS) return i;

//	sprintf(OutFdSep,"%c",FieldSep);
//	sprintf(OutRdSep,"%c",FieldSep);
//	strcat(OutRdSep,RecordSep);

	return dbSUCCESS;
}

int tSQLRequest::parseUpdate(){
	int i,j,len;
	tExpression *WalkFrom;
	tField** PreWalk;

	if ((i=Sql->match(synUPDATE))!=dbSUCCESS) return i;
	if ((i=parseTableList())!=dbSUCCESS) return i;
	if ((i=Sql->match(synSET))!=dbSUCCESS) return i;
	if ((i=parseFieldList())!=dbSUCCESS) return i;
	FirstStep=OutFields;
	OutFields=NULL;
	if ((i=addAllFieldsOut(Tables))!=dbSUCCESS) return i;
	if ((i=setUpdateValue())!=dbSUCCESS) return i;

	if (Sql->Type==synWHERE){
		if ((i=Sql->match(synWHERE))!=dbSUCCESS) return i;
		if ((i=parseExpr(synSEMICOLON,&Root))!=dbSUCCESS) return i;
	}
	if (Sql->Type!=synSEMICOLON) return dbSYNTAXERR;

	WalkFrom=OutFields;
	len=strlen(OutFdSep);
	j=strlen(OutRdSep)+1;
	while (WalkFrom){
		if ((i=spotNode(WalkFrom))!=dbSUCCESS) return i;
		j+=WalkFrom->Length+len;
		WalkFrom=(tExpression *)WalkFrom->WalkNext;
	}
	if ((i=newValue(j))!=dbSUCCESS) return i;

	PreWalk= (tField **)&FirstStep;
//19990325 modify for Root==NULL
	if (Root!=NULL && (i=PostWalk(Root,&PreWalk))!=dbSUCCESS) return i;
//	sprintf(OutFdSep,"%c",FieldSep);
//	sprintf(OutRdSep,"%c",FieldSep);
//	strcat(OutRdSep,RecordSep);

	return dbSUCCESS;
}

int tSQLRequest::parseCreate(){
	int i;
	char TableName[PathLength];
	char FileName[PathLength];
	tRecFile WriteFile;
	char TmpStr[PathLength];
	char TmpStr1[PathLength];

	strcpy(TableName, DataBase);
	if ((i=Sql->match(synCREATE))!=dbSUCCESS) return i;
	if ((i=Sql->match(synTABLE))!=dbSUCCESS) return i;
	if (Sql->Type!=synWORD) return dbSYNTAXERR;
	strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
	TmpStr[Sql->StartLen]='\0';
	strcat(TableName,TmpStr);

	strcpy(FileName,TableName);
	strcat(FileName,InfSuf);
	Sql->match();
	WriteFile.openWrite(FileName);
	if ((i=WriteFile.startWrite())!=dbSUCCESS) return i;
	
	strcpy(WriteFile.RecordSep, InfRecordSep);
	strcpy(WriteFile.FieldSep,InfFieldSep);

	if ((i=WriteFile.addRecordField(strFIELDS,strlen(strFIELDS)))!=dbSUCCESS) return i;
	if ((i=WriteFile.writeRecord())!=dbSUCCESS) return i;
	if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
#ifdef CreateRowid
	if ((i=WriteFile.addRecordField("ROWID",5))!=dbSUCCESS) return i;
	if ((i=WriteFile.addRecordField("CHAR(18)",8))!=dbSUCCESS) return i;
	if ((i=WriteFile.addRecordField("UNIQUE",6))!=dbSUCCESS) return i;
	if ((i=WriteFile.writeRecord())!=dbSUCCESS) return i;
#endif
	do{
		if (Sql->Type!=synWORD) return dbSYNTAXERR;
		strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
		TmpStr[Sql->StartLen]='\0';
		if ((i=WriteFile.addRecordField(TmpStr,strlen(TmpStr)))!=dbSUCCESS) return i;
		Sql->match();
		strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
		TmpStr[Sql->StartLen]='\0';
		if (!strcmp(TmpStr,"number") || !strcmp(TmpStr,"NUMBER")){
			strcpy(TmpStr1,"NUMBER(");
			Sql->match();
			if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
			if (Sql->Type!=synNUMBER) return dbSYNTAXERR;
			strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
			TmpStr[Sql->StartLen]='\0';
			strcat(TmpStr1,TmpStr);
			Sql->match();
			if (Sql->Type==synCOMMA){
				Sql->match();
				if (Sql->Type!=synNUMBER) return dbSYNTAXERR;
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,",");
				strcat(TmpStr1,TmpStr);
				Sql->match();
			}
			if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
			strcat(TmpStr1,")");
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
			strcpy(TmpStr1,"");
			while (Sql->Type != synCOMMA && Sql->Type != synRIGHTBRACE && Sql->Type != synSEMICOLON && Sql->Type != synUNKNOW){
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,TmpStr);
				Sql->match();
				if (Sql->Type != synCOMMA) strcat(TmpStr1," ");
			}
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
		}else if (!strcmp(TmpStr,"char") || !strcmp(TmpStr,"CHAR")
		 || !strcmp(TmpStr,"varchar") || !strcmp(TmpStr,"VARCHAR")
		 || !strcmp(TmpStr,"varchar2") || !strcmp(TmpStr,"VARCHAR2")){
			strcpy(TmpStr1,"CHAR(");
			Sql->match();
			if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
			if (Sql->Type!=synNUMBER) return dbSYNTAXERR;
			strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
			TmpStr[Sql->StartLen]='\0';
			strcat(TmpStr1,TmpStr);
			Sql->match();
			if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
			strcat(TmpStr1,")");
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
			strcpy(TmpStr1,"");
			while (Sql->Type != synCOMMA && Sql->Type != synRIGHTBRACE && Sql->Type != synSEMICOLON && Sql->Type != synUNKNOW){
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,TmpStr);
				Sql->match();
				if (Sql->Type != synCOMMA) strcat(TmpStr1," ");
			}
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
		}else if (!strcmp(TmpStr,"date") || !strcmp(TmpStr,"DATE")){
			if ((i=WriteFile.addRecordField("DATE",4))!=dbSUCCESS) return i;
			Sql->match();
			strcpy(TmpStr1,"");	
			while (Sql->Type != synCOMMA && Sql->Type != synRIGHTBRACE && Sql->Type != synSEMICOLON && Sql->Type != synUNKNOW){
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,TmpStr);
				Sql->match();
				if (Sql->Type != synCOMMA) strcat(TmpStr1," ");
			}
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
		}else if (!strcmp(TmpStr,"long") || !strcmp(TmpStr,"LONG")){
			if ((i=WriteFile.addRecordField("LONG",4))!=dbSUCCESS) return i;
			Sql->match();
			strcpy(TmpStr1,"");	
			while (Sql->Type != synCOMMA && Sql->Type != synRIGHTBRACE && Sql->Type != synSEMICOLON && Sql->Type != synUNKNOW){
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,TmpStr);
				Sql->match();
				if (Sql->Type != synCOMMA) strcat(TmpStr1," ");
			}
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
		}else return dbSYNTAXERR;
		if ((i=WriteFile.writeRecord())!=dbSUCCESS) return i;
		if (Sql->Type!=synCOMMA && Sql->Type!=synRIGHTBRACE) return dbSYNTAXERR;
		if (Sql->Type==synCOMMA) Sql->match();
	} while (Sql->Type != synRIGHTBRACE);
	Sql->match();
	if ((i=WriteFile.closeFile())!=dbSUCCESS) return i;
	strcpy(FileName,TableName);
	strcat(FileName,DatSuf);
	WriteFile.openWrite(FileName);
	if ((i=WriteFile.startWrite())!=dbSUCCESS) return i;
	if ((i=WriteFile.closeFile())!=dbSUCCESS) return i;
	strcpy(FileName,TableName);
	strcat(FileName,TmpSuf);
	WriteFile.openWrite(FileName);
	if ((i=WriteFile.startWrite())!=dbSUCCESS) return i;
	if ((i=WriteFile.closeFile())!=dbSUCCESS) return i;
	return dbSUCCESS;
}


int tSQLRequest::parseDrop(){
	int i;
	char TableName[PathLength];
	char FileName[PathLength];

	if ((i=Sql->match(synDROP))!=dbSUCCESS) return i;
	if ((i=Sql->match(synTABLE))!=dbSUCCESS) return i;
	if (Sql->Type!=synWORD) return dbSYNTAXERR;
	strncpy(FileName,Sql->Sentence+Sql->Start,Sql->StartLen);
	FileName[Sql->StartLen]='\0';
	strcpy(TableName,DataBase);
	strcat(TableName,FileName);
	Sql->match();
	strcpy(FileName,TableName);
	strcat(FileName,InfSuf);
	tRecFile::UnlinkFile(FileName);
	strcpy(FileName,TableName);
	strcat(FileName,TmpSuf);
	tRecFile::UnlinkFile(FileName);
	strcpy(FileName,TableName);
	strcat(FileName,DatSuf);
	tRecFile::UnlinkFile(FileName);

	return dbSUCCESS;
}

int tSQLRequest::setInsertValue(){
	tExpression *WalkFrom=NULL, *NowExpr=NULL, *NowExpr1=NULL;
	tField **NowField=NULL;
		for (CurrentStep=FirstStep, WalkFrom=FirstNodeTab;
			CurrentStep!=NULL && WalkFrom!=NULL; ){
			//WalkFrom: inserted fields 
			//CurrentStep: inserted values 
			if (strcmp(WalkFrom->Name,"ROWID__")==0){
				WalkFrom=(tExpression*)WalkFrom->WalkNext;
				continue;
			}
			for (NowField=(tField **)&OutFields; 
				(*NowField)!=NULL && ((tExpression *)(*NowField))->Left!=WalkFrom->Left;
				NowField=&(*NowField)->WalkNext);
			if (*NowField){
				if (CurrentStep->Length > ((tExpression *)(*NowField))->Left->Length && CurrentStep->Type != dbLONG) {
					Flag=dbOVERFLOW; 
					break;
				}
				if ((((tExpression *)(*NowField))->Left->Type==dbNUMBER) && (CurrentStep->Type != dbNUMBER)) {
					Flag=dbTYPEERR;
					break;
				}
				NowExpr1=(tExpression *)CurrentStep->WalkNext;
				CurrentStep->WalkNext=(*NowField)->WalkNext;
				NowExpr=(tExpression *)(*NowField);
				(*NowField)=(tField *)CurrentStep;
				CurrentStep=NowExpr1;
				delete NowExpr;
				NowExpr=WalkFrom;
				WalkFrom=(tExpression *)WalkFrom->WalkNext;
				delete NowExpr;
			} else 
				break;
		}
		if (CurrentStep!=NULL || WalkFrom!=NULL) {
			while (CurrentStep!=NULL){
				NowExpr=CurrentStep;
				CurrentStep=(tExpression *)CurrentStep->WalkNext;
				delete NowExpr;
			}
			while (WalkFrom!=NULL){
				NowExpr=WalkFrom;
				WalkFrom=(tExpression *)WalkFrom->WalkNext;
				delete NowExpr;
			}
			return (Flag==dbSUCCESS)?dbSYNTAXERR:Flag;
		}
		FirstStep=FirstNodeTab=NULL;
		return dbSUCCESS;
}

int tSQLRequest::setUpdateValue(){
	tExpression *NowExpr=NULL, *NowExpr1=NULL;
	tField **NowField=NULL;
		for (CurrentStep=FirstStep;
			CurrentStep!=NULL; ){
			for (NowField=(tField **)&OutFields; 
				(*NowField) && CurrentStep->Left && CurrentStep->Right
				&& ((tExpression *)(*NowField))->Left!=((tExpression *)(CurrentStep->Left))->Left;
				NowField=&(*NowField)->WalkNext ) ;
			if ((*NowField)&& CurrentStep->Left && CurrentStep->Right){
				/* check overflow ? No.
				if (CurrentStep->Right->Length > ((tExpression *)(*NowField))->Left->Length) {
					Flag=dbOVERFLOW; 
					break;
				}
				*/
				if ((((tExpression *)(*NowField))->Left->Type==dbNUMBER) && (CurrentStep->Right->Type != dbNUMBER)) {
					Flag=dbTYPEERR;
					break;
				}
				CurrentStep->Right->WalkNext=(*NowField)->WalkNext;
				delete (tExpression *)(*NowField);
				(*NowField)=(tField *)CurrentStep->Right;
				((tExpression *)(CurrentStep->Right))->Parent=NULL;
				CurrentStep->Right=NULL;
				NowExpr1=CurrentStep;
				CurrentStep=(tExpression *)CurrentStep->WalkNext;
				delete NowExpr1;
			} else 
				break;
		}
		if (CurrentStep!=NULL) {
			while (CurrentStep!=NULL){
				NowExpr=CurrentStep;
				CurrentStep=(tExpression *)CurrentStep->WalkNext;
				delete NowExpr;
			}
			return (Flag==dbSUCCESS)?dbSYNTAXERR:Flag;
		}
		FirstStep=FirstNodeTab=LastNodeTab=NULL;
		Tables->ControlNode=NULL;
		return dbSUCCESS;

}

int tSQLRequest::parseTableList(){
	char TableName[PathLength]="";
	int j;
	tTable **NewTable;
	do{
//19990406 break -> return for the nextline.
		if (Sql->Type!=synWORD) return dbSYNTAXERR;
		strcpy(TableName,DataBase);
		strncat(TableName,Sql->Sentence+Sql->Start,Sql->StartLen);
		NewTable=&Tables;
		while (*NewTable!=NULL) NewTable=&(*NewTable)->NextTable;
		*NewTable=new tTable();
		if ((j=(*NewTable)->setFile(TableName))!=dbSUCCESS) return j;
		(*NewTable)->Action=Action;
		TableNum++;
		Sql->match();
		if (Sql->Type!=synCOMMA) break;
		Sql->match();
	}while (1);
	return dbSUCCESS;
}

int tSQLRequest::parseFieldList(){
	int j;
	tExpression **NewExpression, **NewExpr1;
	tField *First;
	do{
		NewExpression=&OutFields;
		while (*NewExpression!=NULL) NewExpression=(tExpression**)&(*NewExpression)->WalkNext;
		if ((j=parseExpr(synRIGHTBRACE,NewExpression))!=dbSUCCESS) return j;
		OutFieldNum++;
		
		NewExpr1=(tExpression **)&First;
		PostWalk(*NewExpression,(tField ***)&NewExpr1);

		if (Sql->Type!=synCOMMA) break;
		Sql->match();
	}while (1);
	if (OutFields==NULL) return dbSYNTAXERR;
	return dbSUCCESS;
}

int tSQLRequest::addAllFieldsOut(tTable *Tab){
	tField *NowField;
	int i;
	tExpression **NowExpr=&OutFields;
	while (*NowExpr) NowExpr=(tExpression **)& (*NowExpr)->WalkNext;
	for (i=0,NowField=Tab->Fields; i<Tab->FieldNum && NowField!=NULL; i++, NowField=NowField->WalkNext){
		(*NowExpr)=new tExpression();
		(*NowExpr)->cpField(NowField);
		if (strcmp((*NowExpr)->Name,"ROWID")==0) strcat((*NowExpr)->Name,"__");
		(*NowExpr)->Brand=expNODE;
		(*NowExpr)->Left=NowField;
		(*NowExpr)->Table=Tab;
		NowExpr=(tExpression **)& (*NowExpr)->WalkNext;
	}
	OutFieldNum=Tab->FieldNum;
	return dbSUCCESS;
}

int tSQLRequest::parseExpr(etSYNTAX PreOp, tExpression **Result,tExpression *SetParent){
	tExpression* NewExpr=NULL;
	tExpression* NewExpr1=NULL;
	int i;
	switch (Sql->Type){
	case synLEFTBRACE:
		Sql->match();
		if ((i=parseExpr(synRIGHTBRACE,&NewExpr))!=dbSUCCESS) return i;
		if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
		break;
		/*
	case synNOT:
		Sql->match();
		NewExpr=new tExpression(synNOT);
		if ((i=parseExpr(synNOT,(tExpression**)&(NewExpr->Left),NewExpr))!=dbSUCCESS) return i;
		//((tExpression *)(NewExpr->Left))->Parent=NewExpr;
		break;
		*/
	case synWORD:
	case synNUMBER:
	case synCHAR:
		if ((i=parseField(&NewExpr))!=dbSUCCESS) return i;
		break;
	case synNOT:
	case synSUB:
	case synADD:
	case synSUM:
	case synTO_NUMBER:
	case synCOUNT:
	case synSYSDATE:
	case synTO_CHAR:
	case synTO_DATE:
	case synADD_MONTHS:
	case synLAST_DAY:
	case synMONTHS_BETWEEN:
	case synNEXT_DAY:
	case synLTRIM:
	case synRTRIM:
	case synSUBSTR:
	case synNULL:
		if ((i=parseField(&NewExpr))!=dbSUCCESS) return i;
		break;
	case synRIGHTBRACE:
		return dbBRACEERR;
	default:
		return dbNOTSUPPORT;
	}
	while (PreOp<Sql->Type){
		NewExpr1=new tExpression(Sql->Type);
		NewExpr1->Left=(tField *)NewExpr;
		NewExpr->Parent=(tField *)NewExpr1;
		i=parseExpr(Sql->match(),(tExpression**)&(NewExpr1->Right),NewExpr1);
		if (i!=dbSUCCESS){delete NewExpr1; return i;}
		//((tExpression *)(NewExpr1->Right))->Parent=(tField *)NewExpr1;
		if (Tables){
			i=NewExpr1->checkCalcType();
			if (i!=dbSUCCESS){delete NewExpr1; return i;}
		}
		NewExpr=NewExpr1;
	}
	*Result=NewExpr;
	(*Result)->Parent=SetParent;
	return dbSUCCESS;
}
int tSQLRequest::parseField(tExpression **Result){
	char TableName[NameLength]="";
	int i;
	etSYNTAX SavedType;
	tExpression* NewExpr;

	switch (Sql->Type){
	case synNUMBER:
		*Result=new tExpression(Sql->Type);
		(*Result)->Length=Sql->StartLen;
		(*Result)->Precision=Sql->StartPrecision;
		i=(*Result)->newValue(Sql->Sentence+Sql->Start,Sql->StartLen);
		if (i!=dbSUCCESS) return i;
		Sql->match();
		break;
	case synCHAR:
		*Result=new tExpression(Sql->Type);
		(*Result)->Length=Sql->StartLen-2;
		i=(*Result)->newValue(Sql->Sentence+Sql->Start+1,Sql->StartLen-2);
		if (i!=dbSUCCESS) return i;
		Sql->match();
		break;
	case synWORD:
		*Result=new tExpression();
		(*Result)->Brand=expNODE;
		strcpy(TableName,"");
		if (Sql->NextType==synDOT){
			strncpy(TableName,Sql->Sentence+Sql->Start,Sql->StartLen);
			TableName[Sql->StartLen]='\0';
			Sql->match();
			Sql->match();
			if (Sql->Type!=synWORD) return dbSYNTAXERR;
			if (Tables==NULL){
				i=(*Result)->newValue(TableName,strlen(TableName));
				if (i!=0) return i;
			}
		}
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		if (Tables&&((i=spotTable(TableName,*Result))!=dbSUCCESS)) return i;
		Sql->match();
		break;
	// function para1
	case synNOT:
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if ((i=parseExpr(synNOT,(tExpression**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		//((tExpression*)((*Result)->Left))->Parent=(tField *)(*Result);
		break;
	// function para1
	case synSUB:
	case synADD:
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if ((i=parseExpr(synADD,(tExpression**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		//((tExpression*)((*Result)->Left))->Parent=(tField *)(*Result);
		break;
	// function(para1)
	case synSUM:
	case synTO_NUMBER:
	case synLAST_DAY:
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
		if ((i=parseExpr(synRIGHTBRACE,(tExpression**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		//((tExpression*)((*Result)->Left))->Parent=(tField *)(*Result);
		break;
	//function(para1/*)
	case synCOUNT:
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
		if (Sql->Type==synASTERISK) 
			Sql->match();
		else{
			if ((i=parseExpr(synRIGHTBRACE,(tExpression**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		}
		if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		//if ((*Result)->Left)
		//	((tExpression*)((*Result)->Left))->Parent=(tField *)(*Result);
		break;
	//function[()]
	case synSYSDATE:
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if (Sql->Type==synLEFTBRACE){
			if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
			if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
		}
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		break;
	//function
	case synNULL:
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		break;
	//function(para1,para2)
	case synTO_CHAR:
	case synTO_DATE:
	case synADD_MONTHS:
	case synMONTHS_BETWEEN:
	case synNEXT_DAY:
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
		if ((i=parseExpr(synCOMMA,(tExpression**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		//((tExpression*)((*Result)->Left))->Parent=(tField *)(*Result);
		if ((i=Sql->match(synCOMMA))!=dbSUCCESS) return i;
		if ((i=parseExpr(synRIGHTBRACE,(tExpression**)&(*Result)->Right,*Result))!=dbSUCCESS) return i;
		//((tExpression*)((*Result)->Right))->Parent=(tField *)(*Result);
		if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		break;
	//function(para1[,para2])
	case synLTRIM:
	case synRTRIM:
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
		if ((i=parseExpr(synRIGHTBRACE,(tExpression**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		//((tExpression*)((*Result)->Left))->Parent=(tField *)(*Result);
		if (Sql->Type==synCOMMA){
			if ((i=Sql->match(synCOMMA))!=dbSUCCESS) return i;
			if ((i=parseExpr(synRIGHTBRACE,(tExpression**)&(*Result)->Right,*Result))!=dbSUCCESS) return i;
			//((tExpression*)((*Result)->Right))->Parent=(tField *)(*Result);
		}
		if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		break;
	//function(para1,para2[,para3])
	case synSUBSTR:
		SavedType=Sql->Type;
		*Result=new tExpression(Sql->Type);
		strncpy((*Result)->Name,Sql->Sentence+Sql->Start,Sql->StartLen);
		(*Result)->Name[Sql->StartLen]='\0';
		Sql->match();
		if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
		if ((i=parseExpr(synCOMMA,(tExpression**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		//((tExpression*)((*Result)->Left))->Parent=(tField *)(*Result);
		if ((i=Sql->match(synCOMMA))!=dbSUCCESS) return i;
		if ((i=parseExpr(synRIGHTBRACE,(tExpression**)&(*Result)->Right,*Result))!=dbSUCCESS) return i;
		//((tExpression*)((*Result)->Right))->Parent=(tField *)(*Result);
		if (Sql->Type==synCOMMA){
			if ((i=Sql->match(synCOMMA))!=dbSUCCESS) return i;
			if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
			NewExpr=new tExpression(SavedType);
			strcpy(NewExpr->Name,(*Result)->Name);
			NewExpr->Left=*Result;
			(*Result)->Parent=NewExpr;
			*Result=NewExpr;
			(*Result)->Precision=1;
			if ((i=parseExpr(synRIGHTBRACE,(tExpression**)&(*Result)->Right,*Result))!=dbSUCCESS) return i;
			//((tExpression*)((*Result)->Right))->Parent=(tField *)(*Result);
		}
		if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;
		break;
	default:
		return dbSYNTAXERR;
		break;
	}
	return dbSUCCESS;
}

int tSQLRequest::spotTable(char *TableName, tExpression* SpotField){
	tTable *SearchTable=Tables;
	tField* NowField;
	int i,j;
	while (SearchTable!=NULL){
		if (*TableName=='\0'||strcmp(TableName,SearchTable->Name)==0){
			for (j=0 , NowField=SearchTable->Fields; 
				(j<SearchTable->FieldNum) && (NowField!=NULL); 
				j++, NowField=NowField->WalkNext){
				if (strcmp(NowField->Name, SpotField->Name)==0 
					|| (strcmp(NowField->Name,"ROWID")==0 
					&& strcmp(SpotField->Name,"ROWID__")==0) 
					){
					SpotField->Length=NowField->Length;
					SpotField->Precision=NowField->Precision;
					SpotField->Type=NowField->Type;
					SpotField->Left=NowField;
					SpotField->Table=SearchTable;
					if (SearchTable->ControlNode==NULL){
						SearchTable->ControlNode=SpotField;
						SpotField->Right=LastNodeTab;
						LastNodeTab=SpotField;
						if (FirstNodeTab==NULL)
							FirstNodeTab=SpotField;
						else
							if ((i=SearchTable->getRecord())!=dbSUCCESS) return i;
					}
					return dbSUCCESS;
				}
			}
		}
		SearchTable=SearchTable->NextTable;
	}
#if (ERRORLOG > 1)
	char tmp[100];
	sprintf(tmp,"%s.%s",TableName,SpotField->Name);
	Log::add(ERRORLOGFILE,0,tmp,__LINE__);
#endif
	return dbFIELDNOTFOUND;
}

int tSQLRequest::spotNode(tExpression* WalkStart){
	int i;
	if (WalkStart->Brand!=expNODE){
		if (WalkStart->Left!=NULL) {
			i=spotNode((tExpression *)WalkStart->Left);
			if (i!=dbSUCCESS) return i;
		}
		if (WalkStart->Right!=NULL) {
			i=spotNode((tExpression *)WalkStart->Right);
			if (i!=dbSUCCESS) return i;
		}
	}
	if (WalkStart->Brand!=expNODE) 
		return WalkStart->checkCalcType();
	if (WalkStart->Value){
		i=spotTable(WalkStart->Value,WalkStart);
		WalkStart->deleteValue();
	}else{
		i=spotTable("",WalkStart);
	}
	return i;
}

int tSQLRequest::PostWalk(tExpression* WalkStart, tField ***PreWalk){
	int i;
	if (WalkStart==NULL) return dbSUCCESS;
	if (WalkStart->Brand!=expNODE){
		if (WalkStart->Left!=NULL) {
			i=PostWalk( (tExpression *)WalkStart->Left,PreWalk);
			if (i!=dbSUCCESS) return i;
		}
		if (WalkStart->Right!=NULL) {
			i=PostWalk( (tExpression *)WalkStart->Right,PreWalk);
			if (i!=dbSUCCESS) return i;
		}
	}
	**PreWalk=(tField *)WalkStart;
	*PreWalk=&WalkStart->WalkNext;
	if (WalkStart->CalcOp==synSUM || WalkStart->CalcOp==synCOUNT)
		GroupAction=1;
	return dbSUCCESS;
}

int tSQLRequest::printOut(){
	tExpression *NowField;
	tExpression *NowExpr;
	tField *TableField=Tables->Fields;
	int i,j;
	Value[0]='\0';
	for (i=0,NowField=(tExpression *)OutFields;(i<OutFieldNum) && (NowField!=NULL); i++, NowField=(tExpression *)NowField->WalkNext){
		for (NowExpr=NowField; NowExpr->Brand!=expNODE && NowExpr->Left!=NULL; NowExpr=(tExpression *)NowExpr->Left);
		while(1){
			if ((j=NowExpr->Calculate())!=dbSUCCESS) return j;
			if (NowExpr->Parent==NULL) break;
			NowExpr=(tExpression *)NowExpr->WalkNext;
		}
		if (Action==synINSERT && !strcmp(NowField->Name,"ROWID__")){
			if ((j=setRowid(NowField))!=dbSUCCESS) return j;
		}
		if (Action==synUPDATE || Action==synINSERT) {
			if (!TableField || ((long)strlen(NowField->Value) > TableField->Length && TableField->Type != dbLONG) )
				return dbOVERFLOW;
			TableField=TableField->WalkNext;
		}
/*		if (Action!=synSELECT || strcmp(NowField->Name,"ROWID__")!=0){
			if (NowField->Value) strcat(Value,NowField->Value);
			if (i<OutFieldNum-1) strcat(Value,OutFdSep);
		}
		*/  //replace to the following lines 
		if (Action!=synSELECT && Action!=synDELETE){
			if (! Tables->TmpFile && (i=Tables->openTmp())!=dbSUCCESS) return i;
			if (NowField->Value) Tables->TmpFile->addRecordField(NowField->Value, strlen(NowField->Value));
		} else if (strcmp(NowField->Name,"ROWID__")!=0) {
			if (strlen(Value)+strlen(NowField->Value)>Length){
				expandValue(strlen(Value)+strlen(NowField->Value)+1);
			}
			if (NowField->Value) strcat(Value,NowField->Value);
			if (i<OutFieldNum-1) strcat(Value,OutFdSep);
		}
	}
	if (Action!=synSELECT && Action!=synDELETE ){
		if (! Tables->TmpFile && (i=Tables->openTmp())!=dbSUCCESS) return i;
		Tables->TmpFile->writeRecord();
	}else{
		if (strlen(Value)+strlen(OutRdSep)>Length){
			expandValue(strlen(Value)+strlen(OutRdSep)+1);
		}
		strcat(Value,OutRdSep);
	}
	return dbSUCCESS;
}
int tSQLRequest::setRowid(tField* Field){
	sprintf(Field->Value,"%08d-%04d-%04d",time(NULL)%100000000,Tables->RecordCount/10000%10000, Tables->RecordCount%10000);
	return dbSUCCESS;
}

Frmem::Frmem(){
	SavedPointer=NULL;
}

Frmem::~Frmem(){
	if ( SavedPointer!=NULL) 
		free(SavedPointer);
	SavedPointer=NULL;
}

void Frmem::setP(char *p){
	if ( SavedPointer!=NULL) 
		free(SavedPointer);
	SavedPointer=p;
	return;
}

/*
tSQLCreate::tSQLCreate(char * Path){
	Sql=NULL;
	if (Path) setPath(Path);
}

int tSQLCreate::setPath(char * Path){
	strcpy(TableName,"");
	if (Path) strcat(TableName,Path);
	return dbSUCCESS;
}
int tSQLCreate::parseCreate(){
	int i;
	char FileName[PathLength];
	char TmpStr[PathLength];
	char TmpStr1[PathLength];

	if ((i=Sql->match(synCREATE))!=dbSUCCESS) return i;
	if ((i=Sql->match(synTABLE))!=dbSUCCESS) return i;
	if (Sql->Type!=synWORD) return dbSYNTAXERR;
	strncpy(FileName,Sql->Sentence+Sql->Start,Sql->StartLen);
	FileName[Sql->StartLen]='\0';
	strcat(TableName,FileName);
	strcpy(FileName,TableName);
	strcat(FileName,InfSuf);
	Sql->match();
	WriteFile.openWrite(FileName);
	if ((i=WriteFile.startWrite())!=dbSUCCESS) return i;
	
	strcpy(WriteFile.RecordSep, InfRecordSep);
	strcpy(WriteFile.FieldSep,InfFieldSep);

	if ((i=WriteFile.addRecordField(strFIELDS,strlen(strFIELDS)))!=dbSUCCESS) return i;
	if ((i=WriteFile.writeRecord())!=dbSUCCESS) return i;
	if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
#ifdef CreateRowid
	if ((i=WriteFile.addRecordField("ROWID"))!=dbSUCCESS) return i;
	if ((i=WriteFile.addRecordField("CHAR(18)"))!=dbSUCCESS) return i;
	if ((i=WriteFile.addRecordField("UNIQUE"))!=dbSUCCESS) return i;
	if ((i=WriteFile.writeRecord())!=dbSUCCESS) return i;
#endif
	do{
		if (Sql->Type!=synWORD) return dbSYNTAXERR;
		strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
		TmpStr[Sql->StartLen]='\0';
		if ((i=WriteFile.addRecordField(TmpStr,strlen(TmpStr)))!=dbSUCCESS) return i;
		Sql->match();
		strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
		TmpStr[Sql->StartLen]='\0';
		if (!strcmp(TmpStr,"number") || !strcmp(TmpStr,"NUMBER")){
			strcpy(TmpStr1,"NUMBER(");
			Sql->match();
			if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
			if (Sql->Type!=synNUMBER) return dbSYNTAXERR;
			strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
			TmpStr[Sql->StartLen]='\0';
			strcat(TmpStr1,TmpStr);
			Sql->match();
			if (Sql->Type==synCOMMA){
				Sql->match();
				if (Sql->Type!=synNUMBER) return dbSYNTAXERR;
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,",");
				strcat(TmpStr1,TmpStr);
				Sql->match();
			}
			if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
			strcat(TmpStr1,")");
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
			strcpy(TmpStr1,"");
			while (Sql->Type != synCOMMA && Sql->Type != synRIGHTBRACE && Sql->Type != synSEMICOLON && Sql->Type != synUNKNOW){
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,TmpStr);
				Sql->match();
				if (Sql->Type != synCOMMA) strcat(TmpStr1," ");
			}
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
		}else if (!strcmp(TmpStr,"char") || !strcmp(TmpStr,"CHAR")
		 || !strcmp(TmpStr,"varchar") || !strcmp(TmpStr,"VARCHAR")
		 || !strcmp(TmpStr,"varchar2") || !strcmp(TmpStr,"VARCHAR2")){
			strcpy(TmpStr1,"CHAR(");
			Sql->match();
			if ((i=Sql->match(synLEFTBRACE))!=dbSUCCESS) return i;
			if (Sql->Type!=synNUMBER) return dbSYNTAXERR;
			strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
			TmpStr[Sql->StartLen]='\0';
			strcat(TmpStr1,TmpStr);
			Sql->match();
			if ((i=Sql->match(synRIGHTBRACE))!=dbSUCCESS) return i;
			strcat(TmpStr1,")");
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
			strcpy(TmpStr1,"");
			while (Sql->Type != synCOMMA && Sql->Type != synRIGHTBRACE && Sql->Type != synSEMICOLON && Sql->Type != synUNKNOW){
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,TmpStr);
				Sql->match();
				if (Sql->Type != synCOMMA) strcat(TmpStr1," ");
			}
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
		}else if (!strcmp(TmpStr,"date") || !strcmp(TmpStr,"DATE")){
			if ((i=WriteFile.addRecordField("DATE",4))!=dbSUCCESS) return i;
			Sql->match();
			strcpy(TmpStr1,"");	
			while (Sql->Type != synCOMMA && Sql->Type != synRIGHTBRACE && Sql->Type != synSEMICOLON && Sql->Type != synUNKNOW){
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,TmpStr);
				Sql->match();
				if (Sql->Type != synCOMMA) strcat(TmpStr1," ");
			}
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
		}else if (!strcmp(TmpStr,"long") || !strcmp(TmpStr,"LONG")){
			if ((i=WriteFile.addRecordField("LONG",4))!=dbSUCCESS) return i;
			Sql->match();
			strcpy(TmpStr1,"");	
			while (Sql->Type != synCOMMA && Sql->Type != synRIGHTBRACE && Sql->Type != synSEMICOLON && Sql->Type != synUNKNOW){
				strncpy(TmpStr,Sql->Sentence+Sql->Start,Sql->StartLen);
				TmpStr[Sql->StartLen]='\0';
				strcat(TmpStr1,TmpStr);
				Sql->match();
				if (Sql->Type != synCOMMA) strcat(TmpStr1," ");
			}
			if ((i=WriteFile.addRecordField(TmpStr1,strlen(TmpStr1)))!=dbSUCCESS) return i;
		}else return dbSYNTAXERR;
		if ((i=WriteFile.writeRecord())!=dbSUCCESS) return i;
		if (Sql->Type!=synCOMMA && Sql->Type!=synRIGHTBRACE) return dbSYNTAXERR;
		if (Sql->Type==synCOMMA) Sql->match();
	} while (Sql->Type != synRIGHTBRACE);
	Sql->match();
	if ((i=WriteFile.closeFile())!=dbSUCCESS) return i;
	strcpy(FileName,TableName);
	strcat(FileName,DatSuf);
	WriteFile.openWrite(FileName);
	if ((i=WriteFile.startWrite())!=dbSUCCESS) return i;
	if ((i=WriteFile.closeFile())!=dbSUCCESS) return i;
	strcpy(FileName,TableName);
	strcat(FileName,TmpSuf);
	WriteFile.openWrite(FileName);
	if ((i=WriteFile.startWrite())!=dbSUCCESS) return i;
	if ((i=WriteFile.closeFile())!=dbSUCCESS) return i;
	return dbSUCCESS;
}

*/

