#include "RegExParser.h"

#include "referlink.h"
#include "stroper.h"
#include "expr.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#define DEBUG_THIS_FILE
#endif


int RegExTokenBTree::cmp(char* p1, char* p2){
	RegExToken* t1=(RegExToken*) p1; 
	RegExToken* t2=(RegExToken*) p2;
	while (t1 && !t1->Token.P) t1=t1->Next;
	while (t2 && !t2->Token.P) t2=t2->Next;
	int i=0;
	while (t1 && t2 && t1->Token.P && t2->Token.P){
		if (Type==typeTOKEN_TOKEN) {
			i=RegParser->matchedRegTokenText(t1, t2);
		}else if (Type==typeTOKEN_STRING){
			for (int j=0; j<=t1->Token.L && j<=t2->Token.L; j++){
				i=RegParser->matchCmpChar(t1->Token.P[j], t2->Token.P[j]); 
				if (i) break;
			}
		}
		//i=RegParser->matchCmpChar(t1->Token.P[0], t2->Token.P[0]); //change to token later
		if (i) return i;
		t1=t1->Next;
		t2=t2->Next;
	}
	if (!(t1 && t1->Token.P && t1->Token.P[0]) && t2 && t2->Token.P && t2->Token.P[0]) return -1;
	else if (t1 && t1->Token.P && t1->Token.P[0] && !(t2 && t2->Token.P && t2->Token.P[0])) return 1;
	return 0;
}
int RegExTokenBTree::matchTarget(RegExToken* target, RegExToken* p2, RegExToken** p2_end){
	//target: regular expression 
	//p2: text
	RegExToken* t1=(RegExToken*) target; 
	RegExToken* t2=(RegExToken*) p2;
	while (t1 && !t1->Token.P) t1=t1->Next;
	while (t2 && !t2->Token.P) t2=t2->Next;
	if (p2_end) *p2_end=0; //t2;
	if (!t1) {
		return (!t2 || !t2->Token.P);
	}
	int i=0;
	while (t1 && t2 && t1->Token.P && t2->Token.P){
		if (Type==typeTOKEN_TOKEN) {
			i=RegParser->matchedRegTokenText(t2, t1);
		}else if (Type==typeTOKEN_STRING){
			for (int j=0; j<=t1->Token.L && j<=t2->Token.L; j++){
				i=RegParser->matchCmpChar(t1->Token.P[j], t2->Token.P[j]); 
				if (i) break;
			}
		}
		//i=RegParser->matchCmpChar(t1->Token.P[0], t2->Token.P[0]); //change to token later
		if (i) break;
		if (p2_end) *p2_end=t2;
		t1=t1->Next;
		t2=t2->Next;
	}
	if (!(t1 && t1->Token.P && t1->Token.P[0]) )
		return true;
	else
		return false;
}

RegExTokenBTree::RegExTokenBTree(){
	RegParser=0;
	Type=typeTOKEN_TOKEN;
}
RegExTokenBTree::~RegExTokenBTree(){
	reset();
}
void RegExTokenBTree::reset(){
	BTree::reset();
	RegParser=0;
	Type=typeTOKEN_TOKEN;
}

RegExToken::RegExToken(){
	Next=Prev=SiblNext=SiblPrev=Child=Parent=ExtendNext=0;
	Index=-1;

	TokenType=0;
	IsNegativeToken=0;
	RepeatNFrom=RepeatNTo=1;

	MatchedToList=0;
	MatchedFrom=MatchedTo=MatchedSibl=0;
	MatchedRepeatN=0;

	MatchType=matchDEFAULT;
	MatchStringSet=0;

	SubToken=0;
}

RegExToken::~RegExToken(){
	reset();
}

void RegExToken::reset(){
	if (Next){
		RegExToken* next=Next;
		RegExToken* nextnext=0; 
		while (next){
			nextnext=next->Next; 
			next->Next=0; 
			next->Prev=0; 
			delete next; 
			next=nextnext;
		}
		Next=0;
	}

	Prev=0; //didnot delete Prev

	if (SiblNext){
		RegExToken* next=SiblNext;
		RegExToken* nextnext=0; 
		while (next){
			nextnext=next->SiblNext; 
			next->SiblNext=0; 
			next->SiblPrev=0; 
			delete next; 
			next=nextnext;
		}
		SiblNext=0;
	}

	SiblPrev=0;

	if (ExtendNext){
		RegExToken* next=ExtendNext;
		RegExToken* nextnext=0; 
		while (next){
			nextnext=next->ExtendNext; 
			next->ExtendNext=0; 
			delete next; 
			next=nextnext;
		}
		ExtendNext=0;
	}

	if (Child){
		delete Child; 
		Child=0; 
	}

	Parent=0; 
	Index=-1;

	TokenType=0;
	IsNegativeToken=0;
	RepeatNFrom=RepeatNTo=1;

	clearMatched();

	//NextTokens.reset();

	if (MatchStringSet){
		delete MatchStringSet;
		MatchStringSet=0;
	}

	if (SubToken){
		delete SubToken; 
		SubToken=0;
	}

	ReferLink2* link2=0; 
	for (link2=(ReferLink2*) OpOptions.Next; link2 && link2!=&OpOptions; link2=(ReferLink2*)link2->Next){
		if (link2->Data==RegExToken::opEXPR){
			if (link2->Value.P){
				delete (tExprCalc*) link2->Value.P; 
				link2->Value.P=0; 
			}
		}
	}
	OpOptions.reset();

	for (link2=(ReferLink2*) Conditions.Next; link2 && link2!=&Conditions; link2=(ReferLink2*)link2->Next){
		if (link2->Value.P){
			delete (tExprCalc*) link2->Value.P; 
			link2->Value.P=0; 
		}
	}
	Conditions.reset(); 

}
int RegExToken::clearMatched(){
	if (MatchedToList) {
		delete MatchedToList; 
		MatchedToList=0;
	}

	MatchedFrom=MatchedTo=MatchedSibl=0;
	MatchedRepeatN=0;
	return 0;
}
int RegExToken::clearAllMatched(){ //also clearMatched for all next, child, sibling, and ExtendNext links
	clearMatched();
	if (!MatchType){
		if (Next) Next->clearAllMatched();
		if (SiblNext) SiblNext->clearAllMatched();
		if (Child) Child->clearAllMatched();
		if (ExtendNext) ExtendNext->clearAllMatched();
	}
	return 0;
}
RegExToken* RegExToken::addNextToken(RegExToken* token){
	if (!token) return token; 

	RegExToken* next=Next; 
	Next=token; 
	token->Prev=this; // this.Next <--> token .. token_next <--> next.Prev
	token->Index = this->Index + 1; 

	RegExToken* token_next=token; //two directional, noncyclic chain
	while (token_next->Next) token_next=token_next->Next; 
	token_next->Next=next; 
	if (next) next->Prev=token_next; 

	return token;
}
RegExToken* RegExToken::addSiblToken(RegExToken* token){
	RegExToken* next=SiblNext; 
	SiblNext=token; 
	token->SiblPrev=this; // this.SyblNext <--> token .. token_next <--> next.SyblPrev

	token->TokenText.Set(TokenText.P, TokenText.L, false);

	RegExToken* token_next=token; //two directional, noncyclic chain
	while (token_next->SiblNext) token_next=token_next->SiblNext; 
	token_next->SiblNext=next; 
	if (next) next->SiblPrev=token_next; 
	return token;
}
RegExToken* RegExToken::addChildToken(RegExToken* token){
	for (RegExToken* child=token; child; child=child->SiblNext){
		child->setParentToken(this);
	} //this <-- token.Parent .. child.Parent

	if (Child) token->addSiblToken(Child);	//this.Child --> token.SiblNext <--> Child.SyblPrev
	token->TokenText.Set(TokenText.P, TokenText.L, false);
	Child=token; 
	return token;
}
RegExToken* RegExToken::addExtendedToken(RegExToken* token){
	token->ExtendNext=ExtendNext; //this.ExtendNext --> token --> ExtendNext
	ExtendNext=token;
	return token;
}
RegExToken* RegExToken::setParentToken(RegExToken* token){
	Parent=token; 
	return token;
}

int RegExToken::switchNextToSiblNext(){ //move Next to SiblNext
	if (!Next) return 0; 
	if (SiblNext){ // Next.SiblNext <--> this.SiblNext.SiblPrev
		Next->SiblNext=SiblNext;
		SiblNext->SiblPrev=Next;
	}
	SiblNext=Next; //first.SiblNext <-->first.Next.SiblPrev
	Next=0;
	SiblNext->SiblPrev=this;
	SiblNext->Prev=0;
	return 0;
}
int RegExToken::isMatchedInRange(long repeat){
	return (RepeatNFrom<0 || repeat>=RepeatNFrom) && (RepeatNTo<0 || repeat<=RepeatNTo);
}

