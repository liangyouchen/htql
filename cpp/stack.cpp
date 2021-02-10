// LASTMODIFY CLY20000430
#include "log.h"
#include "stack.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


tStack::tStack(){
	Key=NULL;
	Value=NULL;
	Next=NULL;
	PreSearch=NULL;
	Type=0;
	Data=0;
}

tStack::tStack(char *OneKey, char* TheValue){
	Key=NULL;
	Value=NULL;
	Next=NULL;
	PreSearch=NULL;
	Type=0;
	Data=0;
	if (OneKey) Key=(char*)malloc(sizeof(char)*(strlen(OneKey)+1));
	if (TheValue) Value=(char*)malloc(sizeof(char)*(strlen(TheValue)+1));
	Data=0;
	Type=0;
	if (TheValue && Value==NULL) {
		if (Key) free(Key);
		Key=NULL;
	}
	if (Key && OneKey) if (OneKey) strcpy(Key,OneKey);
	if (Value&& TheValue) strcpy(Value,TheValue);
	
}

tStack::tStack(int StackType, long TheData, const char* OneKey, const char* TheValue)
{
	Key=NULL;
	Value=NULL;
	Next=NULL;
	PreSearch=NULL;
	Data=TheData;
	Type=StackType;
	if (OneKey) Key=(char*)malloc(sizeof(char)*(strlen(OneKey)+1));
	if (TheValue) Value=(char*)malloc(sizeof(char)*(strlen(TheValue)+1));
	if (TheValue && Value==NULL) {
		if (Key) free(Key);
		Key=NULL;
	}
	if (Key && OneKey) if (OneKey) strcpy(Key,OneKey);
	if (Value&& TheValue) strcpy(Value,TheValue);
}

tStack::~tStack(){
	reset();
}

int tStack::reset(){
	if (Key) free(Key);
	if (Value) free(Value);
	if (Next) {
		tStack* old=Next;
		tStack* p=old->Next;
		while (p) {
			old->Next=NULL;
			delete(old);
			old=p;
			p=old->Next;
		}
		delete old;
	}
	Key=NULL;
	Value=NULL;
	Next=NULL;
	PreSearch=NULL;
	Data=0;
	return True;
}

tStack* tStack::add(const char* OneKey, const char* TheValue, int AddToTail){
	tStack* newStack=NULL;
	tStack* tmp;
	if (AddToTail){
		for (tmp=this; tmp->Next; tmp=tmp->Next);
		newStack=new tStack(Type,0,OneKey,TheValue);
		if (!newStack->Key) {
			delete newStack;
			return NULL;
		}
		tmp->Next=newStack;
	}else{
		newStack=new tStack(Type,0,OneKey,TheValue);
		if (!newStack->Key){
			delete newStack;
			return NULL;
		}
		newStack->Next=Next;
		Next=newStack;
	}
	return newStack;
}

tStack* tStack::set(long TheData, const char* OneKey, const char* TheValue){
	tStack* newStack=NULL;
	tStack* tmp;
	if (Type == ordFILO || Type == ordFIFO){
		newStack= add(OneKey, TheValue,Type == ordFIFO);
		newStack->Data = TheData;
		return newStack;
	}

	switch (Type){
	case ordINCDATA:
		for (tmp=this; tmp->Next && TheData > tmp->Next->Data; tmp=tmp->Next);
		/*if (tmp->Next && tmp->Next->Data == TheData){
			if ( tmp->Next->newKey(OneKey) && tmp->Next->newValue(TheValue) )
				return tmp->Next;
			else 
				return NULL;
		}*/
		break;
	case ordDECDATA:
		for (tmp=this; tmp->Next && TheData < tmp->Next->Data; tmp=tmp->Next);
		break;
	case ordINCVALUE:
		for (tmp=this; tmp->Next && strcmp(TheValue, tmp->Next->Value) > 0; tmp=tmp->Next);
		break;
	case ordDECVALUE:
		for (tmp=this; tmp->Next && strcmp(TheValue, tmp->Next->Value) < 0; tmp=tmp->Next);
		break;
	case ordINCKEY:
		for (tmp=this; tmp->Next && strcmp(OneKey, tmp->Next->Key) > 0; tmp=tmp->Next);
		break;
	case ordDECKEY:
		for (tmp=this; tmp->Next && strcmp(OneKey, tmp->Next->Key) < 0; tmp=tmp->Next);
		break;
	default:
		tmp=this;
		break;
	}
	newStack=new tStack(Type,TheData,OneKey,TheValue);
	if (!newStack->Key){
		delete newStack;
		return NULL;
	}
	if (Type==ordINCDATA || Type==ordINCVALUE || Type==ordINCKEY){
		newStack->Next = tmp->Next;
		tmp->Next=newStack;
	}else{
		if (tmp->Next) {
			newStack->Next = tmp->Next->Next;
			tmp->Next->Next=newStack;
		}else{
			tmp->Next=newStack;
		}
	}
	return newStack;
}

char* tStack::newKey(const char* OneKey, long len){
	if (len<=0) len = strlen(OneKey);
	if (!Key || len>=sizeof(Key) ){
		if (Key) free(Key);
		Key=(char*)malloc(sizeof(char)*(len+1));
		if (!Key) return NULL;
	}
	memcpy(Key, OneKey, len);
	Key[len]=0;
	return Key;
}

char* tStack::newValue(const char* TheValue, long len){
	if (len<=0 && TheValue) len = strlen(TheValue);
	else if (len<0) len=0;
	if (!Value || (size_t) len>strlen(Value) ){
		if (Value) free(Value);
		Value=(char*)malloc(sizeof(char)*(len+1));
		if (!Value) return NULL;
	}
	if (TheValue){
		if ((size_t) len>strlen(TheValue)){
			strcpy(Value, TheValue);
		}else{
			memcpy(Value, TheValue, len);
			Value[len]=0;
		}
	}else{
		Value[0]=0;
	}
	return Value;
}

int tStack::match(const char* OneKey){
	if (Key && strcmp(OneKey, Key)==0)	return True;
	return False;
}

tStack* tStack::search(const char* OneKey, char** ValuePointer){
	if (ValuePointer) *ValuePointer=NULL;
	PreSearch=Next;
	while (PreSearch){
		if (PreSearch->match(OneKey)){
			if (ValuePointer) *ValuePointer=PreSearch->Value;
			return PreSearch;
		}
		PreSearch=PreSearch->Next;
	}
	return NULL;
}

tStack* tStack::search(long TheData){
//	*ValuePointer=NULL;
	PreSearch=Next;
	while (PreSearch){
		if (PreSearch->Data == TheData){
			return PreSearch;
		}
		PreSearch=PreSearch->Next;
	}
	return NULL;
}

tStack* tStack::searchNext(const char* OneKey, char** ValuePointer){
	if (ValuePointer) *ValuePointer=NULL;
	if (PreSearch)	PreSearch=PreSearch->Next;
	while (PreSearch){
		if (PreSearch->match(OneKey)){
			if (ValuePointer) *ValuePointer=PreSearch->Value;
			return PreSearch;
		}
		PreSearch=PreSearch->Next;
	}
	return 0;
}

