#include "btree.h"

#include <stdlib.h>
#include <string.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BNode::BNode(){
	for (int i=0; i<BNODE_FULL+1; i++){
		nodes[i]=0;
		keys[i]=0;
	}
	parent=0;
}

BNode::~BNode(){
	for (int i=0; i<BNODE_FULL+1 && nodes[i]; i++){
		delete nodes[i];
		nodes[i]=0;
	}
	parent=0;
}


BTree::BTree(){
	root=new BNode();
	//root->nodes[1] = root; // next;
	//root->nodes[2] = root; // previous;
	total=0;
	NoDuplicate=true;
	IsUniqueKey=true;
}

BTree::~BTree(){
	reset();
	if (root) delete root;
	root=0;
}

void BTree::reset(){
	if (root) {
		delete root;
		root=0;
	}
	root=new BNode();
	//root->nodes[1] = root; // next;
	//root->nodes[2] = root; // previous;
	total=0;
	NoDuplicate=true;
	IsUniqueKey=true;
}
int BTree::empty(){
	int nodup=NoDuplicate;
	reset(); 
	NoDuplicate=nodup;
	return 0;
}

int BTree::cmp(char* p1, char* p2){
	return strcmp(p1, p2);
}
long BTree::printTree(BNode* n, const char* prefix, int to_print_parent, int print_hex){
	return printTree(stdout, n, prefix, to_print_parent, print_hex);
}
long BTree::printTree(FILE*f, BNode* n, const char* prefix, int to_print_parent, int print_hex){
	int i;
	long count=0;
	if (n->nodes[0]){ //it is an inner node;
		char* newprefix=(char*) malloc(sizeof(char)*(strlen(prefix)+10));
		for (i=0; i<BNODE_FULL+1; i++){
			if (n->nodes[i]){ //child nodes, at most BNODE_FULL+1
				sprintf(newprefix, "%s.%d", prefix, i);
				count+=printTree(f, n->nodes[i], newprefix, to_print_parent, print_hex);
			}
			if (n->keys[i]) {//inner key, at most BNODE_FULL keys
				if (to_print_parent && n->parent){
					int j=0;
					for (j=0; j<BNODE_FULL+1 && n->parent->nodes[j]!=n; j++);
					if (j>BNODE_FULL){
						if (print_hex) fprintf(f, "%s (%lx:%s) --> (%s)\n", prefix, n->keys[i],n->keys[i], "error parent");
						else fprintf(f, "%s (%s) --> (%s)\n", prefix, n->keys[i], "error parent");
					}else if (j==0){
						count++;
						if (print_hex) fprintf(f, "%s (%lx:%s) --> (*%lx)\n", prefix, n->keys[i],n->keys[i], n->parent->keys[j]);
						else fprintf(f, "%s (%s) --> (*%s)\n", prefix, n->keys[i], n->parent->keys[j]);
					}else{
						count++;
						if (print_hex) fprintf(f, "%s (%lx:%s) --> (%lx*)\n", prefix, n->keys[i],n->keys[i], n->parent->keys[j-1]);
						else fprintf(f, "%s (%s) --> (%s*)\n", prefix, n->keys[i], n->parent->keys[j-1]);
					}
				}else{
					count++;
					if (print_hex) fprintf(f, "%s (%lx:%s)\n", prefix, n->keys[i],n->keys[i]);
					else fprintf(f, "%s (%s)\n", prefix, n->keys[i]);
				}
			}
		}
		free(newprefix);
	}else{//this is a leaf nodes, at most BNODE_FULL+1 keys
		for (i=0; i<BNODE_FULL+1 && n->keys[i]; i++){
			if (to_print_parent && n->parent){
				int j=0;
				for (j=0; j<BNODE_FULL+1 && n->parent->nodes[j]!=n; j++);
				if (j>BNODE_FULL){
					if (print_hex) fprintf(f, "%s (%lx:%s) --> (%s)\n", prefix, n->keys[i],n->keys[i], "error parent");
					else fprintf(f, "%s (%s) --> (%s)\n", prefix, n->keys[i], "error parent");
				}else if (j==0){
					count++;
					if (print_hex) fprintf(f, "%s (%lx:%s) --> (*%lx)\n", prefix, n->keys[i],n->keys[i], n->parent->keys[j]);
					else fprintf(f, "%s (%s) --> (*%s)\n", prefix, n->keys[i], n->parent->keys[j]);
				}else{
					count++;
					if (print_hex) fprintf(f, "%s (%lx:%s) --> (%lx*)\n", prefix, n->keys[i],n->keys[i], n->parent->keys[j-1]);
					else fprintf(f, "%s (%s) --> (%s*)\n", prefix, n->keys[i], n->parent->keys[j-1]);
				}
			}else{
				count++;
				if (print_hex) fprintf(f, "%s (%lx:%s)\n", prefix, n->keys[i],n->keys[i]);
				else fprintf(f, "%s (%s)\n", prefix, n->keys[i]);
			}
		}
	}
	return count;
}
int BTree::isUniqueKey(){
	return IsUniqueKey;
}