int RegExToken::setupMatchStringTokenSet(RegExParser* parser){
	//set up MatchStringSet to expedite string matching
	switchNextToSiblNext();
	if (!MatchType){
		MatchType = matchTOKEN_SET;
	}
	if (!MatchStringSet){
		MatchStringSet = new RegExTokenBTree; 
	}else{
		MatchStringSet->empty();
	}
	MatchStringSet->RegParser=parser;
	MatchStringSet->NoDuplicate = true;
	if (MatchType==matchWORD_SET || MatchType==matchSTRING_SET){
		MatchStringSet->Type=RegExTokenBTree::typeTOKEN_STRING;
	}
	// add all strings to set
	ReferData str; 
	for (RegExToken* token=SiblNext; token; token=token->SiblNext){
		RegExToken* token1=token; 
		if (token->Child) token1=token->Child->Next; 
		if (token1){
			MatchStringSet->insert((char*) token1); 
		}
	}
	return 0;
}
int RegExToken::setupMatchStringStrSet(RegExParser* parser, ReferLinkHeap* strset, int matchtype){
	//put strings in strset to SiblNext
	int err=0;
	RegExToken* token=this; 
	ReferLink* link; 
	MatchType=RegExToken::matchTOKEN_SET;
	if (matchtype) MatchType=matchtype; 
	for (link=strset->getReferLinkHead(); link; link=link->Next){
		token=token->addSiblToken(new RegExToken);
		if (MatchType==RegExToken::matchTOKEN_SET){
			RegExSyntax syntax; 
			syntax.setSentence(link->Name.P, &link->Name.L, false);
			err=parser->parseRegEx(&syntax, token);
			//err=parser->tokenizeText(link->Name.P, token); 
		}else if (MatchType==RegExToken::matchWORD_SET){
			ReferLinkHeap words; 
			words.setSortOrder(SORT_ORDER_NUM_INC); 
			//tStrOp::splitString(link->Name.P, " ", &words); 
			tStrOp::splitUTF8(link->Name.P, &words); 
			RegExToken* token1=token;
			for (ReferLink* link1=(ReferLink*) words.moveFirst(); link1; link1=(ReferLink*) words.moveNext()){
				token1=token1->addNextToken(new RegExToken);
				token1->Token.Set(link1->Name.P, link1->Name.L, true); 
				token1->TokenText.Set(link1->Name.P, link1->Name.L, false); 
				token1->TokenType=RegExSyntax::synRegQL_TAG;
				//createRegExSubToken(token1);
			}
		}else if (MatchType==RegExToken::matchSTRING_SET){
			token->Token.Set(link->Name.P, link->Name.L, true); 
			token->TokenText.Set(link->Name.P, link->Name.L, false); 
			token->TokenType=RegExSyntax::synRegQL_TAG;
			//createRegExSubToken(token);
		}
	}
	err=setupMatchStringTokenSet(parser); 
	return err;
}
int RegExToken::pushMatchedToList(const char* info){
	RegExToken* matched=new RegExToken; 
	matched->MatchedFrom=MatchedFrom; 
	matched->MatchedTo=MatchedTo; 
	matched->MatchedSibl=MatchedSibl;
	matched->MatchedRepeatN=MatchedRepeatN;
	if (matched->MatchedTo) 
		matched->Index=matched->MatchedTo->Index;

	RegExToken** addto = &MatchedToList; 
	while (*addto && (*addto)->Index>=0 && (*addto)->Index > MatchedTo->Index){
		addto=&(*addto)->Next; 
	}
	matched->addNextToken(*addto);
	*addto=matched;
#ifdef DEBUG_THIS_FILE
	ReferData debug_str(512);
	getMatchedToListInfo(&debug_str, "++pushMatchedToList++", info);
	debug_str.appendFile("$DEBUG_RegExParser.txt");
#endif
	return 0;
}
int RegExToken::takeMatchedToList(RegExToken* from){
	if (from && from->MatchedToList){
		RegExToken** matched=&from->MatchedToList;
		for (; *matched; matched=&(*matched)->Next){
			(*matched)->MatchedFrom = this->MatchedFrom; 
			(*matched)->MatchedRepeatN = this->MatchedRepeatN;
		}
		RegExToken** pos=&this->MatchedToList;
		if (*pos){ //keep the first item (that was just popped here), added to its next
			pos=&(*pos)->Next; 
		}
		(*matched)=*pos;
		*pos=from->MatchedToList;
		from->MatchedToList=0;
		return 1;
	}
	return 0;
}

int RegExToken::popMatchedToList(const char* info){
#ifdef DEBUG_THIS_FILE
	ReferData debug_str(512);
	getMatchedToListInfo(&debug_str, "--popMatchedToList--", info);
	debug_str.appendFile("$DEBUG_RegExParser.txt");
#endif
	if (!MatchedToList) return -1; 
	MatchedFrom = MatchedToList->MatchedFrom;
	MatchedTo = MatchedToList->MatchedTo;
	MatchedSibl = MatchedToList->MatchedSibl;
	MatchedRepeatN = MatchedToList->MatchedRepeatN; 
	RegExToken* matched = MatchedToList; 
	MatchedToList=MatchedToList->Next; 
	matched->Next=0; 
	delete matched; 
	return 0;
}
int RegExToken::deleteMatchedToList(const char* info){
#ifdef DEBUG_THIS_FILE
	ReferData debug_str(512);
	getMatchedToListInfo(&debug_str, "==deleteMatchedToList==", info);
	debug_str.appendFile("$DEBUG_RegExParser.txt");
#endif
	if (MatchedToList){ //delete the matching sequence
		delete MatchedToList; 
		MatchedToList=0;
	}
	return 0;
}

/*
int RegExToken::takeMatchedToList(RegExToken* from){
	RegExToken* matched = from->MatchedToList;
	if (matched){
		while (matched->Next) matched=matched->Next; 
		matched->Next=MatchedToList; 
		MatchedToList=from->MatchedToList;
		from->MatchedToList=0;
	}
	return 0;
}*/

int RegExToken::getMatchedToListInfo(ReferData* info, const char* title, const char* title1){
	char buf[128];
	*info+="=================\r\n";
	if (title) *info += title;
	if (title1) *info += title1; 
	if (title || title1) *info+="\r\n"; 
	sprintf(buf, "-------regex pos=%d: ~:", Index);
	*info += buf;
	*info+=TokenText.P; 
	sprintf(buf, " (%d?) -------\r\n", MatchedRepeatN);
	*info+=buf;
	if (MatchedToList){
		for (RegExToken* matched = MatchedToList; matched; matched=matched->Next){
			sprintf(buf, "seq pos=%d-%d (%d) -- ", matched->MatchedFrom->Index,matched->MatchedTo?matched->MatchedTo->Index:-1, matched->MatchedRepeatN);
			*info+=buf;
			ReferData debug_seq;
			matched->MatchedFrom->getTokenListInfo(&debug_seq, matched->MatchedTo);
			*info+=debug_seq;
			*info+="\r\n";
		}
	}else if (MatchedFrom || MatchedTo){
		sprintf(buf, "seq pos=%d-%d (%d) -- no matched\r\n", MatchedFrom?MatchedFrom->Index:-1,MatchedTo?MatchedTo->Index:-1, MatchedRepeatN);
		*info+=buf;
	}
	*info+="--\r\n\r\n";
	return 0;
}
int RegExToken::getTokenListInfo(ReferData* info, RegExToken* token_to){
	for (RegExToken* debug_tok=this; debug_tok && debug_tok!=token_to; debug_tok=debug_tok->Next){
		ReferData debug_word;
		debug_word="<";
		for (RegExToken* debug_ext=debug_tok; debug_ext; debug_ext=debug_ext->ExtendNext){
			if (debug_word.L>1) debug_word+="/";
			debug_word.Cat(debug_ext->Token.P, debug_ext->Token.L);
		}
		debug_word+=">";
		*info+=debug_word;
	}
	return 0;
}


RegExSyntax::RegExSyntax(){
	InOpMode=0;
}
RegExSyntax::~RegExSyntax(){
	reset();
}
void RegExSyntax::reset(){
	InOpMode=0;
	QLSyntax::reset();
}

