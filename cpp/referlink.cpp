#include "log.h"
#include "referlink.h"
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




ReferLink::ReferLink(){
	Data=0;
	Next=0;
}

ReferLink::~ReferLink(){
	reset();
}

void ReferLink::reset(){
	ReferLink* p=Next;
	while (p){
		Next = p->Next;
		p->Next=0;
		delete p;
		p=Next;
	}
	Next=0;
	Data=0;
}

ReferLinkBTree::ReferLinkBTree(){
	SortOrder = SORT_ORDER_KEY_STR_INC;
	IsCaseSensitive = true;
}

ReferLinkBTree::~ReferLinkBTree(){
	reset();
}

void ReferLinkBTree::reset(){
	BTree::reset();
	SortOrder = SORT_ORDER_KEY_STR_INC;
}

int ReferLinkBTree::cmp(char* p1, char* p2){
	int i=0;
	switch(SortOrder){
	case SORT_ORDER_KEY_STR_INC:
		return ((ReferLink*)p1)->Name.Cmp(&((ReferLink*) p2)->Name, IsCaseSensitive);
	case SORT_ORDER_KEY_STR_DEC:
		return -((ReferLink*)p1)->Name.Cmp(&((ReferLink*) p2)->Name, IsCaseSensitive);
	case SORT_ORDER_VAL_STR_INC:
		return ((ReferLink*)p1)->Value.Cmp(&((ReferLink*) p2)->Value, IsCaseSensitive);
	case SORT_ORDER_VAL_STR_DEC:
		return -((ReferLink*)p1)->Value.Cmp(&((ReferLink*) p2)->Value, IsCaseSensitive);
	case SORT_ORDER_NUM_INC:
		return ((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
	case SORT_ORDER_NUM_DEC:
		return ((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?-1:1;
	case SORT_ORDER_KEY_NUM_INC:
		i=((ReferLink*)p1)->Name.Cmp(&((ReferLink*) p2)->Name, IsCaseSensitive);
		if (i==0) {
			i=((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
		}
		return i;
	case SORT_ORDER_KEY_NUM_DEC:
		i=((ReferLink*)p1)->Name.Cmp(&((ReferLink*) p2)->Name, IsCaseSensitive);
		if (i==0) {
			i=((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
		}
		return -i;
	case SORT_ORDER_VAL_NUM_INC:
		i=((ReferLink*)p1)->Value.Cmp(&((ReferLink*) p2)->Value, IsCaseSensitive);
		if (i==0) {
			i=((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
		}
		return i;
	case SORT_ORDER_VAL_NUM_DEC:
		i=((ReferLink*)p1)->Value.Cmp(&((ReferLink*) p2)->Value, IsCaseSensitive);
		if (i==0) {
			i=((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
		}
		return -i;
	case SORT_ORDER_KEY_VAL_STR_INC:
		i=((ReferLink*)p1)->Name.Cmp(&((ReferLink*) p2)->Name, IsCaseSensitive);
		if (i==0) {
			i=((ReferLink*)p1)->Value.Cmp(&((ReferLink*) p2)->Value, IsCaseSensitive);
		}
		return i;
	case SORT_ORDER_KEY_VAL_STR_DEC:
		i=((ReferLink*)p1)->Name.Cmp(&((ReferLink*) p2)->Name, IsCaseSensitive);
		if (i==0) {
			i=((ReferLink*)p1)->Value.Cmp(&((ReferLink*) p2)->Value, IsCaseSensitive);
		}
		return -i;
	case SORT_ORDER_KEY_VAL_NUM_INC:
		i=((ReferLink*)p1)->Name.Cmp(&((ReferLink*) p2)->Name, IsCaseSensitive);
		if (i==0) {
			i=((ReferLink*)p1)->Value.Cmp(&((ReferLink*) p2)->Value, IsCaseSensitive);
			if (i==0){
				i=((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
			}
		}
		return i;
	case SORT_ORDER_KEY_VAL_NUM_DEC:
		i=((ReferLink*)p1)->Name.Cmp(&((ReferLink*) p2)->Name, IsCaseSensitive);
		if (i==0) {
			i=((ReferLink*)p1)->Value.Cmp(&((ReferLink*) p2)->Value, IsCaseSensitive);
			if (i==0){
				i=((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
			}
		}
		return -i;
	case SORT_ORDER_KEYL_VALL_NUM_INC:
		i=((ReferLink*)p1)->Name.L==((ReferLink*) p2)->Name.L?0:((ReferLink*)p1)->Name.L>((ReferLink*) p2)->Name.L?1:-1;
		if (i==0){
			i=((ReferLink*)p1)->Value.L==((ReferLink*) p2)->Value.L?0:((ReferLink*)p1)->Value.L>((ReferLink*) p2)->Value.L?1:-1;
			if (i==0){
				i=((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
			}
		}
		return i;
	case SORT_ORDER_KEYL_VALL_NUM_DEC:
		i=((ReferLink*)p1)->Name.L==((ReferLink*) p2)->Name.L?0:((ReferLink*)p1)->Name.L>((ReferLink*) p2)->Name.L?1:-1;
		if (i==0){
			i=((ReferLink*)p1)->Value.L==((ReferLink*) p2)->Value.L?0:((ReferLink*)p1)->Value.L>((ReferLink*) p2)->Value.L?1:-1;
			if (i==0){
				i=((ReferLink*)p1)->Data==((ReferLink*) p2)->Data?0:((ReferLink*)p1)->Data>((ReferLink*) p2)->Data?1:-1;
			}
		}
		return -i;
	default:
		return 0;
	}
}


ReferLinkHeap::ReferLinkHeap(){
	Next=0;
	tree=&Heap;
	Total=0;
}
ReferLinkHeap::~ReferLinkHeap(){
	reset();
}

void ReferLinkHeap::reset(){
	if (Next) delete Next;
	Next=0;
	Heap.reset();
	LinkToFind.reset();
	Total=0;
	
	BTreeRecord::reset();
	tree = &Heap;
}

int ReferLinkHeap::empty(){
	if (Next) delete Next;
	Next=0;
	Heap.empty();
	LinkToFind.reset();
	Total=0;

	BTreeRecord::reset();
	tree = &Heap;
	return 0;
}


int ReferLinkHeap::setDuplication(int can_duplicate){
	Heap.NoDuplicate = !can_duplicate;
	return 0;
}
int ReferLinkHeap::setSortOrder(HeapSortOrder sort_order){
	Heap.SortOrder = sort_order;
	return 0;
}

int ReferLinkHeap::setCaseSensitivity(int sensitivity){
	Heap.IsCaseSensitive = sensitivity;
	return 0;
}
int ReferLinkHeap::take(ReferLinkHeap* from, int with_settings){
	if (with_settings){
		setDuplication(!from->Heap.NoDuplicate);
		setSortOrder(from->Heap.SortOrder);
		setCaseSensitivity(from->Heap.IsCaseSensitive); 
	}
	while (from->Next){
		ReferLink* p=from->Next; 
		from->Next = p->Next; 
		p->Next = 0;
		insertLink(p);
	}
	//int err=add(from->Next);
	//from->Next=0;
	from->empty(); 
	return 0;
}

ReferLink* ReferLinkHeap::add(const char* name, const char* value, long data){
	ReferLink* p = new ReferLink;
	if (name) p->Name.Set((char*) name, strlen( name), true);
	if (value) p->Value.Set((char*) value, strlen(value), true);
	p->Data = data;
	return insertLink(p);
}

ReferLink* ReferLinkHeap::add(ReferData* name, ReferData* value, long data){
	ReferLink* p = new ReferLink;
	if (name) p->Name.Set(name->P, name->L, name->P!=0);
	if (value) p->Value.Set(value->P, value->L, value->P!=0);
	p->Data = data;
	return insertLink(p);
}
long ReferLinkHeap::add(ReferLink* list){
	long count=0;
	for (ReferLink* link=list; link; link=link->Next){
		if (add(&link->Name, &link->Value, link->Data)) count++;
	}
	return count;
}
ReferLink* ReferLinkHeap::insertLink(ReferLink*p){
	if (!Heap.insert((char*) p)){
		delete p;
		return 0;
	}
	p->Next = Next;
	Next = p;
	Total++;
	return p;
}
int ReferLinkHeap::resort(){
	Total=0; 
	Heap.empty();
	ReferLink* link; 
	for (link=Next; link; link=link->Next){
		if (Heap.insert((char*) link)) Total++;
	}
	return 0;
}
ReferLink* ReferLinkHeap::findName(ReferData* name){
	if (Heap.SortOrder != SORT_ORDER_KEY_STR_INC && Heap.SortOrder != SORT_ORDER_KEY_STR_DEC){
		return NULL;
	}
	LinkToFind.Name.Set(name->P, name->L, name->P!=0);
	return (ReferLink*) Heap.find((char*) &LinkToFind, 0);
}
ReferLink* ReferLinkHeap::findName(const char* name){
	if (Heap.SortOrder != SORT_ORDER_KEY_STR_INC && Heap.SortOrder != SORT_ORDER_KEY_STR_DEC){
		return NULL;
	}
	LinkToFind.Name=name;
	return (ReferLink*) Heap.find((char*) &LinkToFind, 0);
}
ReferLink* ReferLinkHeap::findValue(ReferData* value){
	if (Heap.SortOrder != SORT_ORDER_VAL_STR_INC && Heap.SortOrder != SORT_ORDER_VAL_STR_DEC){
		return NULL;
	}
	LinkToFind.Value.Set(value->P, value->L, value->P!=0);
	return (ReferLink*) Heap.find((char*) &LinkToFind, 0);
}
ReferLink* ReferLinkHeap::findValue(const char* value){
	if (Heap.SortOrder != SORT_ORDER_VAL_STR_INC && Heap.SortOrder != SORT_ORDER_VAL_STR_DEC){
		return NULL;
	}
	LinkToFind.Value=value;
	return (ReferLink*) Heap.find((char*) &LinkToFind, 0);
}
ReferData* ReferLinkHeap::getValue(const char* name){
	ReferLink* link;
	if (Heap.SortOrder == SORT_ORDER_KEY_STR_INC || Heap.SortOrder == SORT_ORDER_KEY_STR_DEC){
		LinkToFind.Name = name;
		link=(ReferLink*) Heap.find((char*) &LinkToFind, 0);
		return (link)?(&link->Value):0;
	}else if (Heap.SortOrder == SORT_ORDER_VAL_STR_INC || Heap.SortOrder == SORT_ORDER_VAL_STR_DEC){
		LinkToFind.Value = name;
		link=(ReferLink*) Heap.find((char*) &LinkToFind, 0);
		return (link)?(&link->Name):0;
	}else{
		return 0;
	}
}

ReferLink* ReferLinkHeap::findData(long data){
	if (Heap.SortOrder != SORT_ORDER_NUM_INC && Heap.SortOrder != SORT_ORDER_NUM_DEC
	//	&& Heap.SortOrder != SORT_ORDER_VAL_NUM_INC && Heap.SortOrder != SORT_ORDER_VAL_NUM_DEC
		){
		return NULL;
	}
	LinkToFind.Data = data;
	return (ReferLink*) Heap.find((char*) &LinkToFind, 0);
}
ReferLink* ReferLinkHeap::find(ReferData* name, ReferData* value, long data){
	if (name) LinkToFind.Name.Set(name->P, name->L, name->P!=0);
	if (value) LinkToFind.Value.Set(value->P, value->L, value->P!=0);
	LinkToFind.Data = data;

	return (ReferLink*) Heap.find((char*) &LinkToFind, 0);
}
ReferLink* ReferLinkHeap::find(ReferLink* link){
	return (ReferLink*) Heap.find((char*) link, 0);
}

ReferLink* ReferLinkHeap::getCurrReferLink(){
	return (ReferLink*) BTreeRecord::getValue();
}

ReferLink* ReferLinkHeap::getReferLinkHead(){
	return Next;
}
int ReferLinkHeap::remove(ReferLink* link){
	//now the removing is expensive
	
	Heap.remove( (char*) link, 1); //duplicate keys are also removed!! to do later
	ReferLink** p=&Next;
	while (*p && *p!=link){
		p=&(*p)->Next;
	}
	if (*p==link){
		*p=link->Next;
		link->Next=0;
		delete link;
		Total--;
	}
	return 0;
}





ReferLinkNamedHeap::ReferLinkNamedHeap(){
}

ReferLinkNamedHeap::~ReferLinkNamedHeap(){
	reset();
}
void ReferLinkNamedHeap::reset(){
	for (ReferLink* link=HeapMemory.getReferLinkHead(); link; link=link->Next){
		if (link->Data){
			delete (ReferLinkHeap*) (link->Data); 
			link->Data=0; 
		}
	}
	HeapMemory.reset();
}
ReferLinkHeap* ReferLinkNamedHeap::getHeap(const char* name){
	ReferLink* link=HeapMemory.findName(name);
	if (link) return (ReferLinkHeap*) link->Data;
	return 0;
}
ReferLinkHeap* ReferLinkNamedHeap::createHeap(const char* name, int case_sensitive, int duplicate, HeapSortOrder sort_type){
	ReferLink* link=HeapMemory.findName(name);
	if (!link) {
		link=HeapMemory.add(name, 0, 0);
		ReferLinkHeap* set=new ReferLinkHeap;
		set->setCaseSensitivity(case_sensitive);
		set->setDuplication(duplicate);
		set->setSortOrder(sort_type);
		link->Data=(long) set;
	}
	return (ReferLinkHeap*) link->Data;
}
int ReferLinkNamedHeap::deleteHeap(const char* name){
	ReferLink* link=HeapMemory.findName(name);
	if (link){
		if (link->Data) delete (ReferLinkHeap*) (link->Data); 
		link->Data=0;
		HeapMemory.remove(link);
		return true;
	}
	return false;
}




