#include "expr.h"
#include "htmlql.h"
#include "alignment.h"
#include "referlink2.h"
#include "docbase.h" 
#include "RegExParser.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "platform.h"
#include "dirfunc.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



void ExprSyntax::findNext(){
	int sense=0; 
	if (Sentence[Next]==preEXP_CSTDATE && (isDigit(Sentence[Next+NextLen])|| Sentence[Next+NextLen]==preEXP_CSTDATE)){
		while (Sentence[Next+NextLen] && Sentence[Next+NextLen]!=preEXP_CSTDATE){
			NextLen++;
		}
		if (Sentence[Next+NextLen]==preEXP_CSTDATE)
			NextLen++;
		NextType = synEXP_DATE;
	}else if (Sentence[Next]==preEXP_PARA && preEXP_PARA == '%' &&
		( 
			(!isAlpha(Sentence[Next+NextLen]) && !isDigit(Sentence[Next+NextLen])) 
			|| Type==synEXP_RIGHTBRACE || Type==synEXP_WORD || Type==synEXP_NUMBER) 
		){
		NextType = synEXP_MOD;
	}else if (isAlpha(Sentence[Next]) 
		|| Sentence[Next]==preEXP_DATE 
		|| Sentence[Next]==preEXP_NUMBER 
		|| Sentence[Next]==preEXP_CHAR
		|| Sentence[Next]==preEXP_PARA){
#ifdef CASEINSENSITIVE
		Sentence[Next]=toupper(Sentence[Next]);
#endif
		while (isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen])){
#ifdef CASEINSENSITIVE
			Sentence[Next+NextLen]=toupper(Sentence[Next+NextLen]);
#endif
			NextLen++;
		}
//		if (NextLen==1 && Sentence[Next]==preEXP_PARA){
//			NextType=synEXP_MOD;
//		}else{
			NextType=synEXP_WORD;
			NextType=KeyWord();
//		}
	}else if (isDigit(Sentence[Next])){
		while (isDigit(Sentence[Next+NextLen]) ) NextLen++;
		NextType=synEXP_NUMBER;
		if (Sentence[Next+NextLen] == '.'){
			NextLen++;
			NextPrecision=0;
			while (isDigit(Sentence[Next+NextLen])){
				NextLen++;
				NextPrecision++;
			}
		}
		if (Sentence[Next+NextLen] == 'e' || Sentence[Next+NextLen] == 'E'){
			NextLen++;
			if (Sentence[Next+NextLen] == '+' || Sentence[Next+NextLen] == '-'){
				NextLen++;
			}
			while (isDigit(Sentence[Next+NextLen])){
				NextLen++;
			}
		}
	}else{
		switch (Sentence[Next]){
		case '\'':
			while (1){
				if (Sentence[Next+NextLen]=='\\'){ 
					NextLen++; if (Sentence[Next+NextLen]!='\0') NextLen++;
				}else if (Sentence[Next+NextLen]!='\'' && Sentence[Next+NextLen]!='\0') NextLen++;
				else if (Sentence[Next+NextLen]=='\'' && Sentence[Next+NextLen+1]=='\''){
					//for (i=Next+NextLen+1; Sentence[i]!='\0'; i++) Sentence[i]=Sentence[i+1];
					NextLen+=2;
				}
				else break;
			}
			if (Sentence[Next+NextLen]=='\'') {
				NextLen++;
				NextType=synEXP_CHAR;
			}else NextType=synEXP_UNKNOW;
			break;
		case '"':
			while (1){
				if (Sentence[Next+NextLen]!='"' && Sentence[Next+NextLen]!='\0') NextLen++;
				else break;
			}
			if (Sentence[Next+NextLen]=='"') {
				NextLen++;
				NextType=synEXP_WORD;
			}else NextType=synEXP_UNKNOW;
			break;
		case '[':
			while (Sentence[Next+NextLen] && Sentence[Next+NextLen]!=']'){
				NextLen++;
			}
			if (Sentence[Next+NextLen]==']')
				NextLen++;
			NextType = synEXP_WORD;
			break;
		case '!':
			switch (Sentence[Next+NextLen]){
			case '=':
				NextLen++;
				NextType=synEXP_NE;
				break;
			default:
				NextType=synEXP_NOT;
			}
			break;
		case '<':
			switch (Sentence[Next+NextLen]){
			case '>':
				NextLen++;
				NextType=synEXP_NE;
				break;
			case '=':
				NextLen++;
				NextType=synEXP_LE;
				break;
			default:
				NextType=synEXP_LT;
			}
			break;
		case '>':
			if (Sentence[Next+NextLen]=='='){
				NextLen++;
				NextType=synEXP_GE;
			}else NextType=synEXP_GT;
			break;
		case '=':
			if (Sentence[Next+NextLen]=='~'){
				NextLen++;
				sense=0; 
				if (Sentence[Next+NextLen]=='~') {
					NextLen++;
					sense=1; 
				}
				NextType=sense?syn_REGEX_MATCH_CASE:syn_REGEX_MATCH;
			}else if (Sentence[Next+NextLen]=='=') {
				NextLen++;
				NextType=synEXP_EQ;
			}else{
				NextType=synEXP_EQ;
			}
			break;
		case '~':
			sense=0; 
			if (Sentence[Next+NextLen]=='~') {
				NextLen++;
				sense=1; 
			}
			if (Sentence[Next+NextLen]=='=') {
				NextLen++;
				NextType=sense?syn_REGEX_MATCH_CASE:syn_REGEX_MATCH;
			}else {
				NextType=sense?syn_REGEX_SEARCH_CASE:syn_REGEX_SEARCH;
			}
			break;
		case '.':
			if (isDigit(Sentence[Next+NextLen])){
				NextLen++;
				NextPrecision++;
				while (isDigit(Sentence[Next+NextLen])) NextLen++;
				NextType=synEXP_NUMBER;
			}else{
				NextType=synEXP_DOT;
			}
			break;
		case '+':
			if (Sentence[Next+NextLen]=='&'){
				NextLen++;
				NextType=synEXP_STRCAT; 
			}else{
				NextType=synEXP_ADD;
			}
			break;
		case '-':
			if (Sentence[Next+NextLen]=='>'){
				NextLen++;
				NextType = synEXP_ARROW;
			}else{
				NextType=synEXP_SUB;
			}
			break;
		case '(':
			NextType=synEXP_LEFTBRACE;
			break;
		case ')':
			NextType=synEXP_RIGHTBRACE;
			break;
		case ',':
			NextType=synEXP_COMMA;
			break;
		case '/':
			if (Sentence[Next+NextLen]=='&'){
				NextLen++;
				NextType=synEXP_NULL_OR;
			}else 
				NextType=synEXP_DIV;
			break;
		case '%': //already handled in preEXP_PARA
			NextType=synEXP_MOD;
			break;
		case '?': 
			NextType=synEXP_QUESTION;
			break;
		case ':': 
			NextType=synEXP_COLON;
			break;
		case '*':
			NextType=synEXP_ASTERISK;
			break;
		case ';':
			NextType=synEXP_SEMICOLON;
			break;
		case '\0':
			NextType=synQL_END;
			NextLen=0;
			break;
		default:
			NextType=synQL_UNKNOW;
			NextLen=0;
			break;
		}
	}
	return;

}

int ExprSyntax::KeyWord(){
	if (NextType!=synEXP_WORD) return NextType;
	if (NextLen==3&&cmpNoCase(Sentence+Next, NextLen, "AND",NextLen)==0) return synEXP_AND;
	if (NextLen==2&&cmpNoCase(Sentence+Next, NextLen, "OR",NextLen)==0) return synEXP_OR;
	if (NextLen==3&&cmpNoCase(Sentence+Next, NextLen, "NOT",NextLen)==0) return synEXP_NOT;
	if (NextLen==2&&cmpNoCase(Sentence+Next, NextLen, "IN",NextLen)==0) return synEXP_IN;
	//if (NextLen==7&&cmpNoCase(Sentence+Next, NextLen, "MATCHES",NextLen)==0) return synEXP_MATCHES;
	if (NextLen==4&&cmpNoCase(Sentence+Next, NextLen, "LIKE",NextLen)==0) return synEXP_LIKE;
	if (NextLen==2&&cmpNoCase(Sentence+Next, NextLen, "IS",NextLen)==0) return synEXP_IS;
	//if (NextLen==3&&cmpNoCase(Sentence+Next, NextLen, "SUM",NextLen)==0) return synEXP_SUM;
	//if (NextLen==5&&cmpNoCase(Sentence+Next, NextLen, "COUNT",NextLen)==0) return synEXP_COUNT;
	if (NextLen==4&&cmpNoCase(Sentence+Next, NextLen, "NULL",NextLen)==0) return synEXP_NULL;
	if (NextLen==7&&cmpNoCase(Sentence+Next, NextLen, "SYSDATE",NextLen)==0) return synEXP_SYSDATE;
	return synEXP_WORD;

}

int ExprSyntax::takeSyntaxString(int type, long start, long len, ReferData* result){
	if (type==synEXP_CHAR){
		result->Set(Sentence+start+1, len-2, true);
		if (result->L){
			tStrOp::dequoteString(result->P);
			result->L=strlen(result->P);
		}
	}else{
		result->Set(Sentence+start, len, true);
	}
	return 0;
}

tExprField::tExprField(){
	Name=0;
	Length=0;
	Precision=0;
	Type=dbCHAR;
	Value=NULL;
	DoubleValue=0.0;
	BoolValue=True;
	FunValue=0;
	WalkNext=NULL;
}
tExprField::tExprField(char *name){
	setName(name, strlen(name), true);
	NamePrefix.reset();
	Length=0;
	Precision=0;
	Type=dbCHAR;
	Value=NULL;
	DoubleValue=0.0;
	BoolValue=True;
	FunValue=0;
	WalkNext=NULL;
}

tExprField::~tExprField(){
	reset();
}

void tExprField::reset(){
	Name=0;
	StringName.reset();
	NamePrefix.reset();
	Length=0;
	Precision=0;
	Type=dbCHAR;
	deleteValue();
	Value=0;
	StringValue.reset();
	DoubleValue=0.0;
	BoolValue=True;
	FunValue=0;
	RefName.reset();
	WalkNext=NULL;
}

int tExprField::cpField(tExprField* from){
	setName(from->Name);
	NamePrefix = from->NamePrefix.P;
	Length=from->Length;
	Precision=from->Precision;
	Type=from->Type;
	DoubleValue=from->DoubleValue;
	BoolValue=from->BoolValue;
	FunValue = from->FunValue;
	StringValue = from->StringValue;
	Value = StringValue.P;
	RefName = from->RefName.P;
	return dbSUCCESS;
}

int tExprField::setType(char *TypeStr){
	int i;

	if (StrCmp(TypeStr,strLONG, false)==0){
		Length=LongLength;
		Precision=0;
		Type=dbLONG;
		//newValue(Length);
		return dbSUCCESS;
	}
	if (StrCmp(TypeStr,strDATE, false)==0){
		Length=DateLength;
		Precision=0;
		Type=dbDATE;
		//newValue(Length);
		return dbSUCCESS;
	}
	if (strNcmp(TypeStr,strNUMBER,6, false)==0){
		Length=0;
		Precision=0;
		i=sscanf(TypeStr+7,"%d,%d",&Length,&Precision);
		//if ((i!=1)&&(i!=2)) return dbINDEXERR;
		//if (i==1) Precision=0;
		if (Length==0) Length = NumberDefLength;
		if (i<2) Precision = NumberDefPrecision;
		Type=dbNUMBER;
		//newValue(Length);
		return dbSUCCESS;
	}
	if (strNcmp(TypeStr,strCHAR,4, false)==0){
		Length=0;
		i=sscanf(TypeStr+5,"%d",&Length);
		//if (i!=1) return dbINDEXERR;
		if (i!=1){
			Length = LongLength;
		}
		Precision=0;
		Type=dbCHAR;
		//newValue(Length);
		return dbSUCCESS;
	}

	return dbINDEXERR;
}

int tExprField::newValue(char *Str, long Len, int copy){
	StringValue.Set(Str, Len, copy);
	Value = StringValue.P;
	DoubleValue=0;
	BoolValue=0;
//	if (Value && (isdigit(Value[0]) || ((Value[0]=='+' || Value[0]=='-') && isdigit(Value[1]) ) ) ){
	if (Value){
		sscanf(Value, "%lf", &DoubleValue);
		BoolValue=(int) DoubleValue;
	}
	Length = Len;
	return dbSUCCESS;
}


int tExprField::newValue(long Len){
	StringValue.Malloc(Len);
	Value = StringValue.P;
	Length = Len;
	DoubleValue=0;
	BoolValue=0;
	return dbSUCCESS;
}
int tExprField::newDoubleValue(double val, int len, int precision){
	newValue((len+precision>NumberDefLength)?len+precision:NumberDefLength);
	DoubleValue=val;
	Length=len;
	Precision=precision;
	BoolValue=(int) DoubleValue;
	StringPrintf(Value,DoubleValue,Length,Precision);
	StringValue.L=Length;
	Type=dbNUMBER;
	return dbSUCCESS;
}
int tExprField::newDateValue(time_t t){
	newValue(DateLength);
	DoubleValue=t;
	BoolValue=(int) DoubleValue;
	Precision=0;

	int i;
#ifdef DateStandardFormat
	i=DateToChar(t,DateFormat,Value);
#else
	i=StringPrintf(Value,DoubleValue,Length,Precision);
#endif

	StringValue.L=Length;
	Type = dbDATE;
	return i;
}

int tExprField::deleteValue(){
	StringValue.reset();
	Value=0;
	Length=0;
	BoolValue=0;
	DoubleValue=0.0;
	return dbSUCCESS;
}

int tExprField::setName(char* name, long len, int copy){
	long i=0, j=len;  
	int type=getNameStartEnd(name, &i, &j); 
	
	StringName.Set(name+i, j, copy);
	Name=StringName.P;
	return 0;
}
int tExprField::getNameStartEnd(const char* name, long* i, long* len){
	//caller must initialize *i and *len
	if (!name) return 0; 

	int ret=0; 
	if (strchr(preEXP_TYPE_PREFIX, name[*i])){ //type prefix
		ret=name[*i]; 
		(*i)++; 
		if ((*len)>0 && strchr(preEXP_TYPE_PREFIX, name[(*len)-1])) (*len)--;
	}
	if (strchr(preEXP_WORD_PREFIX, name[*i])){ //word prefix
		(*i)++; 
		if ((*len)>0 && strchr(preEXP_WORD_PREFIX, name[(*len)-1])) (*len)--;
	}
	*len-=*i; 

	return ret;
}

int tExprField::setName(char* name){
	return setName(name, name?strlen(name):0, true); 
}

tExprItem::tExprItem(){
	Brand=expUNKNOW;
	CalcOp=synEXP_UNKNOW;
	Parent=Left=Right=NULL;
	Passed=False;
	IsNewGroup=true;
}

tExprItem::tExprItem(int SetCalOp){
	Brand=expUNKNOW;
	CalcOp=(etEXP_SYNTAX)SetCalOp;
	Parent=Left=Right=NULL;
	Passed=False;
	IsNewGroup=true;

	if (CalcOp==synEXP_AND||CalcOp==synEXP_OR||CalcOp==synEXP_NOT||CalcOp==synEXP_LIKE || CalcOp==synEXP_IS) 
		Brand=expBOOL;
	else if (CalcOp==synEXP_EQ||CalcOp==synEXP_LT||CalcOp==synEXP_LE||CalcOp==synEXP_GT||CalcOp==synEXP_GE||CalcOp==synEXP_NE || CalcOp==syn_REGEX_MATCH ||CalcOp==syn_REGEX_MATCH_CASE) 
		Brand=expCOMP;
	else if (CalcOp==synEXP_ADD||CalcOp==synEXP_SUB||CalcOp==synEXP_ASTERISK
		||CalcOp==synEXP_DIV||CalcOp==synEXP_NULL_OR || CalcOp==synEXP_MOD
		||CalcOp==synEXP_QUESTION || CalcOp==syn_REGEX_SEARCH || CalcOp==syn_REGEX_SEARCH_CASE || CalcOp==synEXP_STRCAT)
		Brand=expCALC;
	else if (CalcOp==synEXP_NUMBER) {
		Brand=expCONST;
		Type=dbNUMBER;
	}else if (CalcOp==synEXP_DATE) {
		Brand=expCONST;
		Type=dbDATE;
	}else if (CalcOp==synEXP_CHAR) {
		Brand=expCONST;
		Type=dbCHAR;
	}
}

