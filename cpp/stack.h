#ifndef STR_STACK_H
#define STR_STACK_H		CLY20000430

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef True
#define True		1
#endif
#ifndef False
#define False		0
#endif

class tStack{
	tStack *PreSearch;
public:
	enum {ordFILO, ordFIFO, ordINCKEY, ordDECKEY,ordINCVALUE,ordDECVALUE,ordINCDATA, ordDECDATA };
	int Type;
	char *Value;
	char *Key;
	intptr_t Data;
	tStack *Next;
	tStack* add(const char* OneKey,const char* TheValue, int AddToTail=False);
	tStack* set(intptr_t TheData,const char* OneKey,const char* TheValue);
	char* newKey(const char* OneKey, long len=0);
	char* newValue(const char* TheValue,long len=0);
	int match(const char* OneKey);
	tStack* search(const char* OneKey,char**ValuePointer);
	tStack* search(intptr_t TheData);
	tStack* searchNext(const char* OneKey,char**ValuePointer);
	tStack();
	int reset();
	tStack(char* OneKey, char* TheValue);
	tStack(int StackType, intptr_t TheData, const char* OneKey, const char* TheValue);
	~tStack();
};

#endif

