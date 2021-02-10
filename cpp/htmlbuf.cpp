#include "htmlbuf.h"
#include "stroper.h"
#include "port.h"
#include "log.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "qhtql.h"
#include "expr.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define DEBUG_FILE_OUTPUT


tExprCalc* HttpsContext=0;
ReferData HttpsUserAgent;

///////////////////////////////////////////////////////////////////
//
//        class TagOperation
//
////////////////////////////////////////////////////////////////////

TagOperation::TagOperation(){
}

TagOperation::~TagOperation(){
}

int TagOperation::isTagNameChar(char ch){
	return (isalpha(ch) || isdigit(ch) || ch == '_' || ch == ':' || ch == '-');
}

int TagOperation::isBStr(const char* BStr1, const char* BStr2){
	int i;
	for (i=0;!tStrOp::isBlank(BStr1[i]) && !tStrOp::isBlank(BStr2[i]); i++){
		if (toupper(BStr1[i]) != toupper(BStr2[i]) ) break;
	}
	return (tStrOp::isBlank(BStr1[i]) && tStrOp::isBlank(BStr2[i]) )?i:0;
}

int TagOperation::isAN_Str(const char* AN_Str1, const char* AN_Str2){
	int i;
	for (i=0; tStrOp::isAN_(AN_Str1[i]) && tStrOp::isAN_(AN_Str2[i]); i++){
		if (toupper(AN_Str1[i]) != toupper(AN_Str2[i]) ) break;
	}
	return (!tStrOp::isAN_(AN_Str1[i]) && !tStrOp::isAN_(AN_Str2[i]) )?i:0;
}
int TagOperation::isTagNameStr(const char* AN_Str1, const char* AN_Str2){
	int i;
	for (i=0; isTagNameChar(AN_Str1[i]) && isTagNameChar(AN_Str2[i]); i++){
		if (toupper(AN_Str1[i]) != toupper(AN_Str2[i]) ) break;
	}
	return (!isTagNameChar(AN_Str1[i]) && !isTagNameChar(AN_Str2[i]) )?i:0;
}

unsigned int TagOperation::getAttributeLength(const char* Attr){

	unsigned int i=0;
	int quote=0;
	char* Val=targetValue(Attr, &i, &quote);

	if (quote) i++;
	i+= Val-Attr;

	return i;
}

char* TagOperation::targetValue(const char* Attr, unsigned int* Length, int* Quotation){
	unsigned int i=0;

	//skip attribute name
	while (isTagNameChar(Attr[i])) i++;

	// find '='
	while (Attr[i] && Attr[i]!= '>' && Attr[i]!= '=') i++;
	if (Attr[i] == '=') i++;
	while (tStrOp::isBlank(Attr[i]) > 0) i++;

	//find quotation
	char quote=0;
	if (Attr[i] == '\'' || Attr[i] == '"') quote = Attr[i++];

	//skip the value
	unsigned int pos=i;
	while (Attr[i] && ( (quote && Attr[i]!=quote) || (!quote && Attr[i]!='>' && !tStrOp::isBlank(Attr[i]) ) ) ) i++;
	if (Length) *Length = i-pos;
	if (Quotation) *Quotation=quote;
	return (char*)Attr+pos;
}

char* TagOperation::targetAttribute(const char* Tag, const char* Attr, unsigned int* Length){
	char* p=(char*) Tag;
	if (*p == '<') p++;

	// skip tag name
	while ( isTagNameChar(*p) ) p++;

	unsigned int i;
	while (1){
		//skip blank & find attribue name
		while (*p && *p!='>' && !isTagNameChar(*p) ) p++;
		if (!(*p) || *p == '>') break;
		i=isTagNameStr(p, Attr);
		if (i) {
			if (Length)	*Length=i;
			return p;
		}

		int j= getAttributeLength(p);
		p+=j;
	}
	return NULL;
}


int TagOperation::isStrNoCase(const char* Str1, const char* Str2, unsigned int Length){
	if (!Str1 || !Str2){
		if (!Str1 && !Str2 && !Length) return true;
		else return false;
	}
	unsigned int i;
	for (i=0; i<Length; i++){
		if (Str1[i]=='\0' || Str2[i]=='\0' || toupper(Str1[i])!=toupper(Str2[i])) break;
	}
	return (i==Length || toupper(Str1[i])==(Str2[i]));
}

int TagOperation::isTag(const char* Tag, const char* Name){
	if (!Tag || *Tag != '<' || !Name) return false;
	if (Name && *Name=='<'){
		if (Name[1]=='/')
			return (Tag[1]=='/' && isTagNameStr(Tag+2, Name+2));
		else 
			return isTagNameStr( Tag+1, Name+1);
	}else 
		return isTagNameStr( Tag+1, Name);
}
int TagOperation::isTags(const char* Tag, const char** Names){
	int i=0;
	for (i=0; Names[i]; i++){
		if (isTag(Tag, Names[i])) return i;
	}
	return -1;
}
int TagOperation::isTags(const char* Tag, ReferLinkHeap* Names){
	ReferLink* link;
	int i=0;
	for (link=Names->getReferLinkHead(); link; link=link->Next){
		if (isTag(Tag, link->Name.P)) return i;
		i++;
	}
	return -1;
}
int TagOperation::isEndTags(const char* Tag, ReferLinkHeap* Names){
	ReferLink* link;
	int i=0;
	for (link=Names->getReferLinkHead(); link; link=link->Next){
		if (isEndTag(Tag, link->Name.P)) return i;
		i++;
	}
	return -1;
}

int TagOperation::isEndTag(const char* Tag, const char* Name){
	if (!Tag || Tag[0] != '<' || Tag[1]!='/') return false;
	return (isTagNameStr( Tag+2, Name));
}

int TagOperation::isTag(const char* Tag){
	return (Tag && *Tag=='<' && (isTagNameChar(Tag[1]) || (Tag[1]=='/' && isTagNameChar(Tag[2]) ) ));
}

int TagOperation::isAttributeValue(const char* Tag, const char* Attr, const char* Value, unsigned int Length){
	char* TagAttr=targetAttribute(Tag, Attr);
	if (!TagAttr) return false;
	
	unsigned int TagValueLength=0;
	char* TagValue = targetValue(TagAttr, &TagValueLength);
	if (!TagValue) return false;

	return (Length==TagValueLength && isStrNoCase(Value, TagValue, Length));
}

int TagOperation::likeAttributeValue(const char* Tag, const char* Attr, const char* Pattern, unsigned int Length){

	return false;
}