void RegExSyntax::findNext(){
	if (InOpMode){
		if (isAlpha(Sentence[Next])){
			while (isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen]))
				NextLen++;
			NextType=synQL_WORD;
			return;
		}else if (isDigit(Sentence[Next])){
			while (isDigit(Sentence[Next+NextLen]) ) NextLen++;
			NextType=synQL_NUMBER;
			return;
		}
	}

	switch (Sentence[Next]){
	case '(': 
		NextType=QLSyntax::synQL_LBRACE; 
		break;
	case ')': 
		NextType=QLSyntax::synQL_RBRACE; 
		break;
	case '[': 
		NextType=QLSyntax::synQL_LRECT; 
		break;
	case ']': 
		NextType=QLSyntax::synQL_RRECT; 
		if (InOpMode) InOpMode=false;
		break;
	case '{': 
		NextType=QLSyntax::synQL_LBRACKET; 
		break;
	case '}': 
		NextType=QLSyntax::synQL_RBRACKET; 
		break;
	case ',': 
		NextType=QLSyntax::synQL_COMMA; 
		break;
	case '|': 
		NextType=QLSyntax::synQL_MID; 
		break;
	case '*': 
		NextType=QLSyntax::synQL_STAR; 
		break;
	case '?': 
		NextType=QLSyntax::synQL_QUES; 
		break;
	case '.': 
		NextType=QLSyntax::synQL_DOT; 
		break;
	case '+': 
		NextType=QLSyntax::synQL_ADD; 
		break;
	case '-': 
		NextType=QLSyntax::synQL_DASH; 
		break;
	case '^': 
		NextType=QLSyntax::synQL_TIP; 
		break;
	case '$': 
		NextType=QLSyntax::synQL_DOLLAR; 
		break;
	case '\\':
		if (Sentence[Next+NextLen]=='w'){
			NextType=synRegQL_WORD;
		}else if (Sentence[Next+NextLen]=='W'){
			NextType=synRegQL_NONWORD;
		}else if (Sentence[Next+NextLen]=='b'){
			NextType=synRegQL_WORDBOUND;
		}else if (Sentence[Next+NextLen]=='d'){
			NextType=synRegQL_DIGIT;
		}else if (Sentence[Next+NextLen]=='D'){
			NextType=synRegQL_NONDIGIT;
		}else if (Sentence[Next+NextLen]=='s'){
			NextType=synRegQL_SPACE;
		}else if (Sentence[Next+NextLen]=='S'){
			NextType=synRegQL_NONSPACE;
		}else {
			NextType=synRegQL_TOKEN; 
		}
		NextLen++; 
		break;
	case '&': 
		if (Sentence[Next+1]=='['){
			NextType=synRegQL_OPTAG; 
			NextLen++;
			InOpMode=true;
		}else{
			NextType=QLSyntax::synQL_REF; 
		}
		break;
	case ':': 
		NextType=QLSyntax::synQL_COLON; 
		break;
	case '=': 
		NextType=QLSyntax::synQL_EQ; 
		break;
	case ';': 
		NextType=QLSyntax::synQL_SEMICOLON; 
		break;
	default: 
		NextType=synRegQL_TOKEN;
		break;
	}
}

long RegExSyntax::parseNumber(){
	long len=0; 
	while (Sentence[Start+len]=='\r'||Sentence[Start+len]==' '||Sentence[Start+len]=='\t'||Sentence[Start+len]=='\n') len++;
	while (isdigit(Sentence[Start+len])){
		len++; 
	}
	while (Sentence[Start+len]=='\r'||Sentence[Start+len]==' '||Sentence[Start+len]=='\t'||Sentence[Start+len]=='\n') len++;
	StartLen=len; 

	Next=Start+StartLen; 
	NextLen=1; 
	findNext(); 
	
	long n=-1; 
	if (StartLen) sscanf(Sentence+Start, "%ld", &n); 
	return n;
}


int RegExSyntax::isSpace(char c){
	if (InOpMode)
		return (c==' '||c=='\t'||c=='\n' || c == '\r');
	else
		return 0;
}

int RegExSyntax::isComment(char*p, long* len){
	if (InOpMode){
		if (*p=='#'){
			*len=1;
			while (p[*len] && p[*len]!='\n' && p[*len]!='\r' && p[*len]!=']' && p[*len]!='#') (*len)++;
			while (p[*len]=='\n'||p[*len]=='\r'||p[*len]=='#') (*len)++;
			return 1;
		}else{
			return 0;
		}
	}else
		return 0;
}

RegExKeeper::RegExKeeper(){
	const char* fields[] = {"Name", "Expr", "RegEx", "Parser", "IsList", 0};
	KeepSet.setFieldsNum(ID_FIELDS_NUM, fields);
	KeepSet.newIndexField(ID_Expr, 0, false); 
	KeepSet.newIndexField(ID_Name, 0, true); 
}

RegExKeeper::~RegExKeeper(){
	reset();
}
void RegExKeeper::reset(){
	for (ReferData* tuple=KeepSet.moveFirst(); tuple; tuple=KeepSet.moveNext()){
		if (tuple[ID_RegEx].P){
			delete (RegExToken*) tuple[ID_RegEx].P;
			tuple[ID_RegEx].P=0;
		}
		if (tuple[ID_Parser].P){
			delete (RegExParser*) tuple[ID_Parser].P;
			tuple[ID_Parser].P=0;
		}
	}
	KeepSet.empty();
}
ReferData* RegExKeeper::findRegEx(ReferData* name, ReferData* expr){
	if (name && name->P){
		return KeepSet.findFieldString(KeepSet.FieldNames[ID_Name]->P, name);
	}else{
		return KeepSet.findFieldString(KeepSet.FieldNames[ID_Expr]->P, expr);
	}
}
int RegExKeeper::deleteRegEx(ReferData* name, ReferData* expr){
	ReferData* tuple=findRegEx(name, expr); 
	if (tuple){
		if (tuple[ID_RegEx].P){
			delete (RegExToken*) tuple[ID_RegEx].P;
			tuple[ID_RegEx].P=0;
		}
		if (tuple[ID_Parser].P){
			delete (RegExParser*) tuple[ID_Parser].P;
			tuple[ID_Parser].P=0;
		}
		KeepSet.dropTuple();
		return true;
	}
	return false;
}
ReferData* RegExKeeper::newRegEx(ReferData* name, ReferData* expr){
	ReferData unique_name; 
	if (name && name->P) unique_name=name->P;
	else {
		unique_name="RegEx:"; unique_name+=*expr;
	}
	ReferData* tuple=KeepSet.newTuple();
	tuple[ID_Name]=unique_name; 
	tuple[ID_Expr]=expr->P;
	KeepSet.commitTuple();
	return tuple;
}
ReferData* RegExKeeper::keepRegEx(ReferData* name, ReferData* expr, RegExToken* regex, RegExParser* parser, int islist){
	ReferData* tuple=findRegEx(name, expr);
	if (tuple) return tuple; 
	tuple=newRegEx(name, expr);
	tuple[ID_RegEx].P=(char*) regex;
	tuple[ID_Parser].P=(char*) parser; 
	tuple[ID_IsList].L=islist;
	return tuple;
}


RegExSearcher::RegExSearcher(){
	RegContext = 0;
	RegParser = 0;
	RegToken = 0;
	IsRegList = false;
	TextStart = 0;
}
RegExSearcher::~RegExSearcher(){
	reset();
}
void RegExSearcher::reset(){
	if (RegToken){
		delete RegToken;
		RegToken=0;
	}
	if (RegParser){
		delete RegParser;
		RegParser=0;
	}
	IsRegList = false;
	if (TextStart){
		delete TextStart;
		TextStart = 0;
	}
	RegContext = 0;
}
int RegExSearcher::compile(const char* sentence, int is_list, int case_sensitive){
	reset(); 
	RegToken = new RegExToken;
	if (is_list) RegParser=new ListRegExParser; 
	else RegParser=new RegExParser;

	IsRegList = is_list; 

	RegExSyntax syntax; 
	syntax.setSentence(sentence, 0, false); 

	if (RegContext) RegParser->Context = RegContext; 
	RegParser->CaseSensitive = case_sensitive; 
	int err=RegParser->parseRegEx(&syntax, RegToken);
	return err; 
}
int RegExSearcher::search(const char* text, ReferSet* results, int overlapping, int useindex, int group){
	if (TextStart) delete TextStart; 
	TextStart = new RegExToken;	
	int err=0; 
	if (IsRegList){
		//err=tokenizeListText(text, TextStart);
		err=0;
		return err; //not supported for now
	}else{
		int err=RegParser->tokenizeText(text, TextStart);
	}
	RegParser->Overlap = overlapping;
	//if (group){
		//ReferSet results;
		err=RegParser->searchRegExTokensSet(TextStart->Next, RegToken, results, false, group);//group
		//return buildStrResultsSet(&results, useindex, group);
		//return buildListResultsSet(&results, useindex, group);
	//}
	/*else{
		ReferLinkHeap results;
		results.setSortOrder(SORT_ORDER_NUM_INC);
		err=RegParser->searchRegExTokens(TextStart->Next, RegExpr, &results);
		//return buildStrResults(&results, useindex);
		//return buildListResults(&results, useindex);
	}*/
	return err;
}


RegExParser::RegExParser(){
	CaseSensitive=True;
	Overlap=false;
	getNameHeap=0;
	GetNameHeapPara=0;
	Context=this;
#ifdef DEBUG_THIS_FILE
	ReferData debug_str;
	debug_str.saveFile("$DEBUG_RegExParser.txt");
#endif
}

RegExParser::~RegExParser(){
	reset();
}
void RegExParser::reset(){
	CaseSensitive=True;
	Overlap=false;
	getNameHeap=0;
	GetNameHeapPara=0;
	ParserNameHeaps.reset();
	Context=this;
}

int RegExParser::matchCmpChar(char ch1, char ch2){
	return CaseSensitive?(ch1-ch2):(toupper(ch1)-toupper(ch2));
}

