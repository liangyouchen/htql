#ifndef LAWEB_DOWNLOAD_MAN_H_2005_01_11
#define LAWEB_DOWNLOAD_MAN_H_2005_01_11

#include "ClassLogID.h"
#include "referdata.h"
#include "htbrowser.h"
#include "links.h"
#include "referlock.h"

class LaWebDownloadMan;
class PathNavigationBrowser;
class LaWebDownloadCenter;
class IrbApi;

class LaWebDownloadItem{
public:
	//settings, transmitting to child items
	ReferData Url;
	ReferData Description;
	ReferData UrlMatches;
	ReferData UrlExcludes;
	ReferData ContentFilter; 
	long MaxDepth;

	//only to share within a single item
	ReferData ItemString;
	long ItemData;
	ReferLinkHeap ItemVariables; //only to share within a single item

	static int isDomainMatching(const char* url, const char* domain);
	static int isUrlMatching(const char* url, const char* url_text, const char* match_keys);
	int isDomainMatching(const char* url, ReferLinkHeap* domains);
	static int getDomain(const char* url, ReferData* domain); 

	//results: data
	ReferData ContentData;
	ReferData ContentType;
	ReferData ContentUrl;
	int resetContent();

	//parameters: timeout
	int TimeoutConnect;
	int TimeoutTransfer;
	int TextOnly;
	long MaxSize;
	ReferData CookiePath;

	//parameters: browser
	enum{NavHtBrowser, NavSocket, NavIEBrowser, NavIrbBrowser}; //default is NavHtBrowser
	int NavigationMethod;
	HtBrowser HttpBrowser;
	PathNavigationBrowser* IEBrowser; //deleted when reset
	IrbApi* IrbBrowser; //deleted when reset

	//functions
	LaWebDownloadMan* DownloadMan;
	virtual int startDownload();
	virtual int stopDownload();

	LaWebDownloadCenter* DownloadCenter;
	ReferLink* UrlProperty; //Pointing to DownloadCenter->AllUrls

	enum{rankLowest, rankLow, rankNormal, rankSuper };
	int Rank; 
	long QueueTime; 

	//parameters: control flag
	enum{actNORMAL, actCANCELED};
	int Active;

	//parameters: Auto action
	ReferData SaveFile;
	int AutoSave;
	int AutoDestroy;
	typedef int (*DownloadCallBack)(LaWebDownloadItem*, void*);
	DownloadCallBack AutoCallBack;
	void* AutoCallBackPara;

	//results: status
	enum{downNONE, downQUEUE, downACTIVE, downCOMPLETED};
	int Status;
	int ErrorCode;

	LaWebDownloadItem();
	virtual ~LaWebDownloadItem();
	void reset();
};

class LaWebDownloadCenter;

class LaWebDownloadMan: public ClassLogID{
public:
	int addDownload(LaWebDownloadItem* item);
	int MaxThreadsNum;
	long MaxWaitingQueue;
	enum {brtIrbApi, brtIEIrbApi, brtIrbIrbApi };
	int MainIrbApiType;

	enum {priorHIGH, priorLOW, priorPAUSE, priorSTOP}; 
	int Priority;

	ReferLock Lock;
	int ActiveThreadNum;
	LinkList WaitingQueue;
	LinkList WorkingQueue;
	ReferData DisplayMsg;

	int startDownloadService(); //doesnot clear ToStop
#if defined _WINDOWS  || defined WIN32
	static void doGetHttpFile(void* p);
#else
	static void* doGetHttpFile(void* p);
#endif

	int stopDownload(int to_wait=false);
	int startDownload(); //also clear ToStop
	int ToStop;
	int autoDestroyAll();

	long CountWaiting; //read only
	long CountWorking; //read only
	long CountDownloaded; //read only
	long CountCancelled; //read only
	long CountSites; 

	ReferLock DownloadCenterLock;
	ReferLinkHeap DownloadCenterEntries;
	LaWebDownloadCenter* getDownloadCenter(const char* name); 
	int addDownloadCenter(const char* name, LaWebDownloadCenter* center); //will be automatically deleted by LaWebDownloadMan
	int checkCompletionService(); //need to lock DownloadCenterLock and call from a windows thread
	long LastCompletionTime;

protected:
	int clearCanceledItems(); //lock before calling this function
	Links* fetchNextItem();	//lock before calling this function
	int FetchNextCount;
	int FetchNextMax;//sort the waiting queue when FetchNextCount>=FetchNextMax
	int sortWaitingQueue();
public:
	LaWebDownloadMan();
	virtual ~LaWebDownloadMan();
	virtual void reset();
};

#endif
