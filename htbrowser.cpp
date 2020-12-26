#include "htbrowser.h"
#include "qhtmlql.h"

#define COOKIE_TABLE_NAME "HtBrowserC"
#define COOKIE_TABLE_CREATE "create table HtBrowserC (sessionid varchar(40), host varchar(256), type varchar(20), upd_time varchar(40), name varchar(100), value long)"
	// sessionid, host, type, upd_time, name, value

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define DEBUG_FILE_OUTPUT
int HtBrowserLogNavigationPages=1;
extern char AppCurrentDir[]; //defined in "log.cpp"

HtBrowserCookieItem::HtBrowserCookieItem(){
	UpdateTime=time(0);
}
HtBrowserCookieItem::~HtBrowserCookieItem(){
	reset();
}
void HtBrowserCookieItem::reset(){
	Host.reset();
	Type.reset();
	UpdateTime=time(0);
	Name.reset();
	Value.reset();
}

HtBrowser::HtBrowser(){
	char tmp[256];
	TmpPath = AppCurrentDir;

	SessionTable.Set(COOKIE_TABLE_NAME, strlen(COOKIE_TABLE_NAME),true);
	db.setDatabase(TmpPath.P);
	sprintf(tmp,"%ld", time(0));
	//SessionID.Set(tmp, strlen(tmp), true);
	MaxRefreshWaitSeconds=600;
	MaxAutoNavigateSeconds=600;

	HasSessionTable=0;
	IsMethodSet=0;
	Method=0;
	NavigationStatus=navNONE;
	StopFlag=0;
}

HtBrowser::~HtBrowser(){
	reset();
}
void HtBrowser::reset(){
	char tmp[256];
	Html.reset();
	Variables.reset();
	Cookies.reset();

	if (SessionID.L){
		sprintf(tmp, "delete from %s where sessionid='%s'", SessionTable.P, SessionID.P);
		db.executeSQL(tmp);
	}
	IsMethodSet=0;
	Method=0;
	NavigationStatus=navNONE;
	StopFlag=0;

	htql.reset();

	ReferLink* link=SessionCookies.Next;
	HtBrowserCookieItem* item;
	while (link){
		if (link->Data){
			item=(HtBrowserCookieItem*) link->Data;
			delete item;
			link->Data=0;
		}
		link=link->Next;
	}
	SessionCookies.reset();
}

int HtBrowser::setCookiePath(const char* cookie_path){
	if (SessionID.L){
		char tmp[256];
		TmpPath=cookie_path;
		db.setDatabase(TmpPath.P);
		sprintf(tmp, "select * from %s where sessionid='%s'", SessionTable.P, SessionID.P);
		int i=db.openSet(tmp);
		if (i<0) {
			if (!db.executeSQL(COOKIE_TABLE_CREATE)) HasSessionTable=0;
			else HasSessionTable=1;
			i=db.openSet(tmp);
		}
	}else{ //use SessionCookies in memory
		//no content;
	}
	return 0;
}

int HtBrowser::setUrl(const char* url){
	char tmp[256];
	int i;
	Url.Set((char*) url, strlen(url), true);
	i=getHostUrl((char*)url, &HostUrl);
	if (i<0) return i;


	//clear Variables
	if (SessionID.L){
		if (!HasSessionTable){
			setCookiePath(TmpPath.P);
		}
		sprintf(tmp, "delete from %s where sessionid='%s' and type='v'", SessionTable.P, SessionID.P);
		i=db.executeSQL(tmp);
	}else{ //use SessionCookies in memory
		ReferLink* link=&SessionCookies; 
		ReferLink* link1;
		HtBrowserCookieItem* item;
		while (link && link->Next){
			item=(HtBrowserCookieItem*) link->Next->Data;
			if (item && !item->Type.Cmp("v",1, true)){
				delete item;
				link1=link->Next->Next;
				link->Next->Next=0;
				delete link->Next;
				link->Next=link1;
			}else{
				link=link->Next;
			}
		}
	}

	IsMethodSet=false;
	Method=HtmlBuffer::GET;
	NavigationStatus=navPREPARE_URL;

	return i;
}
int HtBrowser::setHeader(const char* name, const char* value){
	int err=0;
	if (SessionID.L){
		char tmp[256];
		sprintf(tmp, "select * from %s where sessionid='%s' and type='h' and name='%s'", SessionTable.P, SessionID.P, name);
		int i=db.openSet(tmp);
		i=db.moveFirst();
		if (i==0){
			db.editRecord();
			db.setValue("value", value);
			err=db.update();
		}else{
			db.addRecord();
			db.setValue("sessionid", SessionID.P);
			db.setValue("type", "h");
			db.setValue("name", name);
			db.setValue("value", value);
			err=db.update();
		}
	}else{//use SessionCookies in memory
		ReferLink* link=&SessionCookies; 
		HtBrowserCookieItem* item;
		int found=0;
		while (link && link->Next){
			item=(HtBrowserCookieItem*) link->Next->Data;
			if (item && !item->Name.Cmp(name, strlen(name), true) && !item->Type.Cmp("h",1,true)){
				found=1;
				item->Value=value;
			}
			link=link->Next;
		}
		if (!found){
			item=new HtBrowserCookieItem;
			item->Type="h";
			item->Name=name;
			item->Value=value;
			link=new ReferLink;
			link->Data=(long) item;
			link->Next=SessionCookies.Next;
			SessionCookies.Next=link;
		}
	}
	return err;
}