int RegExParser::parseRegEx(RegExSyntax* syntax, RegExToken* first, int is_seq_mode, long stop_token){
	RegExToken* token=first;
	int err=0;
	int is_pure_token=1; 

	while (err>=0 && syntax->Type!=QLSyntax::synQL_END){
		if (syntax->Type==stop_token) break; 

		if (syntax->Type==QLSyntax::synQL_STAR){
			is_pure_token=false;
			token->RepeatNFrom=0;
			token->RepeatNTo=-1;
			syntax->match();
		}else if (syntax->Type==QLSyntax::synQL_ADD){
			is_pure_token=false;
			token->RepeatNFrom=1;
			token->RepeatNTo=-1;
			syntax->match();
		}else if (syntax->Type==QLSyntax::synQL_QUES){
			is_pure_token=false;
			token->RepeatNFrom=0;
			token->RepeatNTo=1;
			syntax->match();
		}else if (syntax->Type==QLSyntax::synQL_TIP && !is_seq_mode){ // [^] is not
			is_pure_token=false;
			first->IsNegativeToken=!first->IsNegativeToken;
			syntax->match();
		}else if (syntax->Type==QLSyntax::synQL_DASH && !is_seq_mode){ // [a-b] is not
			is_pure_token=false;
			syntax->match();
			setRegExTokenRange(syntax, token);
			syntax->match();
		}else if (syntax->Type==QLSyntax::synQL_LBRACKET){  //{n,m} 
			is_pure_token=false;
			syntax->match();
			token->RepeatNFrom=syntax->parseNumber();
			syntax->match();
			if (syntax->Type==QLSyntax::synQL_COMMA){
				syntax->match(); 
				token->RepeatNTo = syntax->parseNumber();
				syntax->match();
			}else if (syntax->Type==QLSyntax::synQL_RBRACKET){
				token->RepeatNTo=token->RepeatNFrom;
			}else{ 
				token->RepeatNTo = syntax->parseNumber();
				syntax->match();
			}
			if (!syntax->match(QLSyntax::synQL_RBRACKET)) return -1;
		}

		if (syntax->Type==stop_token) break; 
	

		if (syntax->Type==QLSyntax::synQL_LBRACE){ //sequence
			is_pure_token=false;
			token=newToken(syntax, token, is_seq_mode); 
			if (first->IsNegativeToken){
				first->IsNegativeToken=2;
			}

			token->addChildToken(new RegExToken); 
			syntax->match(); 
			if ((err=parseRegEx(syntax, token->Child, true, QLSyntax::synQL_RBRACE))<0) return err;
			if (!syntax->match(QLSyntax::synQL_RBRACE)) return -1;
			
		}else if (syntax->Type==QLSyntax::synQL_LRECT){ //set
			is_pure_token=false;
			token=newToken(syntax, token, is_seq_mode); 
			//token->addSiblToken(new RegExToken); 
			syntax->match();
			if ((err=parseRegEx(syntax, token, false, QLSyntax::synQL_RRECT))<0) return err;
			if (!syntax->match(QLSyntax::synQL_RRECT)) return -1;

		}else if (syntax->Type==RegExSyntax::synRegQL_OPTAG){
			syntax->match();
			if ((err=parseRegExOpOptions(syntax, &token, is_seq_mode))<0) return err;
			if (!syntax->match(QLSyntax::synQL_RRECT)) return -1;
			
		}else if (syntax->Type==QLSyntax::synQL_MID && is_seq_mode){ //or
			token=newToken(syntax, token, is_seq_mode); 

			//change Next node to Sibling
			if (first->Next){
				first->switchNextToSiblNext();
			}

			//switch token to first.SiblNext.Child 
			token=first->addSiblToken(new RegExToken);
			token=token->addChildToken(new RegExToken);
			syntax->match(); 

			if (is_pure_token) is_pure_token++;
		}else{
			token=newToken(syntax, token, is_seq_mode); 
			err=parseRegExToken(syntax, token);
			if (token->TokenType!=RegExSyntax::synRegQL_TOKEN) 
				is_pure_token = false;
		}
	}

	if (is_pure_token && first->SiblNext){ 
		//set up MatchStringSet to expedite string matching
		first->setupMatchStringTokenSet(this);
	}
	return err;
}
long RegExParser::addTokens2Heap(RegExToken* sibl_token, ReferLinkHeap* heap){
	RegExToken* token, *next; 
	long count=0;
	for (token=sibl_token; token; token=token->SiblNext){
		ReferData str; 
		next=token->Child?token->Child:token; 
		for (; next; next=next->Next){
			str.Cat(next->Token.P, next->Token.L);
		}
		heap->add(&str, 0, 0);
		count++; 
	}
	return count;
}
int RegExParser::parseRegExOpOptions(RegExSyntax* syntax, RegExToken** token, int is_seq_mode){
	int err=0;
	ReferData option; 
	while (syntax->Type!=QLSyntax::synQL_RRECT){
		syntax->takeStartSyntaxString(&option);
		if (!option.Cmp("name", 4, false) || !option.Cmp("n", 1, false)){
			//assign a name
			syntax->match(); 
			if ((err=parseRegExOpOptionName(syntax, token, is_seq_mode))<0) return err;
		}else if (!option.Cmp("?", 1, false) || !option.Cmp("q", 1, false) ){
			//assign a name
			syntax->match(); 
			if ((err=parseRegExOpOptionCondition(syntax, token, is_seq_mode))<0) return err;
		}else if (!option.Cmp("set", 3, false) || !option.Cmp("s", 1, false) 
			|| !option.Cmp("ws", 2, false) || !option.Cmp("sw", 2, false) 
			|| !option.Cmp("ss", 2, false)
			){
			//match to a set
			syntax->match(); 
			int matchtype=RegExToken::matchTOKEN_SET; 
			if (!option.Cmp("ws", 2, false) || !option.Cmp("sw", 2, false)) matchtype=RegExToken::matchWORD_SET;
			else if (!option.Cmp("ss", 2, false)) matchtype=RegExToken::matchSTRING_SET;
			if ((err=parseRegExOpOptionSet(syntax, token, is_seq_mode, matchtype))<0) return err;
		}else if (syntax->Type!=QLSyntax::synQL_COMMA && syntax->Type!=QLSyntax::synQL_SEMICOLON){ //unknown option
			err=-1;
			break;
		}
		if (syntax->Type==QLSyntax::synQL_COMMA || syntax->Type==QLSyntax::synQL_SEMICOLON){
			syntax->match();
		}
	}
	return err;
}
int RegExParser::parseRegExOpOptionCondition(RegExSyntax* syntax, RegExToken** token, int is_seq_mode){
	if (!syntax->match(QLSyntax::synQL_COLON)) return 0;
	tExprCalc* expr=new tExprCalc();
	int i=expr->setExpression(syntax->Sentence+syntax->Start, 0);
	if (i>=0) i=expr->parse(synEXP_RIGHTBRACE);
	if (i<0) {
		delete expr; 
		return i;
	}
	ReferLink2* link=(*token)->Conditions.insert();
	link->Value.P=(char*) expr;

	syntax->Next = syntax->Start+expr->ExprSentence->Start;
	syntax->NextLen = 0;
	syntax->match();
	syntax->match();
	return 0;
}
int RegExParser::parseRegExOpOptionName(RegExSyntax* syntax, RegExToken** token, int is_seq_mode){
	if (!syntax->match(QLSyntax::synQL_COLON)) return 0;
	if (syntax->Type!=QLSyntax::synQL_WORD) return 0;
	ReferData name;
	syntax->takeStartSyntaxString(&name);
	(*token)->Name.Set(name.P, name.L, true); 
	syntax->match();
	return 0;
}
ReferLinkHeap* RegExParser::findContextNameHeap(const char* name){
	ReferLinkHeap* strset=0; 
	if (Context){
		if (Context->getNameHeap) strset=Context->getNameHeap(this, Context->GetNameHeapPara, name);
		if (!strset) strset=Context->ParserNameHeaps.getHeap(name); 
	}
	if (this!=Context && !strset){
		if (this->getNameHeap) strset=this->getNameHeap(this, GetNameHeapPara, name);
		if (!strset) strset=this->ParserNameHeaps.getHeap(name); 
	}
	return strset;
}
ReferLinkHeap* RegExParser::getContextNameHeap(const char* name){
	ReferLinkHeap* strset=findContextNameHeap(name);
	if (!strset) strset=this->ParserNameHeaps.createHeap(name, this->CaseSensitive);
	return strset;
}

