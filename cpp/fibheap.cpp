#include "fibheap.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////////////////////////////
//
//  FIBONACCI HEAP implementation
//
/////////////////////////// ///////////////////////////
//
//  Author: Liangyou Chen
//  File Nmae: fibheap.cpp
//  Created Date: Feb 13, 2001
//  Version: 1.0
//
////////////////////////////////////////////////////////
//
//  Class: 
//		FibTree: item structure for Fibonacci heap 
//		FibHeap: Base class for Fibonacci heap
//
////////////////////////////////////////////////////////


FibTree::FibTree(){
	p=NULL;
	child=NULL;
	left=this;
	right=this;
	degree=0;
	mark=false;
}

FibTree::~FibTree() {

}

/////////////////////////////////////////////////////
// concatenate two list together
/////////////////////////////////////////////////////
FibTree* FibTree::concatenate(FibTree *list){
	if (!list) return this;

	right->left = list->left;
	list->left->right = right;
	list->left = this;
	right = list;

	return (cmp(list) <0) ? this: list;
}

int FibTree::cmp(FibTree* k){
	if (key == k->key) return 0;
	return (key > k->key )? 1: -1;
}

FibTree* FibTree::maxValue(int is_max){
	FibTree* t=new FibTree;
	t->key = MAX_KEY;
	if (!is_max) t->key = -t->key;
	return t;
}

int FibTree::copyValue(FibTree* from){
	key = from->key;
	return 0;
}

FibHeap::FibHeap(){
	reset();
}

FibHeap::~FibHeap(){
}
void FibHeap::reset(){
	min=NULL;
	n=0;
	t=0;
	m=0;

#ifdef DO_STATISTICS
	num_insert=
	num_unite=
	num_extract_min=
	num_decrease_key=
	num_deleting=
	num_remove_node=
	num_consolidate=
	num_consolidate_cost=
	num_cut=
	num_cascading_cut=0;
#endif

}

/////////////////////////////////////////////////////
// insert x into heap  
/////////////////////////////////////////////////////
void FibHeap::insert(FibTree * x){
	if (!x) return;

#ifdef DO_STATISTICS
	num_insert++;
#endif

	min = x->concatenate(min);
	n++;
	t++; //this is not in text book
	return;
}

/////////////////////////////////////////////////////
// unite with heap H 
/////////////////////////////////////////////////////
void FibHeap::unite(FibHeap* H){

	if (!H || !H->min) return;

#ifdef DO_STATISTICS
	num_unite++;
#endif

	min = H->min->concatenate(min);

	n += H->n;
	m += H->m;  //this is not in text book
	t += H->t;	//this is not in text book

	delete H;
}

/////////////////////////////////////////////////////
// extract minimum key from heap
/////////////////////////////////////////////////////
FibTree* FibHeap::extract_min(){

	FibTree* z = min;
	if (!z) return NULL;
	
#ifdef DO_STATISTICS
	num_extract_min++;
#endif

	FibTree* each_child = z->child;
	if (each_child){
		each_child->p = NULL;
		for (each_child = each_child->right; each_child && each_child != z->child; each_child = each_child->right){
			each_child ->p = NULL;
		}
		min = z->child->concatenate(min);
	}
	min = remove_node(z);
	if (min) {
		if (!consolidate()) return NULL;
	}
	n--;
	return z;
}

/////////////////////////////////////////////////////
// decreate key of x to k
/////////////////////////////////////////////////////
short FibHeap::decrease_key(FibTree* x, FibTree* k){
	
	if (x->cmp(k)<0) return false;

#ifdef DO_STATISTICS
	num_decrease_key++;
#endif

	x->copyValue(k);
	FibTree* y = x->p;
	if (y && x->cmp(y) <0) {
		cut(x,y);
		cascading_cut(y);
	}

	if (x->cmp(min) < 0) 
		min = x;

	return true;
}

