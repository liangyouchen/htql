#ifndef CLY_B_TREE_H
#define CLY_B_TREE_H

#include "log.h"
#define BNODE_HALF	2
#define BNODE_FULL	4

class BNode{
public:
	BNode* nodes[BNODE_FULL+1];
	char* keys[BNODE_FULL+1];
	BNode* parent;

	BNode();
	virtual ~BNode();
};

class BTree {
public:
	virtual int cmp(char* p1, char* p2);
	BNode* root;
	unsigned long total;
	int NoDuplicate;
public:
	virtual int insert(char* p);
	virtual char* find(char* p, int by_pointer);
	virtual long remove(char* p, int by_pointer);
	int isUniqueKey();

	char* findNode(char* p, BNode** firstnode, int* key_i, int by_pointer); //when not found, return NULL, set firstnode to the firstlarger node
	char* findLastNode(char* p, BNode** lastnode, int* key_i, int by_pointer); //when not found, return NULL, set firstnode to the firstlarger node
	char* nextNode(BNode* n, int key_i, BNode** nextnode, int* next_i); //when not found, return NULL, set firstnode to the firstlarger node
	char* previousNode(BNode* n, int key_i, BNode** previousnode, int* previous_i); //when not found, return NULL, set firstnode to the firstlarger node
	long printTree(FILE*f, BNode* n, const char* prefix, int to_print_parent=0, int print_hex=0);
	long printTree(BNode* n, const char* prefix, int to_print_parent=0, int print_hex=0);

	BTree();
	~BTree();
	void reset();
	int empty();

protected:
	int IsUniqueKey; //read only attribute, set by insert() when duplication is detected
	int splitNode(BNode* n, BNode* n1, char* key);
	int dropNode(BNode*n, int node_i);
	int rebalanceNode(BNode* n);
};

class BTreeRecord {
public:
	BTree* tree;

	char* getValue();
	char* moveFirst(char* p=0); //when not found: if (!p) move to the current node, else return 0
	char* moveFirstLarger(char* p=0); //move to the first or larger 
	char* moveLast(char* p=0);//when not found: if (!p) move to the current node, else return 0
	char* moveNext(char* p=0);//when not found: if (!p) move to the current node, else return 0
	char* movePrevious(char* p=0);//when not found: if (!p) move to the current node, else return 0
	int isEOF();
	int isBOF();

	BTreeRecord(BTree* btree=0);
	~BTreeRecord();
	void reset();
public:
	BNode* CurrentNode;
	int CurrentOffset;
};

#endif
