// cly 20001005  -- modify Clear()

#include "bindata.h"

#include <string.h>
#include <malloc.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


tBinData::tBinData(size_t size)
{
	Data=NULL;
	DataLen=0;
	DataSize=0;
	if (size>0){
		Malloc(size);
	}
}

tBinData::~tBinData()
{
	if (Data){
		free(Data);
		Data=NULL;
	}
}

void tBinData::Free()
{
	if (Data){
		free(Data);
		Data=NULL;
	}
	DataLen=0;
	DataSize=0;
}

void tBinData::Clear()
{
	DataLen=0;
//	DataSize=0;
	if (Data) Data[0]='\0';
}

char* tBinData::Malloc(size_t size){
	DataLen=0;
	if (size >= DataSize ){
		DataSize=size+1;
		if (Data) {
			free(Data);
			Data=NULL;
		}
		if ((Data=(char*) malloc(sizeof(char)*DataSize))==NULL){
			DataSize=0;
			return NULL;
		}
	}
	*Data='\0';
	return Data;
}

char* tBinData::GetDataMem(size_t *len, size_t *size){ // result is to be freed by caller
	char* tmp=Data;
	if (len) *len = DataLen;
	if (size) *size = DataSize;
	Data = NULL;
	DataLen=0;
	DataSize=0;
	return tmp;
}

void tBinData::SetDataMem(char* mem, size_t len){ // set data to be the memory
	if (Data){
		free(Data);
		Data=NULL;
	}
	Data=mem;
	DataLen=len;
	DataSize=len+1;
}

char* tBinData::SetValue(const char* Value, size_t Len)
{
	if (Malloc(Len)==NULL) return NULL;
	if (Len) memcpy(Data,Value,Len);
	DataLen=Len;
	Data[DataLen]='\0';
	return Data;
}

char* tBinData::Cat (const char* Value, size_t Len)
{
	if (!Value) return Data;

	operator += (Len);
	memcpy(Data+DataLen,Value,Len);
	DataLen += Len;
	Data[DataLen]='\0';
	
	return Data;
}

char* tBinData::operator += (size_t addsize)
{
	char *p;
	size_t size=addsize;
	if (Data) size += DataLen;

	if (size >= DataSize ){
		if ((p=(char*) malloc(sizeof(char)*(size+1)) )==NULL){
			return NULL;
		}
		*p='\0';
		if (Data){
			memcpy(p, Data, DataLen);
			free(Data);
		}
		Data=p;
		Data[DataLen]='\0';
		DataSize=size+1;
	}
	return Data;
}