char* TagOperation::getLabel(const char* Tag, unsigned int* LabelLength){
	if (LabelLength) *LabelLength=0; 
	if (!Tag) return 0;

	unsigned int i=0;
	if (*Tag == '<' ) i++;
	if (Tag[i] == '/') i++;

	char* p=(char*) Tag+i;

	for (i=0; isTagNameChar(p[i]) ;i++);
	if (LabelLength) *LabelLength = i;
	return p;
}


unsigned int TagOperation::getTagLength(const char* Tag){
	int quote=0;
	unsigned int i=0;
	while (1){
		if (!quote && (Tag[i] == '\'' || Tag[i] == '"')) quote = Tag[i];
		else if (quote && Tag[i] == quote) quote=0;
		else if (!quote && Tag[i]=='>' ) {
			i++;
			break;
		}
		else if (!Tag[i]) break;
		i++;
	}
	return i;
}

char* TagOperation::nextTag(const char* start, unsigned int LengthLimit){
	int quote=0;
	int intag=0;
	unsigned int i=0;
	char* p=(char*) start;
	if (*p == '<' ) {
		intag=1;
		i++;
		p++;
	}
	for (; !LengthLimit || i<LengthLimit; i++){
		if (intag && !quote && (*p == '\'' || *p == '"')) quote = *p;
		else if (intag && quote && *p == quote) quote=0;
		else if (!(*p)) break;
		else if (!quote && *p == '<' ){
			intag=1;

			if (!tStrOp::isBlank(p[1]) ) return p;
	
		}else if (!quote && *p == '>' ){
			intag=0;
		}
		p++;
	}
	return NULL;
}

char* TagOperation::lastTag(const char* start, unsigned int LengthLimit){
	if (!start) return 0;
	int quote=0;
	int intag=0;
	unsigned int i=0;
	char* p=(char*) start;
	if (*p == '>' ) {
		intag=1;
	}else if (*p == '<') {
		intag=0;
	}
	i++;
	p--;
	for (; !LengthLimit || i<LengthLimit; i++){
		if (intag && !quote && (*p == '\'' || *p == '"')) quote = *p;
		else if (intag && quote && *p == quote) quote=0;
		else if (!(*p)) break;
		else if (!quote && *p == '>' ){
			intag=1;

		}else if (!quote && *p == '<' ){
			intag=0;

			if (!tStrOp::isBlank(p[1]) ) return p;
		}
		p--;
	}
	return NULL;
}


char* TagOperation::findEntity(const char* start, const char* TagName, unsigned int LengthLimit){
	int quote=0;
	int intag=0;
	char* p=(char*) start;
	unsigned int i;
	for ( i=0; !LengthLimit || i<LengthLimit; i++){
		if (intag && !quote && (*p == '\'' || *p == '"')) quote = *p;
		else if (intag && quote && *p == quote) quote=0;
		else if (!(*p)) break;
		else if (!quote && *p == '<' && !tStrOp::isBlank(p[1]) ){
			intag=1;
			if (p[1] != '/' && isTagNameStr(p+1, TagName)) return p;
	
		}else if (!quote && *p == '>' ){
			intag=0;
		}
		p++;
	}
	return NULL;
}

unsigned int TagOperation::getEntityLength(const char* Entity, unsigned int LengthLimit){
	int quote=0;
	int intag=1;
	char* p=(char*) Entity+1;  // skip '<'
	char* TagName= p;
	unsigned int i;
	for ( i=1; !LengthLimit || i<LengthLimit; i++){
		if (intag && !quote && (*p == '\'' || *p == '"')) quote = *p;
		else if (intag && quote && *p == quote) quote=0;
		else if (!(*p)) break;
		else if (!quote && *p == '<' && !tStrOp::isBlank(p[1]) ){
			intag=1;
			if ( (p[1] != '/' && isTagNameStr(p+1, TagName) )
				|| ( p[1] == '/' && isTagNameStr(p+2, TagName)) )
				break;
	
		}else if (!quote && *p == '>' ){
			intag=0;
		}
		p++;
	}
	return i;
}
long TagOperation::searchHtmlDateStr(const char* html, long len, ReferLinkHeap* dates, int search_days){
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(html, &len, false);

	long count=0;
	ReferLinkHeap text_all_pos; 
	text_all_pos.setSortOrder(SORT_ORDER_NUM_INC);
	ReferLink* link=0; 

	int IsSkip=false;
	ReferData text_all; 
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA && !IsSkip){
			//search date str in [DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen]
			count+=tStrOp::searchDateStr(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, dates, DataSyntax.Start); 
			if (text_all.L) text_all+=" ";

			link=text_all_pos.add((char*)0, (char*)0, text_all.L); 
			link->Value.L=DataSyntax.Start; 

			text_all.Cat(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen);
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
			if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "Script")) {
				IsSkip = true;
			}
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "Script")) {
				IsSkip = false;
			}
		}

		DataSyntax.match();
	}
	link=text_all_pos.add((char*)0, (char*)0, text_all.L); 
	link->Value.L=DataSyntax.Start; 

	if (!count && search_days && text_all.P){
		count+=tStrOp::searchDateText(text_all.P, text_all.L, dates, 0); 
		ReferLink search; 
		for (ReferLink* link1=dates->getReferLinkHead(); link1; link1=link1->Next){
			link=(ReferLink*) text_all_pos.moveFirstLarger((char*) link1);
			if (link && link->Data > link1->Data) link=(ReferLink*) text_all_pos.movePrevious();
			if (link) {
				link1->Data = link->Value.L + (link1->Data - link->Data); 
			}
		}
		dates->resort(); 
	}

	return count;
}
char TagOperation::firstNonTagCharAfter(char* base, long from_pos, long max_pos, long* nontag_pos){
	int in_tag=false;
	long i=from_pos;
	while (i<max_pos){
		if (!in_tag && !tStrOp::isSpace(base[i]) && base[i]!='&')
			break;
		else if (!in_tag && base[i]=='<') in_tag=true;
		else if (in_tag && base[i]=='>') in_tag=false;
		else if (base[i]=='&' && isStrNoCase(base+i+1, "nbsp", 4)){
			i+=4;
		}
		i++;
	}
	if (i<max_pos){
		if (nontag_pos)*nontag_pos=i;
		return base[i];
	}
	return 0;
}
char TagOperation::firstNonTagCharBefore(char* base, long from_pos, long min_pos, long* nontag_pos){
	int in_tag=false;
	long i=from_pos;
	while (i>=min_pos){
		if (!in_tag && !tStrOp::isSpace(base[i]) )
			break;
		else if (!in_tag && base[i]=='>') in_tag=true;
		else if (in_tag && base[i]=='<') in_tag=false;
		i--;
	}
	if (i>=min_pos){
		if (nontag_pos) *nontag_pos = i;
		return base[i];
	}
	return 0;
}

