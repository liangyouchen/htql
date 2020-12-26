
#include "PathNavigationBrowser.h"
#include "referdata.h"
#include "platform.h"
#include "htql.h"
#include "HTMLCacheFile.h"
#include "qhtql.h"
#include "PathNavigationBrowserControl.h"

#ifdef _WINDOWS
	#include "MyFunction.h"
#endif

#ifndef NO_PathTree
	#include "pathnavigation.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//===========================================================

PathNavigationBrowser::PathNavigationBrowser(){
	Name="PathNavigationBrowser";
	BufferedPage=0;
	BrowserControler=0;
}
PathNavigationBrowser::~PathNavigationBrowser(){
	reset();
}
void PathNavigationBrowser::reset(){
	if (BufferedPage) {
		delete BufferedPage;
		BufferedPage=0;
	}
}
int PathNavigationBrowser::getPageSource(ReferData* source)
{
	if (BufferedPage) {
		source->Set(BufferedPage->Source.P, BufferedPage->Source.L, false);
		return 0;
	}else{
		return -1;
	}
}
int PathNavigationBrowser::getPageUrl(ReferData* url)
{
	if (BufferedPage) {
		url->Set(BufferedPage->Url.P, BufferedPage->Url.L, false);
		return 0;
	}else{
		return -1;
	}
}
int PathNavigationBrowser::getUpdatedSource(ReferData* source)
{
	if (BufferedPage) {
		source->Set(BufferedPage->UpdatedSource.P, BufferedPage->UpdatedSource.L, false);
		return 0;
	}else{
		return -1;
	}
}
int PathNavigationBrowser::getUpdatedUrl(ReferData* url)
{
	if (BufferedPage) {
		url->Set(BufferedPage->UpdatedUrl.P, BufferedPage->UpdatedUrl.L, false);
		return 0;
	}else{
		return -1;
	}
}
int PathNavigationBrowser::navigate(const char* url, int max_wait_sec, int nav_speed)
{
	clearBufferedPage();
	return -1;
}
int PathNavigationBrowser::clickPageItem(const char* item_htql, int to_wait_sec, int nav_speed, PathNavigationBrowser* open_to_browser) //0: automatic, -1: no wait, 1: wait
{
	clearBufferedPage();
	return -1;
}
int PathNavigationBrowser::useForm(const char* form_htql){
	return -1;
}
int PathNavigationBrowser::setFormValue(const char* name, const char* value){
	return -1;
}

int PathNavigationBrowser::fillBrowserForm(const char* form_htql, ReferData* webpath, const char* group_name, PathNavigationTree* tree, int mismatch_level)
{
	//virtual function
	clearBufferedPage(); 

	HTQL ql;
	ql.setSourceData(webpath->P, webpath->L, false);
	ql.setGlobalVariable("g", group_name);

	ReferData group_action;
#ifndef NO_PathTree
	if (tree) tree->getVariable("LAWEB_FORM_ACTION", &group_action, false);
#endif

	ReferData page, url;
	getUpdatedSource(&page); 
	getUpdatedUrl(&url);

	HtmlQL browserql; 
	browserql.setSourceData(page.P, page.L, false); 
	browserql.setQuery(form_htql); 
	browserql.dotQuery(":action &url");
	ReferData form_action; form_action = browserql.getValue(1); 

	if (mismatch_level==0 && cmpFormAction(url.P, form_action.P, group_action.P)){
		return -1;
	}

	//virtual function
	useForm(form_htql); 

	ql.setQuery("<LaWebInputs>.<LaWebInput (Group=g and Name<>'LAWEB_FORM_ACTION' and Comment<>'TRUE')>{Name=:Name; Type=:Type; Value=:tx &html_decode;}");
	ReferData name, value, var_value;
	for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
		value = ql.getValue("Value");
		name = ql.getValue("Name");
#ifndef NO_PathTree
		if (tree && tree->getVariable(name.P, &var_value, false)){ //get variable to replace value
			value=var_value;
		}
#endif
		//virtual function
		setFormValue(name.P, value.P);
	}
	return 0;
}
int PathNavigationBrowser::submitForm(const char* form_htql, const char*sbm_input_htql, int max_wait_sec, int nav_speed, PathNavigationBrowser* open_to_browser)
{
	clearBufferedPage();
	return -1;
}

//-- the following three functions are to be removed;
	int PathNavigationBrowser::clickPageItem(const char* tag_name, const char* name, int use_name, long index1, int nav_speed)
	{	clearBufferedPage(); return -1; }
	int PathNavigationBrowser::fillBrowserForm(ReferData* webpath, int form_index0, const char* group_name, PathNavigationTree* tree, int*fill_count, int* notfill_count)
	{return -1;}
	int PathNavigationBrowser::submitForm(int form_index0, int sbm_input_idx0, const char* smt_input_name, int max_wait_sec, int nav_speed)
	{	clearBufferedPage(); return -1; }