int RegExParser::parseRegExOpOptionSet(RegExSyntax* syntax, RegExToken** token, int is_seq_mode, int matchtype){
	if (!syntax->match(QLSyntax::synQL_COLON)) return 0;
	int err=0;
	ReferData name;
	ReferLinkHeap* strset=0; 
	RegExToken* token1=0; 
	if (syntax->Type==QLSyntax::synQL_WORD){
		syntax->takeStartSyntaxString(&name);

		strset = getContextNameHeap(name.P);

		(*token)=newToken(syntax, (*token), is_seq_mode); 
		token1=(*token)->addChildToken(new RegExToken);
		token1->setupMatchStringStrSet(this, strset, matchtype); 
		syntax->match();
	}
	if (syntax->Type==QLSyntax::synQL_LBRACE){
		syntax->match(); 
		if (!token1) {
			(*token)=newToken(syntax, (*token), is_seq_mode); 
			token1=(*token)->addChildToken(new RegExToken);
		}

		RegExSyntax syntax1; //fixed string type
		syntax1.setSentence(syntax->Sentence+syntax->Start);
		if (matchtype==RegExToken::matchWORD_SET || matchtype==RegExToken::matchSTRING_SET){
			while (syntax1.Type!=QLSyntax::synQL_RBRACE) syntax1.match();
			if (syntax1.Type!=QLSyntax::synQL_RBRACE) return -1;
			if (!strset){
				//create a temporary name
				char tmpname[128]; 
				for (int k=0; k>0; k++){
					sprintf(tmpname, "__tmpname__%d", k);
					if (!findContextNameHeap(tmpname)) break;
				}
				strset = getContextNameHeap(tmpname);
			}
			ReferData names; 
			names.Set(syntax1.Sentence, syntax1.Start, true); 
			tStrOp::splitString(names.P, "|", strset); 

			token1->setupMatchStringStrSet(this, strset, matchtype); 
		}else{
			if ((err=parseRegEx(&syntax1, token1, true, QLSyntax::synQL_RBRACE))<0) return err;

			if (!token1->MatchStringSet)
				token1->setupMatchStringTokenSet(this);
			if (strset) addTokens2Heap( token1->SiblNext, strset);
		}
		if (!syntax1.match(QLSyntax::synQL_RBRACE)) return -1;
		syntax->Next=syntax->Start + syntax1.Start;
		syntax->NextLen=0;
		syntax->match(); syntax->match();
	}
	return 0;
}

int RegExParser::parseRegExToken(RegExSyntax* syntax, RegExToken* token){
	if (syntax->Sentence[syntax->Start]=='\\'){
		token->Token.Set(syntax->Sentence+syntax->Start+1, syntax->StartLen-1, true); 
	}else{
		token->Token.Set(syntax->Sentence+syntax->Start, syntax->StartLen, true); 
	}
	token->TokenText.Set(syntax->Sentence+syntax->Start, syntax->StartLen, false);

	token->TokenType = syntax->Type;
	
	syntax->match();
	return 0;

}
RegExSyntax* RegExParser::newRegExSyntax(){
	return new RegExSyntax; 
}

RegExToken* RegExParser::newToken(RegExSyntax* syntax, RegExToken* token, int is_seq_mode){
	if (is_seq_mode){
		token=token->addNextToken(new RegExToken); 
	}else{
		token=token->addSiblToken(new RegExToken); 
	}
	token->TokenText.Set(syntax->Sentence+syntax->Start, syntax->StartLen, false);
	return token;
}


int RegExParser::setRegExTokenRange(RegExSyntax* syntax, RegExToken* token){
	char buf[128]; 
	sprintf(buf, "%c-%c", token->Token.P[0], syntax->Sentence[syntax->Start]);

	token->Token=buf;

	token->TokenType=RegExSyntax::synRegQL_RANGE;
	return 0;
}

int RegExParser::tokenizeText(const char* text, RegExToken* start, long limit){
	RegExToken* token=start;
	if (text){
		char* p=(char*) text;
		for (p=(char*) text; *p && (limit<0 || p-text<limit); p++){
			token=token->addNextToken(new RegExToken);
			token->Token.Set(p, 1, true); 
			token->TokenText.Set(p, 1, false);
			token->Index=p-text;
		}
		token=token->addNextToken(new RegExToken);
		token->Token.Set(p, 0, false); 
		token->TokenText.Set(p, 0, false);
		token->Index=p-text;
	}
	return 0;
}

int RegExParser::searchRegEx(RegExToken* text, RegExToken* reg_start, RegExToken** text_matched, RegExToken** matched_to ){
	if (text_matched) *text_matched=0;
	if (matched_to) *matched_to=0;
	if (!text) return -1;

	RegExToken* text_to=text;
	int err=0;

	while (1){ //advance text
		err=doMatchRegExSequence(text, reg_start, &text_to);

#ifdef DEBUG_THIS_FILE
		if (err>=0){
			ReferData debug_str; 
			text->getTokenListInfo(&debug_str, text_to); 
			debug_str+="\r\n";
			TRACE(debug_str.P);
			ReferData debug_info; 
			debug_info="\r\n**********************\r\n/////// match ////////\r\n**********************\r\n";
			debug_info+=debug_str;
			debug_info+="**********************\r\n\r\n\r\n";
			debug_info.appendFile("$DEBUG_RegExParser.txt");
		}
#endif

		//advance text
		if (err<0 || text_to==text){
			if (err>=0 && text_to==text){
				break;
			}
			if (text->Next){
				text=text->Next; 
				continue;
			}else{
				if (err>=0) err=-1; 
				break;
			}
		}else {
			break;
		}
	} //advance text

	if (err>=0){
		//if (text->Token.L){
			if (text_matched) *text_matched=text; 
			if (matched_to) *matched_to=text_to; 
		//}else{
		//	err=-1;
		//}
	}

	return err;
}

int RegExParser::isConditionMatched(tExprCalc* expr, RegExToken* reg_start){
	if (!expr) return true;
	setExprVariables(expr, reg_start);
	int err=expr->calculate();
	return expr->getBoolean();
}
int RegExParser::setExprVariables(tExprCalc* expr, RegExToken* reg_start){
	ReferData value;
	for (int i=0; i<expr->FieldsNum; i++){
		tExprField* field=expr->FieldsList[i];
		RegExToken* reg_token=reg_start;
		while (reg_token){
			if (!reg_token->Name.Cmp(&field->StringName, true)){
				break;
			}else if(reg_token->SiblPrev){
				reg_token=reg_token->SiblPrev;
			}else if(reg_token->Prev){
				reg_token=reg_token->Prev;
			}else {
				reg_token=reg_token->Parent;
			}
		}
		if (reg_token){
			RegExToken* t;
			for (t=reg_token->MatchedFrom; t!=reg_token->MatchedTo; t=t->Next){
				field->StringValue.Cat(t->TokenText.P, t->TokenText.L);
				field->Value=expr->StringValue.P;
			}
		}
	}
	return 0;
}