char* TagOperation::toText(const char* Entity, unsigned int* TextLength, unsigned int LengthLimit){
	int quote=0;
	int intag=1;
	char* textstart=NULL;
	
	ToTextResult.Clear();
	char* p=(char*) Entity+1;  // skip '<'
	char* TagName = p;
	unsigned int i;
	for (i=1; !LengthLimit || i<LengthLimit; i++){
		if (intag && !quote && (*p == '\'' || *p == '"')) quote = *p;
		else if (intag && quote && *p == quote) quote=0;
		else if (!(*p)) {
			
			
			if (textstart){
				ToTextResult.Cat(textstart, p-textstart);
			}
			break;

		}else if (!quote && *p == '<' ){
			intag=1;

			if (textstart){
				ToTextResult.Cat(textstart, p-textstart);
				ToTextResult.Cat(" ", 1);
			}

			if (p[1] != '/' && isTagNameStr(p+1, TagName)) break;
			else if (p[1] == '/' && isTagNameStr(p+2, TagName)) break;
	
		}else if (!quote && *p == '>' ){
			intag=0;

			textstart=p+1;
			ToTextResult.Cat(" ", 1);
		}
		p++;
	}
	tStrOp::replaceInplace(ToTextResult.Data,"&nbsp;"," ");
	tStrOp::replaceInplace(ToTextResult.Data,"&gt;",">");
	tStrOp::replaceInplace(ToTextResult.Data,"&lt;","<");
	tStrOp::replaceInplace(ToTextResult.Data,"&nbsp"," ");
	tStrOp::replaceInplace(ToTextResult.Data,"&gt",">");
	tStrOp::replaceInplace(ToTextResult.Data,"&lt","<");
	ToTextResult.DataLen = strlen(ToTextResult.Data);

	if (TextLength) *TextLength=ToTextResult.DataLen;
	return ToTextResult.Data;
}

///////////////////////////////////////////////////////////////////
//
//        class HtmlBuffer
//
////////////////////////////////////////////////////////////////////


HtmlBuffer::HtmlBuffer(){
	strcpy(url_home,"");
	Method=0;
	ContentLength=0;
	ContentFrom=0;
	IsMethodSet=false;
	StopFlag=false;
	TimeoutConnect=0;
	TimeoutTransfer=0;
	TextOnly=false;
	MaxSize=0;
	ToFindHtml=true;
	AgentName.SetValue("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)", strlen("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)") );
	IsNewServer=0;
	Server=0;
	GetHttpRequestPort=80;
	GetHttpRequestSessionType=tPort::ssNone;
	ProtType = ptHTTP; 
}

HtmlBuffer::~HtmlBuffer(){
	reset();
}

void HtmlBuffer::reset(){
	Buffer.Free();
	Variables.reset();
	Cookies.reset();
	HttpHeaders.reset();
	Args.reset();
	Url.Free();
	Method=0;
	ContentLength=0;
	ContentFrom=0;
	IsMethodSet=false;
	StopFlag=false;
	GetHttpRequestPort=80;
	GetHttpRequestSessionType=tPort::ssNone;
	//TimeoutConnect=0;
	//TimeoutTransfer=0;
	//TextOnly=false;
	//MaxSize=0;
	resetServerPort();
	ProtType = ptHTTP; 
}
int HtmlBuffer::resetServerPort(){
	if (IsNewServer && Server){
		delete Server;
	}
	IsNewServer=0;
	Server=0;
	return 0;
}
int HtmlBuffer::setServerPort(tPort* port){
	resetServerPort();
	Server=port;
	return 0;
}
int HtmlBuffer::newServerPort(){
	resetServerPort();
	Server=new tPort; 
	IsNewServer=true;
	return 0;
}
int HtmlBuffer::setUrl(const char* url){
	reset();
	Method = HtmlBuffer::GET;
	IsMethodSet = false;
	Url.SetValue(url, strlen(url));
	return 0;
}

int HtmlBuffer::setMethod(int method){
	IsMethodSet=true;
	Method=method;
	return 0;
}

int HtmlBuffer::setVariable(const char* name, const char* value){
	Variables.set(0, (char*) name, (char*) value);
	if (!IsMethodSet && !strchr(Url.Data, '?') ) Method=POST;
	return 0;
}
int HtmlBuffer::setCookie(const char* name, const char* value){
	Cookies.set(0, (char*) name, (char*) value);
	return 0;
}
int HtmlBuffer::setHeader(const char* name, const char* value){
	HttpHeaders.set(0, (char*) name, (char*) value);
	return 0;
}
int HtmlBuffer::setArg(const char* arg){
	Args.set(0, (char*) arg, "");
	return 0;
}

char* HtmlBuffer::getGetUrl(){
	GetGetUrl.SetValue(Url.Data, Url.DataLen);
	GetGetUrl.Cat("?", 1);

	tStack* cur;
	int arg_count=0;
	for (cur=Args.Next; cur; cur=cur->Next){
		if (!cur->Key) continue;
		if (arg_count ++)
			GetGetUrl.Cat("/", 1);
		GetGetUrl.Cat(cur->Key, strlen(cur->Key));
	}
	int var_count=0;
	for (cur=Variables.Next; cur; cur=cur->Next){
		if (!cur->Key) continue;
		if ((var_count++) || arg_count)
			GetGetUrl.Cat("&", 1);
		GetGetUrl.Cat(cur->Key, strlen(cur->Key));
		GetGetUrl.Cat("=",1);
		if (cur->Value){
			char* p=encodeUrl(cur->Value);
			GetGetUrl.Cat(p, strlen(p));
		}
	}
	return GetGetUrl.Data;
}

