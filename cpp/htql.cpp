#include "htql.h"
#include "qhtql.h"
#include "htmlbuf.h"
#include "dirfunc.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "evalversion.h"

#define VER_EVALUATION_TEST  \
		time_t eval_t0=0; \
		time_t eval_t=time(0); \
		tStrOp::DateToLong(VER_EVALUATION, "YYYY/MM/DD HH24:MM:SS", &eval_t0); \
		if (eval_t - eval_t0 > 3600L*24L*30L*VER_EVALUATION_MONTHS){ \
				printf("HTQL Evaluation purpose only. \n"); \
		} \
		if (eval_t - eval_t0 > 3600L*24L*30L*(VER_EVALUATION_MONTHS+3)){ \
				Html->Buffer.SetValue("HTQL Evaluation purpose only. \n", strlen("HTQL Evaluation purpose only. \n") ); \
				return Html->Buffer.Data; \
		} 


HTQL::HTQL(){
	Parser = new HTQLParser;
	Html = new HtmlBuffer;
	Html->TextOnly=true;
	NullFieldValue.Set("",0,true);
}

HTQL::~HTQL(){
	reset();
	if (Parser) {
		delete Parser;
		Parser=0;
	}
	if (Html){
		delete Html;
		Html=0;
	}
	resetDllFunctions();
}

void HTQL::reset(){
#if (ERRORLOG==3)
	Log::add(LOGFILE,0, "HTQL::reset()", "Html->Buffer.Free();", __LINE__,__FILE__);
#endif
	Html->Buffer.Free();
#if (ERRORLOG==3)
	Log::add(LOGFILE,0, "HTQL::reset()", "Parser->reset();", __LINE__,__FILE__);
#endif
	Parser->reset();
	NullFieldValue.Set("",0,true);
}

int HTQL::setUrlToPost(const char* url, unsigned int* Length){
	if (Length){
		setSourceUrl((char*)url, *Length);
	}else{
		setSourceUrl((char*)url, strlen(url));
	}

	return Html->setUrl(Parser->SourceUrl.P);
}
int HTQL::setUrlCookie(const char* name, const char* value){
	return Html->setCookie(name, value);
}
int HTQL::setUrlArgument(const char* name){
	return Html->setArg(name);
}
int HTQL::setUrlParameter(const char* name, const char* value){
	return Html->setVariable(name, value);
}

char* HTQL::postUrl(){
#ifdef VER_EVALUATION
	VER_EVALUATION_TEST
#endif

	int err;
	if ((err=Html->fetchHtml(0)) <0 ) return NULL;
	setSourceData(Html->Buffer.Data, Html->Buffer.DataLen, false);
	return Html->Buffer.Data;	
}

char* HTQL::getGetUrl(){
	return Html->getGetUrl();
}


char* HTQL::setUrl(const char* url, unsigned int* Length){
	reset();

#ifdef VER_EVALUATION
	VER_EVALUATION_TEST;
#endif

	if (url){
		if (Length){
			setSourceUrl((char*)url, *Length);
		}else{
			setSourceUrl((char*)url, strlen(url));
		}

		if (Html->fetchHtml(Parser->SourceUrl.P) <0 ) return NULL;
		setSourceData(Html->Buffer.Data, Html->Buffer.DataLen, false);
	}else{
		if (postUrl() <0 ) return NULL;
	}
	 
	return Html->Buffer.Data;
}

char* HTQL::setSourceData(const char* data, size_t len, int copy){
#ifdef VER_EVALUATION
	VER_EVALUATION_TEST
#endif
	Parser->reset();
	return Parser->setData((char*)data, len, copy);
}

char* HTQL::setSourceUrl(const char* url, size_t len){
	return Parser->setSourceUrl((char*)url, len);
}


int HTQL::setGlobalVariable(const char* name, const char* value){
	return Parser->setGlobalVariable((char*) name, (char*) value);
}

void HTQL::resetGlobalVariable(){
	Parser->resetGlobalVariables();
}


int HTQL::setQuery(const char* Query, unsigned int* Length){
	Parser->reset();
	if (!Query) return -1; 

	if (Length){
		Parser->setSentence((char*)Query, *Length, true);
	}else{
		Parser->setSentence((char*)Query, strlen(Query), true);
	}
	int err=Parser->parse();

	if (err<0) return err;
	else return isEOF()?1:0;
}

int HTQL::dotQuery(const char* Query, unsigned int* Length){
	int err=0;
	if (!Parser->Sentence.L){
		err= setQuery(Query, Length);
	}else if (Length){
		err= Parser->dotSentence((char*)Query, *Length, true);
	}else{
		err= Parser->dotSentence((char*)Query, strlen(Query), true);
	}
	if (err<0) return err;
	else return isEOF()?1:0;
}

char* HTQL::getSourceUrl(){
	return Parser->getSourceUrl();
}

ReferData& HTQL::getSourceData(){
	return Parser->SourceData;
}

int HTQL::isEOF(){
	return Parser->isEOF();
}
char* HTQL::moveFirst(){
	HTQLItem* p=Parser->moveFirst();
	if (!p) return 0;
	return p->Data.P?p->Data.P:NullFieldValue.P;
}
char* HTQL::moveNext(){
	HTQLItem* p=Parser->moveNext();
	if (!p) return 0;
	return p->Data.P?p->Data.P:NullFieldValue.P;
}
int HTQL::isBOF(){
	return Parser->isBOF();
}
char* HTQL::movePrev(){
	HTQLItem* p=Parser->movePrev();
	if (!p) return 0;
	return p->Data.P?p->Data.P:NullFieldValue.P;
}
char* HTQL::moveLast(){
	HTQLItem* p=Parser->moveLast();
	if (!p) return 0;
	return p->Data.P?p->Data.P:NullFieldValue.P;
}
char* HTQL::getValue(int index){
	return Parser->getField(index);
}
char* HTQL::getValue(const char* name){
	return Parser->getField(name);
}
long HTQL::getFieldOffset(int fieldindex1, long* position, long* offset){
	HTQLItem* item=Parser->getFieldItem(fieldindex1);
	if (position) {
		*position=item?item->SourceOffset:-1;
	}
	if (offset){
		*offset=item?item->Data.L:-1;
	}
	return item?item->SourceOffset:-1;
}

int HTQL::getFieldsCount(){
	return Parser->getFieldsCount();
}

char* HTQL::getFieldName(int index){
	return Parser->getFieldName(index);
}

long HTQL::getTuplesCount(){
	return Parser->getTuplesCount();
}

int HTQL::registerDllFunctions(const char* path, const char* funcname){
	DirFunc::loadLibraries(path, funcname, &DllFunctions);
	if (Parser){
		Parser->addDllFunctions(DllFunctions.getReferLinkHead(), (long) this);
	}
	return 0;
}

int HTQL::resetDllFunctions(){
	DirFunc::freeLibraries(&DllFunctions);
	return 0;
}

int HTQL::useExprContext(tExprCalc* context){
	if (Parser){
		return Parser->ExprContext.useContext(context); 
	}
	return 0;
}


