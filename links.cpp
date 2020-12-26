#include "links.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


Links::Links(void* P){
	p=P;
	Next=Prev=NULL;
	Data=0;
}

Links::~Links(){
	reset();
}

void Links::reset(){
	empty();
	/*
	if (Next) delete Next;
	*/
	p=NULL;
	Next=Prev=NULL;
}

Links* Links::add(void* New, Links* link){
	Links* last=this;
	while (last->Next) last=last->Next;
	if (!link) link=new Links(New);
	else if (New) link->p=New;
	last->Next=link;
	if (!last->Next) return NULL;
	last->Next->Prev=last;
	return last->Next;

/*	Links** n=&Next;
	while (*n) n= &((*n)->Next);
	(*n) = new Links(New);
	if (!(*n)) return NULL;
	(*n)->Prev = this;
	return (*n);
	*/
}

Links* Links::append(void* New, Links* link){
	Links* oldnext=Next;
	if (!link) new Links(New);
	else if (New) link->p=New;
	Next = link;
	if (!Next) {
		Next = oldnext;
		return NULL;
	}
	Next->Prev = this;
	Next->Next = oldnext;
	if (oldnext) oldnext->Prev = Next;

	return Next;
}


Links* Links::insert(void* New, Links* link){
	Links* oldprev=Prev;
	if (!link) new Links(New);
	else if (New) link->p=New;
	Prev = link;
	if (!Prev) {
		Prev = oldprev;
		return NULL;
	}
	Prev->Next = this;
	Prev->Prev = oldprev;
	if (oldprev) oldprev->Next = Prev;

	return Prev;
}

Links* Links::find(void* P){
	if (p == P) return this;
	Links* n;
	for (n=Next; n; n=n->Next){
		if (n->p == P) return n;
	}
	for (n=Prev; n; n=n->Prev){
		if (n->p == P) return n;
	}
	return NULL;
	/*
	if (!Next) return NULL;
	return Next->find(P);
	*/
}

int Links::remove(void* Old){
	if (p == Old) return remove();
	Links* first=this;
	while (first && first->Prev) first=first->Prev;
	int delete_count=0;
	for (Links* n=first; n&&n->Next ; ){
		if (n->p == Old){
			Links* tmp=n->Next;
			n->remove();
			n=tmp;
			delete_count++;
		}else{
			n=n->Next;
		}
		/*
		if (n->Next->p ==Old) {
			Links* now=n->Next;
			if (now->Next) now->Next->Prev = n;
			n->Next = now->Next;
			now->Next=NULL;
			now->Prev=NULL;
			delete now;
			return true;
		}*/
	}
	return delete_count;
}

int Links::remove(int delete_this){
	if (Prev) Prev->Next = Next;
	if (Next) Next->Prev = Prev;
	Prev=Next=NULL;
	if (delete_this) delete this; //must support this.  otherwise, need to modify remove(void*) function
	return true;
}

void Links::empty(){
	Links* n1=Next;
	Links* n2;
	while (n1){
		n2=n1->Next;
		n1->Next=NULL;
		delete n1;
		n1=n2;
	}
	Next=NULL;
	/*
	if (Next) delete Next;
	Next=NULL;
	*/
}

LinkList::LinkList(){
	Head=Tail=0;
}

LinkList::~LinkList(){
	reset();
}

void LinkList::reset(){
	if (Head) delete Head;
	Head=Tail=0;
}

Links* LinkList::add(void*p, int pos, Links* link){
	if (!link) link=new Links(p);
	else {
		link->Next=link->Prev=0;
		if (p) link->p=p;
	}
	if (!link) return 0;

	if (pos==posLinkHead){
		if (!Head){
			Head=Tail=link;
		}else{
			Head=Head->insert(0, link);
		}
		return Head;
	}else{
		if (!Tail){
			Head=Tail=link;
		}else{
			Tail=Tail->append(0, link);
		}
		return Tail;
	}
}
int LinkList::remove(Links*p, int delete_link){
	Links* keep=0;
	if (p==Head){
		keep=Head->Next;
		Head->remove(delete_link);
		Head=keep;
		if (!Head) Tail=0;
	}else if (p==Tail){
		keep=Tail->Prev;
		Tail->remove(delete_link);
		Tail=keep;
		if (!Tail) Tail=Head;
	}else{
		p->remove(delete_link);
	}
	return 0;
}
