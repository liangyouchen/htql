#include <stdio.h> 
#include <stdlib.h>
#include "PathNavigationBrowserControl.h"
#include "PathNavigationBrowser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



PathNavigationBrowserControl::PathNavigationBrowserControl(){
	p_onNavigationTimeout=0;
	p_onNavigationTimeoutPara=0;
}
PathNavigationBrowserControl::~PathNavigationBrowserControl(){
	reset();
}
void PathNavigationBrowserControl::reset(){
	resetBrowsers();
	p_onNavigationTimeout=0;
	p_onNavigationTimeoutPara=0;
}
ReferLink* PathNavigationBrowserControl::newBrowser(ReferData* name, int x0, int y0, int x1, int y1, int show){
	return 0;
}
int PathNavigationBrowserControl::showBrowser(ReferData* name, int show, int x0, int y0, int x1, int y1){
	return -1;
}
int PathNavigationBrowserControl::resetBrowsers(){
	ReferLink* link;
	for (link=Browsers.getReferLinkHead(); link; link=link->Next){
		PathNavigationBrowser* session_browser=(PathNavigationBrowser*) link->Data;
		delete session_browser;
	}
	Browsers.reset();
	return 0;
}
int PathNavigationBrowserControl::moveBrowserWindow(void* browser_view, int x0, int y0, int x1, int y1){
	return 0;
}
PathNavigationBrowser* PathNavigationBrowserControl::getPathBrowser(const char* browser_name){
	ReferLink* link=Browsers.findName(browser_name);
	PathNavigationBrowser* pathbrowser=(PathNavigationBrowser*) link->Data;
	return pathbrowser;
}

int PathNavigationBrowserControl::showBrowserHtml(const char* browser_name, const char*html, int x0, int y0, int x1, int y1){
	int to_show=html && html[0];

	ReferData name;
	name.Set((char*) browser_name, strlen(browser_name), false);

	ReferLink* link=Browsers.findName(&name);

	if (to_show){
		if (!link) link=newBrowser(&name, x0, y0, x1, y1, true);

		PathNavigationBrowser* session_browser=(PathNavigationBrowser*) link->Data;
		char* p=tempnam(NULL, "brows_"); //need to free p
		ReferData tempfile;
		tempfile.Set(p, p?strlen(p):0,false);
		tempfile.setToFree(true);	//free p here
		tempfile+="$cached$.html";

		ReferData source;
		source.Set((char*) html, strlen(html), false);
		source.saveFile(tempfile.P);

		session_browser->navigate(tempfile.P, 10);
		showBrowser(&name, to_show, x0, y0, x1, y1);
	}else{
		if (link){
			showBrowser(&name, false, x0, y0, x1, y1);
		}
	}
	return 0;
}