int RegExParser::doMatchRegExSequence(RegExToken* text,RegExToken* reg_start, RegExToken** matched_to){
	RegExToken* text_to=text;
	RegExToken* reg_next=0; 
	int err=0;

	err=doMatchRegExRepeat(text, reg_start, &text_to);

	*matched_to=text;
	while (err>=0){
		int meet_cond=true;
		if (reg_start->Conditions.Next){
			for (ReferLink2* link=(ReferLink2*) reg_start->Conditions.Next; link && link!=&reg_start->Conditions; link=(ReferLink2*) link->Next){
				if (!isConditionMatched((tExprCalc*) link->Value.P, reg_start))
					meet_cond=false;
			}
		}

		if (meet_cond){
			if (reg_start->Next){
#ifdef DEBUG_THIS_FILE
				ReferData debug_info; 
				debug_info="move on\r\n";
				debug_info.appendFile("$DEBUG_RegExParser.txt");
#endif
				err=doMatchRegExSequence(text_to, reg_start->Next, matched_to);
				if (err>=0) reg_start->takeMatchedToList(reg_start->Next);
			}else{
				*matched_to=text_to;
			}

			if (err>=0) break; 
		}

#ifdef DEBUG_THIS_FILE
		ReferData debug_info; 
		debug_info="try next\r\n";
		debug_info.appendFile("$DEBUG_RegExParser.txt");
#endif
		err=reg_start->popMatchedToList("--doMatchRegExSequence--pop one item"); 
		text_to=reg_start->MatchedTo;
	}

	return err;
}
int RegExParser::doMatchRegExRepeat(RegExToken* text,RegExToken* reg_start, RegExToken** matched_to){ //return err
	//this usually match one reg_start token 
	RegExToken* text_to=text;
	reg_start->MatchedFrom=text_to;
	RegExToken* regex=reg_start; //regex: record the last matching sibling
	if (matched_to) *matched_to=text_to; 

	//if (!text || (!text->Token.P && !text->Next && !text->SiblNext && !text->Child) ) return -1;
	////cannot skip this to match .*

	reg_start->clearMatched();

	int err=0;
	long repeat=0; 
	reg_start->MatchedFrom=text_to;
	reg_start->MatchedTo=text_to;
	reg_start->MatchedSibl=regex;
	reg_start->MatchedRepeatN=repeat; 
	while (1){ //repeat reg_start->RepeatNFrom -- reg_start->RepeatNTo
		RegExToken* repeat_text_to=text_to; 
		repeat++; 

		// use MatchStringSet here ...
		int has_sibling=false;
		if (reg_start->MatchType && reg_start->MatchStringSet){
			BTreeRecord record;
			record.tree = reg_start->MatchStringSet; 
			RegExToken* first=(RegExToken*) record.moveFirstLarger((char*) text_to);
			if (!first) first=(RegExToken*) record.moveLast(); 
			else if (!reg_start->MatchStringSet->matchTarget(first, text_to, 0)){
				first=(RegExToken*) record.movePrevious();
			}
			//RegExToken* first=(RegExToken*) record.moveFirst((char*) repeat_text_to);
			err=-1;
			if (first){
				//get the longest match
				RegExToken* t1=0, *t2=0;
				reg_start->MatchStringSet->matchTarget(first, text_to, &t1);
				while (t1){ 
					t2=t1->Next; 
					t1->Next=0; 
					first=(RegExToken*) record.moveFirst((char*) repeat_text_to);
					int is_matched=first && reg_start->MatchStringSet->matchTarget(first, repeat_text_to, 0);
					t1->Next=t2; 
					if (is_matched){ 
						has_sibling=true;
						err=0;
						text_to=t1->Next;
						if (reg_start->IsNegativeToken==0 || reg_start->IsNegativeToken==2){
							//if is positive matching (not negative), or negative sequence matching [^(xxx)]
							//  push the result to list and continue to the next sibling
							//reg_start->IsNegativeToken==2: [^(xxx)]
							reg_start->MatchedTo=text_to;
							reg_start->MatchedSibl=regex;
							reg_start->MatchedRepeatN=repeat; 
							reg_start->pushMatchedToList("from MatchStringSet");
						}else{
							//to find negative match and we found one matching, don't need to proceed
							break;
						}
					}
					if (t1==repeat_text_to) break;
					t1=t1->Prev;
				}
			}
		}else{
			err=0;
			//regular matching
			if (reg_start->Token.L || reg_start->Child ){ 
				//first match the token itself
				if (reg_start->Token.L){
					if ((err=isMatchedRegToken(text_to, reg_start))>0){
						if (err>20 || err<10 ) //not in 10-20 for special tags
							text_to=text_to->Next;
					}else{
						err=-1;
					}
				}
				if (err>=0 && reg_start->Child ){ //if it has Child, match the child next
					err=doMatchRegExSequence(text_to, reg_start->Child, &text_to); 
				}

				if (err>=0 && reg_start->isMatchedInRange(repeat)){
					has_sibling=true;
					reg_start->MatchedTo=text_to;
					reg_start->MatchedSibl=reg_start;
					reg_start->MatchedRepeatN=repeat; 
					reg_start->pushMatchedToList("match the token");
					reg_start->takeMatchedToList(reg_start->Child); 
#ifdef DEBUG_THIS_FILE
					//TRACE("RegExParser::doMatchRegExRepeat: reg=%s; repeat=%d; ...=%s\n", reg_start->TokenText.P, reg_start->MatchedRepeatN, (text_to&&text_to->TokenText.P)?text_to->TokenText.P:"");
#endif
				}
			}
			//if it has sibling (but not from previous sibling call), match the siblings as well
			if (!reg_start->Token.L && !reg_start->Child && reg_start->SiblNext && !(has_sibling && reg_start->IsNegativeToken==1)){
			//if (!reg_start->Token.L && reg_start->SiblNext && !reg_start->Child){ 
				//test each sibling
				for (regex=reg_start->SiblNext; regex; regex=regex->SiblNext){
					int err1=0;
					text_to=repeat_text_to;
					if ((err1=doMatchRegExSequence(text_to, regex, &text_to))>=0){
						has_sibling=true;
						reg_start->MatchedTo=text_to;
						reg_start->MatchedSibl=regex;
						reg_start->MatchedRepeatN=repeat; 
						reg_start->pushMatchedToList("match sibling");
						if (reg_start->IsNegativeToken==0 || reg_start->IsNegativeToken==2){
							//if is positive matching (not negative), or negative sequence matching [^(xxx)]
							//  push the result to list and continue to the next sibling
							//reg_start->IsNegativeToken==2: [^(xxx)]
						}else{
							//to find negative match and we found one matching, don't need to proceed
							break;
						}
					} 
					if (err1>=0) err=err1;
				}
				if (!has_sibling) err=-1; 
				//handleMultipleNegative(reg_start, has_sibling, repeat, repeat_text_to, &text_to, &regex, &err);
			}
		}
		handleMultipleNegative(reg_start, has_sibling, repeat, repeat_text_to, &text_to, &regex, &err);
		if (err<0) break; //no matches
		if (reg_start->IsNegativeToken==2) break; //to match [^(xxx)]
		if (reg_start->RepeatNTo>=0 && repeat>=reg_start->RepeatNTo) {
			break;
		}
		if (!text_to || !text_to->Token.L) {
			if (reg_start->RepeatNFrom>0 && repeat<reg_start->RepeatNFrom)
				err=-1;
			break;
		}
		if (reg_start->MatchedToList)
			text_to = reg_start->MatchedToList->MatchedTo;
	} //repeat reg_start->RepeatNFrom -- reg_start->RepeatNTo

	if (reg_start->MatchType || reg_start->Token.L || reg_start->Child || reg_start->SiblNext){
		err=reg_start->popMatchedToList("--to result");
		while (err>=0 && !reg_start->isMatchedInRange(reg_start->MatchedRepeatN) )
			err=reg_start->popMatchedToList("--to result, test range");
	}
	if (err<0) reg_start->MatchedTo=reg_start->MatchedFrom;
	text_to=reg_start->MatchedTo;

	if (matched_to) *matched_to=text_to; 

	return err; 
}

int RegExParser::handleMultipleNegative(RegExToken* reg_start, int has_sibling, int repeat, RegExToken* repeat_text_to, RegExToken** text_to, RegExToken** matched_sibl, int* err1){
/*#ifdef DEBUG_THIS_FILE
	if (reg_start->MatchedRepeatN>0){
		ReferData debug_str(512);
		reg_start->getMatchedToListInfo(&debug_str, "**handleMultipleNegative**");
		debug_str.appendFile("$DEBUG_RegExParser.txt");
	}
#endif */
	int err=*err1;
	if (reg_start->IsNegativeToken==2){
		//for negative sequence matching [^(xxx)]
		if (err>=0){
			err=reg_start->popMatchedToList("**handleMultipleNegative**pop the longest match");//pop the longest match
			(*text_to)=reg_start->MatchedTo;
			(*matched_sibl)=reg_start->MatchedSibl;
			while (reg_start->MatchedToList && reg_start->MatchedRepeatN == reg_start->MatchedToList->MatchedRepeatN){
				reg_start->popMatchedToList("IsNegativeToken==2 ... -");
				if (reg_start->MatchedTo->Index < (*text_to)->Index){
					(*text_to)=reg_start->MatchedTo;
					(*matched_sibl)=reg_start->MatchedSibl;
				}
			}
			//reg_start->deleteMatchedToList("IsNegativeToken==2");
		}else{
			(*text_to)=reg_start->MatchedFrom;
			(*matched_sibl)=reg_start;
		}
		long repeat1=1;
		if ((*text_to)->Next && (reg_start->RepeatNTo<0 || repeat1<reg_start->RepeatNTo)){
			while ((*text_to)->Next && (reg_start->RepeatNTo<0 || repeat1<reg_start->RepeatNTo) ){
				//push all shorter sequences as possible matches
				(*text_to)=(*text_to)->Next; 
				reg_start->MatchedTo=(*text_to);
				reg_start->MatchedSibl=(*matched_sibl);
				reg_start->MatchedRepeatN=repeat; //use fixed repeat, not repeat1
				reg_start->pushMatchedToList("IsNegativeToken==2 ... +");
				err=0;
				repeat1++;
			}
		}else{
			err=-1;
		}
	}else if (reg_start->IsNegativeToken){
		if (err>=0){
			reg_start->popMatchedToList("IsNegativeToken");
			err=-1;
		}else if (err<0){
			//err=0;
			//(*text_to)=(repeat_text_to && repeat_text_to->Token.L) ?repeat_text_to->Next:repeat_text_to;
			if (repeat_text_to && repeat_text_to->Token.L){ //not the last token
				err=0;
				(*text_to)=repeat_text_to->Next;
			}else if (reg_start->RepeatNFrom<=0){ //the last token, requiring no match
				err=0;
				(*text_to)=repeat_text_to;
			}else{
				err=-1;
			}
			if (err>=0){
				reg_start->MatchedTo=(*text_to);
				reg_start->MatchedSibl=reg_start;
				reg_start->MatchedRepeatN=repeat; //use fixed repeat, not repeat1
				reg_start->pushMatchedToList("**handleMultipleNegative**push negative sequences");
			}
		}
	}
	if (reg_start->RepeatNFrom==0 && repeat==1){
		err=0;
		reg_start->MatchedTo=reg_start->MatchedFrom;
		reg_start->MatchedSibl=reg_start;
		reg_start->MatchedRepeatN=0; //use fixed repeat, not repeat1
		reg_start->pushMatchedToList("**handleMultipleNegative**push null sequence");
	}
	(*err1)=err;
	return err;
}