int HtmlBuffer::fetchHtml(const char* url1){
	int j=0;
	StopFlag=false;
	if ((j=fetchHtmlHeader(url1))<0) return j;
	if ((j=fetchHtmlContent())<0) return j;
	return SUCCESS;
}
int HtmlBuffer::stopFetchHtml(){
	if (Server) Server->stopPort();
	StopFlag=true;
	return SUCCESS;
}
int HtmlBuffer::fetchHtmlHeader(const char* url){
	int j=0;
	if ((j=getHttpRequest(url))<0) return j; //set GetHttpRequest,GetHttpRequestHost,GetHttpRequestPort
	if (ProtType == ptHTTP){ //not local files
		if ((j=connectHost(GetHttpRequestHost.Data, GetHttpRequestPort, GetHttpRequestSessionType))<0) return j;
		if ((j=prefetch(GetHttpRequest.Data))<0) return j;
	}
	return SUCCESS;
}
int HtmlBuffer::connectHost(const char* host, long port, int session_type){
	int j;
	if (!port) {//is local file
		return SUCCESS;
	}
	if (!Server){
		Server=new tPort; 
		IsNewServer=true;
	}
	if ((j=Server->setHost((char*) host))<0) return j;
	if ((j=Server->setPort(port, session_type))<0) return j;
	if (TimeoutConnect){
		Server->setTimeout(TimeoutConnect, TimeoutConnect, TimeoutConnect); 
	}
	j=Server->Connect();
	return j; 
}
int HtmlBuffer::checkContentType(){
	if (!Buffer.Data) return 0; 
	char *p1=tStrOp::strNstr(Buffer.Data, "Content-Length:", false);
	/*while (!tStrOp::strNcmp(Buffer.Data, "POST ", 5, false) && !p1){
		Server->put("\r\n"); 
		if ((j=Server->get())<0) return j;
		p1=tStrOp::strNstr(Buffer.Data, "Content-Length:", false);
	}*/
	if (p1) {
		p1+=15;
		sscanf(p1, "%ld", &ContentLength);
		char* p2=strstr(p1, "\r\n\r\n");
		if (p2) ContentFrom = p2-Buffer.Data+4;
		//else ContentFrom = p1-Buffer.Data +6;

		//data_length -= ContentFrom;
//Log::add("log.log",data_length,"First data",content_length);
	}
	char* p=getContentType();
	if (!p1){ //no "Content-Length:"
		if (p || !tStrOp::strNcmp(p, "TEXT", 4, false)){
			p=strstr(Buffer.Data,"\r\n\r\n");
			if (p) {
				ContentFrom = p-Buffer.Data + 4; 
				ContentLength=Buffer.DataLen-ContentFrom;
			}
		}
	}
	return 0; 
}

int HtmlBuffer::prefetch(const char* request){
	ContentLength=0;
	ContentFrom=0;
	Buffer.Free();
	int j;
	if (TimeoutConnect){
		Server->setTimeout(TimeoutConnect, TimeoutConnect); 
	}
	if (request) {
		if ((j=Server->put((char*)request))<0) return j;
	}
	if ((j=Server->get())<0) return j;

	if (TimeoutTransfer){
		Server->setTimeout(TimeoutTransfer, TimeoutTransfer);
	}
//Log::add("log.log",j,Server->Result?Server->Result:(char*)"---",1);
//	page="</html>";
//	long data_length=0;
	if (Server->Result){
		Buffer.SetDataMem(Server->Result, Server->Count);
		Server->Result = 0;
		Server->Count = 0;
		while (1){
			//Buffer.SetValue(Server->Result, Server->Count);
			//data_length=i=Server->Count;
			checkContentType();
			if (!tStrOp::strNcmp(Buffer.Data, "POST ", 5, false) && !ContentLength){
				if ((j=Server->get())<0) return j;
			}else if (!tStrOp::strNcmp(Buffer.Data, "GET ", 4, false) && Buffer.Data[Buffer.DataLen-1]!='\n'){
				if ((j=Server->get())<0) return j;
			}else{
				break;
			}
			Buffer.Cat(Server->Result, Server->Count);
		}
//		if (!ContentLength) 
//			page = TagOperation::lastTag(Buffer.Data + i, i);
	}
	return Buffer.DataLen?0:1;
}

int HtmlBuffer::fetchHtmlContent(int to_close){

	int j;
	if (ProtType == ptHTTPS && HttpsContext ){
		tExprCalc expr(HttpsContext); 
		expr.setExpression("https(url, header, HttpsUserAgent)");
		expr.setVariable("url", Url.Data, Url.DataLen, false);
		if (GetHttpRequest.Data){
			expr.setVariable("header", GetHttpRequest.Data, GetHttpRequest.DataLen, false);
		}else{
			expr.setVariable("header", "", 0, true);
		}
		expr.setVariable("HttpsUserAgent", HttpsUserAgent.P?HttpsUserAgent.P:"", HttpsUserAgent.L, true);
		expr.parse();
		expr.calculate();
		char* content = expr.getString();
		if (content){
			Buffer.SetValue(content, expr.Length);
		}else{
			Buffer.SetValue("",0);
		}
		ContentFrom = 0; 
		checkContentType();
		ContentLength=Buffer.DataLen;
		return SUCCESS;
	}
	if (!GetHttpRequestPort && GetHttpRequest.Data){
		char buf[2048];
		FILE* fw=fopen(GetHttpRequest.Data, FILE_READ);
		if (!fw ) return htmlINVALIDSTR;
		int len;
		while ((len=fread(buf, 1, 1024, fw))>0){
			Buffer.Cat(buf, len);
		}
		fclose(fw);
		ContentLength = Buffer.DataLen;

		/*
		char buf[2048];
		int FileHandle=-1;
		FileHandle=open(GetHttpRequest.Data,O_RDONLY);
		if (FileHandle < 0) return htmlINVALIDSTR;
		ContentLength=0;
		Buffer.SetValue(" ",0);
		while ((len=read(FileHandle,buf,2048))>0) {
			Buffer.Cat(buf, len);
		}
		close(FileHandle);
		ContentLength = Buffer.DataLen;
		*/

		if (len<0) return htmlINVALIDSTR;
		return SUCCESS;
	}
	char* page="<html>";
	if (!ContentLength && ToFindHtml) 
		page = TagOperation::lastTag(Buffer.Data + Buffer.DataLen, Buffer.DataLen);

	long i;
	while (
			(ContentLength && (!ContentFrom || (unsigned long) Buffer.DataLen<ContentLength+ContentFrom))
			|| (!ContentLength && ToFindHtml && (!page || page[1]!='/' || !TagOperation::isTagNameStr(page+2, "html")) ) 
			|| (!tStrOp::strNcmp(Buffer.Data, "POST ", 5) && !ContentLength)
		) {
		if (StopFlag) return -1;
		if (TextOnly && tStrOp::strNcmp(ContentType.Data, "text/plain", 10, false) 
			&& tStrOp::strNcmp(ContentType.Data, "text/html", 9, false) 
			&& tStrOp::strNcmp(ContentType.Data, "text/", 5, false) //?? 
			&& tStrOp::strNcmp(ContentType.Data, "application/xml", strlen("application/xml"), false) 
			&& tStrOp::strNcmp(ContentType.Data, "application/rss", strlen("application/rss"), false) 
			) {
			return -2;
		}
		if (MaxSize && (ContentLength>MaxSize || Buffer.DataLen>MaxSize)) {
#ifdef _DEBUG
			TRACE("Stop: ContentLength [%ld], or DataLen [%ld]>MaxSize [%ld]\r\n", ContentLength, Buffer.DataLen, MaxSize);
#endif 
			return -3;
		}

		j=Server->get();
//Log::add("log.log",j,Server->Result?Server->Result:(char*)"---",2);
		if (j>=0 && Server->Result) {
			Buffer.Cat(Server->Result, Server->Count);
			if (!tStrOp::strNcmp(Buffer.Data, "POST ", 5) && !ContentLength)
				checkContentType();
			
			if (ContentLength && !ContentFrom){
				char* p2=strstr(Buffer.Data, "\r\n\r\n");
				if (p2) ContentFrom = p2-Buffer.Data+4;
			}

			i=Server->Count;
//			data_length+=Server->Count;
//Log::add("log.log",data_length,"more data",content_length);
			if ((long)Buffer.DataLen < i) return htqlMEMORY;
//Log::add("log.log",i,"---",Buffer.DataLen);
			if (!ContentLength && ToFindHtml) 
				page = TagOperation::lastTag(Server->Result + i, i);
		}else{
			if (j<0) return j;
			break;
		//page="</html>";
		}
	} 
	//if (!ContentLength && Buffer.Data) ContentLength=strlen(Buffer.Data+ContentFrom);
	if (!ContentLength && Buffer.Data) {
		if (Buffer.DataLen>ContentFrom){
			ContentLength=Buffer.DataLen-ContentFrom;
		}else{
			ContentLength=strlen(Buffer.Data+ContentFrom);
		}
	}
	if (to_close) Server->reset();
//Log::add("log.log",Buffer.DataLen,"result data",content_length);
//Log::add("log.log",i,"---",4);

	return SUCCESS;
}

