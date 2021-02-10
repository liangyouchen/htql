#include "iwsqlsyntax.h"
#include <string.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



void IWSqlSyntax::findNext(){
	unsigned int i;
	if (Sentence[Next]=='#' && (isDigit(Sentence[Next+NextLen])|| Sentence[Next+NextLen]=='#')){
		while (Sentence[Next+NextLen] && Sentence[Next+NextLen]!='#'){
			NextLen++;
		}
		if (Sentence[Next+NextLen]=='#')
			NextLen++;
		NextType = synQL_DATE;
	}else if (isAlpha(Sentence[Next]) ){
#ifdef CASEINSENSITIVE
		Sentence[Next]=toupper(Sentence[Next]);
#endif
		while (isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen])|| Sentence[Next+NextLen]=='#'){
#ifdef CASEINSENSITIVE
			Sentence[Next+NextLen]=toupper(Sentence[Next+NextLen]);
#endif
			NextLen++;
		}
		NextType=synQL_WORD;
		NextType=KeyWord();
	}else if (isDigit(Sentence[Next])){
		while (isDigit(Sentence[Next+NextLen])) NextLen++;
		NextType=synQL_NUMBER;
	}else{
		switch (Sentence[Next]){
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
				NextType=synQL_STRING;
			}else NextType=synQL_UNKNOW;
			break;
		case '[':
			while (Sentence[Next+NextLen] && Sentence[Next+NextLen]!=']'){
				NextLen++;
			}
			if (Sentence[Next+NextLen]==']')
				NextLen++;
			NextType = synQL_WORD;
			break;
		case '"':
			while (1){
				if (Sentence[Next+NextLen]!='"' && Sentence[Next+NextLen]!='\0') NextLen++;
				else if (Sentence[Next+NextLen]=='"' && Sentence[Next+NextLen-1]=='\\'){
					for (i=Next+NextLen-1; Sentence[i]!='\0'; i++) Sentence[i]=Sentence[i+1];
					//NextLen++;
				}
				else break;
			}
			if (Sentence[Next+NextLen]=='"') {
				NextLen++;
				NextType=synQL_WORD;
			}else NextType=synQL_UNKNOW;
			break;
		case '!':
			switch (Sentence[Next+NextLen]){
			case '=':
				NextLen++;
				NextType=synQL_NE;
				break;
			default:
				NextType=synQL_NOT;
			}
			break;
		case '<':
			switch (Sentence[Next+NextLen]){
			case '>':
				NextLen++;
				NextType=synQL_NE;
				break;
			case '=':
				NextLen++;
				NextType=synQL_LE;
				break;
			default:
				NextType=synQL_LT;
			}
			break;
		case '>':
			if (Sentence[Next+NextLen]=='='){
				NextLen++;
				NextType=synQL_GE;
			}else NextType=synQL_GT;
			break;
		case '=':
			if (Sentence[Next+NextLen]=='=') NextLen++;
			NextType=synQL_EQ;
			break;
		case '.':
			if (isDigit(Sentence[Next+NextLen])){
				NextLen++;
				NextPrecision++;
				while (isDigit(Sentence[Next+NextLen])) NextLen++;
				NextType=synQL_NUMBER;
			}else{
				NextType=synQL_DOT;
			}
			break;
		case '+':
			NextType=synQL_ADD;
			break;
		case '-':
			if (Sentence[Next+1]=='>'){
				NextType=synQL_REF;
				NextLen=2;
			}else{
				NextType=synQL_DASH;
			}
			break;
			
		case '(':
			NextType=synQL_LBRACE;
			break;
		case ')':
			NextType=synQL_RBRACE;
			break;
		case '{':
			NextType=synQL_LBRACKET;
			break;
		case '}':
			NextType=synQL_RBRACKET;
			break;
		case ',':
			NextType=synQL_COMMA;
			break;
		case '/':
			NextType=synQL_SLASH;
			break;
		case '*':
			NextType=synQL_STAR;
			break;
		case ';':
			NextType=synQL_SEMICOLON;
			break;
		case ':':
			NextType=synQL_COLON;
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

int IWSqlSyntax::KeyWord(){
	if (NextType!=synQL_WORD) return NextType;

	if ((NextLen==5)&& !cmpNoCase(Sentence+Next, NextLen, "TABLE", 5)) 
		return synSQL_TABLE;
	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "SELECT", 6)) 
		return synSQL_SELECT;
	if ((NextLen==4)&& !cmpNoCase(Sentence+Next, NextLen, "FROM", 4)) 
		return synSQL_FROM;
	if ((NextLen==2)&& !cmpNoCase(Sentence+Next, NextLen, "AS", 2)) 
		return synSQL_AS;
	if ((NextLen==5)&& !cmpNoCase(Sentence+Next, NextLen, "WHERE", 5)) 
		return synSQL_WHERE;
	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "UPDATE", 6)) 
		return synSQL_UPDATE;
	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "INSERT", 6)) 
		return synSQL_INSERT;
	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "DELETE", 6)) 
		return synSQL_DELETE;
	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "CREATE", 6)) 
		return synSQL_CREATE;
	if ((NextLen==5)&& !cmpNoCase(Sentence+Next, NextLen, "GROUP", 5)) 
		return synSQL_GROUP;
	if ((NextLen==5)&& !cmpNoCase(Sentence+Next, NextLen, "ORDER", 5)) 
		return synSQL_ORDER;
	if ((NextLen==2)&& !cmpNoCase(Sentence+Next, NextLen, "BY", 2)) 
		return synSQL_BY;
	if ((NextLen==3)&& !cmpNoCase(Sentence+Next, NextLen, "SET", 3)) 
		return synSQL_SET;
	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "VALUES", 6)) 
		return synSQL_VALUES;
	if ((NextLen==4)&& !cmpNoCase(Sentence+Next, NextLen, "INTO", 4)) 
		return synSQL_INTO;
	if ((NextLen==7)&& !cmpNoCase(Sentence+Next, NextLen, "GROUPBY", 7)) 
		return synSQL_GROUPBY;
	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "HAVING", 6)) 
		return synSQL_HAVING;
	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "UNIQUE", 6)) 
		return synSQL_UNIQUE;
	if ((NextLen==3)&& !cmpNoCase(Sentence+Next, NextLen, "ASC", 3)) 
		return synSQL_ASC;
	if ((NextLen==4)&& !cmpNoCase(Sentence+Next, NextLen, "DESC", 4)) 
		return synSQL_DESC;

	if ((NextLen==3)&& !cmpNoCase(Sentence+Next, NextLen, "AND", 3)) 
		return synSQL_AND;
	if ((NextLen==2)&& !cmpNoCase(Sentence+Next, NextLen, "OR", 2)) 
		return synSQL_OR;
	if ((NextLen==3)&& !cmpNoCase(Sentence+Next, NextLen, "NOT", 3)) 
		return synSQL_NOT;
	if ((NextLen==2)&& !cmpNoCase(Sentence+Next, NextLen, "IN", 2)) 
		return synSQL_IN;

	if ((NextLen==7)&& !cmpNoCase(Sentence+Next, NextLen, "MATCHES", 7)) 
		return synSQL_MATCHES;
	if ((NextLen==4)&& !cmpNoCase(Sentence+Next, NextLen, "LIKE", 4)) 
		return synSQL_LIKE;
	if ((NextLen==2)&& !cmpNoCase(Sentence+Next, NextLen, "IS", 2)) 
		return synSQL_IS;
	if ((NextLen==4)&& !cmpNoCase(Sentence+Next, NextLen, "NULL", 4)) 
		return synSQL_NULL;

	if ((NextLen==6)&& !cmpNoCase(Sentence+Next, NextLen, "NUMBER", 6)) 
		return synSQL_NUMBER;
	if ((NextLen==7)&& !cmpNoCase(Sentence+Next, NextLen, "INTEGER", 7)) 
		return synSQL_INTEGER;
	if ((NextLen==4)&& !cmpNoCase(Sentence+Next, NextLen, "CHAR", 4)) 
		return synSQL_CHAR;
	if ((NextLen==4)&& !cmpNoCase(Sentence+Next, NextLen, "LONG", 4)) 
		return synSQL_LONG;
	if ((NextLen==4)&& !cmpNoCase(Sentence+Next, NextLen, "DATE", 4)) 
		return synSQL_DATE;

	return synQL_WORD;

}