int HtBrowser::setVariable(const char* name, const char* value){
	int err=0;
	if (SessionID.L){
		char tmp[256];
		sprintf(tmp, "select * from %s where sessionid='%s' and type='v' and name='%s'", SessionTable.P, SessionID.P, name);
		int i=db.openSet(tmp);
		i=db.moveFirst();
		if (i==0){
			db.editRecord();
			db.setValue("value", value);
			err=db.update();
		}else{
			db.addRecord();
			db.setValue("sessionid", SessionID.P);
			db.setValue("type", "v");
			db.setValue("name", name);
			db.setValue("value", value);
			err=db.update();
		}
	}else{//use SessionCookies in memory
		ReferLink* link=&SessionCookies; 
		HtBrowserCookieItem* item;
		int found=0;
		while (link && link->Next){
			item=(HtBrowserCookieItem*) link->Next->Data;
			if (item && !item->Name.Cmp(name, strlen(name), true) && !item->Type.Cmp("v",1,true)){
				found=1;
				item->Value=value;
			}
			link=link->Next;
		}
		if (!found){
			item=new HtBrowserCookieItem;
			item->Type="v";
			item->Name=name;
			item->Value=value;
			link=new ReferLink;
			link->Data=(long) item;
			link->Next=SessionCookies.Next;
			SessionCookies.Next=link;
		}
	}

/*	if (!IsMethodSet){
		Method=HtmlBuffer::POST;
	}
*/
	return err;
}

int HtBrowser::setCookie(const char* name, const char* value, const char* host_url){
	if (!host_url) host_url=HostUrl.P;

	if (SessionID.L){
		char tmp[256];

		sprintf(tmp, "select * from %s where sessionid='%s' and host='%s' and type='c' and name='%s'", SessionTable.P, SessionID.P, host_url, name);
		int i=db.openSet(tmp);
		i=db.moveFirst();
		if (i==0){
			db.editRecord();
			db.setValue("value", value);
			db.update();
		}else{
			db.addRecord();
			db.setValue("sessionid", SessionID.P);
			db.setValue("host", host_url);
			db.setValue("type", "c");
			db.setValue("name", name);
			db.setValue("value", value);
			db.update();
		}
	}else{//use SessionCookies in memory
		ReferLink* link=&SessionCookies; 
		HtBrowserCookieItem* item;
		int found=0;
		while (link && link->Next){
			item=(HtBrowserCookieItem*) link->Next->Data;
			if (item && !item->Name.Cmp(name, strlen(name), true) && !item->Type.Cmp("c",1,true) && !item->Host.Cmp(host_url, strlen(host_url), false) ){
				found=1;
				item->Value=value;
			}
			link=link->Next;
		}
		if (!found){
			item=new HtBrowserCookieItem;
			item->Type="c";
			item->Name=name;
			item->Value=value;
			item->Host=host_url;
			link=new ReferLink;
			link->Data=(long) item;
			link->Next=SessionCookies.Next;
			SessionCookies.Next=link;
		}
	}

	return 0;
}