int RegExParser::isMatchedRegToken(RegExToken* text_token, RegExToken* reg_token){
	if (!text_token || !text_token->Token.L){
		if (reg_token->TokenType==RegExSyntax::synQL_TIP){
			return (!text_token->Prev || !text_token->Prev->Prev)?10:0; //^, 10 for special flag
		}else if (reg_token->TokenType==RegExSyntax::synQL_DOLLAR){
			return (!text_token->Next)?20:0; //$, 20 for special flag
		}else if (reg_token->TokenType==RegExSyntax::synRegQL_WORDBOUND){
			if (!text_token->Prev || !text_token->Prev->Prev) return 10;
			else if (!text_token->Next) return 20;
			else return 0;
		}else if (reg_token->RepeatNFrom==0){
			return 11; //{0}, 11 for special flag
		}else{
			return false; 
		}
	}else if (reg_token->TokenType==RegExSyntax::synQL_TIP){
		return (!text_token->Prev || !text_token->Prev->Prev)?10:0; //^, 10 for special flag
	}else if (reg_token->TokenType==RegExSyntax::synQL_DOLLAR){
		return (!text_token->Next)?20:0; //$, 20 for special flag
	}else if (reg_token->TokenType==RegExSyntax::synRegQL_WORDBOUND){
		if (!text_token->Prev || !text_token->Prev->Prev) return 10;
		else if (!text_token->Next) return 20;
		else if ((isalpha(text_token->Token.P[0])||isdigit(text_token->Token.P[0])) && !(text_token->Prev->Token.P && (isalpha(text_token->Prev->Token.P[0])||isdigit(text_token->Prev->Token.P[0]))) ) return 12; //b, 20 for special flag
		else if (!(isalpha(text_token->Token.P[0])||isdigit(text_token->Token.P[0])) && (text_token->Prev->Token.P && (isalpha(text_token->Prev->Token.P[0])||isdigit(text_token->Prev->Token.P[0]))) ) return 13; //b, 20 for special flag
		else return 0;
	}else if (reg_token->TokenType==RegExSyntax::synQL_DOT){
		return text_token && text_token->Token.L>0; //.

	}else if (reg_token->TokenType==RegExSyntax::synRegQL_WORD){
		return isalpha(text_token->Token.P[0]); 
	}else if (reg_token->TokenType==RegExSyntax::synRegQL_NONWORD){
		return !isalpha(text_token->Token.P[0]); 
	}else if (reg_token->TokenType==RegExSyntax::synRegQL_DIGIT){
		return isdigit(text_token->Token.P[0]); 
	}else if (reg_token->TokenType==RegExSyntax::synRegQL_NONDIGIT){
		return !isdigit(text_token->Token.P[0]); 
	}else if (reg_token->TokenType==RegExSyntax::synRegQL_SPACE){
		return text_token->Token.P[0]==' ' || text_token->Token.P[0]=='\t' || text_token->Token.P[0]=='\r' || text_token->Token.P[0]=='\n'; 
	}else if (reg_token->TokenType==RegExSyntax::synRegQL_NONSPACE){
		return !(text_token->Token.P[0]==' ' || text_token->Token.P[0]=='\t' || text_token->Token.P[0]=='\r' || text_token->Token.P[0]=='\n');
	}else if (reg_token->TokenType==RegExSyntax::synRegQL_RANGE){
		return text_token->Token.P[0]>=reg_token->Token.P[0] && text_token->Token.P[0]<=reg_token->Token.P[2];
	}else{
		return matchedRegTokenText(text_token, reg_token)==0;
	}
}
int RegExParser::matchedRegTokenText(RegExToken* text_token, RegExToken* reg_token){ //return true or false
	return matchCmpChar(text_token->Token.P[0], reg_token->Token.P[0]);
}

int RegExParser::searchRegExTokensSet(RegExToken* text, RegExToken* regex, ReferSet* results, int as_text, int withgroup){ //Name.P: token_start, Value.P: token_end (exclude), Data: index
	ReferLinkHeap groups; 
	if (withgroup){
		searchRegExGroups(regex, &groups); 
		results->setFieldsNum(groups.Total+1); 
	}else{
		results->setFieldsNum(1); 
	}
	ReferLink* link;
	long fieldindex0=0;
	results->setFieldName(fieldindex0++, regex->Name.L?regex->Name.P:"all"); 
	if (withgroup){
		for (link=(ReferLink*) groups.moveFirst(); link; link=(ReferLink*) groups.moveNext()){
			results->setFieldName(fieldindex0++, link->Name.P); 
		}
	}
	int err=0;
	if (regex->Next || regex->SiblNext || regex->Child || regex->ExtendNext){
		RegExToken* matched_text=text, *matched_to=0; 
		ReferData result; 
		long count=0;
		while (err>=0 && matched_text && matched_text->Token.L){
			int err1=searchRegEx(matched_text, regex, &matched_text, &matched_to);
			if (err1<0) break;
			ReferData* tuple=results->newTuple(); 
			fieldindex0=0; 
			if (as_text){
				tuple[fieldindex0++].Set(matched_text->TokenText.P, matched_to->TokenText.P-matched_text->TokenText.P, true); 
			}else{
				tuple[fieldindex0++].Set((char*) matched_text, (long) matched_to, false); 
			}
			if (withgroup){
				for (link=(ReferLink*) groups.moveFirst(); link; link=(ReferLink*) groups.moveNext()){
					RegExToken* regex1 = (RegExToken*) link->Value.P;
					if (as_text){
						tuple[fieldindex0++].Set(regex1->MatchedFrom->TokenText.P, regex1->MatchedTo->TokenText.P-regex1->MatchedFrom->TokenText.P, true); 
					}else{
						tuple[fieldindex0++].Set((char*) regex1->MatchedFrom, (long) regex1->MatchedTo, false); 
					}
				}
			}
			results->commitTuple(); 
			if (!Overlap && matched_to != matched_text){
				matched_text = matched_to;
			}else{
				matched_text = matched_text->Next;
			}
		}
	}
	return err;
}

int RegExParser::searchRegExTokens(RegExToken* text, RegExToken* regex, ReferLinkHeap* results){ //Name.P: token_start, Value.P: token_end (exclude), Data: index
	results->setSortOrder(SORT_ORDER_NUM_INC);
	results->resort();

	int err=0;
	if (regex->Next || regex->SiblNext || regex->Child || regex->ExtendNext){
		RegExToken* matched_text=text, *matched_to=0; 
		ReferData result; 
		long count=0;
		ReferLink* link;
		while (err>=0 && matched_text && matched_text->Token.L){
			int err1=searchRegEx(matched_text, regex, &matched_text, &matched_to);
			if (err1<0) break;
			link=results->add((char*)0, 0, ++count); 
			link->Name.Set((char*) matched_text, 0, false);
			link->Value.Set((char*) matched_to, 0, false);
			if (matched_to == matched_text && matched_text->TokenText.L && regex->TokenText.L)
				return 0; //special match
			if (!Overlap && matched_to != matched_text){
				matched_text = matched_to;
			}else{
				matched_text = matched_text->Next;
			}
		}
	}
	return err;
}
int RegExParser::getTokenText(RegExToken* text, RegExToken* text_to, ReferData* result){
	result->Set(text->TokenText.P, (text_to->TokenText.P-text->TokenText.P), false);
	/**result="";
	for (RegExToken* token=text; token && token!=text_to; token=token->Next){
		*result+=token->Token; 
	}*/
	return 0;
}
int RegExParser::searchRegExGroups(RegExToken* regex, ReferLinkHeap* groups){
	if (!groups->Total){
		groups->setSortOrder(SORT_ORDER_NUM_INC);
		groups->setDuplication(true); 
	}
	for (RegExToken* regex1=regex; regex1; regex1=regex1->Next){
		if (regex1->Child){
			ReferLink* link=groups->add((char*) 0, (char*) 0, groups->Total+1);
			if (regex1->Name.L){
				link->Name.Set(regex1->Name.P, regex1->Name.L, false); 
			}else{
				link->Name.Malloc(128);
				sprintf(link->Name.P, "\\%ld", groups->Total);
				link->Name.L=strlen(link->Name.P); 
			}
			link->Value.Set((char*) regex1, 0, false);
			searchRegExGroups(regex1->Child, groups); 
		}
	}
	return 0;
}
int RegExParser::searchRegExTextSet(const char* text, const char* regexpr, ReferSet* results, int as_text){
	RegExSyntax* syntax=newRegExSyntax(); 
	syntax->setSentence(regexpr);

	RegExToken token, text_token; 
	int err=parseRegEx(syntax, &token); 
	if (text){
		err=tokenizeText(text, &text_token);

		err=searchRegExTokensSet(text_token.Next, &token, results, as_text);
	}

	delete syntax; 
	return err; 	
}