tExprItem::~tExprItem(){
	reset();
}

void tExprItem::reset(){
	if (Brand!=expNODE){
		/*
			tExprItem* n;
			while (Left){
			n=(tExprItem*)Left;
			Left=n->Left;
			n->Left=0;
			delete n;
		}
		while (Right){
			n=(tExprItem*)Right;
			Right=n->Right;
			n->Right=0;
			delete n;
		}*/
		if (Left!=NULL) delete (tExprItem*)Left;
		if (Right!=NULL) delete (tExprItem*) Right;
	}
	Parent=Left=Right=NULL;
	Brand=expUNKNOW;
	CalcOp=synEXP_UNKNOW;
	Passed=False;
	IsNewGroup=true;

	tExprField::reset();
}
int tExprItem::checkNumber(int* is_digit, int* is_const){
	*is_digit=false;
	*is_const=false; 
	if (!this ) return false; 
	*is_const = (Brand==expCONST);

	if (!Value) return false;

	int i=0, j=0; 
	char* dot=0;
	for (i=0; i<100 && Value[i]==' ' || Value[i]=='\t'||Value[i]=='\n' ||Value[i]=='\r'; i++);
	if (Value[i]=='-' || Value[i]=='+') i++;
	while (i<100 && Value[i]==' ' || Value[i]=='\t'||Value[i]=='\n' ||Value[i]=='\r') i++;
	if (Value[i]=='.') {
		dot=Value+i;
		i++; 
	}
	*is_digit=tStrOp::isDigit(Value[i]);

	if(*is_digit){
		const char* allowed="0123456789.eE+- \t\r\n"; 
		for (j=i+1; j<100 && Value[j]; j++){
			if (!strchr(allowed, Value[j])){
				*is_digit=false;
				break; 
			}
		}
	}

	if (*is_digit){
		if (!dot) dot=strchr(Value, '.'); 
		if (dot) {
			int precision=j-(dot-Value); 
			if (precision>Precision && precision<100) Precision=precision; 
		}
	}
	if (*is_digit && DoubleValue<Epsilon && -DoubleValue<Epsilon){
		sscanf(Value, "%lf", &DoubleValue); 
	}
	return *is_digit;
}

