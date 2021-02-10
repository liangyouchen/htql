#include "strstack.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


SNode::SNode(){
	init();
}

SNode::~SNode(){
	init();
}

SNode::SNode(char* k, size_t len1, char* v, size_t len2){
	init();
	if (k){
		if (!len1) len1=strlen(k);
		setKey(k, len1);
	}
	if (v){
		if (!len2) len2=strlen(v);
		setValue(v, len2);
	}
}

void SNode::init(){
	value=0;
	sort_order=0;
	next = prev = 0;
	link_to = 0;
}

char* SNode::setKey(char* buf, size_t len){
	return key_str.SetValue(buf, len);
}

long SNode::setKey(long val){
	key = val;
	return key;
}

char* SNode::setValue(char* buf, size_t len){
	return value_str.SetValue(buf, len);
}

long SNode::setValue(long val){
	value = val;
	return value;
}

int SNode::cmp(char* buffer, size_t str_len, char* buf, size_t len){
	if (!buffer || !buf) {
		if (buffer == buf ) return 0;
		return (buffer > buf)?1:-1;
	}
	unsigned int l=(str_len>len)?len:str_len;
	for (unsigned int i=0; i<l; i++){
		if (buffer[i]> buf[i]) 
			return 1;
		else if (buffer[i] < buf[i]) 
			return -1;
	}
	if (str_len == len ) return 0;
	return (str_len > len)?1:-1;
}

int SNode::cmp(long val1, long val2){
	if (val1 == val2) return 0;
	return (val1 > val2)? 1: -1;
}

int SNode::cmp(FibTree* k){
	SNode* k1=(SNode*) k;
	switch (sort_order){
	case ORDER_KEY_STR_INC:
		return cmp(key_str.Data, key_str.DataLen, k1->key_str.Data, k1->key_str.DataLen);
	case ORDER_KEY_STR_DEC: 
		return -cmp(key_str.Data, key_str.DataLen, k1->key_str.Data, k1->key_str.DataLen);
	case ORDER_VAL_STR_INC: 
		return cmp(value_str.Data, value_str.DataLen, k1->value_str.Data, k1->value_str.DataLen);
	case ORDER_VAL_STR_DEC: 
		return -cmp(value_str.Data, value_str.DataLen, k1->value_str.Data, k1->value_str.DataLen);
	case ORDER_KEY_NUM_INC: 
		return cmp(key, k1->key);
	case ORDER_KEY_NUM_DEC: 
		return -cmp(key, k1->key);
	case ORDER_VAL_NUM_INC: 
		return cmp(value, k1->value);
	case ORDER_VAL_NUM_DEC: 
		return -cmp(value, k1->value);
	default: 
		return cmp(key_str.Data, key_str.DataLen, k1->key_str.Data, k1->key_str.DataLen);
	}
}

FibTree* SNode::maxValue(int is_max){	// is_max: true - max; false - min
	SNode* k=new SNode("\127\127\127\127",4,"\127\127\127\127",4);
	k->setKey(MAX_KEY);
	k->setValue(MAX_KEY);

	int i;
	switch (sort_order){
	case ORDER_KEY_STR_INC:
		break;
	case ORDER_KEY_STR_DEC: 
		for (i=0; i<4; i++){
			k->key_str.Data[i] = -k->key_str.Data[i];
		}
		break;
	case ORDER_VAL_STR_INC: 
		break;
	case ORDER_VAL_STR_DEC: 
		for (i=0; i<4; i++){
			k->value_str.Data[i] = -k->value_str.Data[i];
		}
		break;
	case ORDER_KEY_NUM_INC: 
		break;
	case ORDER_KEY_NUM_DEC: 
		k->key = -k->key;
		break;
	case ORDER_VAL_NUM_INC: 
		break;
	case ORDER_VAL_NUM_DEC: 
		k->value = -k->value;
		break;
	default: 
		break;
	}
	return k;
}

int SNode::copyValue(FibTree* from){
	SNode* k=(SNode* )from;
	setKey(k->key_str.Data, k->key_str.DataLen);
	setValue(k->value_str.Data, k->value_str.DataLen);
	setKey(k->key);
	setValue(k->value);
	return 0;
}





StringStack::StringStack(){
	order=ORDER_KEY_STR_INC;
	first=last=current=0;
	link_last=link_first=0;
	link_num=0;
}

StringStack::~StringStack(){
	reset();
}

void StringStack::reset(){
	if (link_first){
		SNode* n1=link_first;
		SNode* n2;
		while (n1){
			n2=n1->link_to;
			delete n1;
			n1=n2;
		}
	}
	first=last=current=0;
	link_last=link_first=0;
	link_num=0;
	heap.reset();
}

void StringStack::setOrder(SortOrder ord){
	order = ord;
}

SNode* StringStack::insert(SNode* n){
	if (!n) return 0;
	if (!link_first) link_first=n;
	else {
		link_last->link_to = n;
	}
	link_last= n;
	n->sort_order = order;
	link_num++;

	heap.insert(link_last);

	return n;
}

SNode* StringStack::moveNext(){
	if (!first) {
		first = last = current = (SNode*) heap.extract_min();
		return current;
	}
	if (current == last){
		current=(SNode*) heap.extract_min();
		if (!current) return 0;
		current->prev = last;
		last->next = current;
		last = current;
		return current;
	}
	if (!current) return 0;
	current = current->next;
	return current;
}

SNode* StringStack::movePrevious(){
	if (current == first) return 0;
	if (!current) {
		current = last;
		return current;
	}
	current = current->prev;
	return current;
}

long StringStack::resort(){
	long count=0;
	heap.reset();
	for (SNode* p=link_first ; p; p=p->link_to){
		p->sort_order = order;
		heap.insert(p);
		count++;
	}
	first=last=current = 0;
	return count;
}

