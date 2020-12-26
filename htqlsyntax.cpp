#include "htqlsyntax.h"
#include "stroper.h"
#include "expr.h"


#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



void HTQLSyntax::findNext(){
	if (Next>=Data.L){
		NextType = synQL_END;
		NextLen = 0;

	}else if (Sentence[Next] == '.'){
		NextType = synQL_DOT;

	}else if (Sentence[Next] == preEXP_PARA){
		NextType = synQL_PARA;

	}else if (Sentence[Next] == '<'){
		NextType = synQL_LTAG;

	}else if (Sentence[Next] == '>'){
		NextType = synQL_RTAG;

	}else if (Sentence[Next] == '/'){
		int isSpecial=false;
		char EnclosedChar=0;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '/' || EnclosedChar || isSpecial)) {
			if (Sentence[Next+NextLen] == '\\') isSpecial = !isSpecial;
			else isSpecial=false;
			if (EnclosedChar){
				if (Sentence[Next+NextLen] == EnclosedChar && !isSpecial){
					EnclosedChar = 0;
				}
			}else{
				if (Sentence[Next+NextLen] == '\'' || Sentence[Next+NextLen] == '"'){
					EnclosedChar = Sentence[Next+NextLen];
				}
			}
			NextLen++;
		}
		if (Next+NextLen < Data.L) NextLen++;
		NextType = synQL_SLASH;

	}else if (Sentence[Next] == '('){
		NextType = synQL_LBRACE;

	}else if (Sentence[Next] == ')'){
		NextType = synQL_RBRACE;

	}else if (Sentence[Next] == '{'){
		NextType = synQL_LBRACKET;

	}else if (Sentence[Next] == '}'){
		NextType = synQL_RBRACKET;

	}else if (Sentence[Next] == ':'){
		NextType = synQL_COLON;

	}else if (Sentence[Next] == '|'){
		NextType = synQL_MID;

	}else if (Sentence[Next] == ','){
		NextType = synQL_COMMA;

	}else if (Sentence[Next] == ';'){
		NextType = synQL_SEMICOLON;

	}else if (Sentence[Next] == '='){
		NextType = synQL_EQ;

	}else if (Sentence[Next] == '&'){
		NextType = synQL_REF;

	}else if (Sentence[Next] == '*'){
		NextType = synQL_STAR;
		while (Sentence[Next + NextLen] == '*') NextLen++;
	}else if (Sentence[Next] == '@'){
		NextType = synQL_AT;
		while (Sentence[Next + NextLen] == '@') NextLen++;

	}else if (Sentence[Next] == '~'){
		NextType = synQL_TILE;

	}else if (Sentence[Next] == '$'){
		NextType = synQL_DOLLAR;

	}else if (Sentence[Next] == '#'){
		NextType = synQL_PER;

	}else if (Sentence[Next] == '-'){
		NextType = synQL_DASH;

	}else if (Sentence[Next] == '\''){
		int isSpecial=false;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '\'' || isSpecial || (Sentence[Next+NextLen]=='\'' && Sentence[Next+NextLen+1]=='\'' ) )) {
			if (Sentence[Next+NextLen] == '\\') isSpecial = !isSpecial;
			else if (!isSpecial && Sentence[Next+NextLen] == '\'' && Sentence[Next+NextLen+1] == '\'') isSpecial = true;
			else isSpecial=false;
			NextLen++;
		}
		if (Next+NextLen < Data.L) 
			NextLen++;
		NextType = synQL_STRING;

	}else if (Sentence[Next] == '"'){
		int isSpecial=false;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '"' || isSpecial || (Sentence[Next+NextLen]=='"' && Sentence[Next+NextLen+1]=='"' ))) {
			if (Sentence[Next+NextLen] == '\\') isSpecial = !isSpecial;
			else if (!isSpecial && Sentence[Next+NextLen] == '"' && Sentence[Next+NextLen+1] == '"') isSpecial = true;
			else isSpecial=false;
			NextLen++;
		}
		if (Next+NextLen < Data.L) 
			NextLen++;
		NextType = synQL_STRING;

	}else if (isAlpha(Sentence[Next]) 
		){
		while (Next+NextLen < Data.L && TagOperation::isTagNameChar(Sentence[Next+NextLen])){ //isAlpha(Sentence[Next])||isDigit(Sentence[Next+NextLen])
			NextLen++;
		}
		NextType=synQL_WORD;

	}else if (isDigit(Sentence[Next])){
		while (Next+NextLen < Data.L && isDigit(Sentence[Next+NextLen])) NextLen++;
		NextType=synQL_NUMBER;
	}else{
		NextType=synQL_UNKNOW;
		NextLen = 0;
	}
}

