#ifndef PATH_NAVIGATION_BROWSER_H_2004_09_20
#define PATH_NAVIGATION_BROWSER_H_2004_09_20

#include "referdata.h"
class PathNavigationTree;
class HTMLCacheFile;
class ReferLink;
class ReferLinkHeap;
class PathNavigationBrowserControl;

//-----------------------------------------
class PathNavigationBrowser{
public:
	ReferData Name;
	PathNavigationBrowserControl* BrowserControler; 
	virtual int getPageSource(ReferData* source);
	virtual int getPageUrl(ReferData* url);
	virtual int getUpdatedSource(ReferData* source);
	virtual int getUpdatedUrl(ReferData* url);

	virtual int navigate(const char* url, int max_wait_sec, int nav_speed=-1);
	virtual int clickPageItem(const char* item_htql, int to_wait_sec=0, int nav_speed=-1, PathNavigationBrowser* open_to_browser=0); //0: automatic, -1: no wait, 1: wait

	virtual int useForm(const char* form_htql);
	virtual int setFormValue(const char* name, const char* value);
	virtual int submitForm(const char* form_htql, const char*sbm_input_htql=0, int max_wait_sec=40, int nav_speed=-1, PathNavigationBrowser* open_to_browser=0);

	virtual int fillBrowserForm(const char* form_htql, ReferData* webpath, const char* group_name, PathNavigationTree* tree, int mismatch_level=0);
								//mismatch_level:0=exact match; 1=action can mismatch; 2=input name can mismatch

	//-- the following three functions are to be removed;
		virtual int clickPageItem(const char* tag_name, const char* name, int use_name, long index1, int nav_speed=-1); 
		virtual int fillBrowserForm(ReferData* webpath, int form_index0, const char* group_name, PathNavigationTree* tree, int*fill_count=0, int* notfill_count=0);
		virtual int submitForm(int form_index0, int sbm_input_idx0=0, const char* smt_input_name=0, int max_wait_sec=40, int nav_speed=-1);
	//-- the above three functions are to be removed;

	virtual int waitTime(double sec);
	virtual int setCookie(const char* name, const char* value);
	virtual int getCookie(const char* name, ReferData* value);
	virtual int getCookies(ReferLinkHeap* cookies);
	virtual int getBrowserFormValues( PathNavigationTree *tree );
	virtual int useFrame(int index1, const char* frames_htql);

	virtual int onNavigationTimeout(const char* current_url, const char* target_url, int retry_n, ReferData* retry_url);

	virtual int showMessagePage(const char* url, int max_wait_sec);
	virtual int showMessageBody(const char* msg, int type=1, long wait_sec=4 ); //1:load from stream; 0:set page content
	virtual	int waitNavigation(int max_sec, int nav_speed=-1);
	virtual int stopBrowser();

	int setBufferedPageOnly(ReferData* url, ReferData* page, int copy=true, ReferLink* cookies=0);
	virtual int setBufferedPage(ReferData* url, ReferData* page, int copy=true, ReferLink* cookies=0);
	virtual int clearBufferedPage();
	HTMLCacheFile* BufferedPage;

	static int cmpFormAction(const char* base_url, const char* form_action, const char* group_action);
public:
	PathNavigationBrowser();
	virtual ~PathNavigationBrowser();
	virtual void reset();
};

#endif
