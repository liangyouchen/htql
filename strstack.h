#ifndef CLY_BTREE_H
#define CLY_BTREE_H

#include "log.h"
#include "fibheap.h" 
#include "bindata.h"

typedef enum{ 
		ORDER_KEY_STR_INC, 
		ORDER_KEY_STR_DEC, 
		ORDER_VAL_STR_INC, 
		ORDER_VAL_STR_DEC, 
		ORDER_KEY_NUM_INC, 
		ORDER_KEY_NUM_DEC, 
		ORDER_VAL_NUM_INC, 
		ORDER_VAL_NUM_DEC, 
	} SortOrder;

class SNode: public FibTree{
public: 
	tBinData key_str;
	tBinData value_str;
	long value;
	SNode* link_to;
	SNode* next;
	SNode* prev;
	int sort_order;

	SNode();
	SNode(char* k, size_t len1=0, char* v=0, size_t len2=0);
	virtual void init();
	virtual ~SNode();

public:
	virtual char* setKey(char* buf, size_t len);
	virtual long setKey(long val);
	virtual char* setValue(char* buf, size_t len);
	virtual long setValue(long val);
	virtual int cmp(char* buffer, size_t str_len, char* buf, size_t len);
	virtual int cmp(long val1, long val2);

	virtual int cmp(FibTree* k);
	virtual FibTree* maxValue(int is_max=true);	// is_max: true - max; false - min
	virtual int copyValue(FibTree* from);
};


class StringStack{
public:

	SortOrder order;
	FibHeap heap;
	SNode* first;
	SNode* last;
	SNode* current;

	SNode* link_last;
	SNode* link_first;
	long link_num;

public:
	StringStack();
	~StringStack();
	void reset();
	void setOrder(SortOrder ord);
	SNode* insert(SNode* n);
	SNode* moveNext();
	SNode* movePrevious();
	long resort();
};

#endif