int BTree::insert(char* p){
	BNode* n=root;
	int i,j, c=0;

	//find the leaf node to insert
	while (n){
		i=0;
		while (i<BNODE_FULL && n->keys[i]){
			c=cmp(p, n->keys[i]); //find the first key that are greater than p
			if (c==0){
				if (NoDuplicate) return false;
				IsUniqueKey=false;
			}else if (c<0) {
				break;
			}
			i++;
		}
		if (n->nodes[0]) n=n->nodes[i]; //it is an internal node, find from the left child
		else break; //it is a leaf
	}

	for (j=BNODE_FULL; j>i; j--){
		n->keys[j] = n->keys[j-1];
	}
	n->keys[i]=p;

	if ( n->keys[BNODE_FULL]){
		//it is full, split the node
		BNode* n1=new BNode();
		if (!n1) return false;
		for (i=BNODE_HALF+1,j=0; i<=BNODE_FULL; i++, j++){
			n1->keys[j] = n->keys[i];
			n->keys[i]=0;
		}
		//n1->nodes[1] = n->nodes[1];
		//n1->nodes[1]->nodes[2] = n1;
		//n1->nodes[2] = n;
		//n->nodes[1] = n1;
		splitNode(n, n1, n->keys[BNODE_HALF]);
		n->keys[BNODE_HALF]=0;
	}
	total++;
	return true;
}
int BTree::splitNode(BNode* n, BNode* n1, char* key){
	BNode* p=n->parent;

	if (!p){ // n is root;
		root=new BNode();
		root->nodes[0]=n;
		root->keys[0]=key;
		root->nodes[1]=n1;
		n->parent = n1->parent = root;
		return true;
	}

	// insert the node into parent
	int i=0, j=0;
	for (i=0; i<=BNODE_FULL && p->nodes[i] != n; i++);

	BNode* n2=p->nodes[BNODE_FULL]; //keep the overflow node

	for (j=BNODE_FULL; j>i; j--){
		p->keys[j] = p->keys[j-1];
		p->nodes[j] = p->nodes[j-1];
	}
	p->keys[i]=key;
	if (i<BNODE_FULL) p->nodes[i+1]=n1;
	else n2=n1;
	n1->parent=p;

	if (n2){
		// the parent is full, split the parent
		BNode* p1 = new BNode();
		for (i=BNODE_HALF+1,j=0; i<=BNODE_FULL; i++, j++){
			p1->keys[j] = p->keys[i];
			p1->nodes[j] = p->nodes[i];
			p1->nodes[j]->parent = p1;
			p->keys[i]=0;
			p->nodes[i]=0;
		}
		p1->nodes[j]=n2;
		p1->nodes[j]->parent=p1;

		splitNode(p, p1, p->keys[BNODE_HALF]);
		p->keys[BNODE_HALF]=0;
	}
	return true;
}
char* BTree::findNode(char* p, BNode** firstnode, int* key_i, int by_pointer){
	BNode* n=root;
	int i, c;
	*firstnode=0;
	*key_i=0;
	BNode* next_n=0;
	int next_i=0;
	while (n){
		i=0;
		while (p && i<BNODE_FULL && n->keys[i]){
			if (p==n->keys[i]) c=0;
			else c=cmp(p, n->keys[i]);

			if (c==0) {
				*firstnode=n;
				*key_i=i; //after found, continue to search from its left child
				break;
			}else if (c<0) {
				break;
			}
			i++;
		}
		if (by_pointer && (*firstnode) && p==(*firstnode)->keys[*key_i]){
			break;
		}
		if (n->nodes[0]) {
			if (n->keys[i]){
				next_n=n; //next larger one
				next_i=i;
			}
			n=n->nodes[i]; //it is an internal node, find from the left child
		} else break; //it is a leaf
	}
	if ((*firstnode)){ //found
		if (by_pointer && p!=(*firstnode)->keys[*key_i]){ 
			//find extact pointer sequentially.
			//cannot find by tree-navigation because the pointer may be under multiple internal nodes, where the values are the same
			while (nextNode(*firstnode, *key_i, firstnode, key_i)){
				if (p==(*firstnode)->keys[*key_i]){ //pointer is found
					return (*firstnode)->keys[*key_i];
				}else if (cmp(p, (*firstnode)->keys[*key_i])){ //not pointer found
					break;
				}
			}
			return 0; 
		}
		return (*firstnode)->keys[*key_i];
	}else{
		if (n && n->keys[i]){
			*firstnode=n; //not found, move to the first position >p
			*key_i=i; //if p is the largest, the key will be NULL
		}else{
			*firstnode=next_n; //not found, move to the first position >p
			*key_i=next_i; //if p is the largest, the key will be NULL
		}
		return 0;
	}
}
char* BTree::findLastNode(char* p, BNode** lastnode, int* key_i, int by_pointer){
	BNode* n=root;
	int i=0, c=0;
	*lastnode=0;
	*key_i=0;
	while (n){
		i=0;
		while (i<BNODE_FULL && n->keys[i]){
			if (p==n->keys[i]) c=0;
			else if (!p) c=1; 
			else c=cmp(p, n->keys[i]);

			if (c==0) {
				*lastnode=n;
				*key_i=i; 
				if (by_pointer && p==n->keys[i]){
					break;
				}
				//else, no break, continue to find the net equal
			}else if (c<0) break;
			i++;
		}
		if (by_pointer && (*lastnode) && p==(*lastnode)->keys[*key_i]){
			break;
		}
		if (n->nodes[0]) n=n->nodes[i]; //it is an internal node, find from the left child
		else break; //it is a leaf
	}
	if ((*lastnode)){ //found
		if (by_pointer && p!=(*lastnode)->keys[*key_i]){ 
			//find extact pointer sequentially.
			//cannot find by tree-navigation because the pointer may be under multiple internal nodes, where the values are the same
			while (previousNode(*lastnode, *key_i, lastnode, key_i)){
				if (p==(*lastnode)->keys[*key_i]){ //pointer is found
					return (*lastnode)->keys[*key_i];
				}else if (cmp(p, (*lastnode)->keys[*key_i])){//not pointer found
					break;
				}
			}
			return 0; 
		}

		return (*lastnode)->keys[*key_i];
	}else{ 
		previousNode(n, i, lastnode, key_i);
		//*lastnode=n; //not found, move to the first position >p
		//*key_i=i; //if p is the largest, the key will be NULL
		return 0;
	}
}