int HtBrowser::setMethod(int method){
	IsMethodSet=1;
	Method=method;
	return Method;
}
int HtBrowser::getHostUrl(const char* url, ReferData* siteurl){
	char* p=strstr((char*) url, "://");
	if (!p) siteurl->Set("localhost", strlen("localhost"), true);
	else {
		char* p1=strchr(p+3,'/');
		if (p1) siteurl->Set(p+3, p1-(p+3), true);
		else siteurl->Set(p+3, strlen(p+3), true);
	}
	return 0;
}

char* HtBrowser::getGetUrl(){
	return Html.getGetUrl();
}


int HtBrowser::navigate(){
	char tmp[256];
	int i=0;
	char* p;

	NavigationStatus=navFETCHING_DATA;
	StopFlag=0;
	int loop=0;
	long start_time=time(0);
	while (time(0)-start_time<MaxAutoNavigateSeconds){
		if (StopFlag) break;
		loop++;
		Html.setUrl(Url.P);
#ifdef DEBUG_FILE_OUTPUT
		Log::add("script.log", loop, "navigate: --> ", Url.P, __LINE__, __FILE__); 
#endif

		ReferData name, value;
		if (SessionID.L){
			sprintf(tmp, "select * from %s where sessionid='%s' and type='v'", SessionTable.P, SessionID.P);
			i=db.openSet(tmp);
			i=db.moveFirst();
			while(i==0){
				p=db.getValue("name"); 
				name.Set(p, strlen(p), true);
				p=db.getValue("value"); 
				value.Set(p, strlen(p), true);

				Html.setVariable(name.P, value.P);

				i=db.moveNext();
			}

			sprintf(tmp, "select * from %s where sessionid='%s' and host='%s' and type='c' ", SessionTable.P, SessionID.P, HostUrl.P);
			i=db.openSet(tmp);
			i=db.moveFirst();
			while(i==0){
				p=db.getValue("name"); 
				name.Set(p, strlen(p), true);
				p=db.getValue("value"); 
				value.Set(p, strlen(p), true);

				Html.setCookie(name.P, value.P);

				i=db.moveNext();
			}
		}else{//use SessionCookies in memory
			ReferLink* link=&SessionCookies; 
			HtBrowserCookieItem* item;
			while (link && link->Next){
				item=(HtBrowserCookieItem*) link->Next->Data;
				if (item && !item->Type.Cmp("v",1,true)){
					Html.setVariable(item->Name.P, item->Value.P);
				}else if (item && !item->Type.Cmp("c",1,true) && !item->Host.Cmp(&HostUrl, false) ){
					Html.setCookie(item->Name.P, item->Value.P);
				}else if (item && !item->Type.Cmp("h",1,true)){
					Html.setHeader(item->Name.P, item->Value.P);
				}
				link=link->Next;
			}

		}

		Html.setMethod(Method);

		if ((i=Html.fetchHtml(0))<0) {
			NavigationStatus=navERROR;
			StopFlag=0;
			return i;
		}

#ifdef DEBUG_FILE_OUTPUT
		if (HtBrowserLogNavigationPages){
sprintf(tmp, "last_navigate_%d.html", loop);
unlink(tmp);
Html.saveFileAll(tmp);
		}else{
unlink("last_navigate.html");
Html.saveFileAll("last_navigate.html");
		}
#endif

		saveHtmlCookies(&Html);

		if (StopFlag) break;

		p=Html.getContentVar("Location");
		if (p && strcmp(p,"") && strcmp(p, Url.P) ){
			ReferData newurl;
			HTQLParser::mergeUrl(Url.P, p, &newurl);
			setUrl(newurl.P);
			continue;
		}

		if (!tStrOp::strNcmp(Html.ContentType.Data, "text/html", 9, false)){
			HtmlQL ql;
			ql.setSourceData(Html.getContentData(), Html.getContentLength(), false);
			ql.setSourceUrl(Url.P, Url.L);
			ql.setQuery("<meta (http_equiv like 'REFRESH')>:content {url=/'url=' INSEN/2; wait=/;/1}");
			if (!ql.isEOF()){
				p=ql.getValue("wait");
				long wait=p?atoi(p):0;
				if (wait < MaxRefreshWaitSeconds ){
					if (wait>5 && !StopFlag) 
						sleep(wait);
					p=ql.getValue("url");
					if (p) {
						ReferData newurl;
						HTQLParser::mergeUrl(Url.P, p, &newurl);
						setUrl(newurl.P);
						continue;
					}
				}
			}
		}
		break;
	}
	NavigationStatus=navCOMPLETED;
	StopFlag=0;
	return 0;
}
int HtBrowser::stopNavigation(){
	if (NavigationStatus!=navCOMPLETED && NavigationStatus!=navERROR && NavigationStatus!=navNONE) {
		StopFlag=true;
		Html.stopFetchHtml();
	}
	return 0;
}
int HtBrowser::clickItem(const char* item_htql){
	HtmlQL ql;
	ql.setSourceData(getContentData(), getContentLength(), false);
	ql.setSourceUrl(Url.P, Url.L);
	ql.setQuery(item_htql);
	ql.dotQuery(" &get_url &url");
	char* url=ql.getValue(1);
	if (url && strlen(url)>0){
		setUrl(url);
		return navigate();
	}else
		return -1;

}
int HtBrowser::useForm(int form_index, const char* html){
	char buf[256];
	sprintf(buf, "<form>%d", form_index);
	return useForm(buf, html);
}
int HtBrowser::useForm(const char* form_htql, const char* html){
	HtmlQL ql;
	if (html){
		ql.setSourceData(html, strlen(html), false);
	}else{
		ql.setSourceData(Html.Buffer.Data, strlen(Html.Buffer.Data), false);
	}
	ql.setSourceUrl(Url.P, Url.L);
	ql.setQuery(form_htql);
	ql.dotQuery("&form_inputs .<form_inputs>:action");
	if (ql.isEOF()) return 1;

	char* url=ql.getValue(1);
	if (!url) return 1;
	setUrl(url);
	
	ql.setQuery(form_htql);
	ql.dotQuery(":method");
	ReferData method;
	if (!ql.isEOF()){
		method=ql.getValue(1);
		if (!method.Cmp("post", 4, false)){
			setMethod(HtmlBuffer::POST);
		}
	}
	ql.setQuery(form_htql);
	ql.dotQuery("&form_inputs .<form_inputs>.<form_input>{name=:name; id=:id; value=:value;}");
	ql.moveFirst();
	char* name, *value;
	while (!ql.isEOF()){
		name=ql.getValue("name");
		if (!name || !name[0]) name=ql.getValue("id"); 
		value=ql.getValue("value");
		if (name && *name ) setVariable(name, value?value:"" );

		ql.moveNext();
	}
	return 0;
}