/////////////////////////////////////////////////////
// delete node x
/////////////////////////////////////////////////////
short FibHeap::deleting(FibTree* x){

#ifdef DO_STATISTICS
	num_deleting++;
#endif

	FibTree* m=x->maxValue(false);
	short r1=decrease_key(x, m);
	delete m;
	FibTree* r2=extract_min();

	return r1 && (r2 || !min);
}

/////////////////////////////////////////////////////
// remove z from heap;
/////////////////////////////////////////////////////
FibTree* FibHeap::remove_node(FibTree* z){

#ifdef DO_STATISTICS
	num_remove_node++;
#endif

	z->left->right = z->right;
	z->right->left = z->left;
	if (z == z->right) return NULL;
	return z->right;
}

/////////////////////////////////////////////////////
// get max degree of the heap
/////////////////////////////////////////////////////
int FibHeap::max_degree(){
	if (n<=1) return 0;
	double m=log(double(n))/log(double(2))+1;

	return (int)m + 1;
}

/////////////////////////////////////////////////////
// consolidate heap
/////////////////////////////////////////////////////
short FibHeap::consolidate(){

	if (!min || min == min->right) return true;

#ifdef DO_STATISTICS
	num_consolidate++;
#endif

	int D = max_degree();
	FibTree** A = (FibTree**) malloc (sizeof(FibTree*)*D);
	if (!A) return false;
	memset(A,0,sizeof(FibTree*)*D);

	FibTree *w=min, *x=NULL, *y=NULL;
	FibTree *last = min->left;
	int finish = false;
	while (!finish){
		x=w;
		if (w == last) finish = true;
		w=w->right;

		int d=x->degree;
		while (A[d]){

#ifdef DO_STATISTICS
	num_consolidate_cost++;
#endif
			y=A[d];
			if (x->cmp(y) >0 ){
				FibTree* tmp = x;
				x=y;
				y=tmp;
			}
			link(y,x);
			A[d]=NULL;
			d++;
			if (d >= D) {
				free(A);
				return false;
			}
		}
		A[d] = x;
	};
	
	min=NULL;
	for (int i=0; i<D; i++){
		if (A[i]){
			if (!min || min->cmp(A[i]) > 0 ) 
				min=A[i];
		}
	}

	free(A);
	return true;
}

/////////////////////////////////////////////////////
// link y as x's child
/////////////////////////////////////////////////////
void FibHeap::link(FibTree* y, FibTree* x){
	if (!y || !x ) return;
	remove_node(y);
	y->left = y->right = y;
	y->mark = false;
	y->p = x;

	x->child = y->concatenate(x->child);
	x->degree ++;
}

/////////////////////////////////////////////////////
// remove x from y's children list
/////////////////////////////////////////////////////
void FibHeap::cut(FibTree* x, FibTree* y){

#ifdef DO_STATISTICS
	num_cut++;
#endif

	FibTree* c = remove_node(x);
	x->p = NULL;
	x->left = x->right = x;
	x->mark = false;

	min= x->concatenate(min);

	if (y->child == x) y->child = c;
	y->degree --;

}

/////////////////////////////////////////////////////
// cascading cut y
/////////////////////////////////////////////////////
void FibHeap::cascading_cut(FibTree* y){

#ifdef DO_STATISTICS
	num_cascading_cut++;
#endif

	FibTree* z = y->p;
	if (z ){
		if (y->mark){
			cut(y,z);
			cascading_cut(z);
		}else{
			y->mark = true;
		}
	}
}

/////////////////////////////////////////////////////
// delete x and its children
/////////////////////////////////////////////////////
void FibHeap::release_heap(FibTree* x){
	if (!x) return;
	
	FibTree* tmp;
	FibTree* c = x->child;
	if (c){
		do{
			tmp = c->right;
			release_heap(c);
			c=tmp;
		}while (c && c!= x->child);
	}
	
	delete x;
}