char* BTree::nextNode(BNode* n, int key_i, BNode** nextnode, int* next_i){
	*nextnode=n;
	if (key_i<0) { //at BOF
		*next_i=0; 
		return n->keys[0];
	}else if (key_i>BNODE_FULL || !n->keys[key_i]){ //at EOF
		*next_i=key_i-1;
		return n->keys[key_i-1];
	}else{ //reguar
		*next_i=key_i+1;
	}
	if (!n) return 0;

	if (n->nodes[0]) { //an internal node
		n=n->nodes[key_i+1];
		while (n->nodes[0]) n=n->nodes[0]; 
		*nextnode=n;
		*next_i=0;
		return n->keys[0];
	}else{ //a leaf node
		if (n->keys[key_i+1]){ //there is a next key in this node
			*nextnode=n;
			*next_i=key_i+1;
			return n->keys[key_i+1];
		}else{ //the next key is in the parent
			while (n->parent){
				int i=0;
				for (i=0; i<=BNODE_FULL && n->parent->nodes[i]!=n; i++);
				if (i<BNODE_FULL && n->parent->nodes[i+1]){// internal node
					*nextnode=n->parent;
					*next_i=i;
					return n->parent->keys[i];
				}else {
					n=n->parent;
				}
			}
			return 0;
		}
	}
	return 0;
}
char* BTree::previousNode(BNode* n, int key_i, BNode** previousnode, int* previous_i){
	*previousnode=n;
	if (!n) return 0;
	if (key_i<0){ //at BOF
		*previous_i=key_i;
		return 0;
	}else if (key_i>BNODE_FULL || !n->keys[key_i]){ //at EOF
		*previous_i=key_i-1;
		if (key_i>=0) return n->keys[key_i-1];
		else return 0;
	}else{ //reguar
		*previous_i=key_i-1;
	}
	if (!n) return 0;

	int i=0;
	if (n->nodes[0]) { //an internal node
		n=n->nodes[key_i];
		while (1) {
			for (i=0; i<BNODE_FULL && n->keys[i]; i++);

			if (n->nodes[0]) n=n->nodes[i]; //internal node
			else break; //leaf
		}
		*previousnode=n;
		*previous_i=i-1;
		return n->keys[i-1];
	}else{ //a leaf node
		if (key_i>0){ //there is a previous key in this node
			*previousnode=n;
			*previous_i=key_i-1;
			return n->keys[key_i-1];
		}else{ //the previous key is in the parent
			while (n->parent){
				int i=0;
				for (i=0; i<=BNODE_FULL && n->parent->nodes[i]!=n; i++);
				if (i>0){// internal node
					*previousnode=n->parent;
					*previous_i=i-1;
					return n->parent->keys[i-1];
				}else {
					n=n->parent;
				}
			}
			return 0;
		}
	}
	return 0;
}