int HtmlBuffer::fetchHtmlContent(FILE* fw, int to_close){
	if (!fw) fw=stdout;
	int j;
	if (ProtType == ptHTTPS && HttpsContext ){
		tExprCalc expr(HttpsContext); 
		expr.setVariable("url", Url.Data, Url.DataLen, false);
		if (GetHttpRequest.Data){
			expr.setVariable("header", GetHttpRequest.Data, GetHttpRequest.DataLen, false);
		}else{
			expr.setVariable("header", "", 0, true);
		}
		expr.setVariable("HttpsUserAgent", HttpsUserAgent.P?HttpsUserAgent.P:"", HttpsUserAgent.L, true);
		expr.setExpression("https(url, header, HttpsUserAgent)");
		expr.parse();
		expr.calculate();
		char* content = expr.getString();
		if (content){
			Buffer.SetValue(content, strlen(content));
		}else{
			Buffer.SetValue("",0);
		}
		ContentFrom = 0; 
		checkContentType();
		ContentLength=Buffer.DataLen-ContentFrom;
		fwrite(Buffer.Data+ContentFrom, 1, Buffer.DataLen-ContentFrom, fw);
		return SUCCESS;
	}
	if (!GetHttpRequestPort && GetHttpRequest.Data){
		char buf[2048];
		int FileHandle=-1;
		FileHandle=open(GetHttpRequest.Data,O_RDONLY);
		if (FileHandle < 0) return htmlINVALIDSTR;
		ContentLength=0;
		while ((j=read(FileHandle,buf,2048))>0) {
			fwrite(buf, 1, j, fw);
			ContentLength+= j;
		}
		close(FileHandle);

		if (j<0) return htmlINVALIDSTR;
		return SUCCESS;
	}
	char* page="<html>";
	if (!ContentLength && ToFindHtml) 
		page = TagOperation::lastTag(Buffer.Data + Buffer.DataLen, Buffer.DataLen);

	fwrite(Buffer.Data+ContentFrom, 1, Buffer.DataLen-ContentFrom, fw);
	unsigned long LengthCount=Buffer.DataLen;
	long i;
	while ((ContentLength && (!ContentFrom || LengthCount<ContentLength+ContentFrom)) 
		|| (!ContentLength && ToFindHtml && (!page || page[1]!='/' 
			|| !TagOperation::isTagNameStr(page+2, "html"))
			) 
		) {
		if (StopFlag) return -1;
		if (TextOnly && tStrOp::strNcmp(ContentType.Data, "text/", 5, false) 
			&& tStrOp::strNcmp(ContentType.Data, "application/xml", strlen("application/xml"), false)
			&& tStrOp::strNcmp(ContentType.Data, "application/rss", strlen("application/rss"), false) 
			) 
			return -2;
		if (MaxSize && (ContentLength>MaxSize || Buffer.DataLen>MaxSize)) return -3;

		j=Server->get();
		if (j>=0 && Server->Result) {
			fwrite(Server->Result, 1, Server->Count, fw);
			LengthCount+= Server->Count;

			if (ContentLength && !ContentFrom){
				Buffer.Cat(Server->Result, Server->Count);
				char* p2=strstr(Buffer.Data, "\r\n\r\n");
				if (p2) ContentFrom = p2-Buffer.Data+4;
			}

			i=Server->Count;
			if (!ContentLength && ToFindHtml) 
				page = TagOperation::lastTag(Server->Result + i, i);
		}else{
			if (j<0) return j;
			break;
		}
	} 
	if (!ContentLength) ContentLength=strlen(Buffer.Data+ContentFrom);
	if (to_close) Server->reset();

	return SUCCESS;
}

