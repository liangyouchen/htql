#include "qlsyntax.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "referdata.h"
#include "stroper.h"


#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////////////////
//
//        class QLSyntax
//
////////////////////////////////////////////////////////////////////


QLSyntax::QLSyntax(){
	Sentence=0;
	StartLen=0;
	Start=0;
	StartPrecision=0;
	Type=synQL_UNKNOW;
	Next=0;
	NextLen=0;
	NextPrecision=0;
	NextType=synQL_UNKNOW;
}

QLSyntax::~QLSyntax(){
	reset();
}

void QLSyntax::reset(){
	//if (Sentence!=0) free(Sentence); // change to Data
	Data.reset();
	Sentence=0;
	StartLen=0;
	Start=0;
	StartPrecision=0;
	Type=synQL_UNKNOW;
	Next=0;
	NextLen=0;
	NextPrecision=0;
	NextType=synQL_UNKNOW;
}

int QLSyntax::setSentence(const char* S,long* Length, int copy){
	reset();

	unsigned int i;
	if (Length) i=*Length;
	else if (S) i=strlen(S);
	else i=0;

/*	if ((Sentence=(char *)malloc(sizeof(char)* (i+2) ))==0)return -1;
	strncpy(Sentence,S,i);
	Sentence[i]=0;
*/
	Data.Set((char*)S, i, copy);
	Sentence = Data.P;

#if (ERRORLOG > 2)
	Log::add(ERRORLOGFILE,i,Sentence,__LINE__);
#endif
	match(); 
	match();
	return 0;
}

int QLSyntax::match(const char* word, int case_sensitive){
	if (!word){
		if (!StartLen) {
			match(); 
			return true;
		}else 
			return false; 
	}
	if (StartLen!=(long) strlen(word)) return false; 
	if (tStrOp::strNcmp(Sentence+Start, (char*) word, StartLen, case_sensitive)) return false; 

	match(); 
	return true;
}

int QLSyntax::match(int ExpectType){
	if (Type!=ExpectType)
		return false;
	match();
	return true;
}
int QLSyntax::matchSyntax(QLSyntax* subparser){
	Next=Start+subparser->Start;
	NextLen=0; 
	int err=match(); 
	if (err>=0) err=match(); 
	//if (subparser->Start==0 && err>=0) err=match(); 
	return err;
}

int QLSyntax::match(){
	int PreviousType=Type;
	matchNext();

	Start=Next;
	StartLen=NextLen;
	StartPrecision=NextPrecision;
	Type=NextType;

	if (Type == synQL_END ) return Type;

	Next+=NextLen;
	NextPrecision=0;
	NextType=synQL_UNKNOW;
	NextLen=1;

	if (Next>=Data.L) {
		Next = Data.L;
		NextLen = 0;
		NextType = synQL_END;
		return Type;
	}
	long len;
	while (Next<Data.L){
		if (isSpace(Sentence[Next])) Next++;
		else if (isComment(Sentence+Next, &len)) Next+=len;
		else break;
	}

	findNext();

	return PreviousType;
}
void QLSyntax::matchNext(){
	return ;
}

void QLSyntax::findNext(){
	if (Next >= Data.L) {
		NextLen=0;
		NextType=synQL_END;
	}else{
		while (Next+NextLen<Data.L && !isSpace(Sentence[Next+NextLen])) 
			NextLen++;
		NextType=synQL_STRING;
	}
	return ;
}

int QLSyntax::KeyWord(){
	if (NextType!=synQL_WORD) return NextType;
	return synQL_WORD;
}
int QLSyntax::takeSyntaxString(int type, long start, long len, ReferData* result){
	result->Set(Sentence+start, len, true);
	/*
	if (type==synEXP_CHAR){
		result->Set(Sentence+start+1, len-2, true);
		if (result->L){
			tStrOp::dequoteString(result->P);
			result->L=strlen(result->P);
		}
	}else{
		result->Set(Sentence+start, len, true);
	}*/
	return 0;
}
int QLSyntax::takeStartSyntaxString(ReferData* result){
	return takeSyntaxString(Type, Start, StartLen, result);
}
int QLSyntax::takeNextSyntaxString(ReferData* result){
	return takeSyntaxString(NextType, Next, NextLen, result);
}

int QLSyntax::isSpace(char c){
	return (c==' '||c=='\t'||c=='\n' || c == '\r');
}

int QLSyntax::isComment(char*, long* len){
	return 0;
}

int QLSyntax::isAlpha(char c){
	return ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c=='_'));
}
int QLSyntax::isDigit(char c){
	return ((c>='0')&&(c<='9'));
}

int QLSyntax::copyFrom(QLSyntax* OtherSyntax){
	Type = OtherSyntax->Type;
	NextType = OtherSyntax->NextType;
	Start = OtherSyntax->Start;
	StartLen = OtherSyntax->StartLen;
	StartPrecision = OtherSyntax->StartPrecision;
	Next = OtherSyntax->Next;
	NextLen = OtherSyntax->NextLen;
	NextPrecision = OtherSyntax->NextPrecision;
	return true;
}

int QLSyntax::cmpNoCase(const char* str1, size_t len1, const char* str2, size_t len2){
	size_t i=0;
	int re=0;
	while (i<len1 && i<len2)
	{
		if ((unsigned char) toupper(str1[i]) < (unsigned char)toupper(str2[i]))
			return -1;
		if ((unsigned char) toupper(str1[i]) > (unsigned char)toupper(str2[i]))
			return 1;
		i++;
	}

	if (len1 < len2)
		return -1;
	if (len1 > len2)
		return 1;
	return 0;
}
