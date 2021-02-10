#ifndef HTMLCacheFile_H_CLY_2011_04_01
#define HTMLCacheFile_H_CLY_2011_04_01

#include "referdata.h"
#include "referlink.h"
#include "htmlql.h"
#include "HtPageModel.h"

class HTMLCacheFile
{
public:
	ReferData Url;
	ReferData Source; //persistent source, by MyHtmlFun::saveDocument2File
	ReferData UpdatedUrl;
	ReferData UpdatedSource; //updated source, e.t., updated by java script
	ReferData ResultUrl;
	ReferData ResultSource; //updated source after action
	ReferData Focus; 
	ReferData FocusText;
	ReferData FramesHtql; //<IFRAME>1.<Frame>2....
	ReferData PostData; 

	int setFrameIndex(long frame_index, ReferLink* frames); //Use this function to also set the appropriate UseSource
	long FrameIndex; //<0: use Source; 0: use UpdatedSource; other; use UpdatedSource
	enum {useSOURCE, useUPDATED_SOURCE};
	int UseSource; //0: use Source, 1: use UpdatedSource; 
				//Usually set according to FrameIndex, however, if FrameIndex>=1, UseSource may set -1 to use the frame source
	int setHtqlSource(HTQL* ql, int copy);
	int adjustFocusWithSource();
	int getFormInfo(const char* focus, int* form_index1, ReferData* form_action);

	HTMLCacheFile();
	~HTMLCacheFile();
	void reset();
};


#endif
