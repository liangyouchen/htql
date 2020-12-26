#ifndef CLY_HT_BROWSER_H_20030108
#define CLY_HT_BROWSER_H_20030108

#include "htmlbuf.h"
#include "txtset.h"
#include "referdata.h"
#include "referlink.h"
#include "htmlql.h"

extern int HtBrowserLogNavigationPages;

class HtBrowserCookieItem{
public:
	ReferData Host;
	ReferData Type;
	time_t UpdateTime;
	ReferData Name;
	ReferData Value;

	HtBrowserCookieItem();
	~HtBrowserCookieItem();
	void reset();
};

class HtBrowser{
public:
	HtmlBuffer Html;
	ReferData TmpPath;
	ReferData SessionTable;
	int HasSessionTable;
	ReferData Url;
	ReferData HostUrl;

	ReferData SessionID; 
	TxtSet db;

	ReferLink SessionCookies; //Pointing to HtBrowserCookieItem*, set the SessionID to null and use in memory SessionCookies;

	int MaxRefreshWaitSeconds;
	int MaxAutoNavigateSeconds;
	int Method;
	int IsMethodSet;
	enum {navNONE, navPREPARE_URL, navFETCHING_DATA, navERROR, navCOMPLETED};
	int NavigationStatus;

	HtBrowser();
	~HtBrowser();
	void reset();
	int setCookiePath(const char* cookie_path);

	int setUrl(const char* url);	//reset Variables; Method is GET; HostUrl is valued
	int setVariable(const char* name, const char* value); //Method is changed to POST
	int setHeader(const char* name, const char* value); 
	int setCookie(const char* name, const char* value, const char* host_url=0);
	int getCookie(const char* name, ReferData* value, ReferLinkHeap* all_cookies=0);
	int setMethod(int method);
	int navigate();
	int stopNavigation();
	int clickItem(const char* item_htql);

	int useForm(int form_index, const char* html=0);	//index from 1; if html=0 use browser page
	int useForm(const char* form_htql, const char* html=0);

	HTQL* getHtql();
	char* getGetUrl();
	int getHostUrl(const char* url, ReferData* siteurl);
	char* getContentData();
	long getContentLength();
	char* getBufferData();
	long getBufferLength();

	HtmlQL htql;
	int setQuery(const char* Query, unsigned int* Length=0);
	int dotQuery(const char* Query, unsigned int* Length=0);
	int isEOF();
	char* moveFirst();
	char* moveNext();
	char* getValue(int index=0);
	char* getValue(const char* name);

	int getFieldsCount();
	char* getFieldName(int index);

//	void* scriptCommand(char* command, void* arg1=0, void* arg2=0, void* arg3=0, void* arg4=0);

protected:
	int saveHtmlCookies(HtmlBuffer* html);
	char* getHttpVar(const char* header, const char* name, char** position=0);
	char* getHttpVarValue(const char* var);
	char* getHttpVarAttrValue(const char* var, const char* attrname, char** position=0);
protected:
	tStack Variables;
	tStack Cookies;

	int StopFlag;
	ReferData GetHttpVar;
	ReferData GetHttpVarValue;
	ReferData GetHttpVarAttrValue;
};


#endif