void HTQLTagSelSyntax::findNext(){
	if (Next>=Data.L){
		NextType = synQL_END;
		NextLen = 0;

	}else if (Sentence[Next] == '<' || Sentence[Next] == '['){
		NextType = synQL_LTAG;

	}else if (Sentence[Next] == '>' || Sentence[Next] == ']'){
		NextType = synQL_RTAG;

	}else if (Sentence[Next] == '/'){
		NextType = synQL_SLASH;

	}else if (Sentence[Next] == '('){
		NextType = synQL_LBRACE;

	}else if (Sentence[Next] == ')'){
		NextType = synQL_RBRACE;

	}else if (Sentence[Next] == ':'){
		NextType = synQL_COLON;

	}else if (Sentence[Next] == ','){
		NextType = synQL_COMMA;

	}else if (Sentence[Next] == '=' && Next+1 < Data.L && Sentence[Next+1] == '~'){
		NextType = synQL_MAPPING;
		NextLen=2;

	}else if (Sentence[Next] == '~'){
		NextType = synQL_TILE;

	}else if (Sentence[Next] == '$'){
		NextType = synQL_DOLLAR;

	}else if (Sentence[Next] == '#'){
		NextType = synQL_PER;

	}else if (Sentence[Next] == '-'){
		NextType = synQL_DASH;

	}else if (Sentence[Next] == '*'){
		NextType = synQL_STAR;

	}else if (Sentence[Next] == '@'){
		NextType = synQL_AT;

	}else if (Sentence[Next] == '\''){
		int isSpecial=false;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '\'' || isSpecial || (Sentence[Next+NextLen]=='\'' && Sentence[Next+NextLen+1]=='\'' ) )) {
			if (Sentence[Next+NextLen] == '\\') isSpecial = !isSpecial;
			else if (!isSpecial && Sentence[Next+NextLen] == '\'' && Sentence[Next+NextLen+1] == '\'') isSpecial = true;
			else isSpecial=false;
			NextLen++;
		}
		if (Next+NextLen < Data.L) 
			NextLen++;
		NextType = synQL_STRING;

	}else if (Sentence[Next] == '"'){
		int isSpecial=false;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '"' || isSpecial|| (Sentence[Next+NextLen]=='"' && Sentence[Next+NextLen+1]=='"' ) )) {
			if (Sentence[Next+NextLen] == '\\') isSpecial = !isSpecial;
			else if (!isSpecial && Sentence[Next+NextLen] == '"' && Sentence[Next+NextLen+1] == '"') isSpecial = true;
			else isSpecial=false;
			NextLen++;
		}
		if (Next+NextLen < Data.L) 
			NextLen++;
		NextType = synQL_STRING;

	}else if (isAlpha(Sentence[Next]) 
		){
#ifdef CASEINSENSITIVE
		Sentence[Next]=toupper(Sentence[Next]);
#endif
		while (Next+NextLen < Data.L && TagOperation::isTagNameChar(Sentence[Next+NextLen])){//isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen])){
#ifdef CASEINSENSITIVE
			Sentence[Next+NextLen]=toupper(Sentence[Next+NextLen]);
#endif
			NextLen++;
		}
		NextType=synQL_WORD;

	}else if (isDigit(Sentence[Next])){
		while (Next+NextLen < Data.L && isDigit(Sentence[Next+NextLen])) NextLen++;
		NextType=synQL_NUMBER;
	}else{
		NextType=synQL_UNKNOW;
		NextLen = 0;
	}
}

