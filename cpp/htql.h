#ifndef CLY_HTQL_H_20030610
#define CLY_HTQL_H_20030610

#include "referdata.h"
#include "referlink.h"
#include "platform.h"

class HTQLParser;
class HtmlBuffer;
class tExprCalc; 

class HTQL{
public:
	HtmlBuffer* Html;
	HTQLParser* Parser;

	HTQL();
	~HTQL();
	virtual void reset();

	char* setSourceData(const char* data, size_t len, int copy=false); 
	char* setSourceUrl(const char* url, size_t len);
	char* getSourceUrl();
	ReferData& getSourceData();

	int setUrlToPost(const char* url, unsigned int* Length=0);
	int setUrlCookie(const char* name, const char* value);
	int setUrlArgument(const char* name);
	int setUrlParameter(const char* name, const char* value);
	char* getGetUrl();
	char* postUrl();

	int setGlobalVariable(const char* name, const char* value);
	void resetGlobalVariable();
	virtual char* setUrl(const char* url=0, unsigned int* Length=0); //including both setBuffer and setUrlString
	int setQuery(const char* Query, unsigned int* Length=0);
	int dotQuery(const char* Query, unsigned int* Length=0);
	int isEOF();
	virtual char* moveFirst();
	virtual char* moveNext();
	virtual char* movePrev();
	virtual char* moveLast();
	int isBOF();
	virtual char* getValue(int index1=0);
	virtual char* getValue(const char* name);
	virtual long getFieldOffset(int fieldindex1, long* position=0, long* offset=0);

	int getFieldsCount();
	char* getFieldName(int index1=0);
	long getTuplesCount();

	ReferLinkHeap DllFunctions; 
	int registerDllFunctions(const char* path, const char* funcname); 
	int resetDllFunctions(); 
	int useExprContext(tExprCalc* context);

protected:
	ReferData NullFieldValue;
};

#endif