int tExprItem::Calculate(tExprCalc* calc, int initial_group){
	int i;//,n;
	time_t now=0;
	ReferData func_name;
	ReferLink* link;
	ReferLinkHeap results; 
	RegExParser regex; 
	if (calc && calc->RegExContext) regex.Context = calc->RegExContext; 

	//checkCalcType();
	int leftdigit=0, rightdigit=0; 
	int leftconst=0, rightconst=0; 
	if (Left) ((tExprItem*)Left)->checkNumber(&leftdigit, &leftconst); 
	if (Right) ((tExprItem*)Right)->checkNumber(&rightdigit, &rightconst); 

	switch (Brand){
	case expNODE:
		cpField(Left);
		if (Left->Type==dbNUMBER && !StrCmp(Value,"", false)){
			newValue("0", 1, true);
		}
		if ((Left->Type==dbCHAR || Left->Type==dbLONG) && Value){
			DoubleValue=0;
			sscanf(Value, "%lf", &DoubleValue);
		}
		//set value
	case expCONST:
		switch (Type) {
		case dbNUMBER:
			DoubleValue=0.0;
			if (Value) sscanf(Value, "%lf", &DoubleValue);
			Length = Value?strlen(Value):0;
			break;
		case dbDATE:
#ifdef DateStandardFormat
			DateToLong(Value,DateFormat,&now);
			DoubleValue=now;
#else
			DoubleValue=0.0;
			sscanf(Value, "%lf", &DoubleValue);
#endif
			Length = strlen(Value);
		case dbCHAR:
		case dbLONG:
			DoubleValue=0.0;
			if (Value) sscanf(Value, "%lf", &DoubleValue);
			break; //convert special characters? may convert multiple times. do it elsewhere.
		default:
			break;
		}
		BoolValue=(DoubleValue!=0);
		break;
	case expBOOL:
		Length=1; Precision=0;
		newValue(NumberDefLength);
		switch (CalcOp){
		case synEXP_NOT:
			BoolValue=!(((tExprItem *)Left)->BoolValue);
			break;
		case synEXP_OR:
			BoolValue=(((tExprItem *)Left)->BoolValue||((tExprItem*)Right)->BoolValue);
			break;
		case synEXP_AND:
			BoolValue=(((tExprItem *)Left)->BoolValue&&((tExprItem *)Right)->BoolValue);
			break;
		case synEXP_LIKE:
			BoolValue=StringLike(((tExprItem *)Left)->Value,((tExprItem *)Right)->Value);
			break;
		case synEXP_IS:
			if (((tExprItem *)Right)->Brand == expBOOL){
				BoolValue = ((tExprItem *)Left)->Value&&strcmp(((tExprItem *)Left)->Value, "")
					?((tExprItem *)Right)->BoolValue
					:!((tExprItem *)Right)->BoolValue;
			}else{
				BoolValue=!StrCmp(((tExprItem *)Left)->Value,((tExprItem *)Right)->Value, false);
			}
			break;
		default:
			BoolValue=False;
		}
		DoubleValue=BoolValue; Length=1; Precision=0;
		i=StringPrintf(Value,DoubleValue,Length,Precision);
		break;
	case expCOMP:
		Length=1; Precision=0;
		newValue(NumberDefLength);
		switch (Left->Type){
		case dbDATE:
			if (Right->Type!=dbDATE){
				BoolValue=False;
				break;
			}
			switch (CalcOp){
			case synEXP_EQ:
				BoolValue=(DateCmp(Left,Right)==0);
				break;
			case synEXP_NE:
				BoolValue=(DateCmp(Left,Right)!=0);
				break;
			case synEXP_LT:
				BoolValue=(DateCmp(Left,Right)<0);
				break;
			case synEXP_GT:
				BoolValue=(DateCmp(Left,Right)>0);
				break;
			case synEXP_LE:
				BoolValue=(DateCmp(Left,Right)<=0);
				break;
			case synEXP_GE:
				BoolValue=(DateCmp(Left,Right)>=0);
				break;
			case syn_REGEX_MATCH:
			case syn_REGEX_MATCH_CASE:
				regex.CaseSensitive = CalcOp==syn_REGEX_MATCH_CASE; 
				BoolValue=(regex.matchRegExText(Left->Value, Right->Value));
				break;
			default:
				BoolValue=False;
				break;
			}
			break;
		case dbCHAR:
		case dbLONG:
			switch (CalcOp){
			case synEXP_EQ:
				if (leftdigit && !leftconst && rightdigit && (!rightconst||!Right||Right->Type==dbNUMBER||Right->Type==dbDATE))
					BoolValue=(NumCmp(Left,Right)==0);
				else
					BoolValue=(StrCmp(Left->Value,Right->Value)==0);
				break;
			case synEXP_NE:
				if (leftdigit && !leftconst && rightdigit && (!rightconst||!Right||Right->Type==dbNUMBER||Right->Type==dbDATE))
					BoolValue=(NumCmp(Left,Right)!=0);
				else
					BoolValue=(StrCmp(Left->Value,Right->Value)!=0);
				break;
			case synEXP_LT:
				if (leftdigit && !leftconst && rightdigit && (!rightconst||!Right||Right->Type==dbNUMBER||Right->Type==dbDATE))
					BoolValue=(NumCmp(Left,Right)<0);
				else
					BoolValue=(StrCmp(Left->Value,Right->Value)<0);
				break;
			case synEXP_GT:
				if (leftdigit && !leftconst && rightdigit && (!rightconst||!Right||Right->Type==dbNUMBER||Right->Type==dbDATE))
					BoolValue=(NumCmp(Left,Right)>0);
				else
					BoolValue=(StrCmp(Left->Value,Right->Value)>0);
				break;
			case synEXP_LE:
				if (leftdigit && !leftconst && rightdigit && (!rightconst||!Right||Right->Type==dbNUMBER||Right->Type==dbDATE))
					BoolValue=(NumCmp(Left,Right)<=0);
				else
					BoolValue=(StrCmp(Left->Value,Right->Value)<=0);
				break;
			case synEXP_GE:
				if (leftdigit && !leftconst && rightdigit && (!rightconst||!Right||Right->Type==dbNUMBER||Right->Type==dbDATE))
					BoolValue=(NumCmp(Left,Right)>=0);
				else
					BoolValue=(StrCmp(Left->Value,Right->Value)>=0);
				break;
			case syn_REGEX_MATCH_CASE:
			case syn_REGEX_MATCH:
				regex.CaseSensitive = CalcOp==syn_REGEX_MATCH_CASE; 
				BoolValue=(regex.matchRegExText(Left->Value, Right->Value));
				break;
			default:
				BoolValue=False;
				break;
			}
			break;
		case dbNUMBER:
			switch (CalcOp){
			case synEXP_EQ:
				BoolValue=(NumCmp(Left,Right)==0);
				break;
			case synEXP_NE:
				BoolValue=(NumCmp(Left,Right)!=0);
				break;
			case synEXP_LT:
				BoolValue=(NumCmp(Left,Right)<0);
				break;
			case synEXP_GT:
				BoolValue=(NumCmp(Left,Right)>0);
				break;
			case synEXP_LE:
				BoolValue=(NumCmp(Left,Right)<=0);
				break;
			case synEXP_GE:
				BoolValue=(NumCmp(Left,Right)>=0);
				break;
			case syn_REGEX_MATCH:
			case syn_REGEX_MATCH_CASE:
				regex.CaseSensitive = CalcOp==syn_REGEX_MATCH_CASE; 
				BoolValue=(regex.matchRegExText(Left->Value, Right->Value));
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
		DoubleValue=BoolValue; 
		i=StringPrintf(Value,DoubleValue,Length,Precision);
		break;
	case expCALC:
		switch (CalcOp){
		case synEXP_ADD:
			if ( leftdigit && !Right ) {
				newDoubleValue(Left->DoubleValue, Left->Length, Left->Precision);
			}else if (!Left ||  !Right) {
				newDoubleValue(0, NumberDefLength, 0);
				//return dbTYPEERR;
			}else if ( ( Left->Type==dbDATE && Right->Type==dbNUMBER ) || ( Right->Type==dbDATE && Left->Type==dbNUMBER ) ){
				if ((i=newDateValue((time_t)(Left->DoubleValue+Right->DoubleValue)))!=0) return i;
			}else if ( leftdigit && rightdigit ) {
				if (Left->Length >= Length) Length = Left->Length+1;
				if (Right->Length >= Length) Length = Right->Length+1;
				newDoubleValue(Left->DoubleValue+Right->DoubleValue, Length, Left->Precision+Right->Precision);
			}else if ( ( Left->Type==dbCHAR || Right->Type==dbCHAR || Left->Type==dbLONG || Right->Type==dbLONG )){
				newValue(Left->Length+Right->Length+1);
				sprintf(Value,"%s%s",(Left->Value)?Left->Value:"",Right->Value?Right->Value:"");
				sscanf(Value, "%lf", &DoubleValue);
				BoolValue=DoubleValue!=0;
				StringValue.L=strlen(Value);
			}else{
				newDoubleValue(Left->DoubleValue+Right->DoubleValue, NumberDefLength, Left->Precision+Right->Precision);
				//return dbTYPEERR;
			}
			break;
		case synEXP_SUB:
			if ( leftdigit && !Right) {
				newDoubleValue(-Left->DoubleValue, (Length>=Left->Length)?Length:Left->Length, Precision);
			}else if (!Left ||  !Right) {
				newDoubleValue(0, NumberDefLength, 0);
				//return dbTYPEERR;
			}else if ( Left->Type==dbDATE && Right->Type==dbNUMBER ){
				if ((i=newDateValue((time_t)(Left->DoubleValue-Right->DoubleValue)))!=0) return i;
			}else if ( leftdigit && rightdigit ) {
				if (Left->Length >= Length) Length = Left->Length+1;
				if (Right->Length >= Length) Length = Right->Length+1;
				newDoubleValue(Left->DoubleValue-Right->DoubleValue, Length, Left->Precision+Right->Precision);
			}else{
				newDoubleValue(Left->DoubleValue-Right->DoubleValue, NumberDefLength, Left->Precision+Right->Precision);
				//return dbTYPEERR;
			}
			break;
		case synEXP_ASTERISK:
			if ( leftdigit && rightdigit ) {
				newDoubleValue(Left->DoubleValue * Right->DoubleValue, Left->Length+Right->Length+1, Left->Precision+Right->Precision);
			}else{
				newDoubleValue(Left->DoubleValue*Right->DoubleValue, NumberDefLength, Left->Precision+Right->Precision);
				//return dbTYPEERR;
			}
			break;
		case synEXP_DIV:
			if ( Left && leftdigit && Right && rightdigit ) {
				DoubleValue=Left->DoubleValue / Right->DoubleValue;
				Precision=(int)(-log(DoubleValue)/log(10.0)+4);
				if (Precision<4) Precision=4; 
				newDoubleValue(DoubleValue, Left->Length+Right->Length+1, Precision);
			}else{
				return dbTYPEERR;
			}
			break;
		case syn_REGEX_SEARCH: 
		case syn_REGEX_SEARCH_CASE: 
			deleteValue();
			if (Left && Left->Value && Right && Right->Value){
				results.reset();
				results.setSortOrder(SORT_ORDER_NUM_INC);
				regex.CaseSensitive = CalcOp==syn_REGEX_SEARCH_CASE; 
				regex.searchRegExText(Left->Value, Right->Value, &results);
				if (results.Total){
					link=(ReferLink*) results.moveFirst();
					newValue(link->Value.P, link->Value.L, true);
					BoolValue = true; //special treat for ~ operation
				}
			}
			break;
		case synEXP_STRCAT: 
			newValue(Left->Length+Right->Length+1);
			sprintf(Value,"%s%s",(Left->Value)?Left->Value:"",Right->Value?Right->Value:"");
			sscanf(Value, "%lf", &DoubleValue);
			BoolValue=DoubleValue!=0;
			StringValue.L=strlen(Value);
			break;

		case synEXP_NULL_OR:
			if (Left && Left->Value){
				newValue(Left->Length+1);
				for (i=strlen(Left->Value)-1; i>=0; i--){
					if (!isSpace(Left->Value[i])) break;
				}
				i++;
				if (i>=0){
					strncpy(Value, Left->Value, i+1);
					Value[i+1]=0;
				}
				StringValue.L=strlen(Value);
			}else if (Right && Right->Value) {
				newValue(Right->Length);
				sprintf(Value,"%s",Right->Value);
				StringValue.L=strlen(Value);
			}else {
				deleteValue();
			}
			break;
		case synEXP_MOD:
			//if ( Left->Type==dbNUMBER && Right->Type==dbNUMBER ) {
				newValue((Left->Length > Length)?Left->Length:Length);
				if (((long)Right->DoubleValue) == 0) {
					DoubleValue=0;
					return dbOVERFLOW;
				}
				DoubleValue=((long)Left->DoubleValue) % ((long)Right->DoubleValue);
				i=StringPrintf(Value,DoubleValue,Length,Precision);
				if (i!=0) return i;
				StringValue.L=strlen(Value);
			//}else{
			//	return dbTYPEERR;
			//}
			break;

		case synEXP_NULL:
			Length=0;
			break;
		case synEXP_SYSDATE:
			if (StrCmp(Value,"", false)) break;
			if ((i=newDateValue(time(NULL)))!=0) return i;
			break;

		case synEXP_WORD:
			int (*fun)(tExprCalc*, tExprItem*, void*);
			fun=0;
			//FunValue:*ReferLink; Name:func_name; Data:*fun; 
			//					   Value.P:parameter; 
			if (NamePrefix.L){
				//always need to resolve object name
				link = calc->findFunctionName(&StringName, &NamePrefix, this); 
				if (link) fun = (int (*)(tExprCalc*, tExprItem*, void*)) link->Data;
			}else if (FunValue){
				link=(ReferLink*) FunValue;
				fun=(int (*)(tExprCalc*, tExprItem*, void*)) link->Data;
			}else if (!calc){
				return dbSYNTAXERR;
			}else{
				if (strchr(preEXP_TYPE_PREFIX, StringName.P[0])){
					func_name.Set(StringName.P+1, StringName.L-1, false);
				}else{
					func_name.Set(StringName.P, StringName.L, false);
				}
				link = calc->findFunctionName(&func_name, &NamePrefix, this); 
				if (link) fun = (int (*)(tExprCalc*, tExprItem*, void*)) link->Data;
				FunValue=link;
			}
			if (!fun) return dbSYNTAXERR;
			IsNewGroup=initial_group; 
			if ((i=(*fun)(calc, this, (void*) link->Value.P))<0) return i;
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

int tExprItem::NumCmp(tExprField *Num1, tExprField *Num2){
	if (Num1->DoubleValue-Num2->DoubleValue >Epsilon) return 1;
	if (Num1->DoubleValue-Num2->DoubleValue <-Epsilon) return -1;
	return 0;
}

int tExprItem::DateCmp(tExprField *Date1, tExprField* Date2){
	if (Date1->DoubleValue-Date2->DoubleValue >Epsilon) return 1;
	if (Date1->DoubleValue-Date2->DoubleValue <-Epsilon) return -1;
	return 0;
}

int tExprItem::checkCalcType(){
	int i;
	if (Brand == expNODE){
		Length = Left->Length;
		//Type=Left->Type;
		Precision = Left->Precision;
	}else 
	switch (CalcOp){
	case synEXP_WORD: //const
		if (Value && tStrOp::isDigit(Value[0])){
			Length=strlen(Value);
			Type=dbNUMBER;
			Precision=NumberDefPrecision;
		}
		break;
	case synEXP_NOT:
		Brand=expBOOL;
		break;
	case synEXP_ADD:
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
			return 0;
			//return dbTYPEERR;
		}
		break;
	case synEXP_SUB:
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
		}else if ( Left->Type==dbCHAR || Right->Type==dbCHAR || Left->Type==dbLONG || Right->Type==dbLONG ){
			Length=Left->Length+Right->Length;
			Length+=AddAddLength;
			Type=dbNUMBER;
			Precision=0;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else{
			return 0;
			//return dbTYPEERR;
		}
		break;
	case synEXP_ASTERISK:
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
		}else if ( Left->Type==dbCHAR || Right->Type==dbCHAR || Left->Type==dbLONG || Right->Type==dbLONG ){
			Length=Left->Length + Right->Length;
			Length+=AddAddLength;
			Type=dbNUMBER;
			Precision=Left->Precision + Right->Precision;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else{
			return 0;
			//return dbTYPEERR;
		}
		break;
	case synEXP_DIV:
		Brand=expCALC;
		if ( Left && Left->Type==dbNUMBER && Right && Right->Type==dbNUMBER ) {
			Length=Left->Length + Right->Length;
			Length+=AddAddLength;
			Type=dbNUMBER;
			Precision=Left->Precision + Right->Precision;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else if ( ( Left &&(Left->Type==dbCHAR || Left->Type==dbLONG)) || 
			(Right && (Right->Type==dbCHAR ||  Right->Type==dbLONG ))){
			Length=AddAddLength;
			if (Left) Length += Left->Length;
			if (Right) Length+= Right->Length;
			Type=dbCHAR;
			if ((i=newValue(Length))!=dbSUCCESS)
				return i;
		}else{
			return 0;
			//return dbTYPEERR;
		}
		break;
	case synEXP_EQ:
	case synEXP_LT:
	case synEXP_GT:
	case synEXP_LE:
	case synEXP_GE:
	case synEXP_NE:
		if (Left->Type == dbDATE || Right->Type == dbDATE){
			Left->Type = Right->Type = dbDATE;
		}else if (Left->Type == dbNUMBER || Right->Type == dbNUMBER){
			Left->Type = Right->Type = dbNUMBER;
		}
		break;
	case synEXP_MOD:
		Brand=expCALC;
		Type=dbNUMBER;
		Length=Left->Length;
		Length+=SumAddLength;
		Precision=0;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
		break;

	case synEXP_SYSDATE:
		Brand=expCALC;
		Type=dbDATE;
		Length=DateLength;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
		break;
	case synEXP_NULL:
		Brand=expBOOL;
		Length=1;
		if ((i=newValue(Length))!=dbSUCCESS) return i;
	default:
		break;
	}
	return dbSUCCESS;
}
int tExprCalc::getExprErrorMsg(int err, ReferData* errormsg){
	switch (err){
	case dbMEMORYERR: *errormsg+="Memory error."; break;
	case dbSYNTAXERR: *errormsg+="Syntax error."; break;
	case dbLOCKFAIL: *errormsg+="Lock fail error."; break;
	case dbOPENFAIL: *errormsg+="File open fail."; break;
	case dbFILEERR: *errormsg+="File read error."; break;
	case dbINDEXERR: *errormsg+="Table define error."; break;
	case dbDATAERR: *errormsg+="Table data error."; break;
	case dbFIELDNOTFOUND: *errormsg+="Field not exist."; break;
	case dbSQLNOTSET: *errormsg+="Request not set."; break;
	case dbOVERFLOW: *errormsg+="Overflow."; break;
	case dbTYPEERR: *errormsg+="Type error."; break;
	case dbFILEWRITE: *errormsg+="File write error."; break;
	case dbFILECLOSE: *errormsg+="File close error."; break;
	case dbFILECOMMIT: *errormsg+="Data write back error."; break;
	case dbDATEFORMAT: *errormsg+="Date format error."; break;
	case dbBRACEERR: *errormsg+="Left-right parentheses not match."; break;
	case dbNOTSUPPORT: *errormsg+="Unsupported keywords."; break;
	default: *errormsg+="Unkown error."; break;
	}
	return 0;
}

tExprCalc::tExprCalc(tExprCalc* context){
	Root=0;
	FirstStep=0;
	CurrentStep=0;
	ExprSentence=0;
	Fields = 0;
	FieldsList = 0;
	FieldsNum = 0;
	//Functions = 0;
	//FunctionsList = 0;
	//FunctionsNum = 0;
	IsNameCaseSensitive = false;
	HaveGroupFunction = false;
	Context=this;
	if (context) Context = context;
	Variables.setDuplication(false);
	Variables.setCaseSensitivity(false);
	RegExContext=0;
	FunctionContext=0;
	ObjectFunctions.setCaseSensitivity(false);
	ObjectFunctions.setDuplication(false);

	RegisteredFunctions.setCaseSensitivity(false);
	RegisteredFunctions.setDuplication(false);
	InternalFunctions.setCaseSensitivity(false);
	InternalFunctions.setDuplication(false);
	//registerFunction("substr", functionSubstr, 0);
	//group operation
	addInternalFunction("count", functionCount, 0);
	addInternalFunction("sum", functionSum, 0);
	addInternalFunction("max", functionMax, 0);
	addInternalFunction("min", functionMin, 0);

	//string operation
	addInternalFunction("substr", functionSubstr, 0);
	addInternalFunction("mid", functionSubstr, 0);
	addInternalFunction("ltrim", functionLtrim, 0);
	addInternalFunction("rtrim", functionRtrim, 0);
	addInternalFunction("strcat", functionStrcat, 0);
	addInternalFunction("strcmp", functionStrcmp, 0);
	addInternalFunction("strcmpi", functionStrcmpi, 0);
	addInternalFunction("strlen", functionStrlen, 0);
	addInternalFunction("strfind", functionStrFind, 0);
	addInternalFunction("instr", functionStrFind, 0);
	addInternalFunction("replace", functionReplace, 0);
	addInternalFunction("lower", functionLower, 0);
	addInternalFunction("upper", functionUpper, 0);
	addInternalFunction("format", functionFormat, 0);
	addInternalFunction("packnull", functionPackNull, 0);
	addInternalFunction("get_email", functionGetEmails, 0);
	addInternalFunction("get_emails", functionGetEmails, 0);

	addInternalFunction("ischar", functionIsChar, 0);
	addInternalFunction("isnumber", functionIsNumber, 0);
	addInternalFunction("isblank", functionIsBlank, 0);
	addInternalFunction("isempty", functionIsBlank, 0);
	addInternalFunction("isphonenumber", functionIsPhoneNumber, 0);

	addInternalFunction("time", functionTime, 0);
	addInternalFunction("date", functionDate, 0);
	addInternalFunction("to_number", functionToNumber, 0);
	addInternalFunction("to_char", functionToChar, 0);
	addInternalFunction("to_date", functionToDate, 0);
	addInternalFunction("months_between", functionMonthsBetween, 0);
	addInternalFunction("add_months", functionAddMonths, 0);
	addInternalFunction("last_day", functionLastDay, 0);
	addInternalFunction("next_day", functionNextDay, 0);
	addInternalFunction("DateDiff", functionDateDiff, 0);

	addInternalFunction("html_encode", functionHtmlEncode, 0);
	addInternalFunction("html_decode", functionHtmlDecode, 0);
	addInternalFunction("url_encode", functionUrlEncode, 0);
	addInternalFunction("url_decode", functionUrlDecode, 0);
	addInternalFunction("utf8_encode", functionUTF8Encode, 0);

	addInternalFunction("rand", functionRand, 0);
	addInternalFunction("srand", functionSRand, 0);
	addInternalFunction("round", functionRound, 0);
	addInternalFunction("floor", functionFloor, 0);
	addInternalFunction("ceil", functionCeil, 0);
	addInternalFunction("abs", functionAbs, 0);


	addInternalFunction("ReadFile", functionReadFile, 0);
	addInternalFunction("SaveFile", functionSaveFile, 0);
	addInternalFunction("RenameFile", functionRenameFile, 0);
	addInternalFunction("DeleteFile", functionDeleteFile, 0);
	addInternalFunction("Mkdir", functionMkDir, 0);
	addInternalFunction("AppendFile", functionAppendFile, 0);
	addInternalFunction("GetFilePath", functionGetFilePath, 0);
	addInternalFunction("GetFileName", functionGetFileName, 0);
	addInternalFunction("TempPath", functionTempPath, 0);


	addInternalFunction("text_words", functionTextWords, 0);
	addInternalFunction("string_comp", functionStringComp, 0);
	addInternalFunction("htql", functionHtql, 0);
	addInternalFunction("match_local_score", functionMatchLocalScore, 0);
	addInternalFunction("get_number", functionGetNumber, 0);


}
tExprCalc::~tExprCalc(){
	reset();
	Context=this;
}
void tExprCalc::reset(){
	deleteValue();
	if (Root) delete Root;
	Root=0;
	FirstStep=0;
	CurrentStep=0;
	if (ExprSentence) delete ExprSentence;
	ExprSentence=0;
	tExprField *NowField=Fields,*NextField;
	while (NowField!=NULL) {
		NextField=NowField->WalkNext;
		delete NowField;
		NowField=NextField;
	}
	Fields=0;
	if (FieldsList){ 
		free(FieldsList);
		FieldsList=0;
	}
	FieldsNum = 0;
	UnkownFunctions.reset();
	UnkownFunctions.setDuplication(false);
	UnkownFunctions.setCaseSensitivity(false);
	if (RegExContext) {
		delete RegExContext;
		RegExContext=0;
	}
	ObjectFunctions.empty();
	/*
	NowField=Functions;
	while (NowField!=NULL) {
		NextField=NowField->WalkNext;
		delete NowField;
		NowField=NextField;
	}
	Functions=0;
	if (FunctionsList){ 
		free(FunctionsList);
		FunctionsList=0;
	}
	FunctionsNum = 0;*/
	HaveGroupFunction=false;
	Variables.reset();
	Variables.setDuplication(false);
	Variables.setCaseSensitivity(false);
	AssignName.reset();
	return;
}

int tExprCalc::useContext(tExprCalc* context_calc, int clear_contextvars){
	if (context_calc){
		Context = context_calc;
		if (clear_contextvars) clearContextVars();
		return 0;
	}else{
		return -1;
	}
}
int tExprCalc::takeContextVars(tExprCalc* context){
	Context=this; 
	clearContextVars();
	Fields=context->Fields;
	context->Fields=0;
	UnkownFunctions.take(&context->UnkownFunctions, true); 
	for (ReferLink* link=context->RegisteredFunctions.getReferLinkHead(); link; link=link->Next){
		ReferLink* link1=RegisteredFunctions.findName(&link->Name);
		if (!link1) link1=RegisteredFunctions.add(&link->Name, 0, 0);
		link1->Data=link->Data; 
		link1->Value.Set(link->Value.P, link->Value.L, false); 
	}
	return 0; 
}
int tExprCalc::clearContextVars(){
	tExprField *NowField=Context->Fields,*NextField;
	while (NowField!=NULL) {
		NextField=NowField->WalkNext;
		delete NowField;
		NowField=NextField;
	}
	Context->Fields=0;
	Context->UnkownFunctions.reset();
	Context->UnkownFunctions.setDuplication(false);
	Context->UnkownFunctions.setCaseSensitivity(false);
/*
	NowField=Context->Functions;
	while (NowField!=NULL) {
		NextField=NowField->WalkNext;
		delete NowField;
		NowField=NextField;
	}
	Context->Functions=0; */
	return 0;
}

int tExprCalc::addNewField(tExprItem *new_field){
	char* n=new_field->Name;
	char* type;
	if (n[0]==preEXP_DATE){
		type = strDATE;
		n=n+1;
	}else if (n[0]==preEXP_NUMBER ){
		type = strNUMBER;
		n=n+1;
	}else if (n[0] == preEXP_CHAR){
		type = strCHAR;
		n=n+1;
	}else{
		type = strLONG;
	}
	new_field->setType(type);

	ReferLink* link=findVariableName(&new_field->StringName); 
	//ReferLink* link=Variables.findName(&new_field->StringName); //const values
	//if (!link) link=Context->Variables.findName(&new_field->StringName);
	if (link){
		new_field->newValue(link->Value.P, link->Value.L, 0);
		new_field->Brand=expCONST;
	}else{
		//add unkown fields to Context
		tExprField** NowField= &Context->Fields;
		while (*NowField && (cmpFieldName(n, (*NowField)->Name) || cmpFieldName(new_field->NamePrefix.P, (*NowField)->NamePrefix.P))){
			NowField=&((*NowField)->WalkNext);
		}
		if (!(*NowField)){
			*NowField = new tExprField(new_field->Name);
			(*NowField)->NamePrefix=new_field->NamePrefix;
			(*NowField)->setType(type);

			/*ReferLink* link;
			if ((link=Variables.findName(&new_field->StringName)) || (link=Context->Variables.findName(&new_field->StringName))){
				(*NowField)->newValue(link->Value.P, link->Value.L, 0);
			}*/
		}
		new_field->Left=(*NowField);
	}
	new_field->checkCalcType();
	return dbSUCCESS;
}
ReferLink* tExprCalc::setVariable(const char*name, const char* value, long len, int copy){
	//ReferLink* link=Variables.findName(name);
	ReferData var_name;
	var_name.Set((char*) name, name?strlen(name):0, false); 
	ReferLink* link=findVariableName(&var_name); 
	//ReferData name1;
	//name1=name;
	if (!link) link=Variables.add(&var_name, 0, 0);
	if (value && !len) len=strlen(value);
	link->Value.Set((char*) value, len, copy);
	return link;
}

ReferLink* tExprCalc::registerFunction(const char* fun_name, int (*fun)(tExprCalc*, tExprItem*, void*), void* p){
	ReferData name;
	name.Set((char*) fun_name, strlen(fun_name), false);
	ReferLink* link = RegisteredFunctions.findName(&name);
	if (link) {
		link->Data = (long) fun;
		link->Value.Set((char*) p, 0, false);
	}else{
		link = RegisteredFunctions.add(&name, 0, (long)fun);
		link->Value.Set((char*) p, 0, false);
	}
	return link;
}
ReferLink* tExprCalc::addInternalFunction(const char* fun_name, int (*fun)(tExprCalc*, tExprItem*, void*), void* p){
	ReferData name;
	name.Set((char*) fun_name, strlen(fun_name), false);
	ReferLink* link = InternalFunctions.findName(&name);
	if (link) {
		link->Data = (long) fun;
		link->Value.Set((char*) p, 0, false);
	}else{
		link = InternalFunctions.add(&name, 0, (long)fun);
		link->Value.Set((char*) p, 0, false);
	}
	return link;
}
int tExprCalc::functionDllFunction(tExprCalc*calc, tExprItem*item, void* p){ //ReferLink*p; p->Name:name; p->Value.P:func, p->Value.L:n para
	int argc=calc->getFuncParaTotal(item); 
	if (calc->getFuncParaIndex(item)<argc) return 0;

	char** argv=(char**) malloc(sizeof(char*)*(argc+1));
	for (int i=0; i<argc; i++){
		argv[i]=calc->getFuncParaValue(item, i+1); 
	}
	argv[argc]=0; 

	int (*dllfunc)(int, char**, char**, long*);
	dllfunc=(int (*)(int, char**, char**, long*)) ((ReferLink*) p)->Value.P;
	char* results=0; 
	long len=0;
	dllfunc(argc, argv, &results, &len);
	if (results){
		item->newValue(results, len, true);
#ifdef _WINDOWS
		//The DLL function needs to use GlobalAlloc() function to allocate memoe
		GlobalFree(results);
#else
		free(results);
#endif
	}
	if (argv) free(argv); 
	return 0;
}
long tExprCalc::addDllFunctions(ReferLink* loadedfuncs, long para){
	long count=0; 
	for (ReferLink* link=loadedfuncs; link; link=link->Next){
		if (link->Name.L && link->Value.P){
			link->Data=para; 
			if (registerFunction(link->Name.P, functionDllFunction, link)){
				count++;
			}
		}
	}
	return count;	
}
int tExprCalc::spotNode(tExprItem* WalkStart){
	int i;
	if (WalkStart->Brand!=expNODE){
		if (WalkStart->Left!=NULL) {
			i=spotNode((tExprItem *)WalkStart->Left);
			if (i!=dbSUCCESS) return i;
		}
		if (WalkStart->Right!=NULL) {
			i=spotNode((tExprItem *)WalkStart->Right);
			if (i!=dbSUCCESS) return i;
		}
	}
	if (WalkStart->Brand!=expNODE) 
		return WalkStart->checkCalcType();
/*	if (WalkStart->Value){
		i=spotTable(WalkStart->Value,WalkStart);
		WalkStart->deleteValue();
	}else{
		i=spotTable("",WalkStart);
	}*/
	return i;
}

int tExprCalc::PostWalk(tExprItem* WalkStart, tExprField ***PreWalk){
	int i;
	if (WalkStart==NULL) return dbSUCCESS;
	if (WalkStart->Brand!=expNODE){
		if (WalkStart->Left!=NULL) {
			i=PostWalk( (tExprItem *)WalkStart->Left,PreWalk);
			if (i!=dbSUCCESS) return i;
		}
		if (WalkStart->Right!=NULL) {
			i=PostWalk( (tExprItem *)WalkStart->Right,PreWalk);
			if (i!=dbSUCCESS) return i;
		}
	}
	**PreWalk=(tExprField *)WalkStart;
	*PreWalk=&WalkStart->WalkNext;
	return dbSUCCESS;
}

int tExprCalc::checkBranch(void* p, tExprItem* WalkStart, int (*CheckFunc)(void* p, tExprItem* CheckItem), int WalkOrder){
	int i;
	if (WalkStart==NULL) return dbSUCCESS;
	if (WalkOrder == walkPRIOR_ORDER) if ((i=CheckFunc(p, WalkStart))<0) return i;
	if (WalkStart->Brand!=expNODE){
		if (WalkStart->Left!=NULL) {
			if ((i=checkBranch(p, (tExprItem *)WalkStart->Left,CheckFunc))<0) return i;
		}
		if (WalkOrder == walkMID_ORDER) if ((i=CheckFunc(p, WalkStart))<0) return i;
		if (WalkStart->Right!=NULL) {
			if ((i=checkBranch(p, (tExprItem *)WalkStart->Right,CheckFunc))<0) return i;
		}
	}
	if (WalkOrder == walkPOST_ORDER) if ((i=CheckFunc(p, WalkStart))<0) return i;
	return i;
}
int tExprCalc::checkAssignValueExpression(tExprCalc* expr, tExprItem* expr_item, ReferLinkHeap* assign_names){
	if (!expr_item) return -1;
	if (expr_item->Brand == expBOOL && expr_item->CalcOp == synEXP_AND){
		checkAssignValueExpression(expr, (tExprItem*)expr_item->Left, assign_names);
		checkAssignValueExpression(expr, (tExprItem*)expr_item->Right, assign_names);
	} else if ( (expr_item->Brand == expCOMP && expr_item->CalcOp == synEXP_EQ) 
		|| (expr_item->Brand == expBOOL && expr_item->CalcOp == synEXP_IS)  ) {
		tExprItem* left = (tExprItem*)expr_item->Left;
		tExprItem* right = (tExprItem*)expr_item->Right;
		if (left->Brand != expNODE){
			left = right;
			right= (tExprItem*)expr_item->Left;
		}
		int is_const=true;
		if (left->Brand == expNODE && checkConstValueExpression(expr, right, &is_const)>=0 && is_const){
			ReferData name;
			ReferData value;
			char buf[30];
			name=left->Left->Name;
			if (expr_item->CalcOp == synEXP_IS){
				if (!right->BoolValue) value=""; //need to change later
				else return 0;
			}else if (right->Brand == expBOOL || right->Brand == expCOMP){
				sprintf(buf, "%d",right->BoolValue);
				value = buf;
			}else if (right->Type == dbNUMBER){
				//sprintf(buf, "%lf",right->DoubleValue);
				//value = buf;
				value=right->Value;
			}else if (right->Type == dbDATE){
				value=right->Value;
			}else {
				value=right->Value;
			}
			assign_names->add(&name, &value, 0);
		}
	}
	return 0;
}

int tExprCalc::checkConstValueExpression(tExprCalc* expr,  tExprItem* expr_item, int* is_const){
	int i=0;
	if (expr_item->Brand == expNODE){
		*is_const= false;
		return 0;
	}else if (expr_item->Brand == expCONST){
		return expr_item->Calculate(expr);
	}
	
	if (expr_item->Left) {
		if ((i=checkConstValueExpression(expr, (tExprItem*) expr_item->Left, is_const))<0) return i;
		if (!*is_const) return 0;
	}
	if (expr_item->Right) {
		if ((i=checkConstValueExpression(expr, (tExprItem*) expr_item->Right, is_const))<0) return i;
		if (!*is_const) return 0;
	}
	return expr_item->Calculate(expr);
}
int tExprCalc::parseExpr(etEXP_SYNTAX PreOp, tExprItem **Result,tExprItem *SetParent){
	tExprItem* NewExpr=NULL;
	tExprItem* NewExpr1=NULL;
	int i;
	switch (ExprSentence->Type){
	case synEXP_LEFTBRACE:
		ExprSentence->match();
		if ((i=parseExpr(synEXP_RIGHTBRACE,&NewExpr))!=dbSUCCESS) return i;
		if (!ExprSentence->match(synEXP_RIGHTBRACE)) return dbBRACEERR;
		break;
	case synEXP_WORD:
	case synEXP_NUMBER:
	case synEXP_CHAR:
	case synEXP_DATE:
		if ((i=parseField(&NewExpr))!=dbSUCCESS) return i;
		break;
	case synEXP_NOT:
	case synEXP_SUB:
	case synEXP_ADD:
	case synEXP_SYSDATE:
	case synEXP_NULL:
		if ((i=parseField(&NewExpr))!=dbSUCCESS) return i;
		break;
	case synEXP_RIGHTBRACE:
		return dbBRACEERR;
	default:
		return dbNOTSUPPORT;
	}
	while (PreOp<ExprSentence->Type
		&& !(PreOp>=synEXP_SUB && PreOp<=synEXP_ADD && ExprSentence->Type>=synEXP_SUB && ExprSentence->Type<=synEXP_ADD) //+,-
		&& !(PreOp>=synEXP_DIV && PreOp<=synEXP_MOD && ExprSentence->Type>=synEXP_DIV && ExprSentence->Type<=synEXP_MOD) 
		&& !(PreOp>=synEXP_EQ && PreOp<=synEXP_GT && ExprSentence->Type>=synEXP_EQ && ExprSentence->Type<=synEXP_GT) 
		){
		NewExpr1=new tExprItem( ExprSentence->Type);
		NewExpr1->Left=(tExprField *)NewExpr;
		NewExpr->Parent=(tExprField *)NewExpr1;
		i=parseExpr((etEXP_SYNTAX)ExprSentence->match(),(tExprItem**)&(NewExpr1->Right),NewExpr1);
		if (i!=dbSUCCESS){delete NewExpr1; return i;}
		//((tExprItem *)(NewExpr1->Right))->Parent=(tExprField *)NewExpr1;
//		if (Tables){
			i=NewExpr1->checkCalcType();  //20040716
			if (i!=dbSUCCESS){delete NewExpr1; return i;}
//		}
		NewExpr=NewExpr1;
	}
	*Result=NewExpr;
	(*Result)->Parent=SetParent;
	return dbSUCCESS;
}
ReferLink* tExprCalc::findFunctionName(ReferData* func_name, ReferData* class_name, tExprItem*item, tExprCalc* calc){ //may contain prefix
	// add class name, each class provide a context of function names
	if (!calc) calc = this; 
	ReferLink* link=0; 
	if (class_name && class_name->L){
		// search for *class* contexts function link, need
		link=findObjectFunction(func_name, class_name, item, calc); 
	}
	if (!link || !link->Data){
		link=RegisteredFunctions.findName(func_name);
		if (!link || !link->Data){
			//func_name is Class()
			link = ObjectFunctions.findName(func_name); 
			if (link && link->Data){
				//find its __init__() function
				ReferLink* (*object_func)(tExprCalc*calc, tExprItem*item, ReferData*func_name, ReferData*class_name, void*p);
				object_func = (ReferLink* (*)(tExprCalc*, tExprItem*, ReferData*, ReferData*, void*)) link->Data; 
				ReferData func_name1; 
				func_name1="__init__";
				link=(*object_func)(this, item, &func_name1, func_name, link->Value.P );
			}
		}
	}
	if ((!link || !link->Data) && Context && Context!=this){
		link=Context->findFunctionName(func_name, class_name, item, calc);
	}
	if ((!link || !link->Data) && FunctionContext && FunctionContext!=this){
		link=FunctionContext->findFunctionName(func_name, class_name, item, calc);
	}
	if (!link || !link->Data) link=InternalFunctions.findName(func_name);
	if (!link || !link->Data) link=UnkownFunctions.findName(func_name);
	//
	return link;
}
ReferLink* tExprCalc::findVariableName(ReferData* var_name){ //may contain prefix
	ReferLink* link=Variables.findName(var_name); //const values
	//
	if (!link && Context && Context!=this) {
		link=Context->findVariableName(var_name);
	}
	return link;
}
ReferLink* tExprCalc::findObjectFunction(ReferData* func_name, ReferData* class_name, tExprItem*item, tExprCalc* calc){
	//get the original class name
	ExprSyntax syntax; 
	ReferData class_name1;
	getPrefixName(class_name->P, 0, &class_name1); 
	if (item) item->RefName.reset();

	ReferLink* found_link=0; 
	for (ReferLink* link=ObjectFunctions.getReferLinkHead(); link; link=link->Next){
		//sequentially navigate all registered classes, may not be efficient, need to use caching later ...
		if (link->Data){
			ReferLink* (*func)(tExprCalc*calc, tExprItem*, ReferData*func_name, ReferData*class_name, void*p);
			func = (ReferLink* (*)(tExprCalc*, tExprItem*, ReferData*, ReferData*, void*)) link->Data; 
			// need to use class_name and func_name to determine matching
			found_link=(*func)(calc, item, func_name, &class_name1, link->Value.P );
			if (found_link) break;
		}
	}
	return found_link;
}
ReferLink* tExprCalc::registerObjectFunctionClass(const char* class_name, ReferLink* (*fun)(tExprCalc*, tExprItem*, ReferData*, ReferData*, void*), void* p){
	ReferLink* link=ObjectFunctions.findName(class_name); 
	if (!link){
		link=ObjectFunctions.add(class_name, 0, 0);
	}
	if (link){
		link->Data = (long) fun; 
		link->Value.P = (char*) p;
	}
	return link;
}

int tExprCalc::parseField(tExprItem **Result){
//	char TableName[NameLength]="";
	int i;
	etEXP_SYNTAX SavedType;
	tExprItem* NewExpr;
	time_t now;
	long start;
	int para_num;
	ReferData func_name;
	//tExprField** NowField;
	ReferLink* link=0;

	switch (ExprSentence->Type){
	case synEXP_NUMBER:
		*Result=new tExprItem(ExprSentence->Type);
		(*Result)->Length=ExprSentence->StartLen;
		(*Result)->Precision=ExprSentence->StartPrecision;
		i=(*Result)->newValue(ExprSentence->Sentence+ExprSentence->Start,ExprSentence->StartLen);
		if (i!=dbSUCCESS) return i;
		ExprSentence->match();
		break;
	case synEXP_CHAR:
		*Result=new tExprItem(ExprSentence->Type);
		(*Result)->newValue(ExprSentence->StartLen-2);
		i=ExprSentence->takeStartSyntaxString(&(*Result)->StringValue); 
	//	ASSERT((*Result)->Value==(*Result)->StringValue.P); //or just change the (*Result)->Value
	//	(*Result)->Length=ExprSentence->StartLen-2;
	//	i=(*Result)->newValue(ExprSentence->Sentence+ExprSentence->Start+1,ExprSentence->StartLen-2);
		if (i!=dbSUCCESS) return i;
		ExprSentence->match();
		break;
	case synEXP_DATE:
		*Result=new tExprItem(ExprSentence->Type);
		now=0;
		i=DateToLong(ExprSentence->Sentence+ExprSentence->Start+1,DateFormat,&now);
		if (i!=dbSUCCESS) return i;
		if ((i=(*Result)->newDateValue(now))!=0) return i;
		ExprSentence->match();
		break;
	case synEXP_WORD:
		SavedType=(etEXP_SYNTAX)ExprSentence->Type;
		start=ExprSentence->Start;
	while (ExprSentence->NextType == synEXP_DOT || ExprSentence->NextType == synEXP_ARROW){
			ExprSentence->match();
			ExprSentence->match();
			if (ExprSentence->Type!=synEXP_WORD && !ExprSentence->isAlpha(ExprSentence->Sentence[ExprSentence->Start])){
				return dbTYPEERR;
			}
		}
		*Result=new tExprItem(SavedType);
		if (start != ExprSentence->Start){ 
			(*Result)->NamePrefix.Set(ExprSentence->Sentence+start, ExprSentence->Start-start, true); //function prefix, including the trailing '.' or '->'
		}
		(*Result)->setName(ExprSentence->Sentence+ExprSentence->Start,ExprSentence->StartLen, true); //function name, including type-prefix
		if (strchr(preEXP_TYPE_PREFIX, (*Result)->StringName.P[0])){
			func_name.Set((*Result)->StringName.P+1, (*Result)->StringName.L-1, false); //function name without type prefix
		}else{
			func_name.Set((*Result)->StringName.P, (*Result)->StringName.L, false);
		}
		//func_name.Set((*Result)->StringName.P, (*Result)->StringName.L, false);
		if (ExprSentence->NextType == synEXP_LEFTBRACE){
			ExprSentence->match();
			ExprSentence->match();
			(*Result)->Brand = expCALC;
			link=findFunctionName( &func_name, &(*Result)->NamePrefix, (*Result));
			if (!link){
				link=Context->UnkownFunctions.add(&func_name, &(*Result)->NamePrefix, 0);
			}

			//if (!Context->RegisteredFunctions.findName(&func_name) 
			//	&& !Context->InternalFunctions.findName(&func_name)){
			//	link=Context->UnkownFunctions.findName(&func_name);
			//	if (!link){
			//		link=Context->UnkownFunctions.add(&func_name, &(*Result)->NamePrefix, 0);
			//	}
				/*NowField= &Context->Functions;
				while (*NowField && cmpFieldName(func_name.P, (*NowField)->Name) ){ //put unknown function into list
					NowField=&((*NowField)->WalkNext);
				}
				if (!(*NowField)){
					*NowField = new tExprField(func_name.P);
					(*NowField) ->NamePrefix.Set((*Result)->NamePrefix.P, (*Result)->NamePrefix.L, (*Result)->NamePrefix.Type);
				}
				*/
				//external program register to the external functions in Context directly, or set the Data a function pointer
				//see: IWSqlParser::checkExpressionFunctions() for one example
			//}
			para_num=0;
			while (ExprSentence->Type != synEXP_RIGHTBRACE && ExprSentence->Type != synEXP_END && ExprSentence->Type != synEXP_UNKNOW){
				para_num++;
				if (para_num==1){
					if ((i=parseExpr(synEXP_RIGHTBRACE,(tExprItem**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
				}else if (para_num == 2){
					if ((i=parseExpr(synEXP_RIGHTBRACE,(tExprItem**)&(*Result)->Right,*Result))!=dbSUCCESS) return i;
				}else{
					(*Result)->Info = para_num-1;
					NewExpr=new tExprItem(SavedType);
					NewExpr->setName((*Result)->StringName.P, (*Result)->StringName.L, true);
					NewExpr->NamePrefix = (*Result)->NamePrefix;
					NewExpr->Left=*Result;
					(*Result)->Parent=NewExpr;
					*Result=NewExpr;
					(*Result)->Brand = expCALC;
					(*Result)->Precision=1;
					if ((i=parseExpr(synEXP_RIGHTBRACE,(tExprItem**)&(*Result)->Right,*Result))!=dbSUCCESS) return i;
					//((tExprItem*)((*Result)->Right))->Parent=(tExprField *)(*Result);
				}
				if (ExprSentence->Type == synEXP_COMMA)	ExprSentence->match();
				else if (ExprSentence->Type != synEXP_RIGHTBRACE) return dbBRACEERR;
			}
			(*Result)->Info = para_num;
			if (!ExprSentence->match(synEXP_RIGHTBRACE)) return dbBRACEERR;
		}else{
			(*Result)->Brand=expNODE;
			if ((i=addNewField(*Result))!=dbSUCCESS) return i;
			//if (Tables&&((i=spotTable(TableName,*Result))!=dbSUCCESS)) return i;
			ExprSentence->match();
		}
		break;
	// function para1
	case synEXP_NOT:
		*Result=new tExprItem(ExprSentence->Type);
		(*Result)->setName(ExprSentence->Sentence+ExprSentence->Start,ExprSentence->StartLen, true);
		ExprSentence->match();
		if ((i=parseExpr(synEXP_NOT,(tExprItem**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i; //20040716
		//((tExprItem*)((*Result)->Left))->Parent=(tExprField *)(*Result);
		break;
	// function para1
	case synEXP_SUB:
	case synEXP_ADD:
		*Result=new tExprItem(ExprSentence->Type);
		(*Result)->setName(ExprSentence->Sentence+ExprSentence->Start,ExprSentence->StartLen, true);
		ExprSentence->match();
		if ((i=parseExpr(synEXP_ADD,(tExprItem**)&(*Result)->Left,*Result))!=dbSUCCESS) return i;
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i; //20040716
		//((tExprItem*)((*Result)->Left))->Parent=(tExprField *)(*Result);
		break;

	//function[()]
	case synEXP_SYSDATE:
		*Result=new tExprItem(ExprSentence->Type);
		(*Result)->setName(ExprSentence->Sentence+ExprSentence->Start,ExprSentence->StartLen,true);
		ExprSentence->match();
		if (ExprSentence->Type==synEXP_LEFTBRACE){
			if (!ExprSentence->match(synEXP_LEFTBRACE)) return dbBRACEERR;
			if (!ExprSentence->match(synEXP_RIGHTBRACE)) return dbBRACEERR;
		}
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;//20040716
		break;
	//function
	case synEXP_NULL:
		*Result=new tExprItem(ExprSentence->Type);
		(*Result)->setName(ExprSentence->Sentence+ExprSentence->Start,ExprSentence->StartLen, true);
		ExprSentence->match();
		if ((i=(*Result)->checkCalcType())!=dbSUCCESS) return i;//20040716
		break;

	default:
		return dbSYNTAXERR;
		break;
	}
	return dbSUCCESS;
}

int tExprCalc::getFuncParaTotal(tExprItem*item){
	int total=item->Info; //start from Info, which is the total before item;
	tExprItem* check_item=0, *last_item=item;
	for (check_item=(tExprItem *)item->Parent; check_item; check_item=(tExprItem *)check_item->Parent){
		if (check_item->StringName.Cmp(&item->StringName, true)==0 && 
			check_item->NamePrefix.Cmp(&item->NamePrefix, true)==0 &&
			check_item->Info == last_item->Info+1 
			){
			total++;
			last_item=check_item; 
		}else{
			break;
		}
	}
	return total;
}
int tExprCalc::getFuncParaIndex(tExprItem*item){
	return item->Info;
}
tExprItem* tExprCalc::getFuncParaItem(tExprItem*item, int index1){
	tExprItem* check_item=0;
	if (index1<=2 && item->Info<=2){
		if (index1==1){
			return item->Left?(tExprItem*)item->Left:0;
		}else{
			return item->Right?(tExprItem*)item->Right:0;
		}
	}else if (index1==item->Info){
		return item->Right?(tExprItem*)item->Right:0;
	}
	if (item->Info<index1){
		int index=item->Info;
		for (check_item=(tExprItem *)item->Parent; check_item && index<index1; check_item=(tExprItem *)check_item->Parent){
			if (check_item->StringName.Cmp(&item->StringName, true)==0 && 
				check_item->NamePrefix.Cmp(&item->NamePrefix, true)==0 
				){
				index++;
			}else{
				break;
			}
		}
		if (index==index1 && check_item && check_item->Right) 
			return (tExprItem*) check_item->Right;
		else 
			return 0;
	}else{// item->Info > index1
		int index=item->Info-1;
		for (check_item=(tExprItem *)item->Left; check_item && index>index1 && index>2; check_item=(tExprItem *)check_item->Left){
			if (check_item->StringName.Cmp(&item->StringName, true)==0 && 
				check_item->NamePrefix.Cmp(&item->NamePrefix, true)==0 
				){
				index--;
			}else{
				break;
			}
		}
		if (index==index1 && check_item && check_item->Right) 
			return (tExprItem*) check_item->Right;
		else if (index1==1 && index<=2 && check_item && check_item->Left){
			return (tExprItem*) check_item->Left;
		}else
			return 0;
	}
	return 0;
}
char* tExprCalc::getFuncParaValue(tExprItem*item, int index1){
	tExprItem* item1=getFuncParaItem(item, index1);
	if (item1) return item1->Value; 
	else return 0; 
}
long tExprCalc::getFuncParaLong(tExprItem*item, int index1){
	tExprItem* item1=getFuncParaItem(item, index1);
	if (item1) return item1->StringValue.getLong(); 
	else return 0; 
}
double tExprCalc::getFuncParaDouble(tExprItem*item, int index1){
	tExprItem* item1=getFuncParaItem(item, index1);
	if (item1) return item1->StringValue.getDouble(); 
	else return 0.0; 
}
char* tExprCalc::getFuncName(tExprItem*item){
	if (strchr(preEXP_TYPE_PREFIX, item->StringName.P[0])){
		return item->StringName.P+1;
	}else{
		return item->StringName.P;
	}
}
int tExprCalc::getPrefixName(const char* prefix, int index0, ReferData* name){
	if (!prefix || !*prefix) return -1; 
	char* p1=(char*) prefix;
	char* p2=0, *p3=0;
	int index=0;
	while (p1 && *p1){
		p2=strchr(p1, '.');
		p3=strstr(p1, "->");
		if (p3 && (!p2 || p3<p2)) p2=p3; 
		if (index++>=index0){
			if (p2){
				name->Set(p1, p2-p1, true);
			}else{
				*name = p1; 
			}
			return 0; 
		}
		p1=p2;
		if (p1 && *p1=='.') p1+=1;
		else if (p1 && *p1=='-' && p1[1]=='>'){
			p1+=2;
		}
	}
	return -1;
}

int tExprCalc::reformulateExpression(tExprItem* expr_item, int (*node_transform)(tExprItem*, ReferData*, void*), ReferData* condition, void*p){
	ReferData left_cond, right_cond;
	int num;
	tExprItem* next_item;

	if (!expr_item) return 0;

	switch (expr_item->Brand){
	case expNODE:
		if (node_transform && (*node_transform)(expr_item, &left_cond, p)==0){
			*condition += left_cond;
		}else{
			*condition+=expr_item->Left->NamePrefix;
			*condition+=expr_item->Left->Name;
		}
		break;
	case expCONST:
		switch (expr_item->Type){
		case dbNUMBER:
			*condition+=expr_item->Value;
			break;
		case dbDATE:
			*condition+="#";
			*condition+=expr_item->Value;
			*condition+="#";
			break;
		case dbCHAR:
		case dbLONG:
			*condition+="'";
			*condition+=expr_item->Value;
			*condition+="'";
			break;
		default:
			break;
		}
		break;
	case expBOOL:
		switch (expr_item->CalcOp){
		case synEXP_NOT:
			*condition+="(not ";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=")";
			break;
		case synEXP_OR:
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+="(";
			*condition+=left_cond;
			*condition+=" or ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_AND:
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+="(";
			*condition+=left_cond;
			*condition+=" and ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_LIKE:
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+="(";
			*condition+=left_cond;
			*condition+=" like ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_IS:
			*condition+="(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=" is ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_NULL:
			*condition+="null";
			break;
		default:
			break;
		}
		break;
	case expCOMP:
		*condition+="(";
		reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
		*condition+=left_cond;
		switch (expr_item->CalcOp){
		case synEXP_EQ:
			*condition+=" = ";
			break;
		case synEXP_NE:
			*condition+=" <> ";
			break;
		case synEXP_LT:
			*condition+=" < ";
			break;
		case synEXP_GT:
			*condition+=" > ";
			break;
		case synEXP_LE:
			*condition+=" <= ";
			break;
		case syn_REGEX_MATCH_CASE:
			*condition+=" =~~ ";
			break;
		case syn_REGEX_MATCH:
			*condition+=" =~ ";
			break;
		case synEXP_GE:
			*condition+=" >= ";
			break;
		default:
			break;
		}
		reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
		*condition+=right_cond;
		*condition+=")";
		break;
	case expCALC:
		switch (expr_item->CalcOp){
		case synEXP_ADD:
			if (!expr_item->Right){
				reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
				*condition+=left_cond;
			}else{
				*condition+="(";
				reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
				*condition+=left_cond;
				*condition+=" + ";
				reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
				*condition+=right_cond;
				*condition+=")";
			}
			break;
		case synEXP_SUB:
			if (!expr_item->Right){
				*condition+="(- ";
				reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
				*condition+=left_cond;
				*condition+=")";
			}else{
				*condition+="(";
				reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
				*condition+=left_cond;
				*condition+=" - ";
				reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
				*condition+=right_cond;
				*condition+=")";
			}
			break;
		case synEXP_ASTERISK:
			*condition+="(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=" * ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_DIV:
			*condition+="(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=" / ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case syn_REGEX_SEARCH:
		case syn_REGEX_SEARCH_CASE:
			*condition+="(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			if (expr_item->CalcOp==syn_REGEX_SEARCH_CASE){
				*condition+=" ~~ ";
			}else{
				*condition+=" ~ ";
			}
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_STRCAT:
			*condition+="(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=" +& ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_NULL_OR:
			*condition+="(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=" /& ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_MOD:
			*condition+="(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=" % ";
			reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
			*condition+=right_cond;
			*condition+=")";
			break;
		case synEXP_SUM:
			*condition+="sum(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=")";
			break;
		case synEXP_COUNT:
			*condition+="count(";
			reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
			*condition+=left_cond;
			*condition+=")";
			break;
		case synEXP_NULL:
			*condition+="null";
			break;
		case synEXP_WORD:
			if (expr_item->Info<=2){
				*condition+=expr_item->Name;
				*condition+="(";
				if (expr_item->Info>=1){
					reformulateExpression((tExprItem *)expr_item->Left, node_transform, &left_cond, p);
					*condition+=left_cond;
				}
				if (expr_item->Info>=2){
					reformulateExpression((tExprItem *)expr_item->Right, node_transform, &right_cond, p);
					*condition+=", ";
					*condition+=right_cond;
				}
				num=expr_item->Info;
				next_item = (tExprItem *)expr_item->Parent;
				while (next_item && next_item->Info==num+1 
						&& !tStrOp::strNcmp(next_item->Name, expr_item->Name, strlen(expr_item->Name)+1, false) 
						)
				{
					reformulateExpression((tExprItem *)next_item->Right, node_transform, &right_cond, p);
					*condition+=", ";
					*condition+=right_cond;
					num=next_item->Info;
					next_item = (tExprItem *)next_item->Parent;
				}
				*condition+=")";
			}else{
				return reformulateExpression((tExprItem *)expr_item->Left, node_transform, condition, p);
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return 0;
}

int tExprCalc::setExpression(char* Sentence, long *Len){
	if (ExprSentence) reset();

	ExprSentence = new ExprSyntax;
	int i;
	i=ExprSentence->setSentence(Sentence, Len);
	return i;
}

int tExprCalc::parse(etEXP_SYNTAX PreOp, int built_varlist){
	if (FieldsList) {
		free(FieldsList);
		FieldsList=0;
	}
	/*if (FunctionsList){
		free(FunctionsList);
		FunctionsList=0;
	}*/
	UnkownFunctions.reset();
	UnkownFunctions.setDuplication(false);
	UnkownFunctions.setCaseSensitivity(false);
	int i;
	if ((i=parseExpr(PreOp, &Root)) != dbSUCCESS) return i;
	if (Root){
		setName(Root->Name);
		NamePrefix = Root->NamePrefix;
	}

	tExprField** PreWalk= (tExprField **)&FirstStep;
	if (Root!=NULL && (i=PostWalk(Root,&PreWalk))!=dbSUCCESS) return i;

	if (built_varlist) 
		if ((i=buildVarsList())<0) return i;

	return dbSUCCESS;

}
int tExprCalc::buildVarsList(){
	if (FieldsList) {
		free(FieldsList);
		FieldsList=0;
	}
	tExprField* NowField;  //modify 2004/07/06: add Context-> before Fields in this function
	FieldsNum=0;
	for (NowField=Context->Fields; NowField; NowField=NowField->WalkNext){
		FieldsNum++;
	}
	if (FieldsNum){
		FieldsList = (tExprField**) malloc(sizeof(tExprField*)* FieldsNum);
		if (!FieldsList) return dbMEMORYERR;
		int i=0;
		for (NowField=Context->Fields; NowField; NowField=NowField->WalkNext){
			FieldsList[i++]=NowField;
		}
	}

	/*
	if (FunctionsList){
		free(FunctionsList);
		FunctionsList=0;
	}
	//modify 2004/07/06: add Context-> before Functions in this function
	FunctionsNum=0;
	for (NowField=Context->Functions; NowField; NowField=NowField->WalkNext){
		FunctionsNum++;
	}
	if (FunctionsNum){
		FunctionsList = (tExprField**) malloc(sizeof(tExprField*)* FunctionsNum);
		if (!FunctionsList) return dbMEMORYERR;
		int i=0;
		for (NowField=Context->Functions; NowField; NowField=NowField->WalkNext){
			FunctionsList[i++]=NowField;
		}
	}*/
	return dbSUCCESS;
}

int tExprCalc::cmpFieldName(const char* field_name, const char* test_name){
	if (!field_name){
		if (!test_name) return 0;
		else return -1;
	}
	if (!test_name) return 1;

	long i1=0, len1=field_name?strlen(field_name):0;  
	int type1=getNameStartEnd(field_name, &i1, &len1); 
	long i2=0, len2=test_name?strlen(test_name):0;  
	int type2=getNameStartEnd(test_name, &i2, &len2); 

	if (len1!=len2) return len1-len2; 
	return tStrOp::strNcmp((char*) field_name+i1, (char*) test_name+i2, len1, IsNameCaseSensitive);
}

int tExprCalc::setField(int Index, char* Value,long Length, int copy){
	if (Index > FieldsNum) return dbOVERFLOW;
	if (Length<0) {
		Length=Value?strlen(Value):0;
	}
	FieldsList[Index-1]->newValue(Value, Length, copy);
	//FieldsList[Index-1]->StringValue.Set(Value, strlen(Value), false);
	//FieldsList[Index-1]->Value = Value;
	//if (Length==0 && Value) Length = strlen(Value);
	//FieldsList[Index-1]->Length = Length+1;
//	FieldsList[Index-1]->setType(Value, Length);
	return dbSUCCESS;
}


int tExprCalc::setField(char* name, char* value,long length, int copy){
	tExprField** NowField; //modify 2004/07/06 add Context before Fields
	for (NowField=&Context->Fields; *NowField; NowField=&(*NowField)->WalkNext){
		if (!cmpFieldName((*NowField)->Name, name))
			break;
	}
	if (length<0) {
		length=value?strlen(value):0;
	}
	if (!(*NowField) ) {
		*NowField = new tExprField(name);
		(*NowField)->NamePrefix=0;
		(*NowField)->setType(strCHAR);
		(*NowField)->newValue(value, length, copy);
	}else{
		(*NowField)->newValue(value, length, copy);
		//if (length==0 && value) length = strlen(value);
		//NowField->Length = length+1;
	//	NowField->setType(value, length);
		//NowField->Value = value;
		//NowField->StringValue.Set(value, strlen(value), false);
	}
	return dbSUCCESS;
}

tExprField* tExprCalc::getField(char* Name){
	tExprField* NowField; //modify 2004/07/06 add Context before Fields
	for (NowField=Context->Fields; NowField; NowField=NowField->WalkNext){
		if (!cmpFieldName(NowField->Name, Name)) break;
	}
	return NowField;
}

int tExprCalc::calculate(int initial_group){
	if (!Root) return 0;
	int i;
	for (CurrentStep = FirstStep; CurrentStep; ){
		if ((i=CurrentStep->Calculate(this, initial_group))!=dbSUCCESS) return i;
		//debug check for this: CurrentStep.Brand, CurrentStep.CalcOp, CurrentStep.Value, CurrentStep.Name
		if (!CurrentStep->Parent ) break;
		else{
			if (((tExprItem*)(CurrentStep->Parent))->Brand==expBOOL){
				switch (((tExprItem*)(CurrentStep->Parent))->CalcOp){
				case synEXP_AND:
					if (!CurrentStep->BoolValue) 
						CurrentStep=(tExprItem *)CurrentStep->Parent;
					else
						CurrentStep=(tExprItem *)CurrentStep->WalkNext;
					break;
				case synEXP_OR:
					if (CurrentStep->BoolValue) 
						CurrentStep=(tExprItem *)CurrentStep->Parent;
					else
						CurrentStep=(tExprItem *)CurrentStep->WalkNext;
					break;
				default:
					CurrentStep=(tExprItem *)CurrentStep->WalkNext;
					break;
				}
			}else{
				CurrentStep=(tExprItem *)CurrentStep->WalkNext;
			}
		}
	}
	cpField(Root);
	return 0;
}
char* tExprCalc::getString(){
	return Root? Root->Value : Value;
}
int tExprCalc::getBoolean(){
	return Root? Root->BoolValue: BoolValue;
}
double tExprCalc::getDouble(){
	return Root? Root->DoubleValue: DoubleValue;
}
long tExprCalc::getLong(){
	double d= Root? Root->DoubleValue: DoubleValue;
	if (d<-0.0005) d-=0.001; else d+=0.001;
	return (long) d;
}

int tExprCalc::functionCount(tExprCalc*calc, tExprItem*item, void* p){
	int total_para=calc->getFuncParaTotal(item); 
	if (calc->getFuncParaIndex(item)!=total_para) return 0; 

	int initial_group=item->IsNewGroup; 

	if (initial_group || !item->StringValue.P){
		item->newValue(NumberDefLength);
		item->DoubleValue=total_para;
	}else{
		item->DoubleValue+=total_para;
	}
	int i=item->StringPrintf(item->Value,item->DoubleValue,item->Length,item->Precision);
	if (i!=0) return i;
	item->StringValue.L=strlen(item->Value);
	return 0;
}
int tExprCalc::functionSum(tExprCalc*calc, tExprItem*item, void* p){
	int initial_group=item->IsNewGroup; 
	if (item->Info<=2){ //the first item
		if (initial_group || !item->StringValue.P){
			item->newDoubleValue(0, NumberDefLength, item->Precision); 
		}
	}
	if (item->Left){
		if (item->Length<item->Left->Length+1) item->Length=item->Left->Length+1; 
		if (item->Precision<item->Left->Precision) item->Precision=item->Left->Precision; 
		item->DoubleValue+=item->Left->DoubleValue;  
	}
	if (item->Right){
		if (item->Length<item->Right->Length+1) item->Length=item->Right->Length+1; 
		if (item->Precision<item->Right->Precision) item->Precision=item->Right->Precision; 
		item->DoubleValue+=item->Right->DoubleValue;  
	}
	int i=item->StringPrintf(item->Value,item->DoubleValue,item->Length,item->Precision);
	if (i!=0) return i;
	item->StringValue.L=strlen(item->Value);
	return 0;
}
int tExprCalc::functionMax(tExprCalc*calc, tExprItem*item, void* p){
	int initial_group=item->IsNewGroup; 
	if (item->Info<=2){ //the first item
		if (initial_group || !item->StringValue.P){
			item->newDoubleValue(-1e20, NumberDefLength, item->Precision); 
		}
	}
	if (item->Left){
		if (item->Left->DoubleValue>item->DoubleValue){
			item->Length=item->Left->Length; 
			item->Precision=item->Left->Precision; 
			item->DoubleValue=item->Left->DoubleValue;  
		}
	}
	if (item->Right){
		if (item->Right->DoubleValue>item->DoubleValue){
			item->Length=item->Right->Length; 
			item->Precision=item->Right->Precision; 
			item->DoubleValue=item->Right->DoubleValue;  
		}
	}
	int i=item->StringPrintf(item->Value,item->DoubleValue,item->Length,item->Precision);
	if (i!=0) return i;
	item->StringValue.L=strlen(item->Value);
	return 0;
}
int tExprCalc::functionMin(tExprCalc*calc, tExprItem*item, void* p){
	int initial_group=item->IsNewGroup; 
	if (item->Info<=2){ //the first item
		if (initial_group || !item->StringValue.P){
			item->newDoubleValue(1e20, NumberDefLength, item->Precision); 
		}
	}
	if (item->Left){
		if (item->Left->DoubleValue<item->DoubleValue){
			item->Length=item->Left->Length; 
			item->Precision=item->Left->Precision; 
			item->DoubleValue=item->Left->DoubleValue;  
		}
	}
	if (item->Right){
		if (item->Right->DoubleValue<item->DoubleValue){
			item->Length=item->Right->Length; 
			item->Precision=item->Right->Precision; 
			item->DoubleValue=item->Right->DoubleValue;  
		}
	}
	int i=item->StringPrintf(item->Value,item->DoubleValue,item->Length,item->Precision);
	if (i!=0) return i;
	item->StringValue.L=strlen(item->Value);
	return 0;
}

int tExprCalc::functionSubstr(tExprCalc*calc, tExprItem*item, void* p){
	long n;

	if (item->Info == 1){ //one parameter
		item->newValue(item->Left->Value, item->Left->Length);

	}else if (item->Info == 2) { //two parameter, && item->Right->Type==dbNUMBER
		n = (long) item->Right->DoubleValue;
		if (n<0) n=0;
		else if (n>item->Left->Length) n=item->Left->Length;
		item->newValue(item->Left->Value + n, item->Left->Length - n);

	}else if (item->Info == 3) { //three parameter, && item->Right->Type==dbNUMBER
		n = (long) item->Right->DoubleValue;
		if (n<0) n=0;
		if (n>item->Left->Length) n=item->Left->Length;
		item->newValue(item->Left->Value, n);

	}else {
		item->newValue(item->Left->Value, item->Left->Length);
	}

	item->Type = dbCHAR;

	return 0;
}

int tExprCalc::functionLtrim(tExprCalc*calc, tExprItem*item, void* p){
	char* str;
	if ( item->Left && item->Left->Value){
		if (item->Right && item->Right->Value) {
			str=item->Left->Value;
			while (*str!='\0' && strchr(item->Right->Value,*str) )
				str++;
			item->newValue(str, strlen(str), true);
		}else{
			str=item->Left->Value;
			while (*str!='\0' && isspace(*str))
				str++;
			item->newValue(str, strlen(str), true);
		}
	}else{
		item->newValue("", 0, true);
	}

	item->Type = dbCHAR;

	return 0;
}

int tExprCalc::functionRtrim(tExprCalc*calc, tExprItem*item, void* p){
	item->newValue("", 0, true);
	char* str;
	if ( item->Left && item->Left->Value){
		if (item->Right && item->Right->Value){
			str=item->Left->Value+strlen(item->Left->Value)-1;
			while (str>=item->Left->Value && strchr(item->Right->Value,*str) )
				str--;
			if (str>=item->Left->Value){
				item->newValue(item->Left->Value, str+1-item->Left->Value, true);
			}
		}else{
			str=item->Left->Value+strlen(item->Left->Value)-1;
			while (str>=item->Left->Value && isspace(*str))
				str--;
			if (str>=item->Left->Value){
				item->newValue(item->Left->Value,str+1-item->Left->Value, true);
			}
		}
	}

	item->Type = dbCHAR;

	return 0;
}

int tExprCalc::functionStrcat(tExprCalc*calc, tExprItem*item, void* p){
	long len=0;
	if (item->Left) len+=item->Left->Length;
	if (item->Right) len+=item->Right->Length;

	int initial_group=item->IsNewGroup; 
	if (!initial_group && calc->getFuncParaIndex(item)==calc->getFuncParaTotal(item)){
		len+=item->Length; 
		ReferData old_data; 
		old_data.Set(item->StringValue.P, item->StringValue.L, true);
		item->newValue(len); 
		if (old_data.P) strcpy(item->Value, old_data.P); 
	}else{
		item->newValue(len); 
	}

	if (item->Left && item->Left->Value) strcat(item->Value, item->Left->Value);
	if (item->Right && item->Right->Value) strcat(item->Value, item->Right->Value);

	item->Type = dbCHAR;
	item->StringValue.L=strlen(item->Value);
	item->Length = item->StringValue.L;

	return 0;
}
int tExprCalc::functionStrcmp(tExprCalc*calc, tExprItem*item, void* p){
	long len=0;
	if (item->Left) len+=item->Left->Length;
	if (item->Right) len+=item->Right->Length;

	int ret=0; 
	
	if ((!item->Left || !item->Left->Value) && (!item->Right || !item->Right->Value )){
		ret=0; 
	}else if (!item->Left || !item->Left->Value){
		ret=-1;
	}else if (!item->Right || !item->Right->Value ){
		ret=1;
	}else{
		ret=tStrOp::StrCmp(item->Left->Value, item->Right->Value, true);
	}

	item->newDoubleValue(ret, 2, 0);
	return 0;
}
int tExprCalc::functionStrcmpi(tExprCalc*calc, tExprItem*item, void* p){
	long len=0;
	if (item->Left) len+=item->Left->Length;
	if (item->Right) len+=item->Right->Length;

	int ret=0; 
	
	if ((!item->Left || !item->Left->Value) && (!item->Right || !item->Right->Value )){
		ret=0; 
	}else if (!item->Left || !item->Left->Value){
		ret=-1;
	}else if (!item->Right || !item->Right->Value ){
		ret=1;
	}else{
		ret=tStrOp::StrCmp(item->Left->Value, item->Right->Value, false);
	}

	item->newDoubleValue(ret, 2, 0);
	return 0;
}
int tExprCalc::functionReplace(tExprCalc*calc, tExprItem*item, void* p){
	if (calc->getFuncParaIndex(item)==calc->getFuncParaTotal(item) ){
		char* str=getFuncParaValue(item, 1);
		char* source=getFuncParaValue(item, 2);
		char* target=getFuncParaValue(item, 3);
		char* case_sensitive=getFuncParaValue(item, 4);

		int isensitive=1; 
		if (case_sensitive && 
			(!tStrOp::strNcmp(case_sensitive, "false", 5, false) 
			|| !tStrOp::strNcmp(case_sensitive, "0", 1, false)) 
			)
			isensitive=false;

		if (!target) target="";
		char* newstr=0;
		if (source){
			newstr=tStrOp::replaceMalloc(str, source, target, 0, isensitive);
		}
		
		if (newstr) {
			item->newValue(newstr, strlen(newstr), true);
		}else{
			item->newValue(10);
		}
		if (newstr) free(newstr);
	}

	return 0;
}
int tExprCalc::functionLower(tExprCalc*calc, tExprItem*item, void* p){ //lower(str)
	if (calc->getFuncParaIndex(item)==calc->getFuncParaTotal(item) ){
		char* str=getFuncParaValue(item, 1);
		if (str) {
			item->newValue(str, strlen(str), true);
			for (long i=0; i<item->StringValue.L; i++){
				item->StringValue.P[i]=tolower( item->StringValue.P[i] );
			}
		}else{
			item->newValue(10);
		}
	}

	return 0;
}
int tExprCalc::functionUpper(tExprCalc*calc, tExprItem*item, void* p){ //upper(str)
	if (calc->getFuncParaIndex(item)==calc->getFuncParaTotal(item) ){
		char* str=getFuncParaValue(item, 1);
		if (str) {
			item->newValue(str, strlen(str), true);
			for (long i=0; i<item->StringValue.L; i++){
				item->StringValue.P[i]=toupper( item->StringValue.P[i] );
			}
		}else{
			item->newValue(10);
		}
	}

	return 0;
}

int tExprCalc::functionStrlen(tExprCalc*calc, tExprItem*item, void* p){ //strlen(str)
	item->newDoubleValue((item->Left&&item->Left->Value)?strlen(item->Left->Value):0, NumberDefLength, 0);

	return 0;
}
int tExprCalc::functionStrFind(tExprCalc*calc, tExprItem*item, void* p){ //strlen(str)
	char* str=getFuncParaValue(item, 1);
	char* target=getFuncParaValue(item, 2);
	char* startpos0=getFuncParaValue(item, 3);
	if (!str) str="";
	if (!target) target="";
	long start_from=0;
	if (startpos0) start_from=atoi(startpos0);
	if (start_from<0) start_from=0;
	
	char* foundstr=0;
	if (start_from>=0 && start_from<(long)strlen(str)) {
		foundstr=strstr(str+start_from, target);
	}

	long index=-1; 
	if (foundstr) index=foundstr-str;

	item->newDoubleValue(index, NumberDefLength, 0);

	return 0;
}
int tExprCalc::functionFormat(tExprCalc*calc, tExprItem*item, void* p){ //format(fmt, ...)
	int params=calc->getFuncParaTotal(item);
	if (calc->getFuncParaIndex(item) < params) return 0;

	char* fmt=calc->getFuncParaValue(item, 1);
	int pindex=2; 
	char* para;
	long width=0, prec=0;

	const char* flags="+- 0#";
	const char* lengths="hlL";

	char* p1=fmt;
	ReferData buf, shortfmt;
	ReferData result(128);
	while (p1 && *p1){
		char* p2=strchr(p1, '%'); 
		if (p2){
			buf.Set(p1, p2-p1, true);
			convertSpaceSpecial(buf.P); 
			buf.L=strlen(buf.P); 
			result+=buf;

			char* p3=p2+1;
			//%%
			if (*p3=='%'){
				result+="%"; 
			}else{
				//flags
				if (strchr(flags, *p3)) p3++; 

				//width
				width=0; 
				sscanf(p3, "%ld", &width); 
				while (*p3 && tStrOp::isDigit(*p3) ) p3++; 

				//prec
				prec=0;
				if (*p3=='.') {
					p3++; 
					sscanf(p3, "%ld", &prec); 
					while (*p3 && tStrOp::isDigit(*p3) ) p3++; 
				}

				//length
				if (strchr(lengths, *p3)) p3++; 

				para=calc->getFuncParaValue(item, pindex++);
				if (!para) para="";
				shortfmt.Set(p2, p3-p2+1, true); 
				if (*p3=='s'){
					//string
					if ((long)strlen(para)>width) width=strlen(para); 
					buf.Malloc( ((width+64)/128+1)*128 );
					sprintf(buf.P, shortfmt.P, para);
				}else if (*p3=='c'){
					//char
					buf.Malloc( ((width+64)/128+1)*128 );
					sprintf(buf.P, shortfmt.P, *para);
				}else if (*p3=='d' || *p3=='i' || *p3=='o' || *p3=='u'  || *p3=='x' || *p3=='X' || *p3=='p' || *p3=='n'){
					//long
					long val=0;
					sscanf(para, "%ld", &val); 
					buf.Malloc( ((width+64)/128+1)*128 );
					sprintf(buf.P, shortfmt.P, val);
				}else if (*p3=='e' || *p3=='E' || *p3=='f' || *p3=='g'  || *p3=='G'){
					//double
					double val=0;
					sscanf(para, "%lf", &val); 
					buf.Malloc( ((width+64)/128+1)*128 );
					sprintf(buf.P, shortfmt.P, val);
				}else{
					//no format
					buf.Set(p2, p3-p2, true);
					pindex--; 
					p3--;
				}
				buf.L=strlen(buf.P);
				result+=buf; 
			}
			p1=p3+1; 
		}else{//no %
			buf=p1;
			convertSpaceSpecial(buf.P); 
			buf.L=strlen(buf.P); 
			result+=buf;
			p1=0;
		}
	}

	item->newValue(result.P, result.L, false);
	result.setToFree(false);

	return 0;
}
int tExprCalc::functionGetEmails(tExprCalc*calc, tExprItem*item, void* p){ //get_email(text, fmt)
	int params=calc->getFuncParaTotal(item);
	if (calc->getFuncParaIndex(item) < params) return 0;

	char* text=calc->getFuncParaValue(item, 1);
	char* fmt=calc->getFuncParaValue(item, 2); //support later

	ReferData result; 
	tStrOp::GetEmails(text, fmt, &result);

	item->newValue(result.P, result.L, false);
	result.setToFree(false);
	return 0;
}

int tExprCalc::functionTextWords(tExprCalc*calc, tExprItem*item, void* p){ //text_words(str)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;
	char* text=calc->getFuncParaValue(item, 1); 
	ReferData type;
	type=calc->getFuncParaValue(item, 2); 
	ReferData option;
	option=calc->getFuncParaValue(item, 3); 
	
	ReferData result;
	PDocBase::getTextWords(text, type.P, option.P, &result); 
	if (!type.Cmp("count",strlen("count"),false)){
		item->newDoubleValue(result.getLong(), NumberDefLength, 0);
	}else{
		item->newValue(result.P, result.L, true); 
	}
	
	return 0;
}
int tExprCalc::functionHtmlEncode(tExprCalc*calc, tExprItem*item, void* p){	//html_encode(str)
	if (item->Left && item->Left->Value) {
		int is_malloc=0;
		char* str=tStrOp::encodeHtml(item->Left->Value, &is_malloc);
		item->newValue(str, strlen(str), !is_malloc);
		item->StringValue.setToFree(true); 
	}else{
		item->newValue(0);
	}
	return 0;
}
int tExprCalc::functionHtmlDecode(tExprCalc*calc, tExprItem*item, void* p){	//html_decode(str)
	if (item->Left && item->Left->Value) {
		ReferData value;
		value=item->Left->Value;
		tStrOp::decodeHtml(value.P); value.L=strlen(value.P);
		item->newValue(value.P, value.L, true);	
	}else{
		item->newValue(0);
	}
	return 0;
}
int tExprCalc::functionUrlEncode(tExprCalc*calc, tExprItem*item, void* p){	//url_encode(str)
	if (item->Left && item->Left->Value) {
		char* str=tStrOp::encodeUrl(item->Left->Value);
		item->newValue(str, strlen(str));
		free(str);
	}else{
		item->newValue(0);
	}
	return 0;
}
int tExprCalc::functionUrlDecode(tExprCalc*calc, tExprItem*item, void* p){	//url_decode(str)
	if (item->Left && item->Left->Value) {
		char* str=tStrOp::decodeUrl(item->Left->Value);
		item->newValue(str, strlen(str));
		free(str);
	}else{
		item->newValue(0);
	}
	return 0;
}

int tExprCalc::functionToNumber(tExprCalc*calc, tExprItem*item, void* p){
	double Double;
	if ( item->Left && item->Left->Type==dbCHAR ||item->Left->Type==dbLONG) {
		item->DoubleValue=0;
		if (item->Left->Value) sscanf(item->Left->Value,"%lf",&item->DoubleValue);
		item->newDoubleValue(item->DoubleValue, NumberDefLength, 0);
		Double=item->DoubleValue-(long)item->DoubleValue;
		item->Precision=0;
		while (item->Precision<item->Length-1 && Double>Epsilon){
			item->Precision++;
			Double = Double*10-int(Double*10);
		}
		int i=StringPrintf(item->Value,item->DoubleValue,item->Length,item->Precision);
		if (i!=0) return i;
	}else if ( item->Left && item->Left->Type==dbDATE) {
		item->newDoubleValue(item->Left->DoubleValue, NumberDefLength, 0);
	}else{
		item->newDoubleValue(item->Left?item->Left->DoubleValue:0, NumberDefLength, 0);
	}

	return 0;
}

int tExprCalc::functionIsChar(tExprCalc*calc, tExprItem*item, void* p){	//ischar(str)
	int is_alpha=false;
	if (item->Left && item->Left->Value){
		char* first=item->Left->Value;
		while (*first && isspace(*first)) first++;
		is_alpha=isalpha(*first) || *first=='_';
	}
	item->newDoubleValue(is_alpha, NumberDefLength, 0);
	return 0;
}
int tExprCalc::functionIsNumber(tExprCalc*calc, tExprItem*item, void* p){	//isnumber(str)
	int is_numeric=false;
	if (item->Left && item->Left->Value){
		char* first=item->Left->Value;
		while (*first && isspace(*first)) first++;
		is_numeric=(*first>='0' && *first<='9');
	}
	item->newDoubleValue(is_numeric, NumberDefLength, 0);
	return 0;
}
int tExprCalc::functionIsPhoneNumber(tExprCalc*calc, tExprItem*item, void* p){	//isphonenumber(str)
	int is_numeric=false;
	if (item->Left && item->Left->Value){
		is_numeric=tStrOp::isValidPhoneNum(item->Left->Value);
	}
	item->newDoubleValue(is_numeric, NumberDefLength, 0);
	return 0;
}
int tExprCalc::functionIsBlank(tExprCalc*calc, tExprItem*item, void* p){	//isblank(str)
	int is_blank=true;
	if (item->Left && item->Left->Value){
		char* first=item->Left->Value;
		while (*first && isspace(*first)) first++;
		is_blank=!(*first);
	}
	item->newDoubleValue(is_blank, NumberDefLength, 0);
	return 0;
}
int tExprCalc::functionTime(tExprCalc*calc, tExprItem*item, void* p){//time(t)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	time_t now=time(0);
	if (item->Info>0 && item->Left){
		now=(time_t)(item->Left->DoubleValue);
	}
	item->newDoubleValue(now, NumberDefLength, 0); 

/*
	if (time>0){ //time component of the datetime
		struct tm t;
		tm* t1=localtime(&now);
		memcpy(&t,t1,sizeof(tm));
		t.tm_year=70;
		t.tm_mon=0;
		t.tm_mday=1;
		t.tm_wday=0;
		now=mktime(&t);
		item->newDateValue(now); 
	}else{ //integer time
		item->newDoubleValue(time(0), NumberDefLength, 0); 
	}
*/
	return 0;
}
int tExprCalc::functionDate(tExprCalc*calc, tExprItem*item, void* p){//date(d)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	time_t now=time(0);
	if (item->Info>0 && item->Left){
		now=(time_t)(item->Left->DoubleValue);
	}
	item->newDateValue(now); 

/*
	struct tm t;
	tm* t1=localtime(&now);
	memcpy(&t,t1,sizeof(tm));
	t.tm_hour=19; //refer tStrOp::DateToLong(), use time 0
	t.tm_min=0;
	t.tm_sec=0;
	now=mktime(&t);
	item->newDateValue(now); 
*/
	return 0;
}
int tExprCalc::functionSRand(tExprCalc*calc, tExprItem*item, void* p){	//srand()
	unsigned int randseed=0;
	if (item->Info==1 && item->Left){
		randseed=(unsigned int) item->Left->DoubleValue;
	}
	srand(randseed);

	item->newDoubleValue(randseed, NumberDefLength, 0);

	return 0;
}
int tExprCalc::functionRand(tExprCalc*calc, tExprItem*item, void* p){//rand()
	static unsigned int randseed=0;
	if (!randseed) {
		randseed=time(0);
		srand(randseed);
	}
	int randmax=0;
	if (item->Info>=1 && item->Left && item->Left->Value){
		sscanf(item->Left->Value,"%d",&randmax);
	}

	item->newValue(NumberDefLength);
	int t=rand();
	if (randmax>1 || randmax==0){
		if (randmax>1) t=t%randmax;
		item->newDoubleValue(t, NumberDefLength, 0);
	}else if (randmax==1) {
		item->newDoubleValue((t%10000)/(double)10000, NumberDefLength, 4);
	}
	return 0;
}
int tExprCalc::functionRound(tExprCalc*calc, tExprItem*item, void* p){//round()
	if (item->Info !=1) return dbSYNTAXERR;

	long rounded=0;
	if (item->Left) {
		rounded=(long) (item->Left->DoubleValue+1e-10);
		if (item->Left->DoubleValue-rounded>=0.5) rounded++;
	}

	item->newDoubleValue(rounded, NumberDefLength, 0);
	return 0;
}
int tExprCalc::functionFloor(tExprCalc*calc, tExprItem*item, void* p){//round()
	if (item->Info !=1) return dbSYNTAXERR;

	long rounded=0;
	if (item->Left) rounded=(long) (item->Left->DoubleValue+1e-10);

	item->newDoubleValue(rounded, NumberDefLength, 0);
	return 0;
}
int tExprCalc::functionCeil(tExprCalc*calc, tExprItem*item, void* p){//round()
	if (item->Info !=1) return dbSYNTAXERR;

	long rounded=0;
	if (item->Left) rounded=(long) (item->Left->DoubleValue+1-1e-10);

	item->newDoubleValue(rounded, NumberDefLength, 0);
	return 0;
}
int tExprCalc::functionAbs(tExprCalc*calc, tExprItem*item, void* p){//abs()
	if (item->Info !=1) return dbSYNTAXERR;

	double val=0; 
	if (item->Left) val=item->Left->DoubleValue; 
	if (val<0) val=-val; 

	item->newDoubleValue(val, NumberDefLength, item->Left->Precision);
	return 0;
}
int tExprCalc::functionToChar(tExprCalc*calc, tExprItem*item, void* p){
	//date to char conversion: to_char(sysdate(), 'YYYYMMDDHH23MI')
	time_t now=0;
	char* str;
	int i;

	//set format
	char* fmt=DateFormat;
	if (item->Right && item->Right->Value) 
		fmt=item->Right->Value;

	str=(char*)malloc(sizeof(char)*(strlen(fmt)+100));
	if (!str) return dbMEMORYERR;

	//set time
	if (item->Left && item->Left->Value) now=(time_t) item->Left->DoubleValue;
	else now=time(0);

	//convert
	DateToChar(now,fmt,str);

	item->Length=strlen(str)+1;
	if ((i=item->newValue(item->Length))!=dbSUCCESS){
		free (str);
		return i;
	}
	strcpy(item->Value,str);
	free(str);

	item->Type = dbCHAR;
	item->StringValue.L=strlen(item->Value);

	return 0;
}

int tExprCalc::functionToDate(tExprCalc*calc, tExprItem*item, void* p){
	//to_date('2007/9/15:13:03:11', 'YYYY/MM/DD:HH24:MI:SS', now)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	char* datestr=calc->getFuncParaValue(item, 1); 
	char* fmt1=calc->getFuncParaValue(item, 2); 
	char* date1=calc->getFuncParaValue(item, 3); //default date time_t

	char* fmt=DateFormat;
	if (fmt1 && *fmt1) fmt=fmt1;

	time_t now=0;
	if (date1 && *date1) sscanf(date1,"%ld",&now);

	tExprItem* item1=calc->getFuncParaItem(item, 1); 
	//if ( item->Left && (item->Left->Type==dbCHAR||item->Left->Type==dbLONG)) {
	if ( item1 && (item1->Type==dbCHAR||item1->Type==dbLONG)) {
		DateToLong(item1->Value,fmt,&now);
	}else{
		time_t now1=0;
		if (item1) now1=(long) item1->DoubleValue;
		if (now1>0 && now1<3000)
			DateToLong(item1->Value,fmt,&now);
		else now=now1;
	}
	item->newDateValue(now);

	return 0;
}

time_t tExprCalc::getDateFromCharType(tExprField*item){
	time_t now=0;
	if (item){
		if (item->Type==dbCHAR || item->Type==dbLONG){
			//assume it is in date format, convert to date
#ifdef DateStandardFormat
			DateToLong(item->Value,DateFormat,&now);
#else
			sscanf(item->Value, "%ld", &now);
#endif
			item->DoubleValue=now;
		}else{
			now=(time_t) item->DoubleValue;
		}
	}
	return now;
}
int tExprCalc::functionMonthsBetween(tExprCalc*calc, tExprItem*item, void* p){
	time_t now;
	tm* pt;
	int i,n;

	now=getDateFromCharType(item->Right);
	pt=localtime(&now);
	n=pt->tm_year;
	i=pt->tm_mon;
	now=getDateFromCharType(item->Left);
	pt=localtime(&now);
	n-=pt->tm_year;
	i-=pt->tm_mon;
	item->newDoubleValue(n*12+i, NumberDefLength, 0);

	return 0;
}

int tExprCalc::functionAddMonths(tExprCalc*calc, tExprItem*item, void* p){
	//add_months('2007/9/15', 2)
	time_t now=getDateFromCharType(item->Left);

	tm* pt;
	pt=localtime(&now);
	pt->tm_mon +=(int)(item->Right?item->Right->DoubleValue:0);
	pt->tm_year +=pt->tm_mon/12;
	pt->tm_mon %=12;
	if (pt->tm_mon <=0){
		pt->tm_mon+=12;
		pt->tm_year--;
	}
	now=mktime(pt);

	item->newDateValue(now);

	return 0;
}

int tExprCalc::functionLastDay(tExprCalc*calc, tExprItem*item, void* p){
	//last day of the month: last_day('2007/9/15');
	time_t now=getDateFromCharType(item->Left);

	tm* pt;
	int n;
	pt=localtime(&now);
	n=MonthDays(pt->tm_year,pt->tm_mon);
	pt->tm_mday=n;
	now=mktime(pt);

	item->newDateValue(now);

	return 0;
}

int tExprCalc::functionNextDay(tExprCalc*calc, tExprItem*item, void* p){
	//next_day('2007/9/15', 'MONDAY')
	time_t now=getDateFromCharType(item->Left);

	tm* pt;
	int i=0,n;
	pt=localtime(&now);
	if (item->Right && item->Right->Value){
		for (i=0; i<7;i++){
			if (tStrOp::StrCmp(item->Right->Value,S_DAY[i])==0) 
				break;
		}
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

	item->newDateValue(now);

	return 0;
}
int tExprCalc::functionDateDiff(tExprCalc*calc, tExprItem*item, void* p){	//DateDiff(interval, date1, date2); //, [firstdayofweek], [firstweekofyear])
	long diff=0; 
	if (item->Info==3){
		time_t now1, now2;
		tm* pt;
		tm t1, t2;

		char* type=((tExprItem*) item->Left)->Left->Value;

		now1=getDateFromCharType(((tExprItem*) item->Left)->Right); //second parameter
		pt=localtime(&now1);
		memcpy(&t1, pt, sizeof(tm));
		now2=getDateFromCharType(item->Right); //third parameter
		pt=localtime(&now2);
		memcpy(&t2, pt, sizeof(tm));

		if (type){
			if (!strcmp(type, "year") || !strcmp(type, "yyyy") || !strcmp(type, "yy") ){
				diff=t1.tm_year - t2.tm_year;
			}else if (!strcmp(type, "month") || !strcmp(type, "mm") || !strcmp(type, "m") ){
				diff=(t1.tm_year - t2.tm_year)*12+(t1.tm_mon - t2.tm_mon);
			}else if (!strcmp(type, "dayofyear") || !strcmp(type, "dy") || !strcmp(type, "y") ){
				diff=(t1.tm_yday-t2.tm_yday); 
			}else if (!strcmp(type, "day") || !strcmp(type, "dd") || !strcmp(type, "d") ){
				diff=(t1.tm_year - t2.tm_year)*365+(t1.tm_yday-t2.tm_yday); //approximate!!
				//diff=(now1-6*60*60)/60/60/24 - (now2-6*60*60)/60/60/24;
				//diff=(now1-now2)/60/60/24;
			}else if (!strcmp(type, "week") || !strcmp(type, "wk") || !strcmp(type, "ww") ){
				diff=((t1.tm_year-t2.tm_year)*365+(t1.tm_yday-t1.tm_wday)-(t2.tm_yday-t2.tm_wday))/7;
				//diff=(now1-6*60*60)/60/60/24/7 - (now2-6*60*60)/60/60/24/7;
				//diff=(now1-now2)/60/60/24/7;
			}else if (!strcmp(type, "hour") || !strcmp(type, "hh") || !strcmp(type, "h") ){
				diff=((t1.tm_year - t2.tm_year)*365+(t1.tm_yday-t2.tm_yday))*24+t1.tm_hour-t2.tm_hour;
				//diff=(now1-6*60*60)/60/60 - (now2-6*60*60)/60/60;;
				//diff=(now1-now2)/60/60;
			}else if (!strcmp(type, "minute") || !strcmp(type, "min") || !strcmp(type, "n")){
				diff=now1/60 - now2/60;
				//diff=(now1-now2)/60;
			}else if (!strcmp(type, "second") || !strcmp(type, "ss") || !strcmp(type, "s")){
				diff=(now1-now2);
			}
		}else{
			diff=(now1-now2);
		}

		item->newDoubleValue(diff, NumberDefLength, 0);
	}
	return 0;	
}
int tExprCalc::functionReadFile(tExprCalc*calc, tExprItem*item, void* p){
	ReferData data;
	ReferData file;

	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	if (item && item->Left && item->Info>=1){
		file.Set(item->Left->StringValue.P, item->Left->StringValue.L, false);
	}
	if (file.L){
		data.readFile(file.P);
	}
	item->newValue(data.P, data.L );
	item->Type = dbCHAR;
	return 0;
}
int tExprCalc::functionSaveFile(tExprCalc*calc, tExprItem*item, void* p){
	ReferData data;
	ReferData file;

	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	if (item && item->Left && item->Info>=1){
		file.Set(item->Left->StringValue.P, item->Left->StringValue.L, false);
	}
	if (item && item->Right && item->Info>=2){
		data.Set(item->Right->StringValue.P, item->Right->StringValue.L, false);
	}
	if (file.L){
		data.saveFile(file.P);
	}
	item->newValue(data.P, data.L );
	item->Type = dbCHAR;
	return 0;
}
int tExprCalc::functionAppendFile(tExprCalc*calc, tExprItem*item, void* p){
	ReferData data;
	ReferData file;

	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	if (item && item->Left && item->Info>=1){
		file.Set(item->Left->StringValue.P, item->Left->StringValue.L, false);
	}
	if (item && item->Right && item->Info>=2){
		data.Set(item->Right->StringValue.P, item->Right->StringValue.L, false);
	}
	if (file.L){
		data.appendFile(file.P);
	}
	item->newValue(data.P, data.L );
	item->Type = dbCHAR;
	return 0;
}
int tExprCalc::functionRenameFile(tExprCalc*calc, tExprItem*item, void* p){
	ReferData file1;
	ReferData file2;

	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	if (item && item->Left && item->Info>=1){
		file1.Set(item->Left->StringValue.P, item->Left->StringValue.L, false);
	}
	if (item && item->Right && item->Info>=2){
		file2.Set(item->Right->StringValue.P, item->Right->StringValue.L, false);
	}
	int err=0;
	if (file1.L && file2.L){
		err=rename(file1.P, file2.P);
	}
	item->newDoubleValue(err, 10, 0);
	return 0;
}
int tExprCalc::functionDeleteFile(tExprCalc*calc, tExprItem*item, void* p){
	ReferData file1;

	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	if (item && item->Left && item->Info>=1){
		file1.Set(item->Left->StringValue.P, item->Left->StringValue.L, false);
	}
	int err=0;
	if (file1.L){
		err=unlink(file1.P);
	}
	item->newDoubleValue(err, 10, 0);
	return 0;
}
int tExprCalc::functionMkDir(tExprCalc*calc, tExprItem*item, void* p){
	ReferData file1;

	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	if (item && item->Left && item->Info>=1){
		file1.Set(item->Left->StringValue.P, item->Left->StringValue.L, false);
	}
	int err=0;
	if (file1.L){
#ifdef WIN32
		err=mkdir(file1.P);
#else
		err=mkdir(file1.P, S_IRWXU);
#endif
	}
	item->newDoubleValue(err, 10, 0);
	return 0;
}

int tExprCalc::functionTempPath(tExprCalc*calc, tExprItem*item, void* p){
					//TempPath()
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	ReferData temp_path; 
	DirFunc::getTempPath(&temp_path);

	item->newValue(temp_path.P, temp_path.L);
	item->Type = dbCHAR;
	return 0;
}

int tExprCalc::functionGetFilePath(tExprCalc*calc, tExprItem*item, void* p){	
				//GetFilePath(filename) 
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;
	ReferData filename; 
	filename=calc->getFuncParaValue(item, 1);
	ReferData filepath, name; 
	tStrOp::getFilePath(filename.P, &filepath, &name); //not including '/' or'\\'

	item->newValue(filepath.P, filepath.L, true); 
	return 0;	
}
int tExprCalc::functionGetFileName(tExprCalc*calc, tExprItem*item, void* p){	
				//GetFileName(filename)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;
	ReferData filename; 
	filename=(const char*) calc->getFuncParaValue(item, 1);
	ReferData filepath, name; 
	tStrOp::getFilePath(filename.P, &filepath, &name); 

	item->newValue(name.P, name.L, true); 
	return 0;
}
int tExprCalc::functionPackNull(tExprCalc*calc, tExprItem*item, void* p){ //packnull(text)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	tExprItem* item1=getFuncParaItem(item, 1);

	item->newValue(item1->StringValue.L+1); 
	long j=0; 
	for (long i=0; i<item1->StringValue.L; i++){
		if (item1->StringValue.P[i] && item1->StringValue.P[i]!=-1 && item1->StringValue.P[i]!=-2) 
			item->StringValue.P[j++]=item1->StringValue.P[i];
	}
	item->StringValue.P[j]=0; 
	item->StringValue.L=j; 
	return 0;
}
int tExprCalc::functionUTF8Encode(tExprCalc*calc, tExprItem*item, void* p){ //utf8(text)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	char* source=calc->getFuncParaValue(item, 1); 

	if (source){
		char*dest=encodeUTF8(source);
		if (dest){
			item->newValue(dest, strlen(dest) ); 
			free(dest); 
		}
	}
	return 0;
}

int tExprCalc::functionHtql(tExprCalc*calc, tExprItem*item, void* p){ //htql(source, query, url)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	char* source=calc->getFuncParaValue(item, 1); 
	char* query=calc->getFuncParaValue(item, 2); 
	char* url=calc->getFuncParaValue(item, 3); 

	HtmlQL ql;
	if (source) ql.setSourceData(source, strlen(source), false);
	if (url && *url) ql.setSourceUrl(url, strlen(url) );
	if (item->Info>=4){
		for (int i=4; i<=item->Info; i+=2){
			char* var=calc->getFuncParaValue(item, i); 
			char* val=calc->getFuncParaValue(item, i+1); 
			ql.setGlobalVariable(var, val);
		}
	}
	if (query) ql.setQuery(query);
	char* result=ql.getValue(1);
	item->newValue(result, result?strlen(result):0);
	return 0;
}
int tExprCalc::functionGetNumber(tExprCalc*calc, tExprItem*item, void* p){ //get_number(str, index1)
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	char* str=calc->getFuncParaValue(item, 1);
	ReferData index1;
	index1=calc->getFuncParaValue(item, 2);

	ReferData value; 
	tStrOp::getNumber(str, index1.getLong(), &value);

	item->newDoubleValue(value.getDouble(), 20, 10);
	return 0;
}

int tExprCalc::functionMatchLocalScore(tExprCalc*calc, tExprItem*item, void* p){ //match_local_score(str1, str2, match_type)
	if (item->Info == 1){ //one parameter
		item->newValue(item->Left->Value, strlen(item->Left->Value), false);

	}else if (item->Info == 2) { //two parameter
		item->newValue(item->Left->Value, item->Left->Length, false);
		item->Value=item->Right->Value;
	}else if (item->Info == 3) { //three parameter
		HyperTagsAlignment align;
		if (!item->Right->StringValue.Cmp("WORD", 4, false)){
			align.setAlignType(HyperTagsAlignment::TYPE_WORD);
		}else{
			align.setAlignType(HyperTagsAlignment::TYPE_CHAR);
		}
		align.IsCaseSensitive=false;
		ReferData result;
		double cost=0;
		ReferData str2;
		str2.Set(item->Value, strlen(item->Value), false);
		align.alignHyperTagsText(&item->Left->StringValue, &str2, &result, &cost);

		item->newDoubleValue(cost, NumberDefLength, 5);
	}else return dbSYNTAXERR;

	item->Type = dbNUMBER;

	return 0;	
}
int tExprCalc::functionStringComp(tExprCalc*calc, tExprItem*item, void* p){ //string_comp(str1, str2, comp_type, option); compare two strings
	if (calc->getFuncParaTotal(item)!=item->Info) return 0;

	char* str1=calc->getFuncParaValue(item, 1);
	char* str2=calc->getFuncParaValue(item, 2);
	ReferData comp_type;
	comp_type=calc->getFuncParaValue(item, 3);
	ReferData option;
	option=calc->getFuncParaValue(item, 4);

	//for prefix
	if (!comp_type.Cmp("prefix_length", strlen("prefix_length"), false) || !comp_type.Cmp("prefix", strlen("prefix"), false)){
		long i=0; 
		int is_casesensitive=option.Cmp("FALSE",5,false) && option.Cmp("0",1,true); //default: not sensitive
		for (i=0; str1[i] && str2[i]; i++){
			if ( (is_casesensitive && str1[i]!=str2[i])
				|| (!is_casesensitive && toupper(str1[i])!=toupper(str2[i])) ) {
				break;
			}
		}
		if (!comp_type.Cmp("prefix_length", strlen("prefix_length"), false)){
			item->newDoubleValue(i, NumberDefLength, 0);
		}else{
			item->newValue(str1, i, true);
		}

	//for local matches
	}else if (!comp_type.Cmp("localstr_score", strlen("localstr_score"), false) 
		|| !comp_type.Cmp("localstr", strlen("localstr"), false)
		){
		HyperTagsAlignment align;
		align.setAlignType(HyperTagsAlignment::TYPE_CHAR);
		align.IsCaseSensitive=option.Cmp("FALSE",5,false) && option.Cmp("0",1,true);
		ReferData result;
		double cost=0;
		ReferData rstr1, rstr2;
		rstr1.Set(str1, str1?strlen(str1):0,false); 
		rstr2.Set(str2, str2?strlen(str2):0,false); 
		align.alignHyperTagsText(&rstr1, &rstr2, &result, &cost);

		if (!comp_type.Cmp("localstr_score", strlen("localstr_score"), false)){
			item->newDoubleValue(cost, NumberDefLength, 5);
		}else{
			HtmlQL ql; 
			ql.setSourceData(result.P, result.L, false); 
			ql.setQuery("<max_positions>.<position>:from_pos, source_pos"); 
			long from_pos=0, source_pos=0; 
			char* pos=ql.getValue(1); 
			if (pos && *pos) sscanf(pos, "%ld", &from_pos); 
			pos=ql.getValue(2); 
			if (pos && *pos) sscanf(pos, "%ld", &source_pos); 
			if (source_pos<from_pos) source_pos=from_pos; 

			item->newValue(str1+from_pos, source_pos-from_pos+1, true); 
		}

	//global match of two str
	}else if (!comp_type.Cmp("align_cost", strlen("align_cost"), false)
		|| !comp_type.Cmp("align_length", strlen("align_length"), false)
		|| !comp_type.Cmp("align_str1", strlen("align_str1"), false)
		|| !comp_type.Cmp("align_str2", strlen("align_str2"), false)
		){
		StrAlignment align; 
		align.IsCaseSensitive=option.Cmp("FALSE",5,false) && option.Cmp("0",1,true); 
		double cost=0;
		long result_len=0; 
		char* result_str1=0, *result_str2=0; 
		align.CompareStrings(str1, str2, &cost, &result_len, &result_str1, &result_str2); 

		if (!comp_type.Cmp("align_length", strlen("align_length"), false)){
			item->newDoubleValue(result_len, NumberDefLength, 0); 
		}else if (!comp_type.Cmp("align_str1", strlen("align_str1"), false)){
			item->newValue(result_str1, result_str1?strlen(result_str1):0, true); 
		}else if (!comp_type.Cmp("align_str2", strlen("align_str2"), false)){
			item->newValue(result_str2, result_str1?strlen(result_str2):0, true); 
		}else{
			item->newDoubleValue(cost, NumberDefLength, 5); 
		}

		if (result_str1) free(result_str1); 
		if (result_str2) free(result_str2); 
	}

	return 0;
}



