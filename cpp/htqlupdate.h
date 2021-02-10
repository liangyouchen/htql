#ifndef HTQL_UPDATE_H_CLY_2003_03_11
#define HTQL_UPDATE_H_CLY_2003_03_11

#include "referdata.h"

class HTQLFunction;
class HTQLParser;

class HTQLFunctionUpdateStruct{
public:
	enum {
		funINSERT_BEFORE, 
		funINSERT_AFTER, 
		funINSERT_INSIDE_HEAD, 
		funINSERT_INSIDE_TAIL, 
		funDELETE, 
		funUPDATE,
		funSET_ATTRIBUTE, 
		funDELETE_ATTRIBUTE,
	};
	ReferData UpdatedData;
	long LastOffset;
	int Action;
	int ToUpdateSource;

	static int functionUpdate(char*p, void* call_from, ReferData* tx); 
	static int functionUpdateFunPrepare(char* p, void* call_from, ReferData* tx);
	static int functionUpdateFunComplete(char* p, void* call_from, ReferData* tx);
	static void functionUpdateFunFinalRelease(HTQLFunction* fun);
	static int addUpdateFunction(HTQLParser* parser, int action, char* funname, char* description);//&insert(value);

	HTQLFunctionUpdateStruct();
	~HTQLFunctionUpdateStruct();
	void reset();
};


#endif