int HTQLTagSelSyntax::KeyWord(){
	if (NextType!=synQL_WORD) return NextType;
	if ((NextLen==4)&&!tStrOp::strNcmp(Sentence+Next,"LIKE",NextLen, false)) return synQL_LIKE;
	if ((NextLen==2)&&!tStrOp::strNcmp(Sentence+Next,"EQ",NextLen, false)) return synQL_EQ;
	if ((NextLen==2)&&!tStrOp::strNcmp(Sentence+Next,"NE",NextLen, false)) return synQL_NE;
	if ((NextLen==2)&&!tStrOp::strNcmp(Sentence+Next,"GT",NextLen, false)) return synQL_GT;
	if ((NextLen==2)&&!tStrOp::strNcmp(Sentence+Next,"LT",NextLen, false)) return synQL_LT;
	if ((NextLen==2)&&!tStrOp::strNcmp(Sentence+Next,"GE",NextLen, false)) return synQL_GE;
	if ((NextLen==2)&&!tStrOp::strNcmp(Sentence+Next,"LE",NextLen, false)) return synQL_LE;
	return synQL_WORD;
}


HTQLTagDataSyntax::HTQLTagDataSyntax(){
	XMLTagType=0;
	NextXMLTagType=0;
	IsXML=0;
}

HTQLTagDataSyntax::~HTQLTagDataSyntax(){
	XMLTagType=0;
	NextXMLTagType=0;
	IsXML=0;
	QLSyntax::reset();
}

int HTQLTagDataSyntax::isSpace(char){
	return false;
}

void HTQLTagDataSyntax::matchNext(){
	XMLTagType = NextXMLTagType;
	return;
}

void HTQLTagDataSyntax::findNext(){
	if (Next>=Data.L||!Sentence[Next]){
		NextType = synQL_END;
		NextLen = 0;
		NextXMLTagType=synQL_XML_DATA;

	}else if (Sentence[Next] == '<' && !ToFindEnclosedTag.L){
		NextXMLTagType=synQL_XML_DATA;
		if (Next+1 < Data.L && Sentence[Next+1] == '/' ){
			NextType = synQL_END_TAG;
			NextLen=2;
		}else if (Next+3<Data.L && !strncmp(Sentence+Next+1, "!--", 3)){
			NextType = synQL_COMMENT;
			char*p = strstr(Sentence+Next+4, "-->");
			if (p){
				NextLen=p-Sentence-Next+3;
				if (NextLen > Data.L-Next){
					NextLen = Data.L-Next;
				}
			}else{
				NextLen = Data.L - Next;
			}
			NextType=synQL_COMMENT;
			return;
		}else if (Next+2<Data.L && Sentence[Next+1] == '!' && isalpha(Sentence[Next+2]) ){
			//need to be improved ...
			NextType=synQL_OTHERS;
			NextXMLTagType=synQL_XML_DTD_TAG;
			NextLen=2;
		}else if (Next+2<Data.L && Sentence[Next+1] == '!' && !tStrOp::strNcmp(Sentence+Next+2, "[CDATA[", 7, false) ){
			NextType=synQL_DATA; 
			char*p = strstr(Sentence+Next+9, "]]>");
			if (p){
				NextLen=p-Sentence-Next+3;
				if (NextLen > Data.L-Next){
					NextLen = Data.L-Next;
				}
			}else{
				NextLen = Data.L - Next;
			}
			return; 
		}else if (Next+1<Data.L && Sentence[Next+1]=='?'){
			//need to be improved ...
			NextType=synQL_OTHERS;
			NextXMLTagType=synQL_XML_XID_TAG;
			NextLen=2;
			if (!tStrOp::strNcmp(Sentence+Next+NextLen, "XML", 3, false)){
				IsXML=true;
			}
		}else{
			NextType = synQL_START_TAG;
			NextLen=1;
		}
		int isSpecial=false;
		char lastNonSpaceChar=0;
		char EnclosedChar=0;
		int canbeSINGLE_TAG=true;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '>' || EnclosedChar)) {
			if (EnclosedChar){//in the string
				if ((Sentence[Next+NextLen] == EnclosedChar && !isSpecial) || 
					(EnclosedChar=='=' && 
						(tStrOp::isSpace(Sentence[Next+NextLen])||Sentence[Next+NextLen]=='>') //||Sentence[Next+NextLen]=='\''||Sentence[Next+NextLen]=='"') 
					) 
					){
					EnclosedChar = 0;
					lastNonSpaceChar=0;
					if (Sentence[Next+NextLen]=='>') {
						canbeSINGLE_TAG=false;
						break;
					}
				}
				if (Sentence[Next+NextLen] == '\\') isSpecial = !isSpecial;
				else isSpecial=false;
			}else{//in the tag, not in string
				if (Sentence[Next+NextLen] == '\'' || Sentence[Next+NextLen] == '"'){
					if (Next+NextLen>0 && Sentence[Next+NextLen-1]!='\'' && Sentence[Next+NextLen-1]!='"') //on in case somebody have a typo of double "", the last " will be ignore
						EnclosedChar = Sentence[Next+NextLen];
				}else if (lastNonSpaceChar=='=' && !tStrOp::isSpace(Sentence[Next+NextLen]) 
					&& Sentence[Next+NextLen]!='>'){
					EnclosedChar = '='; //special enclose char
					lastNonSpaceChar=0;
				}else if (!tStrOp::isSpace(Sentence[Next+NextLen])){
					lastNonSpaceChar=Sentence[Next+NextLen];
				}
			}
			NextLen++;
		}
		if (Next+NextLen < Data.L){
			NextLen++;
			if (NextType==synQL_END_TAG && NextLen==3){
				NextXMLTagType = synQL_XML_NULL_TAG;
			}else if (NextType == synQL_START_TAG && Sentence[Next+NextLen-2]=='/' && canbeSINGLE_TAG){
				NextXMLTagType = synQL_XML_SINGLE_TAG;
			}

			if (NextType == synQL_START_TAG){
				if (TagOperation::isTag(Sentence+Next, "Script") && (NextXMLTagType != synQL_XML_SINGLE_TAG)) 
					ToFindEnclosedTag="Script";
				//else if (TagOperation::isTag(Sentence+Next, "Pre") ) 
				//	ToFindEnclosedTag="Pre";
			}
		}else{
			NextType = synQL_DATA;
		}

	}else {
		NextLen=0;
		NextType=synQL_DATA;
		NextXMLTagType=synQL_XML_DATA;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '<' || ( ToFindEnclosedTag.L && !TagOperation::isEndTag(Sentence+Next+NextLen, ToFindEnclosedTag.P) )) ) {
			NextLen++;
		}
		if (Next+NextLen < Data.L && Sentence[Next+NextLen] == '<' && ToFindEnclosedTag.L && TagOperation::isEndTag(Sentence+Next+NextLen, ToFindEnclosedTag.P) ){
			ToFindEnclosedTag.reset();
		}
	}
}


