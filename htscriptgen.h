#ifndef HTSCRIPT_GENERATION_CLY_20030831
#define HTSCRIPT_GENERATION_CLY_20030831

class tStack;
class ReferData;

#include "htscript.h"

class HtScriptGen {
public:
	HtBrowserScript script;

	int generateWebView(char* url, int form_index, tStack* values, ReferData* result, ReferData* result_table=0);
				// url: url of the web database
				// values: sample values; values->Name: ="" -- automatic match name
	int generateHtmlTable(char* url, char* candiate_htql, ReferData* result, ReferData* result_table=0);
	HtScriptGen();
	~HtScriptGen();
	void reset();
};

#endif