int HtmlBuffer::getHttpRequest(const char* url1, char** host1, long* port1, char** request){
	char* url=(char*) url1;
	if (!url) {
		url=Url.Data;
		if (!IsMethodSet){
			if (!Variables.Next || strchr(url, '?') ) Method=GET;
			else Method=POST;
		}
		if (!url) return -1; 
	}else{
		if (Url.Data!=url) 
			Url.SetValue(url, strlen(url));
		Method=GET;
	}

	GetHttpRequest.Malloc(100);
	GetHttpRequestHost.Free();
	GetHttpRequestPort = 80;
	GetHttpRequestSessionType=tPort::ssNone;

	char host[256]="";
	char buf[2048]="";
	char filename[256];
	char* page="/";

	unsigned prot_offset = 7;
	strcpy(url_home, "");
	unsigned long i=0;
	long j;
	if (TagOperation::isStrNoCase(url,"HTTP://",7) ){
		prot_offset = 7;
		ProtType = ptHTTP; 
	}else if (TagOperation::isStrNoCase(url,"HTTPS://",8) ){
		prot_offset = 8;
		//GetHttpRequestPort = 443 ;
		GetHttpRequestSessionType=tPort::ssSSL;
		ProtType = ptHTTPS; 
	}else {
		ProtType = ptFILE;
		unsigned long Offset=0;
		if (TagOperation::isStrNoCase(url,"FILE://",7)){
			Offset=7;
#ifndef unix
			if (url[7]=='/') Offset=8;
#endif
		}
		for (j=Offset; url[j] && url[j]!='#' && url[j]!='?'; j++);
		strncpy(filename, url+Offset, j-Offset);
		if (j-Offset > 255) return htmlINVALIDSTR; 
		filename[j-Offset] = 0;
		if (Offset>0) {//file://
			char* p=decodeUrl(filename);
			strcpy(filename, p);
		}
		GetHttpRequest.SetValue(filename, strlen(filename));
		GetHttpRequestHost.SetValue("Localhost", strlen("Localhost"));
		GetHttpRequestPort = 0;

		for (j=strlen(filename)-1; j>=0; j--){
			if (filename[j]=='.') break;
		}
		if (j>=0){
			if (tStrOp::strNcmp((char*) filename+j, ".HTM", 4, false)==0)
				ContentType.SetValue("text/html", strlen("text/html"));
			else if (tStrOp::strNcmp((char*)filename+j, ".TXT", 4, false)==0) 
				ContentType.SetValue("text/plain", strlen("text/plain") );
			else 
				ContentType.SetValue("application/octetstream", strlen("application/octetstream"));
		}

		if (host1){
			*host1= GetHttpRequestHost.Data;
		}
		if (port1){
			*port1 = GetHttpRequestPort;
		}
		if (request){
			*request = GetHttpRequest.Data;
		}
		return SUCCESS;
	}
	for ( i=prot_offset; !tStrOp::isBlank(url[i]) && url[i]!='/' && url[i]!=':' && url[i]!='?'; i++);
	strncpy(host, url+prot_offset, i-prot_offset);
	host[i-prot_offset]='\0';

	if (url[i]==':' && isdigit(url[i+1]) ) {
		GetHttpRequestPort=atoi(url+i+1);
		i++;
		while (!tStrOp::isBlank(url[i]) && url[i]!='/' ) i++;
	}

	strncpy(url_home, url, i);
	url_home[i]=0;

	if (url[i] == '/' || url[i] == '?' ) page=(char*) url+i;

	page=decodeHtml(page);
	page=encodeGetUrl(page);

	tBinData params;
	params.Malloc(0);

	tStack* cur;
	int arg_count=0;
	for (cur=Args.Next; cur; cur=cur->Next){
		if (!cur->Key) continue;
		if (arg_count ++)
			GetGetUrl.Cat("/", 1);
		GetGetUrl.Cat(cur->Key, strlen(cur->Key));
	}
	int var_count=0;
	for (cur=Variables.Next; cur; cur=cur->Next){
		if (!cur->Key) continue;
		if ((var_count ++) || arg_count){
			params.Cat("&", 1);
		}
		sprintf(buf, "%s=", cur->Key);
		params.Cat(buf, strlen(buf));
		if (cur->Value){
			char* p=encodeUrl(cur->Value);
			params.Cat(p, strlen(p));
		}
	}

	if (Method == HtmlBuffer::GET){
		GetHttpRequest.SetValue("GET ", 4);
		if (page[0]=='?') GetHttpRequest.Cat("/", 1);
		GetHttpRequest.Cat(page, strlen(page) );

		if (!strchr(page,'?')){
			if (Variables.Next && Variables.Next->Key){
				GetHttpRequest.Cat("?", 1 );
				GetHttpRequest.Cat(params.Data, params.DataLen);
			}
		}else if (Variables.Next && Variables.Next->Key) {
			char* p=strchr(page, '?');
			if (p){
				char* p1, *p2, *p3, *p0=page, *p4;
				tBinData n, v;
				tStack url_names_list;
				GetHttpRequest.SetValue("GET ", 4);
				p1=p+1;
				while (p1 && *p1){
					p2=strchr(p1,'=');
					if (!p2) p2=p1+strlen(p1);
					n.SetValue(p1, p2-p1);
					if (*p2=='=') p2++;
					p3=strchr(p2, '&');
					if (!p3) p3=strchr(p2, '#');
					if (!p3) p3=p2+strlen(p2);
					v.SetValue(p2, p3-p2);

					if (Variables.search(n.Data, &p4)){
						GetHttpRequest.Cat(p0, p2-p0);
						if (p4){
							p4=encodeUrl(p4);
							GetHttpRequest.Cat(p4, strlen(p4));
						}
						p0=p3;
					}
					url_names_list.set(0, n.Data, v.Data);

					if (*p3=='&') p3++;
					else break;
					p1=p3;
				}
				if (*p0){
					GetHttpRequest.Cat(p0, strlen(p0));
				}
				for (cur=Variables.Next; cur; cur=cur->Next){
					if (!cur->Key) continue;
					if (url_names_list.search(cur->Key, 0)) continue;
					GetHttpRequest.Cat("&", 1);
					sprintf(buf, "%s=", cur->Key);
					GetHttpRequest.Cat(buf, strlen(buf));
					if (cur->Value){
						char* p=encodeUrl(cur->Value);
						GetHttpRequest.Cat(p, strlen(p));
					}
				}
			}
		}

		sprintf(buf," HTTP/1.0\r\n");
		GetHttpRequest.Cat(buf, strlen(buf));
		if (TextOnly){
			sprintf(buf, "Accept: Text/*\r\nAccept-Encoding: identity; q=1, *;q=0\r\n");
		}else{
			sprintf(buf, "Accept: */*\r\n");
		}
		GetHttpRequest.Cat(buf, strlen(buf));
		//sprintf(buf, "Accept-Language: en-us\r\nAccept-Encoding: gzip, deflate\r\n");
		//GetHttpRequest.Cat(buf, strlen(buf));

		sprintf(buf, "User-Agent: ");
		GetHttpRequest.Cat(buf, strlen(buf));
		GetHttpRequest.Cat(AgentName.Data, AgentName.DataLen); 
		sprintf(buf, "\r\n");
		GetHttpRequest.Cat(buf, strlen(buf));

		sprintf(buf,"HOST: %s\r\n", host);
		GetHttpRequest.Cat(buf, strlen(buf));
		//sprintf(buf,"Connection: close\r\n");
		//GetHttpRequest.Cat(buf, strlen(buf));

		tBinData cookies;
		for (cur=Cookies.Next; cur; cur=cur->Next){
			//sprintf(buf, "Set-Cookie: %s=%s\r\n", cur->Key?cur->Key:"", cur->Value?cur->Value:"");
			sprintf(buf, "%s=%s; ", cur->Key?cur->Key:"", cur->Value?cur->Value:"");
			cookies.Cat(buf, strlen(buf));
		}
		if (cookies.DataLen){
			GetHttpRequest.Cat("Cookie: ", strlen("Cookie: "));
			GetHttpRequest.Cat(cookies.Data, cookies.DataLen);
			GetHttpRequest.Cat("\r\n", 2);
		}
		tBinData header;
		for (cur=HttpHeaders.Next; cur; cur=cur->Next){
			sprintf(buf, "%s: %s\r\n", cur->Key?cur->Key:"", cur->Value?cur->Value:"");
			header.Cat(buf, strlen(buf));
		}
		if (header.DataLen){
			GetHttpRequest.Cat(header.Data, header.DataLen);
		}

		GetHttpRequest.Cat("\r\n",2);
	}else if (Method==HtmlBuffer::POST){
		GetHttpRequest.SetValue("POST ", 5);
		if (page[0]=='?') GetHttpRequest.Cat("/", 1);
		GetHttpRequest.Cat(page, strlen(page) );
		//sprintf(buf," HTTP/1.0\r\nHOST: %s\r\n", host);
		//GetHttpRequest.Cat(buf, strlen(buf));
		sprintf(buf," HTTP/1.0\r\n");
		GetHttpRequest.Cat(buf, strlen(buf));
		if (TextOnly){
			sprintf(buf, "Accept: Text/*\r\nAccept-Encoding: identity; q=1, *;q=0\r\n");
		}else{
			sprintf(buf, "Accept: */*\r\n");
		}
		GetHttpRequest.Cat(buf, strlen(buf));
		//sprintf(buf, "Accept-Language: zh-cn\r\nAccept-Encoding: gzip, deflate\r\n");
		//GetHttpRequest.Cat(buf, strlen(buf));
		sprintf(buf, "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)\r\n");
		GetHttpRequest.Cat(buf, strlen(buf));
		sprintf(buf,"HOST: %s\r\n", host);
		GetHttpRequest.Cat(buf, strlen(buf));
		//sprintf(buf,"Connection: close\r\n");
		//GetHttpRequest.Cat(buf, strlen(buf));

		sprintf(buf, "Content-Type: application/x-www-form-urlencoded\r\n");
		GetHttpRequest.Cat(buf, strlen(buf));

		sprintf(buf, "Content-Length: %ld\r\n", params.DataLen);
		GetHttpRequest.Cat(buf, strlen(buf));

		tBinData cookies;
		for (cur=Cookies.Next; cur; cur=cur->Next){
			//sprintf(buf, "Set-Cookie: %s=%s\r\n", cur->Key?cur->Key:"", cur->Value?cur->Value:"");
			//if (cookies.DataLen) cookies.Cat("; ",2);
			sprintf(buf, "%s=%s; ", cur->Key?cur->Key:"", cur->Value?cur->Value:"");
			cookies.Cat(buf, strlen(buf));
		}
		if (cookies.DataLen){
			GetHttpRequest.Cat("Cookie: ", strlen("Cookie: "));
			GetHttpRequest.Cat(cookies.Data, cookies.DataLen);
			GetHttpRequest.Cat("\r\n", 2);
		}
		tBinData header;
		for (cur=HttpHeaders.Next; cur; cur=cur->Next){
			sprintf(buf, "%s: %s\r\n", cur->Key?cur->Key:"", cur->Value?cur->Value:"");
			header.Cat(buf, strlen(buf));
		}
		if (header.DataLen){
			GetHttpRequest.Cat(header.Data, header.DataLen);
		}

		GetHttpRequest.Cat("\r\n",2);

		GetHttpRequest.Cat(params.Data, params.DataLen);
	}

	GetHttpRequestHost.SetValue(host, strlen(host));
	if (host1){
		*host1= GetHttpRequestHost.Data;
	}
	if (port1){
		*port1 = GetHttpRequestPort;
	}
	if (request){
		*request = GetHttpRequest.Data;
	}

#ifdef DEBUG_FILE_OUTPUT
	FILE* f;
	f=fopen("lasthttprequest.txt", FILE_WRITE);
	fwrite(GetHttpRequest.Data, GetHttpRequest.DataLen, 1, f);
	fclose(f);
#endif

#ifdef _DEBUG
	if (GetHttpRequest.Data && strlen(GetHttpRequest.Data)<128){
		TRACE("%s", GetHttpRequest.Data);
	}
#endif
	return 0;
}

