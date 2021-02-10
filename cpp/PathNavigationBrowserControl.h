#ifndef PathNavigationBrowserControl_H_CLY2005_10_09
#define PathNavigationBrowserControl_H_CLY2005_10_09

#include "referdata.h"
#include "referlink.h"
class PathNavigationBrowser;

class PathNavigationBrowserControl{
public:
	ReferLinkHeap Browsers; //Data: PathNavigationBrowser; Name: name; Value.P: implemented window; Value.L: 0=not show, 1=show, 2=force show
public: 
	virtual ReferLink* newBrowser(ReferData* name, int x0=-1, int y0=-1, int x1=-1, int y1=-1, int show=false);
	virtual int showBrowser(ReferData* name, int show, int x0=-1, int y0=-1, int x1=-1, int y1=-1);
	virtual int moveBrowserWindow(void* browser_view, int x0=-1, int y0=-1, int x1=-1, int y1=-1);
	virtual int showBrowserHtml(const char* browser_name, const char*html=0, int x0=-1, int y0=-1, int x1=-1, int y1=-1);
	virtual PathNavigationBrowser* getPathBrowser(const char* browser_name); 
	virtual int resetBrowsers();

	int (*p_onNavigationTimeout)(void* para, PathNavigationBrowser*browser, const char* current_url, const char* target_url, int retry_n, ReferData* retry_url);
	void* p_onNavigationTimeoutPara;

public:
	PathNavigationBrowserControl();
	virtual ~PathNavigationBrowserControl();
	virtual void reset();
};

#endif
