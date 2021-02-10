#include "htmlql.h"
#include "log.h"
#include "qhtmlql.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HtmlQL::HtmlQL(){
	if (Parser) {
		delete Parser;
		Parser= 0;
	}
	Parser= new HtmlQLParser;
	
	HtmlQLParser::addHtmlQLFunctions(Parser);
}

HtmlQL::~HtmlQL(){
	
}

int HtmlQL::setPageMark(int* pagemark, int marklevel){
	if (Parser) {
		((HtmlQLParser*) Parser)->FindHtqlPageMark=pagemark; 
		((HtmlQLParser*) Parser)->FindHtqlPageMarkLevel=marklevel; 
	}
	return 0;
}


