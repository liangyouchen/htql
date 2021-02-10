// LASTMODIFY CLY20000430
#include "freemem.h"

#include <string.h>
#include <malloc.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


tFreeMem::tFreeMem(long size)
{
	MallocMem=NULL;
	MallocMemLen=0;
	if (size>0){
		Malloc(size);
	}
}

tFreeMem::~tFreeMem()
{
	if (MallocMem){
		free(MallocMem);
		MallocMem=NULL;
	}
}

void tFreeMem::Free()
{
	if (MallocMem){
		free(MallocMem);
		MallocMem=NULL;
	}
	MallocMemLen=0;
}

char* tFreeMem::Malloc(long size){
	if (size >= MallocMemLen ){
		MallocMemLen=size+1;
		if (MallocMem) {
			free(MallocMem);
			MallocMem=NULL;
		}
		if ((MallocMem=(char*) malloc(sizeof(char)*MallocMemLen))==NULL){
			MallocMemLen=0;
			return NULL;
		}
		*MallocMem='\0';
	}
	return MallocMem;
}

char* tFreeMem::GetDataMem(long *size){ // result is to be freed by caller
        char* tmp=MallocMem;
        if (size) *size = MallocMemLen;
	MallocMem=NULL;
	MallocMemLen=0;
        return tmp;
}

void tFreeMem::SetDataMem(char* mem, long size){ // set data to be the memory
	if (MallocMem){
		free(MallocMem);
		MallocMem=NULL;
	}
	MallocMemLen=size;
	MallocMem=mem;
}

char* tFreeMem::GetValue()
{
	return MallocMem;
}

char* tFreeMem::SetValue(const char* Value)
{
	if (Malloc(strlen(Value))==NULL) return NULL;
	strcpy(MallocMem,Value);
	return MallocMem;
}

char* tFreeMem::operator += (const char* Value)
{
	if (!Value) return MallocMem;

	operator += (strlen(Value));
	strcat(MallocMem,Value);

	return MallocMem;
}

char* tFreeMem::operator += (long addsize)
{
	char *p;
	long size=addsize;
	if (MallocMem) size += strlen(MallocMem);

	if (size >= MallocMemLen ){
		if ((p=(char*) malloc(sizeof(char)*(size+1)) )==NULL){
			return NULL;
		}
		*p='\0';
		if (MallocMem){
			strcpy(p, MallocMem);
			free(MallocMem);
		}
		MallocMem=p;
		MallocMemLen=size+1;
	}
	return MallocMem;
}

int tFreeMem::operator == (const char* Value)
{
	return (Compare(Value) == 0);
}

int tFreeMem::Compare(const char* Value)
{
	if (MallocMem == Value) return 0;
	if (!MallocMem) return -1;
	if (!Value) return 1;
	return strcmp(MallocMem, Value);
}

char* tFreeMem::Replace(const char* oldstr, const char* newstr)
{
	if (!MallocMem) return NULL;
	if (!newstr || !oldstr) return MallocMem;
	char* p=MallocMem;
	long offset;
	long size;
	while ((p=strstr(p,oldstr))!=NULL){
		offset=p-MallocMem;
		size=strlen(MallocMem)+strlen(newstr);

		if ((p=(char*) malloc(sizeof(char)*(size+1)) )==NULL){
			return NULL;
		}
		strncpy(p, MallocMem, offset);
		p[offset]='\0';
		strcat(p, newstr);
		strcat(p, MallocMem+ offset +strlen(oldstr) );

		free(MallocMem);
		MallocMem=p;
		MallocMemLen=size+1;

		p=MallocMem+ offset +strlen(newstr);
	}
	return MallocMem;
}