char* HtBrowser::getContentData(){
	return Html.getContentData();
}

long HtBrowser::getContentLength(){
	return Html.getContentLength();
}
char* HtBrowser::getBufferData(){
	return Html.Buffer.Data;
}
long HtBrowser::getBufferLength(){
	return Html.Buffer.DataLen;
}
int HtBrowser::getCookie(const char* name, ReferData* value, ReferLinkHeap* all_cookies){
	if (value) value->reset();
	char* header=Html.getContentHeader();
	char* from;
	if (!header) return false;
	ReferData name1, value1;
	char* cookie=0, *p=0;
	char* cookie_header[] = {"Cookie", "Set-Cookie"};
	int found=false;
	long count=0;
	for (int header_index=0; header_index<2; header_index++){
		from = header;
		while (from){
			cookie=getHttpVar(from, cookie_header[header_index], &from);
			if (!cookie) break;

			cookie=getHttpVarValue(cookie);
			p=strchr(cookie, '=');
			if (p) {
				name1.Set(cookie, p-cookie, true);
				value1.Set(p+1, strlen(p+1), false);
			}else{
				name1.Set(cookie, strlen(cookie), false);
				value1.Set("",0,true);
			}

			if (name && !name1.Cmp(name, strlen(name), true)){
				if (value) value->Set(value1.P, value1.L, true);
				found=true;
				if (!all_cookies) return true;
			}
			if (all_cookies){
				all_cookies->add(&name1,  &value1, ++count);
				/*
				ReferLink* link=new ReferLink;
				link->Name.Set(name1.P, name1.L, true);
				link->Value.Set(value1.P, value1.L,true);
				link->Next=*all_cookies;
				*all_cookies=link;
				all_cookies=&link->Next;*/
			}

			from++;
		}
	}
	return found;
}

