#ifndef IrbNavigationBrowser_H_20110320
#define IrbNavigationBrowser_H_20110320

#include "PathNavigationBrowser.h"
class HtmlBuffer; 

class IrbNavigationBrowser: public PathNavigationBrowser{
public:
	long Port; 
	ReferData ServerAddr; 
	HtmlBuffer* Connector;
	int ToExitIRobot; 

	virtual int getPageSource(ReferData* source);
	virtual int getPageUrl(ReferData* url);
	virtual int getUpdatedSource(ReferData* source);
	virtual int getUpdatedUrl(ReferData* url);

	virtual int navigate(const char* url, int max_wait_sec, int nav_speed=-1);
	virtual int clickPageItem(const char* item_htql, int to_wait_sec=0, int nav_speed=-1, PathNavigationBrowser* open_to_browser=0); //0: automatic, -1: no wait, 1: wait
	virtual int useForm(const char* form_htql);
	virtual int setFormValue(const char* name, const char* value);

	//virtual int fillBrowserForm(const char* form_htql, ReferData* webpath, const char* group_name, PathNavigationTree* tree, int mismatch_level=0);
								//mismatch_level:0=exact match; 1=action can mismatch; 2=input name can mismatch
	virtual int submitForm(const char* form_htql, const char*sbm_input_htql=0, int max_wait_sec=40, int nav_speed=-1, PathNavigationBrowser* open_to_browser=0);

	virtual int setCookie(const char* name, const char* value);
	virtual int getCookie(const char* name, ReferData* value);
	virtual int getCookies(ReferLinkHeap* cookies);
	virtual int useFrame(int index1, const char* frames_htql);

	virtual int setBufferedPage(ReferData* url, ReferData* page, int copy=true, ReferLink* cookies=0);

	//states to monitor navigation page
	int ToSaveLastVisitState; //wheather to save last page in LastVisitPage 
	ReferData LastVisitPage;
	unsigned long LastVisitTime;
	ReferData TempPath;

	static int getIRobotFullName(ReferData* fullname, ReferData* fullpath); 
	static int runCommand(const char* appname, const char* param, const char* rundir, long showopt, ReferData* errmsg);

	long nextAvailablePort(); 
	int connectBrowser(const char* host=0, long port=0, int to_release=-1, int to_start=1); 
	int disconnectBrowser(); 
	int sendBrowserCommand(const char* command, ReferData* result); 
public:
	IrbNavigationBrowser();
	virtual ~IrbNavigationBrowser();
	virtual void reset();
};

#endif

