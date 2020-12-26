#ifndef BIN_DATA_H
#define BIN_DATA_H

#include "log.h"
#include <stddef.h>

class tBinData{
public:
	tBinData(size_t size=0);
	~tBinData();

	char* Data;
	size_t DataLen;
	size_t DataSize;

	char* Malloc(size_t size);
	void Free();
	void Clear();
	char* SetValue(const char* Value, size_t Len);

	char* Cat(const char* Value, size_t Len);
	char* operator += (size_t addsize);
	char* GetDataMem(size_t *len=0, size_t *size=0); // result is to be freed by caller
	void SetDataMem(char* mem, size_t len); // set data to be the memory
};

#endif