char* BTree::find(char* p, int by_pointer){
	BNode* firstnode=0;
	int key_i=0;
	return findNode(p, &firstnode, &key_i, by_pointer);
}

long BTree::remove(char* p, int by_pointer){
	BNode* n=0, *nextnode=0;
	int key_i=0, next_i=0, j=0;
	int count=0;
	while (findNode(p, &n, &key_i, by_pointer)){
		if (n->nodes[0]){ //internal node, fill with the next key
			nextNode(n, key_i, &nextnode, &next_i);
			n->keys[key_i]=nextnode->keys[next_i];
			n=nextnode; //remove the next key
			key_i=next_i;
		}

		//remove the leaf keys
		j=key_i+1; //remove a single key
		if (!by_pointer) {
			for (; j<=BNODE_FULL && n->keys[j] && cmp(p, n->keys[j]) == 0; j++); //remove multiple keys
		}
		count+=j-key_i;
		while (j<=BNODE_FULL && n->keys[j]) {
			n->keys[key_i++]=n->keys[j++];  //move the rest ahead
		}
		for (j=key_i; j<=BNODE_FULL; j++){
			n->keys[j]=0; //empty the tails
		}
		rebalanceNode(n);

		if (by_pointer) break;//remove a single key
	}

	total-=count;
	return count;
}