int HtmlBuffer::getHttpResponseStatus(){
	if (!Buffer.Data) return 0;

	int status=0;
	char* p=Buffer.Data;
	if (!tStrOp::strNcmp(p, "HTTP/",5, false)){
		char* p1=p+5;
		for (int i=0; i<6; i++){
			if (!tStrOp::isSpace(*p1)) p1++;
			else break;
		}
		if (tStrOp::isSpace(*p1)){
			sscanf(p1+1, "%d", &status);
		}
	}
	return status;
}
int HtmlBuffer::getHttpMethod(){
	if (!Buffer.Data) return -1;
	char* p=Buffer.Data;
	if (!tStrOp::strNcmp(p, "GET",3, false) && p[3]==' '){
		return GET; 
	}else if (!tStrOp::strNcmp(p, "POST",4, false) && p[4]==' '){
		return POST; 
	}
	return -1; 
}

char* HtmlBuffer::getHttpUrl(){
	if (!Buffer.Data) return 0;

	char* p=Buffer.Data;
	if (!tStrOp::strNcmp(p, "GET",3, false)){
		p+=3; 
	}else if (!tStrOp::strNcmp(p, "POST",4, false)){
		p+=4; 
	}
	while (*p==' ' || *p=='\t') p++; 

	char*p1=strstr(p, "\r\n"); 
	if (p1) *p1=0;
	
	char*p2=tStrOp::strNrstr(p, "HTTP/"); 
	if (!p2) p2=p+strlen(p); 
	p2--; 
	while (p2>=p && (*p2==' ' || *p2=='\t')) p2--; 
	p2++; 

	GetHttpUrl.SetValue(p, p2-p); 
	if (p1) *p1='\r';

	return GetHttpUrl.Data; 
}

int HtmlBuffer::saveToFile(const char* data, size_t len, const char* filename){
	int f=1;
	if (filename && strcmp(filename,"-")){
		unlink(filename);
		f=open(filename, O_CREAT | O_WRONLY | O_BINARY, 0700);
		if (f<0) return f;
	}
//Log::add("log.log",Buffer.DataLen,"save data",ContentFrom);
	if (write (f, data, len)<0) return -1;
	if (f>1) {
		close(f);
		//commit(f);
	}
	return SUCCESS;
}

