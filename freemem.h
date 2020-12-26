#ifndef FREE_MEM_H
#define FREE_MEM_H		CLY20000430

#include "log.h"

class tFreeMem{
public:
	tFreeMem(long size=0);
	~tFreeMem();
	char* Malloc(long size);
	void Free();
	char* GetDataMem(long* size=0);
	void SetDataMem(char* mem, long size);
	char* SetValue(const char* Value);
	char* GetValue();
	char* operator += (const char* Value);
	char* operator += (long addsize);
	int operator == (const char* Value);
	int Compare(const char * Value);
	char* Replace(const char* oldstr, const char* newstr);
private:
	char* MallocMem;
	long MallocMemLen;
};

#endif

