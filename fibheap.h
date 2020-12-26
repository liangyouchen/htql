#ifndef FIBONACCI_HEAP_H
#define FIBONACCI_HEAP_H

///////////////////////////////////////////////////////
//
//  FIBONACCI HEAP implementation
//
/////////////////////////// ///////////////////////////
//
//  Author: Liangyou Chen
//  File Nmae: fibheap.h
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

#include "log.h"

#define MAX_KEY 0x7FFFFFF	//max value of key
//#define DO_STATISTICS	1	// to do statistocs

//////////////////////////////////////////////////////////
//		FibTree: item structure for Fibonacci heap 
///////////////////////////////////////////////////////////
class FibTree {
public:
	long key;		// key

	FibTree* p;		// parent
	FibTree* child;	// child 
	FibTree* left;	// left sibling
	FibTree* right;	// right sibling
	int degree;		// number of children in the child list
	short mark;		// wheather it has lost a child since the last time it is made the child of another node

	FibTree();
	virtual ~FibTree();		

	FibTree* concatenate(FibTree* list);	//concatenate two list together
	virtual int cmp(FibTree* k);
	virtual FibTree* maxValue(int is_max=true);	// is_max: true - max; false - min
	virtual int copyValue(FibTree* from);
};

class FibHeap {
public:
	FibTree* min;		// the root of the tree containing a minimum key
	unsigned long n;	// the nodes currently in heap
	unsigned long t;	// the number of trees in the root list of heap
	unsigned long m;	// the number of marked nodes in H

	FibHeap();
	~FibHeap();
	void reset();

#ifdef DO_STATISTICS
	unsigned long num_insert;
	unsigned long num_unite;
	unsigned long num_extract_min;
	unsigned long num_decrease_key;
	unsigned long num_deleting;
	unsigned long num_remove_node;
	unsigned long num_consolidate;
	unsigned long num_consolidate_cost;
	unsigned long num_cut;
	unsigned long num_cascading_cut;
#endif

	void insert(FibTree* x);	// insert x into heap  
	void unite(FibHeap* H);		// unite with heap H 
	FibTree* extract_min();		// extract minimum key from heap
	short decrease_key(FibTree* x, FibTree* k);	// decreate key of x to k
	short deleting(FibTree* x);	// delete node x
	void release_heap(FibTree* x);	// delete x and its children

protected:
	FibTree* remove_node(FibTree* z);	// remove z from heap;
	short consolidate();		// consolidate heap
	int max_degree();			// get max degree of the heap
	void link(FibTree* y, FibTree* x);	// link y as x's child
	void cut(FibTree* x, FibTree* y);	// remove x from y's children list
	void cascading_cut(FibTree* y);		// cascading cut y
};


#endif
