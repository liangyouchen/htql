#ifndef HTML_BUFFER_H
#define HTML_BUFFER_H

#include "bindata.h"
#include "stack.h"
#include "port.h"

#define SUCCESS  0
#define tagMEMORY	-2001
#define htmlINVALIDSTR  -2002
#define htqlMEMORY		-2003
#define htqlSYNTAXERR	-2004

#define MAX_HOME_LENGTH	120
class ReferLinkHeap;
class tExprCalc;

///////////////////////////////////////////////////////////////////
//
//        class TagOperation
//
////////////////////////////////////////////////////////////////////

class TagOperation {
public:


	static int isTagNameChar(char ch); 
				//| true: if is alpha-cumeric or under score or ':'
				//| false: if is not
				//-------------------------

	static int isBStr(const char* BStr1, const char* BStr2); 
				// compare string ends with blank
				//| 0: if is not equal 
				//| length: if equal 
				//--------------------------------
	static int isTagNameStr(const char* AN_Str1, const char* AN_Str2);

	static int isAN_Str(const char* AN_Str1, const char* AN_Str2); 
				//| compare string ends with non alpha-numeric and underscore '_'
				//| 0: if is not equal without case
				//| length: if equal without case
				//--------------------------------

	static unsigned int getAttributeLength(const char* Attr);
				// Length of Attribute to the end of its value

	static char* targetAttribute(const char* Tag, const char* Attr, unsigned int* Length=NULL);
				// return position of Attribute and Length

	static char* targetValue(const char* Attr, unsigned int* Length=NULL, int* Quotation=NULL);
				//| return pointer to the value of attribute, not quotation
				//| if there is quotation, Quotation is the '\'' or '"', 
				//| else Quotation is 0
				//| Length is the value length, not include quotation

				//--------------------------------------------
	static int isStrNoCase(const char* Str1, const char* Str2, unsigned int Length);

	static int isTag(const char* Tag, const char* Name);
				//| compare Tag and Name without case.  Name can start with/without '</'
				//| Tag is pointed to a '<'
				//------------------------------------

	static int isTags(const char* Tag, const char** Names);
				//| compare Tag and Names without case.  
				//| Tag is pointed to a '<'
				//| return index 0-n for success or -1 for error
				//------------------------------------
	static int isTags(const char* Tag, ReferLinkHeap* Names);
				//| compare Tag and Names.Name without case.  
				//| Tag is pointed to a '<'
				//| return index 0-n for success or -1 for error
				//------------------------------------

	static int isEndTags(const char* Tag, ReferLinkHeap* Names);
				//| compare Tag and Names without case.  
				//| Tag is pointed to a '</'
				//| return index 0-n for success or -1 for error
				//------------------------------------
	static int isEndTag(const char* Tag, const char* Name);
				//| compare Tag and Name without case.  
				//| Tag is pointed to a '</'
				//------------------------------------

	static int isTag(const char* Tag);
				// it is '<' and tag or '</' and tagname

	static int isAttributeValue(const char* Tag, const char* Attr, const char* Value, unsigned int Length);
				// is Value equal to value of Attr?

	static int likeAttributeValue(const char* Tag, const char* Attr, const char* Pattern, unsigned int Length);
				// is value of Attr like Value?

	static unsigned int getTagLength(const char* Tag);
				// the length from '<' to '>'

	static char* getLabel(const char* Tag, unsigned int* LabelLength=0);
				// the label of tag.

	static char* nextTag(const char* start, unsigned int LengthLimit=0);
	static char* lastTag(const char* start, unsigned int LengthLimit=0);

	static char* findEntity(const char* start, const char* TagName, unsigned int LengthLimit=0);
	static long searchHtmlDateStr(const char* html, long len, ReferLinkHeap* dates, int search_days=0);

	static unsigned int getEntityLength(const char* Entity, unsigned int LengthLimit=0);
				//| from the start of entity '<' 
				//| to the end of endtity '</' 
				//| or next start point '<'
				//| or the end of text
				//---------------------------------------------

	tBinData ToTextResult;
	char* toText(const char* Entity, unsigned int* TextLength=0, unsigned int LengthLimit=0);

	static char firstNonTagCharAfter(char* base, long from_pos, long max_pos, long* nontag_pos=0);
	static char firstNonTagCharBefore(char* base, long from_pos, long min_pos, long* nontag_pos=0);

	TagOperation();

	~TagOperation();

};

///////////////////////////////////////////////////////////////////
//
//        class HtmlBuffer
//
////////////////////////////////////////////////////////////////////


extern tExprCalc* HttpsContext;

class HtmlBuffer{
public:
	tBinData Buffer;
	char url_home[MAX_HOME_LENGTH];

	HtmlBuffer();
	~HtmlBuffer();
	void reset();

	tBinData Url;
	enum {GET, POST};
	int Method;
	int IsMethodSet;
	tStack Variables;
	tStack Cookies;
	tStack Args;
	tStack HttpHeaders;
	tBinData AgentName;
	enum {ptHTTP, ptHTTPS, ptFTP, ptFILE};
	int ProtType; 
	
	int setUrl(const char* url);	//reset Variables and Cookies; init Method as POST
	int setVariable(const char* name, const char* value);
	int setCookie(const char* name, const char* value);
	int setHeader(const char* name, const char* value); 
	int setArg(const char* arg);
	int setMethod(int method);
	char* getGetUrl();

	int fetchHtml(const char* url);		//actually fetch data to Buffer
		//includes: fetchHtmlHeader + fetchHtmlContent
	int stopFetchHtml();

	char* getContentType();
	long getContentLength();
	char* getContentVar(const char* name);
	char* getContentHeader();
	char* getContentData();
	int getHttpResponseStatus();
	char* getHttpUrl();
	int getHttpMethod();

	int saveFileAll(const char* filename);
	int saveFileContent(const char* filename);
	int saveToFile(const char* data, size_t len, const char* filename);

	int fetchHtmlHeader(const char* url);
			//includes: getHttpRequest + connectHost + prefetch
	int getHttpRequest(const char* url, char** host=0, long* port=0, char** request=0);
			//an HTTP statement
			//GET: include URL and COOKIES
			//POST: include URL, VARIABLES, andd COOKIES
	int connectHost(const char* host, long port, int session_type); 
	int prefetch(const char* request);
			//fetch the first response
			//extract: ContentType, ContentLength, ContentFrom

	int fetchHtmlContent(int to_close=true);	//fetch to Buffer
	int fetchHtmlContent(FILE* fw, int to_close=true);	//fetch to file (when data is large)
	int checkContentType(); 

	char* encodeUrl(char* url);
	char* decodeUrl(char* url);
	char* encodeGetUrl(char* url);
	char* decodeHtml(char* text);

	int TimeoutConnect;
	int TimeoutTransfer;
	int TextOnly;
	long MaxSize;
	long ContentFrom;
	unsigned long ContentLength;
	tBinData ContentType;
	int ToFindHtml;

	tPort* Server;
	int IsNewServer;
	int setServerPort(tPort* port);
	int resetServerPort();
	int newServerPort(); 

protected:
	int StopFlag;
	tBinData EncodeGetUrl;
	tBinData EncodeUrl;
	tBinData DecodeHtml;
	tBinData GetContentVar;
	tBinData GetGetUrl;
	tBinData GetHttpRequest;
	tBinData GetHttpRequestHost;
	tBinData GetHttpUrl; 
	long GetHttpRequestPort;
	int GetHttpRequestSessionType;
	tBinData GetContentHeader;
};


#endif
