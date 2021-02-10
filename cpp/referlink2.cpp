#include "log.h"
#include "referlink2.h"
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef WIN32
#include <comdef.h>
#include <atlconv.h>
#endif

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




ReferLink2::ReferLink2(){
	Prev=this;
	Next=this;
}

ReferLink2::~ReferLink2(){
	reset();
	Prev=0;
	Next=0; //so that ReferLink can delete itself
}

void ReferLink2::reset(){
	ReferLink2* p=(ReferLink2*)Next;
	while (p && p!=this){
		Next = p->Next;
		p->Next=0;
		delete p;
		p=(ReferLink2*) Next;
	}
	Data=0;
	Prev=this;
	Next=this; 
}

ReferLink2* ReferLink2::insert(ReferLink2*p){
	if (!p) {
		p=new ReferLink2;
	}
	if (!this) return p; 

	ReferLink2* p_last=p->Prev;
	if (!p_last) p_last=p; 
	if (Prev){
		p->Prev=Prev;
		Prev->Next=p;
		p_last->Next=this;
		Prev=p_last;
	}else{
		p_last->Next=this;
		p->Prev = this;
		//p->Next=p->Prev=this;
		//Next=Prev=p;
		Next=p;
		Prev=p_last; 
	}
	return p;
}

int ReferLink2::remove(ReferLink2*p){
	if (!p) return 0;

	p->Prev->Next=p->Next;
	((ReferLink2*)p->Next)->Prev=p->Prev;

	p->Next=p->Prev=0;
	delete p;

	/*if (p->Next==p->Prev){
		if (p->Next && p->Next!=this){
			p->Next->Next=0;
			((ReferLink2*)p->Next)->Prev=0;
		}
		p->Next=p->Prev=0;
		delete p;
		if (p!=this) {
			Next=Prev=0;
		}
	}else{ 
		p->Prev->Next=p->Next;
		((ReferLink2*)p->Next)->Prev=p->Prev;
		if (p->Prev->Next==p->Prev){
			p->Prev->Next=((ReferLink2*)p->Next)->Prev=0;
		}
		p->Next=p->Prev=0;
		delete p;
	} */
	return 0; 
}


