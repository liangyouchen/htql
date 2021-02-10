
#include "HTMLCacheFile.h"
#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE
#endif

HTMLCacheFile::HTMLCacheFile(){
	FrameIndex=-1;
	UseSource=useSOURCE;
}
HTMLCacheFile::~HTMLCacheFile(){
	reset();
}
void HTMLCacheFile::reset(){
	FrameIndex=-1;
	FramesHtql.reset();
	Url.reset();
	UpdatedUrl.reset();
	Source.reset();
	UpdatedSource.reset();
	Focus.reset();
	UseSource=useSOURCE;
	PostData.reset();
}
int HTMLCacheFile::setFrameIndex(long frame_index, ReferLink* frames){ //Use this function to also set the appropriate UseSource
	if (frames){
		FramesHtql.reset();
		for (ReferLink* link=frames; link; link=link->Next){
			FramesHtql+=link->Name;
			FramesHtql+=".";
		}
		FrameIndex=0;
	}
	FrameIndex=frame_index;
	if (FrameIndex<0) 
		UseSource=useSOURCE;
	else 
		UseSource=useUPDATED_SOURCE;
	return 0;
}
int HTMLCacheFile::setHtqlSource(HTQL* ql, int copy){
	if (UseSource==useSOURCE){
		ql->setSourceData(Source.P, Source.L, copy);
		ql->setSourceUrl(Url.P, Url.L);
	}else{
		ql->setSourceData(UpdatedSource.P, UpdatedSource.L, copy);
		ql->setSourceUrl(UpdatedUrl.P, UpdatedUrl.L);
	}
	return 0;
}

int HTMLCacheFile::adjustFocusWithSource(){
	HtmlQL ql;
	ReferData page;
	if (UseSource==useSOURCE){
		page.Set(Source.P, Source.L, false);
	}else{
		page.Set(UpdatedSource.P, UpdatedSource.L, false);
	}
	ql.setSourceData(page.P, page.L, false);
	ql.setQuery(Focus.P);
	if (!ql.isEOF()){
		ql.dotQuery("&position.<position>:from,to");
		long from=-1, to=-1;
		from=atoi(ql.getValue(1));
		to=atoi(ql.getValue(2));

		//find if the best text is in the range of from and to
		ReferLinkHeap text_pos;
		HtPageModel page_model;
		double* text_scores=0;
		page_model.setModelPage(&page, false);
		if (FocusText.L<500 && page_model.addModelScore(&FocusText, true, HtPageModel::MODEL_PLAIN_PATTERN, 1)>=0) {
			page_model.findBestPos(&text_pos);
		}
		ReferLink* link;
		if (text_pos.Total>0){
			int found=false;
			for (link=(ReferLink*) text_pos.moveFirst(); link; link=(ReferLink*) text_pos.moveNext()){
				if (link->Value.L>=from && link->Value.L<=to) found=true;
			}
			if (!found){ 
				//regenerate focus
				page_model.addModelScore(&Focus, false, HtPageModel::MODEL_HTQL, 0.1);
				page_model.recombineScores();
				text_pos.reset();
				page_model.findBestPos(&text_pos);
				link=(ReferLink*) text_pos.moveFirst();
				char buf[128];
				sprintf(buf, "&offset(%ld,0) &find_focus_htql", link->Value.L);
				ql.setQuery(buf);
#ifdef DEBUG_THIS_FILE
				ReferData segment;
				if (link->Value.L>200) segment.Set(page.P+link->Value.L-200, 200, true);
				else segment.Set(page.P, link->Value.L, true);
				TRACE("\r\nHTMLCacheFile::adjustFocusWithSource\r\nModel text: %s\r\n", FocusText.P);
				TRACE("Relocated Position: %s\r\n", segment.P);
				TRACE("Adjusting Query: %s\r\n", buf);
				TRACE("Old Focus: %s; New Focus: %s\r\n", Focus.P, ql.getValue(1));
#endif
				if (!ql.isEOF()) Focus=ql.getValue(1);
			}
		}
	}
	return 0;
}
int HTMLCacheFile::getFormInfo(const char* focus, int* form_index1, ReferData* form_action){
	//set form_index
	HtmlQL ql;
	setHtqlSource(&ql, false);
	//ql.setSourceData(Source.P, Source.L, false);
	//ql.setSourceUrl(Url.P, Url.L);

	int form_index=0;
	ql.setQuery(focus);
	ql.dotQuery("&position.<position>:from");
	long pos0=0, pos1=0, pos2=0;
	char* p=ql.getValue(1);
	if (p&&*p) sscanf(p, "%ld", &pos0);
	ql.setQuery("<form (et is not null)> &position.<position>:from, to");
	int i=0;
	for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
		i++;
		pos1=pos2=0;
		p=ql.getValue(1);
		if (p&&*p) sscanf(p, "%ld", &pos1);
		p=ql.getValue(2);
		if (p&&*p) sscanf(p, "%ld", &pos2);
		if (pos1<=pos0 && pos2>=pos0) {
			form_index=i;
			break;
		}
	}
	//if (form_index<=0) form_index=1;

	if (form_index1){
		*form_index1=form_index;
	}

	//set form_action
	if (form_action && form_index>0){
		char buf[128];
		sprintf(buf, "<form (et is not null)>%d {action=:action &url; name=:name}", form_index);
		/*ql.setQuery(focus);
		ql.dotQuery("&tag_parent('FORM'). <form> {action=:action &url; name=:name}");
		*/
		ql.setQuery(buf);
		*form_action=ql.getValue(1);
	}

	if (form_index1 && *form_index1<0){
		return -1;
	}else{
		return 0;
	}
}