int RegExParser::searchRegExText(const char* text, const char* regexpr, ReferLinkHeap* results){
	RegExSyntax* syntax=newRegExSyntax(); 
	syntax->setSentence(regexpr);

	RegExToken token, text_token; 
	int err=parseRegEx(syntax, &token); 
	if (text){
		err=tokenizeText(text, &text_token);

		ReferLinkHeap token_results; 
		err=searchRegExTokens(text_token.Next, &token, &token_results);

		ReferData result; 
		for (ReferLink* link=(ReferLink*) token_results.moveFirst(); link; link=(ReferLink*) token_results.moveNext()){
			getTokenText((RegExToken*) link->Name.P, (RegExToken*) link->Value.P, &result);
			ReferLink* link1=results->add(&result, 0, link->Data);
			link1->Value.Set(((RegExToken*) link->Name.P)->TokenText.P, ((RegExToken*) link->Value.P)->TokenText.P-((RegExToken*) link->Name.P)->TokenText.P, false);
		}
	}

	delete syntax; 
	return err; 
}
int RegExParser::matchRegExText(const char* text, const char* regexpr){
	RegExSyntax* syntax=newRegExSyntax(); 
	syntax->setSentence(regexpr);

	RegExToken token, text_token; 
	int err=parseRegEx(syntax, &token); 
	err=tokenizeText(text, &text_token);

	RegExToken *matched_to=0; 
	err=doMatchRegExSequence(text_token.Next, &token, &matched_to);
	int ret= (err>=0 && matched_to && (!matched_to->Token.L || !matched_to->Next));

	delete syntax; 
	return ret;
}
int RegExParser::createRegExSubToken(RegExToken* regexpr){ 
	int err=0;
	if (!regexpr->SubToken){
		regexpr->SubToken=new RegExToken; 
		RegExSyntax* syntax=newRegExSyntax(); 
		syntax->setSentence(regexpr->Token.P, &regexpr->Token.L, false);
		err=parseRegEx(syntax, regexpr->SubToken); 
		delete syntax; 
	}
	return err;
}
int RegExParser::matchRegExSubToken(RegExToken* text, RegExToken* regexpr){ //return boolean;
	int err=0;
	if (!regexpr->SubToken){
		err=createRegExSubToken(regexpr);
	}else{
		regexpr->SubToken->clearAllMatched();
	}

	if (!text->SubToken){
		text->SubToken=new RegExToken; 
		err=tokenizeText(text->Token.P, text->SubToken);
	}

	RegExToken *matched_to=0; 
	err=doMatchRegExSequence(text->SubToken->Next, regexpr->SubToken->Next?regexpr->SubToken->Next:regexpr->SubToken, &matched_to);
	//int ret= (err>=0 && matched_to && (!matched_to->Token.L || !matched_to->Next));
	//return ret;
	if (err>=0 && (!matched_to || !matched_to->Token.L || !matched_to->Next)) return 0; 
	RegExToken* token=text->SubToken->Next;
	matched_to=regexpr->SubToken;
	while (token && !token->Token.P) token=token->Next;
	while (matched_to && !matched_to->Token.P) matched_to=matched_to->Next;
	while (token && token->Token.L && matched_to && matched_to->Token.L 
		&& (err=matchedRegTokenText(token, matched_to))==0){
		token=token->Next;
		matched_to=matched_to->Next; 
	}
	if (!token || !token->Token.L) return -1;
	else if (!matched_to || !matched_to->Token.L) return 1; 
	else return err;
}

//========== TokenRegExSyntax ===========================================

int TokenRegExSyntax::isSpace(char c){
	return (c==' ' || c=='\t' || c=='\r' || c=='\n'); 
}



//========== TokenRegExParser ==============================================

int TokenRegExParser::parseRegExToken(RegExSyntax* syntax, RegExToken* token){
	if (syntax->Sentence[syntax->Start]=='<'){
		RegExSyntax keep;
		keep.copyFrom(syntax); 
		while (syntax->Type!=QLSyntax::synQL_END && syntax->Sentence[syntax->Start]!='>'){
			syntax->match(); 
		}
		if (syntax->Sentence[syntax->Start]=='>'){
			syntax->match(); 
			token->Token.Set(syntax->Sentence+keep.Start+1, syntax->Start-keep.Start-2, true); 
			token->TokenText.Set(syntax->Sentence+keep.Start, syntax->Start-keep.Start, false); 
			token->TokenType=RegExSyntax::synRegQL_TAG;
			createRegExSubToken(token);
		}else{
			syntax->copyFrom(&keep);
			return -1;
		}
	}else{
		return RegExParser::parseRegExToken(syntax, token); 
	}
	return 0;
}

int TokenRegExParser::tokenizeText(const char* text, RegExToken* start, long limit){
	RegExToken* token=start;
	char* p=(char*) text;
	char* tag_start=0; 
	long i=0;
	long n_tokens = 0;
	for (p=(char*) text; *p && (limit<0 || n_tokens<limit ); p++){
		if (*p=='<') tag_start=p; 
		else if (*p=='>' && tag_start){
			token=token->addNextToken(new RegExToken);
			token->Token.Set(tag_start+1, p-tag_start-1, true); 
			token->TokenText.Set(tag_start, p-tag_start+1, false);
			token->Index=i++;
			n_tokens ++;
			tag_start=0; 
		}else if (! (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n')  && !tag_start){
			token=token->addNextToken(new RegExToken);
			token->Token.Set(p, 1, true); 
			token->TokenText.Set(p, 1, false); 
			token->Index=i++; 
			n_tokens ++;
		}
	}
	token=token->addNextToken(new RegExToken);
	token->Token.Set(p, 0, false); 
	token->TokenText.Set(p, 0, false);
	token->Index=i++;
	return 0;
}
int TokenRegExParser::matchedRegTokenText(RegExToken* text_token, RegExToken* reg_token){ //return true or false
	if (reg_token->TokenType==RegExSyntax::synRegQL_TAG){
		RegExParser parser; 
		return parser.matchRegExSubToken(text_token, reg_token);
	}else{
		return matchCmpChar(text_token->Token.P[0], reg_token->Token.P[0]);
	}
	return 0;
}
RegExSyntax* TokenRegExParser::newRegExSyntax(){
	return new TokenRegExSyntax;
}

//========== ListRegExParser ==============================================

int ListRegExParser::parseRegExToken(RegExSyntax* syntax, RegExToken* token){
	if (syntax->Sentence[syntax->Start]=='<'){
		RegExSyntax keep;
		keep.copyFrom(syntax); 
		RegExToken* token1=token;
		long token1_start=syntax->Start; 
		while (syntax->Type!=QLSyntax::synQL_END && syntax->Sentence[syntax->Start]!='>'){
			syntax->match(); 
			if (syntax->Sentence[syntax->Start]=='/'){
				if (token1_start!=keep.Start){
					token1=token1->addExtendedToken(new RegExToken);
				}
				token1->Token.Set(syntax->Sentence+token1_start+1, syntax->Start-token1_start-1, true); 
				token1->TokenText.Set(syntax->Sentence+token1_start, syntax->Start-token1_start+1, false); 
				token1->TokenType=RegExSyntax::synRegQL_TAG;
				token1_start=syntax->Start;
				createRegExSubToken(token1);
			}
		}
		if (syntax->Sentence[syntax->Start]=='>'){
			if (token1_start!=keep.Start){
				token1=token1->addExtendedToken(new RegExToken);
			}
			token1->Token.Set(syntax->Sentence+token1_start+1, syntax->Start-token1_start-1, true); 
			token1->TokenText.Set(syntax->Sentence+token1_start, syntax->Start-token1_start+1, false); 
			token1->TokenType=RegExSyntax::synRegQL_TAG;
			token1_start=syntax->Start;
			createRegExSubToken(token1);

			syntax->match(); 
		}else{
			syntax->copyFrom(&keep);
			return -1;
		}
	}else{
		return RegExParser::parseRegExToken(syntax, token); 
	}
	return 0;
}

int ListRegExParser::tokenizeText(const char* text, RegExToken* start, long limit){
	RegExToken* token=start;
	char* p=(char*) text;
	char* tag_start=0; 
	RegExToken* token1=0;
	int is_special=false;
	long index=0;
	for (p=(char*) text; *p && (limit<0 || index<limit) ; p++){
		if (*p=='\\' && !is_special) {
			is_special=true;
		}else{
			if (*p=='<' && !is_special) {
				tag_start=p; 
			}else if ((*p=='>'||*p=='/') && !is_special && tag_start){
				if (*p=='>'){
					token=token->addNextToken(new RegExToken);
					token1=token;
				}else if (*p=='/' && token1){
					token1=token1->addExtendedToken(new RegExToken);
				}
				if (token1){
					token1->Token.Set(tag_start+1, p-tag_start-1, true); 
					token1->TokenText.Set(tag_start, p-tag_start+1, false); 
					token1->Index=index;
				}
				if (*p=='>'){
					tag_start=0; 
					index++;
				}else{
					tag_start=p; 
				}
			}else if (! (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n')  && !tag_start){
				token=token->addNextToken(new RegExToken);
				token->Token.Set(p, 1, true); 
				token->TokenText.Set(p, 1, false); 
				token->Index=index++; 
			}
			is_special=false;
		}
	}
	token=token->addNextToken(new RegExToken);
	token->Token.Set(p, 0, false); 
	token->TokenText.Set(p, 0, false);
	token->Index=index;
	return 0;
}
int ListRegExParser::matchedRegTokenText(RegExToken* text_token, RegExToken* reg_token){ //return true or false
	if (reg_token->TokenType==RegExSyntax::synRegQL_TAG){
		int c=matchRegExSubToken(text_token, reg_token);
		if (c) return c; 
		while (text_token->ExtendNext && reg_token->ExtendNext){
			text_token=text_token->ExtendNext;
			reg_token=reg_token->ExtendNext;
			c=matchRegExSubToken(text_token, reg_token);
			if (c) return c; 
		}
		if (!text_token->ExtendNext && !reg_token->ExtendNext) return 0;
		else if (!text_token->ExtendNext) return -1;
		else if (!reg_token->ExtendNext) return 1;
		else return 0;
		//return (!text_token->ExtendNext && !reg_token->ExtendNext);
	}else{
		return matchCmpChar(text_token->Token.P[0], reg_token->Token.P[0]);
	}
	return 0;
}
RegExSyntax* ListRegExParser::newRegExSyntax(){
	return new TokenRegExSyntax;
}