int HtBrowser::saveHtmlCookies(HtmlBuffer* html){
	char* header=html->getContentHeader();
	char* from;
	if (!header) return 0;
	char* cookie=0, *p=0;
	ReferData name, value;
	char* cookie_header[] = {"Cookie", "Set-Cookie"};
	for (int header_index=0; header_index<2; header_index++){
		from = header;
		while (from){
			cookie=getHttpVar(from, cookie_header[header_index], &from);
			if (!cookie) break;

			cookie=getHttpVarValue(cookie);
			p=strchr(cookie, '=');
			if (p) {
				name.Set(cookie, p-cookie, true);
				value.Set(p+1, strlen(p+1), true);
			}else{
				name.Set(cookie, strlen(cookie), false);
				value.Set("",0,true);
			}

			setCookie(name.P, value.P);

			from++;
		}
	}
	return 0;
}

char* HtBrowser::getHttpVar(const char* header, const char* name, char** position){
	char tmp[256];

	if (position) *position = 0;
	sprintf(tmp, "\n%s:", name);
	char *p1=tStrOp::strNstr((char*) header, tmp, false);
	GetHttpVar.reset();
	if (p1){
		if (position) *position = p1+1;
		p1+=strlen(tmp);
		while (tStrOp::isSpace(*p1)) p1++;
		char* p2=strchr(p1, '\r');
		if (!p2) p2=strchr(p1, '\n');
		if (p2) GetHttpVar.Set(p1, p2-p1, true);
		return GetHttpVar.P;
	}else return 0;
}

char* HtBrowser::getHttpVarValue(const char* var){
	char* p=strchr((char*) var, ';');
	if (!p) p=(char*) var+strlen(var);
	while (*(p-1) == ' ') p--;
	GetHttpVarValue.Set((char*) var, p-var, true);
	return GetHttpVarValue.P;
}
char* HtBrowser::getHttpVarAttrValue(const char* var, const char* attrname, char** position){
	char tmp[256];
	sprintf(tmp, "%s=", attrname);
	if (position) *position = 0;

	char* p=strchr((char*) var, ';');
	if (!p) return 0;
	p++;

	char* p1=tStrOp::strNstr(p, tmp, false);
	if (!p1) return 0;

	if (position) *position = p1;
	p1+=strlen(tmp);

	while (tStrOp::isSpace(*p1)) p1++;
	char* p2=strchr(p1, ';');
	if (p2) GetHttpVarAttrValue.Set(p1, p2-p1, true);
	else GetHttpVarAttrValue.Set(p1, strlen(p1), true);
	return GetHttpVarAttrValue.P;
}

HTQL* HtBrowser::getHtql(){
	return &htql;
}

int HtBrowser::setQuery(const char* Query, unsigned int* Length){
	htql.setSourceData(getContentData(), getContentLength(), false);
	htql.setSourceUrl(Url.P, Url.L);
	return htql.setQuery(Query, Length);
}
int HtBrowser::dotQuery(const char* Query, unsigned int* Length){
	return htql.dotQuery(Query, Length);
}
int HtBrowser::isEOF(){
	return htql.isEOF();
}
char* HtBrowser::moveFirst(){
	return htql.moveFirst();
}
char* HtBrowser::moveNext(){
	return htql.moveNext();
}
char* HtBrowser::getValue(int index){
	return htql.getValue(index);
}
char* HtBrowser::getValue(const char* name){
	return htql.getValue();
}

int HtBrowser::getFieldsCount(){
	return htql.getFieldsCount();
}
char* HtBrowser::getFieldName(int index){
	return htql.getFieldName(index);
}


/*
void* HtBrowser::scriptCommand(char* command, void* arg1, void* arg2, void* arg3, void* arg4){
	if (tStrOp::strNcmp(command, "setUrl",strlen(command)+1,false)==0){
		return (void*) setUrl((const char*) arg1);
	}else if (tStrOp::strNcmp(command, "setVariable",strlen(command)+1,false)==0){
		return (void*) setVariable((const char*) arg1, (const char*) arg2);
	}else if (tStrOp::strNcmp(command, "useForm",strlen(command)+1,false)==0){
		return (void*) useForm((int) arg1);
	}else if (tStrOp::strNcmp(command, "setCookie",strlen(command)+1,false)==0){
		return (void*) setCookie((const char*) arg1, (const char*) arg2);
	}else if (tStrOp::strNcmp(command, "navigate",strlen(command)+1,false)==0){
		return (void*) navigate();
	}else if (tStrOp::strNcmp(command, "saveFileAll",strlen(command)+1,false)==0){
		return (void*) Html.saveFileAll((const char*) arg1);
	}
	return 0;
}

  */

