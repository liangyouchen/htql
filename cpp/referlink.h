#ifndef REFER_LINK_H
#define REFER_LINK_H

#include <stddef.h> 
#include "referdata.h"

class ReferLink{
public:
	ReferData Name;
	ReferData Value;
	long Data;
	ReferLink* Next;
	
	ReferLink();
	~ReferLink();
	void reset();
};

#include "btree.h"
#include "heapsortorder.h"

class ReferLinkBTree: public BTree{
public:
	int IsCaseSensitive;
	HeapSortOrder SortOrder;
	virtual int cmp(char* p1, char* p2);

	ReferLinkBTree();
	~ReferLinkBTree();
	void reset();
};

class ReferLinkHeap: public BTreeRecord{
public:
	ReferLink* Next;
	ReferLink LinkToFind;
public:
	ReferLinkBTree Heap;
	unsigned long Total;
	ReferLinkHeap();
	~ReferLinkHeap();
	void reset();

	int setDuplication(int can_duplicate);
	int setSortOrder(HeapSortOrder sort_order);
	int setCaseSensitivity(int sensitivity=true);

	ReferLink* add(ReferData* name, ReferData* value, long data);
	ReferLink* add(const char* name, const char* value, long data);
	long add(ReferLink* list);
	int remove(ReferLink* link); //remove a single link pointer
	int empty();
	int resort(); 
	int take(ReferLinkHeap* from, int with_settings=false); 
	ReferLink* findName(ReferData* name);
	ReferLink* findName(const char* name);
	ReferLink* findValue(ReferData* value);
	ReferLink* findValue(const char* value);
	ReferLink* findData(long data);
	ReferLink* find(ReferData* name, ReferData* value, long data);
	ReferLink* find(ReferLink* link);
	ReferData* getValue(const char* name);
	ReferLink* getCurrReferLink();
	ReferLink* getReferLinkHead();
	ReferLink* insertLink(ReferLink*p);
};

class ReferLinkNamedHeap {
public:
	ReferLinkHeap HeapMemory;
	ReferLinkHeap* getHeap(const char* name);
	ReferLinkHeap* createHeap(const char* name, int case_sensitive=false, int duplicate=false, HeapSortOrder sort_type=SORT_ORDER_KEY_STR_INC);
	int deleteHeap(const char* name);

public:
	ReferLinkNamedHeap(); 
	~ReferLinkNamedHeap();
	void reset();
};


#endif