void HTQLAttrDataSyntax::findNext(){
	if (Next>=Data.L){
		NextType = synQL_END;
		NextLen = 0;

	}else if (Sentence[Next] == '<'){
		NextType = QLSyntax::synQL_LTAG;
	}else if (Sentence[Next] == '>'){
		NextType = QLSyntax::synQL_RTAG;
	}else if (Sentence[Next] == '='){
		NextType = QLSyntax::synQL_EQ;
	}else if (Sentence[Next] == '\'' || Sentence[Next] == '"'){
		int isSpecial=false;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != Sentence[Next] || isSpecial)) {
			if (Sentence[Next+NextLen] == '\\') isSpecial = !isSpecial;
			else isSpecial=false;
			NextLen++;
		}
		if (Next+NextLen < Data.L) 
			NextLen++;
		NextType = synQL_STRING;
	}else if (Type==QLSyntax::synQL_EQ && Next<Data.L && Sentence[Next] != ' ' 
		&& Sentence[Next+NextLen] != '\t' && Sentence[Next] != '\r' && Sentence[Next] != '\n'){
		while (Next+NextLen<Data.L && Sentence[Next+NextLen] != ' ' && Sentence[Next+NextLen] != '>'
			&& Sentence[Next+NextLen] != '\t' && Sentence[Next+NextLen] != '\r' && Sentence[Next+NextLen] != '\n')
			NextLen++;
		NextType = synQL_STRING;
	}else if (isAlpha(Sentence[Next]) || isDigit(Sentence[Next]) 
		){
		while (Next+NextLen < Data.L && TagOperation::isTagNameChar(Sentence[Next+NextLen])){ //(isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen])|| Sentence[Next+NextLen]==':' || Sentence[Next+NextLen]=='-')){
			NextLen++;
		}
		NextType=synQL_WORD;

	}else{
		NextType=synQL_UNKNOW;
		NextLen = 0;
	}
}