//-- the above three functions are to be removed;


int PathNavigationBrowser::useFrame(int index1, const char* frames_htql)
{return -1;}
int PathNavigationBrowser::waitTime(double seconds)
{
	long second=(long) seconds;
	int msec=int ((seconds-second)*1000);
	if (second>0){
#if 0 && (defined(WIN32) && !defined(_CONSOLE))
		#pragma message(__FILE__ " ***** Windows Version *****")
		WindowsWait(second, 0);
#else
		#pragma message(__FILE__ " ***** NO Windows Version *****")
		sleep(second);	
#endif
	}
	if (msec>0) {
#if 0 && (defined(WIN32) && !defined(_CONSOLE))
		WindowsWaitM(msec, 0);
#else
		usleep(msec);
#endif
	}
	return 0;
}

int PathNavigationBrowser::setCookie(const char* name, const char* value)
{return -1;}
int PathNavigationBrowser::getCookie(const char* name, ReferData* value)
{return -1;}
int PathNavigationBrowser::getCookies(ReferLinkHeap* cookies)
{return -1;}

int PathNavigationBrowser::waitNavigation(int max_sec, int nav_speed)
{
	return 0;
}
int PathNavigationBrowser::stopBrowser(){
	return 0;
}
int PathNavigationBrowser::onNavigationTimeout(const char* current_url, const char* target_url, int retry_n, ReferData* retry_url){
	if (BrowserControler && BrowserControler->p_onNavigationTimeout){
		BrowserControler->p_onNavigationTimeout(
			BrowserControler->p_onNavigationTimeoutPara,
			0, //PathNavigationBrowser*
			current_url, 
			target_url, 
			retry_n, 
			retry_url);
	}
	/*
	ReferData status; 
	if (!retry_url->L){
		status="Timeout: ";
	}else{
		char buf[128];
		sprintf(buf, "Timeout %d, retrying ... ", retry_n);
		status=buf;
	}
	status+=target_url; 
	showStatusText(status.P); 
	*/
	return 0; 
}

int PathNavigationBrowser::showMessagePage(const char* url, int max_wait_sec)
{return navigate(url, max_wait_sec);}

int PathNavigationBrowser::showMessageBody(const char* msg, int type, long wait_sec){
	return -1;
}

int PathNavigationBrowser::getBrowserFormValues( PathNavigationTree *tree )
{
	return -1;
}
int PathNavigationBrowser::cmpFormAction(const char* base_url, const char* form_action, const char* group_action){
	if (!group_action || !group_action[0]) return 0;
	if (!form_action || !form_action[0]) return 1;

	ReferData form_url, group_url;
	HTQLParser::mergeUrl((char*)base_url, (char*)form_action, &form_url);
	HTQLParser::mergeUrl((char*)base_url, (char*)group_action, &group_url);

	char* p=0;
	p=strchr(form_url.P, '?');
	if (p) *p=0;
	p=strchr(form_url.P, '#');
	if (p) *p=0;
	p=strrchr(form_url.P, '/');
	if (p && p>form_url.P && *(p-1)!=':' && *(p-1)!='/') *p=0;
	form_url.L=strlen(form_url.P);

	p=strchr(group_url.P, '?');
	if (p) *p=0;
	p=strchr(group_url.P, '#');
	if (p) *p=0;
	p=strrchr(group_url.P, '/');
	if (p && p>group_url.P && *(p-1)!=':' && *(p-1)!='/') *p=0;
	group_url.L=strlen(group_url.P);

	return form_url.Cmp(&group_url, true);
}
int PathNavigationBrowser::setBufferedPage(ReferData* url, ReferData* page, int copy, ReferLink* cookies){
	return setBufferedPageOnly(url, page, copy, cookies);
}
int PathNavigationBrowser::setBufferedPageOnly(ReferData* url, ReferData* page, int copy, ReferLink* cookies){
	if (!BufferedPage){
		BufferedPage=new HTMLCacheFile;
	}else{
		BufferedPage->reset();
	}
	BufferedPage->Url.Set(url->P, url->L, true);
	BufferedPage->Source.Set(page->P, page->L, copy);
	BufferedPage->UpdatedUrl.Set(BufferedPage->Url.P, BufferedPage->Url.L,false);
	BufferedPage->UpdatedSource.Set(BufferedPage->Source.P, BufferedPage->Source.L, false);

	return 0;
}
int PathNavigationBrowser::clearBufferedPage(){ //Use Browser data instead of the memory data
	if (BufferedPage){
		delete BufferedPage;
		BufferedPage=0;
	}
	return 0;
}