int HtmlBuffer::saveFileAll(const char* filename){
	return saveToFile(Buffer.Data, Buffer.DataLen, filename);
}

int HtmlBuffer::saveFileContent(const char* filename){
	if (!Buffer.Data) return 1;
	if (ContentFrom < (long) Buffer.DataLen){
		return saveToFile(Buffer.Data+ContentFrom, Buffer.DataLen-ContentFrom, filename);
	}else {
		return saveToFile(Buffer.Data, Buffer.DataLen, filename);
	}
}

char* HtmlBuffer::getContentType(){
	if (ContentLength==0) ContentType.SetValue("text/plain", strlen("text/plain"));
	else if (ContentLength<50000) ContentType.SetValue("text/plain", strlen("text/plain"));
	else ContentType.SetValue("binary/octet-stream", strlen("binary/octet-stream"));

	char *p1=tStrOp::strNstr(Buffer.Data, "Content-Type:", false);
	if (p1){
		p1+=13;
		while (tStrOp::isSpace(*p1)) p1++;
		char* p2=strchr(p1, '\r');
		if (!p2) p2=strchr(p1, '\n');
		if (p2) ContentType.SetValue(p1, p2-p1);
	}
	return ContentType.Data;
}

char* HtmlBuffer::getContentVar(const char* name){
	char tmp[256];
	sprintf(tmp, "\n%s:", name);
	char *p1=tStrOp::strNstr(Buffer.Data, tmp, false);
	GetContentVar.Malloc(128);
	if (p1 && (!ContentFrom || ContentFrom>p1-Buffer.Data)){
		p1+=strlen(tmp);
		while (tStrOp::isSpace(*p1)) p1++;
		char* p2=strchr(p1, '\r');
		if (!p2) p2=strchr(p1, '\n');
		if (p2) GetContentVar.SetValue(p1, p2-p1);
		return GetContentVar.Data;
	}else return 0;
}

char* HtmlBuffer::getContentHeader(){
	if (ContentFrom>0 && ContentFrom <= (long)Buffer.DataLen) {
		GetContentHeader.SetValue(Buffer.Data, ContentFrom);
	}else{
		GetContentHeader.Free();
	}
	return GetContentHeader.Data;
}

char* HtmlBuffer::getContentData(){
	if (ContentFrom < (long)Buffer.DataLen) return Buffer.Data + ContentFrom;
	else return Buffer.Data;
}

long HtmlBuffer::getContentLength(){
	if (ContentLength) return ContentLength;
	else if (ContentFrom < (long)Buffer.DataLen) return Buffer.DataLen - ContentFrom;
	else return Buffer.DataLen;
}

char* HtmlBuffer::decodeHtml(char* text){
	DecodeHtml.SetValue(text, strlen(text));
	tStrOp::replaceInplace(DecodeHtml.Data, "&nbsp;", " ");
	tStrOp::replaceInplace(DecodeHtml.Data, "&lt;", "<");
	tStrOp::replaceInplace(DecodeHtml.Data, "&gt;", ">");
	tStrOp::replaceInplace(DecodeHtml.Data, "&quot;", "\"");
	tStrOp::replaceInplace(DecodeHtml.Data, "&amp;", "&");
	return DecodeHtml.Data;
}

char* HtmlBuffer::encodeGetUrl(char* url){
	char buf[10];
	size_t len=strlen(url);
	EncodeGetUrl.Malloc(2*len);

	size_t i;
	for (i=0; i<len; i++){
		switch (url[i]){
		case '\r':
		case '\n':
		case '\t':
		case ' ':
			sprintf(buf, "%%%02X", url[i]);
			EncodeGetUrl.Cat(buf, 3);
			break;
//		case ' ':
//			EncodeGetUrl.Cat("+", 1);
//			break;
		default:
			EncodeGetUrl.Cat(url+i, 1);
		}
		if (EncodeGetUrl.DataLen+3 >= EncodeGetUrl.DataSize) EncodeGetUrl += len;
	}
	return EncodeGetUrl.Data;
}

char* HtmlBuffer::encodeUrl(char* url){
	//also see tStrOp::encodeUrl();
	char buf[10], buf1[10];
	size_t len=strlen(url);
	EncodeUrl.Malloc(2*len);

	size_t i;
	for (i=0; i<len; i++){
		if (0 && i<len-1 && (url[i]&0x80) && (url[i+1]&0x80) ){
			buf1[0]=url[i];
			buf1[1]=url[i+1];
			buf1[2]=0; 

			char*p=tStrOp::encodeUTF8(buf1);
			EncodeUrl.Cat(p, strlen(p)); 
			i++;
			free(p); 
		}else{
			switch (url[i]){
			case '\r':
			case '\n':
			case '%':
			case '&':
			case '?':
			case '#':
			case '\t':
			case ':':
			case '~':
			case '(':
			case ')':
			case '<':
			case '>':
			case '|':
			case ',':
			case '"':
			case '\'':
			case '\\':
				sprintf(buf, "%%%02X", url[i]);
				EncodeUrl.Cat(buf, 3);
				break;
			case ' ':
				EncodeUrl.Cat("+", 1);
				break;
			default:
				if (url[i]<=0){
					sprintf(buf, "%%%02X", (unsigned char) url[i]);
					EncodeUrl.Cat(buf, 3);
				}else{
					EncodeUrl.Cat(url+i, 1);
				}
			}
		}
		if (EncodeUrl.DataLen+3 >= EncodeUrl.DataSize) EncodeUrl += len;
	}
	return EncodeUrl.Data;
}

char* HtmlBuffer::decodeUrl(char* url){
	char buf[10];
	size_t len=strlen(url);
	EncodeUrl.Malloc(len);

	size_t i=0;
	while (i<len){
		switch (url[i]){
		case '%':
			buf[0]=buf[1]=0;
			sscanf(url+i+1, "%02X", buf);
			if (buf[0]){
				EncodeUrl.Cat(buf,1);
				i+=2;
			}else{
				EncodeUrl.Cat(url+i, 1);
			}
			break;
		case '+':
			EncodeUrl.Cat(" ", 1);
			break;
		default:
			EncodeUrl.Cat(url+i, 1);
		}
		if (EncodeUrl.DataLen+3 >= EncodeUrl.DataSize) EncodeUrl += len;
		i++;
	}
	return EncodeUrl.Data;
}