int BTree::rebalanceNode(BNode* n){
	int i=0, j=0; 
	for (i=0; i<BNODE_HALF && n->keys[i]; i++);
	if (i<BNODE_HALF && n->parent){ //less than half
		//check the right sibling
		int parent_i=0, sibling_i=0;
		while (parent_i<=BNODE_FULL && n->parent->nodes[parent_i] && n->parent->nodes[parent_i] != n) parent_i++;
		if (parent_i<BNODE_FULL && n->parent->nodes[parent_i+1] && n->parent->nodes[parent_i+1]){
			//have right sibling
			sibling_i=0;
			while (sibling_i<BNODE_FULL+1 && n->parent->nodes[parent_i+1]->keys[sibling_i]) sibling_i++;
			if (sibling_i+i < BNODE_FULL){ //merge with the right
				n->keys[i++]=n->parent->keys[parent_i];
				for (j=0; j<sibling_i; j++){
					n->keys[i+j]=n->parent->nodes[parent_i+1]->keys[j];
					n->parent->nodes[parent_i+1]->keys[j]=0;
				}
				for (j=0; j<=sibling_i; j++) {
					n->nodes[i+j]=n->parent->nodes[parent_i+1]->nodes[j];
					if (n->nodes[0] && n->nodes[i+j]) n->nodes[i+j]->parent=n; //set the new subtree parent as n
					n->parent->nodes[parent_i+1]->nodes[j]=0;
				}
				//drop the parent key: parent_i
				dropNode(n->parent, parent_i+1);
			}else{ //lots of siblings on the right, take some keys from the right;
				//take the parent key here
				n->keys[i++]=n->parent->keys[parent_i];
				n->nodes[i]=n->parent->nodes[parent_i+1]->nodes[0];
				if (n->nodes[0] && n->nodes[i]) n->nodes[i]->parent=n; //set the new subtree parent as n

				j=0;
				while (i<BNODE_HALF){ //copy from the right
					n->keys[i++]=n->parent->nodes[parent_i+1]->keys[j++];
					n->nodes[i]=n->parent->nodes[parent_i+1]->nodes[j];
					if (n->nodes[0] && n->nodes[i]) n->nodes[i]->parent=n; //set the new subtree parent as n
				}
				//update parent key
				n->parent->keys[parent_i]=n->parent->nodes[parent_i+1]->keys[j++];
				n->parent->nodes[parent_i+1]->nodes[0]=n->parent->nodes[parent_i+1]->nodes[j];

				//move the right sibling
				sibling_i=0;
				while (n->parent->nodes[parent_i+1]->keys[j]){
					n->parent->nodes[parent_i+1]->keys[sibling_i++]=n->parent->nodes[parent_i+1]->keys[j++];
					n->parent->nodes[parent_i+1]->nodes[sibling_i]=n->parent->nodes[parent_i+1]->nodes[j];
				}
				while (sibling_i<=BNODE_FULL) {
					n->parent->nodes[parent_i+1]->keys[sibling_i++]=0;
					if (sibling_i<=BNODE_FULL) n->parent->nodes[parent_i+1]->nodes[sibling_i]=0;
				}
			}
		}else if (parent_i>0){ 
			//have left sibling
			sibling_i=0;
			while (sibling_i<BNODE_FULL+1 && n->parent->nodes[parent_i-1]->keys[sibling_i]) sibling_i++;
			if (sibling_i+i < BNODE_FULL){ //merge to the left
				n->parent->nodes[parent_i-1]->keys[sibling_i++]=n->parent->keys[parent_i-1];
				for (j=0; j<i; j++){
					n->parent->nodes[parent_i-1]->keys[sibling_i+j]=n->keys[j];
					n->keys[j]=0;
				}
				for (j=0; j<=i; j++) {
					n->parent->nodes[parent_i-1]->nodes[sibling_i+j]=n->nodes[j];
					if (n->parent->nodes[parent_i-1]->nodes[0] && n->nodes[j]) 
						n->nodes[j]->parent=n->parent->nodes[parent_i-1]; //set the new subtree parent as n
					n->nodes[j]=0;
				}
				//drop the parent key: parent_i-1
				dropNode(n->parent, parent_i);
			}else{ //lots of siblings on the left, take some keys from the left;
				//move right for some space
				j=sibling_i-BNODE_HALF;
				while (i>0){
					n->keys[i+j-1]=n->keys[i-1];
					n->nodes[i+j]=n->nodes[i];
					i--;
				}
				//take the parent key here
				n->keys[j-1]=n->parent->keys[parent_i-1];
				n->nodes[j]=n->nodes[0];
				j--;
				//take keys from the left
				while (j>0){
					n->keys[j-1]=n->parent->nodes[parent_i-1]->keys[BNODE_HALF+j];
					n->nodes[j]=n->parent->nodes[parent_i-1]->nodes[BNODE_HALF+j+1];
					if (n->nodes[0] && n->nodes[j]) n->nodes[j]->parent=n; //set the new subtree parent as n
					n->parent->nodes[parent_i-1]->keys[BNODE_HALF+j]=0;
					n->parent->nodes[parent_i-1]->nodes[BNODE_HALF+j+1]=0;
					j--;
				}
				//update the parent key
				n->parent->keys[parent_i-1]=n->parent->nodes[parent_i-1]->keys[BNODE_HALF];
				n->nodes[0]=n->parent->nodes[parent_i-1]->nodes[BNODE_HALF+1];
				if (n->nodes[0]) n->nodes[0]->parent=n; //set the new subtree parent as n
				n->parent->nodes[parent_i-1]->keys[BNODE_HALF]=0;
				n->parent->nodes[parent_i-1]->nodes[BNODE_HALF+1]=0;
			}
		}
	}else if (i==0 && n->nodes[0]) { //the root node is empty, but there are children nodes

		root=n->nodes[0];
		root->parent=0;
		n->nodes[0]=0;
		delete n;
	}
	return 0;
}
int BTree::dropNode(BNode*n, int node_i){
	//drop node and key
	if (n->nodes[node_i]) {
		delete n->nodes[node_i];
		n->nodes[node_i]=0;
	}
	if (node_i>0) n->keys[node_i-1]=0;

	//move nodes and keys ahead
	int i=node_i-1; //key i
	while (i<BNODE_FULL && n->keys[i+1]){
		if (i>=0) n->keys[i]=n->keys[i+1];
		i++;
		if (i<BNODE_FULL) n->nodes[i]=n->nodes[i+1];
	}
	if (i>=0) n->keys[i]=0;
	n->nodes[i+1]=0;
	
	//check the number of keys;
	if (i<BNODE_HALF){
		rebalanceNode(n);
	}
	return 0;
}


