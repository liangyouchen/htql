#ifndef MYLINK_H
#define MYLINK_H

#include "log.h"
#include <stdio.h>

class Links {
public:
	long Data;
	void* p;
	Links* Next; // Next=0 for the last link
	Links* Prev; // Prev=0 for the first link
	Links* find(void* P);
	Links* add(void* New, Links* link=0); // add to the tail of link;
	Links* append(void* New, Links* link=0); // append to next
	Links* insert(void* New, Links* link=0); // insert before
	int remove(void* Old); // remove where p = Old
	int remove(int delete_this=true); // remove this;
	void empty(); // delete the Next;

	Links(void* P=NULL);
	~Links();
	void reset();
};

class LinkList{
public:
	Links* Head;
	Links* Tail;

	enum{posLinkHead, posLinkTail};
	Links* add(void*p, int pos=posLinkTail, Links* link=0); 
	int remove(Links*p, int delete_link=true);

	LinkList();
	~LinkList();
	void reset();
};

#endif