BTreeRecord::BTreeRecord(BTree* btree){
	tree=btree;
	CurrentNode=0;
	CurrentOffset=0;
	if (tree) moveFirst();
}

BTreeRecord::~BTreeRecord(){
	reset();
}

void BTreeRecord::reset(){
	tree=0;
	CurrentNode=0;
	CurrentOffset=0;
}

char* BTreeRecord::getValue(){
	if (!CurrentNode || CurrentOffset<0) return 0;
	return CurrentNode->keys[CurrentOffset];
}
char* BTreeRecord::moveFirst(char* p){
	char* first=tree->findNode(p, &CurrentNode, &CurrentOffset, 0);
	if (p) {
		if (!first) {
			CurrentNode=0; 
		}
		return first;
	}else return getValue();
}
char* BTreeRecord::moveFirstLarger(char* p){ //move to the first or larger 
	char* first=tree->findNode(p, &CurrentNode, &CurrentOffset, 0);
	return getValue();
}
char* BTreeRecord::moveLast(char* p){
	char* last=tree->findLastNode(p, &CurrentNode, &CurrentOffset, 0);
	if (p) {
		if (!last) {
			CurrentNode=0; 
		}
		return last;
	}else return getValue();
}
char* BTreeRecord::moveNext(char* p){
	char* next=tree->nextNode(CurrentNode, CurrentOffset, &CurrentNode, &CurrentOffset);
	if (p){
		if (next && !tree->cmp(next, p)) return next;
		else {
			CurrentNode=0;
			return 0;
		}
	}else{
		return next;
	}
}
char* BTreeRecord::movePrevious(char* p){
	char* previous= tree->previousNode(CurrentNode, CurrentOffset, &CurrentNode, &CurrentOffset);
	if (p){
		if (previous && !tree->cmp(previous, p)) return previous;
		else {
			CurrentNode=0;
			return 0;
		}
	}else{
		return previous;
	}
}
int BTreeRecord::isEOF(){
	return (!CurrentNode || CurrentOffset<0 || !CurrentNode->keys[CurrentOffset]);
}
int BTreeRecord::isBOF(){
	return (!CurrentNode || CurrentOffset < 0);
}
