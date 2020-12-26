#ifndef REFER_SET_H_CLY_2006_11_19
#define REFER_SET_H_CLY_2006_11_19

#include "referlink2.h"

class ReferSetBTree;
class ReferSqlParser;
class tExprCalc;
class HTQL;
class IWDriverInterface;

class ReferSet {
public:
	enum {REC_TUPLE=0, REC_NEWTUPLE=1}; //set at DataCursor->Data
	ReferLink2 DataHead;  //a null root node
	ReferData ReferSetName;
	ReferData** FieldNames;	//FieldNames[i]: pointing to field names
	ReferLinkHeap FieldNameIndex; //index of field names;

	//For indexing: only add or delete, no update is allow
	ReferLinkHeap IndexNameList; //Name: Index name; Value.P: ReferSetBTree
	ReferLinkHeap UniqueIndexList; //Name: Index name; Value.P: ReferSetBTree, redundant to IndexNameList

	long FieldsNum;
	long TupleCount;
	long RowNum;

	ReferLink2* DataCursor;

	int setFieldsNum(long fieldnum=0, const char**fieldnames=0);	
	int setFieldName(long fieldindex0, const char* fieldname); //it will set FieldNameIndex
	long getFieldIndex(const char* fieldname); 
	long getFieldIndex(ReferData* fieldname); 

	ReferData* newTuple(ReferData* tuple=0, int free_tuple=true);
	ReferData* borrowTuple(ReferSet* from, int force_borrow=1); //force_borrow=0:by reference; =1:borrow tuple; =2:new and copy tuple
	int setFieldValue(const char* fieldname, const char* fieldvalue, int copy=true); 
	int setFieldValue(ReferData* fieldname, ReferData* fieldvalue, int copy=true); 
	int commitTuple();

	//some of the newIndex function also call useIndex, but some not; need to check for consistency later 
	ReferSetBTree* newIndex(const char* indexname, int indexfieldsnum, const long* fieldslist, const long* fieldsflag, int is_unique=false, tExprCalc* sortexprs=0);
	ReferSetBTree* newIndex(const char* indexname, const char** fieldsname, const long* fieldsflag, int is_unique=false);
	ReferSetBTree* newIndex(const char* index_name, tExprCalc* fields_walklist, int is_unique=false);
	ReferSetBTree* newIndexField(const long fieldindex0, const long fieldflag, int is_unique=false);
	ReferSetBTree* getIndex(const char* indexname=0);
	ReferSetBTree* useIndexName(const char* indexname);
	ReferSetBTree* useIndex(ReferSetBTree* index);
	int dropIndex(const char* indexname);

	long filterByCondition(const char* condition, ReferSet* results=0, tExprCalc* context=0, int to_delete=false, int force_borrow=0);
	long filterByCondition(ReferData* condition, ReferSet* results=0, tExprCalc* context=0, int to_delete=false, int force_borrow=0);
	long executeSQL(ReferData* sql, ReferSet* results=0, tExprCalc* context=0);
	long executeSQL(const char* sql, ReferSet* results=0, tExprCalc* context=0);
	ReferData* findFieldString(const char* fieldname, ReferData* fieldvalue); 
	ReferData* getQueryTuple(ReferLinkHeap* name_values=0, int caller_delete=false);

	ReferData* moveFirst(ReferData* tuple=0);
	ReferData* moveNext(ReferData* tuple=0);
	ReferData* moveLast(ReferData* tuple=0);
	ReferData* movePrevious();
	ReferData* getTuple();
	int getFieldValue(long fieldindex0, ReferData* fieldvalue);
	int getFieldValue(const char* fieldname, ReferData* fieldvalue);
	ReferData* getField(long fieldindex0);
	ReferData* getField(const char* fieldname);
	ReferData* getField(ReferData* fieldname);
	ReferData* getField(ReferData*tuple, const char* fieldname);
	ReferData* getField(ReferData*tuple, ReferData* fieldname);
	int isEOF();
	int isBOF();

	int dropTuple(ReferLink2* tuple=0, int delete_data=true);
	int empty();
	int projectFields(ReferLinkHeap* fieldmap, ReferSet* projset=0, tExprCalc* context=0, ReferSet* groupby_index=0, int sel_ins_upd=0); //need to re-index projset
	int copyStruct(ReferSet* from); //fields and index
	int copyFields(ReferSet* from); //fields
	int copyIndices(ReferSet* from); //index

	int rebuildAllIndex();
	int resizeAllTuples(long original_fieldsnum, long new_fieldsnum);

	//ReferLinkHeap Variables;
	//ReferLink* setVariable(const char* name, const char* value, int copy=true); // use context instead
	int setExprVariables(ReferData* tuple, tExprCalc* expr, long rownum);
	ReferSetBTree* chooseIndex(ReferLinkHeap* names);
	int getCSVData(ReferData* results, const char* sql=0, int save_csv_header=0, const char* field_sep=0, const char* line_sep=0, const char*quotation=0);
	int getXMLData(ReferData* results, int to_encode_html=false, const char* sql=0, const char* record_tag=0, const char* field_tag=0);
	long loadXMLData(ReferData* data, int to_decode_html=false, const char* with_name=0, const char* record_tag=0, const char* field_tag=0);
	long loadCSVData(ReferData* data, int first_line_isname, const char*line_sep=0, const char* field_sep=0, int quoted_field=0);
	long loadHTQLData(HTQL* ql);
#if (!defined(NOIWEB) && !defined(NOIWIRB))
	long loadDBData(IWDriverInterface* iw);
#endif

	ReferSet(const char* name=0);
	~ReferSet();
	void reset();
	void resetIndexCursor();

public: //protected
	int checkUniqueIndex(ReferLink* tuple);
	int addtoAllIndex(ReferLink* tuple);
	int dropfromAllIndex(ReferLink* tuple);
	int dropOneTupleNoIndex(ReferLink2* tuple, int delete_data);
	long parseUpdateQuery(ReferSqlParser* parser, tExprCalc* context);
	long parseInsertQuery(ReferSqlParser* parser, tExprCalc* context);

	BTreeRecord UseIndex;
	ReferData* QueryTuple;
};

class ReferSetBTree:public BTree{
public:
	enum {
		FLAG_DESCENDING=0x1, 
		FLAG_CASE_INSENSITIVE=0x2, 
		FLAG_CHAR=0x4,
		FLAG_INTEGER=0x8,
		FLAG_FLOAT=0x10,
		FLAG_REFER_P=0x20,
		FLAG_REFER_L=0x40,
	};
	int isStringFlag(long flag);

	ReferData IndexName;
	long* IndexFields; //need to set manually, but will be destroyed automatically
	long* IndexFlags; //need to set manually, but will be destroyed automatically
	int IndexNum; //need to set manually
	enum {TYPE_REFLINK, TYPE_REFDATA}; 
	int TypeOfP; //Change TypeOfP before adding any data

	tExprCalc** SortExpressionsIndex; 
	tExprCalc* SortExpressions;
	ReferSet* SortSet; //pointer only
	int SortExpressionsNum;
	int resetSortExpressions();
	int setSortExpressions(tExprCalc* sortexprs, ReferSet* sortset); //copy sortexprs structure

	//the following use TYPE_REFDATA, while newIndex in ReferSet uses TYPE_REFLINK
	int sortReferSet(ReferSet* set, int indexfieldsnum, const long* fieldslist, const long* fieldsflag, int remove_duplicate=false, tExprCalc* sortexprs=0);
	int sortReferSet(ReferSet* set, const char** fieldsname, const long* fieldsflag, int remove_duplicate=false);
	int sortReferSetField(ReferSet* set, const long fieldindex0, const long fieldflag, int remove_duplicate=false);

	int allocateIndex(int indexnum, const long*fields_list=0, const long*flags_list=0);
	int sortReferSet(ReferSet* set, int remove_duplicate);
	int findIndexField(long field0); //return 0 based, or -1 if nonexist
	virtual int cmp(char* p1, char* p2);
	virtual int insert(char* p); //always insert the ReferLink2

	ReferSetBTree();
	~ReferSetBTree();
	void reset();
	void resetBTree();
};

class ReferSetHeap: public ReferSet{
public:
	long KeyField;
	ReferSetBTree* KeyIndex;

	int setKeyField(const long key_field0, const long key_field_flag=0);
	ReferData* findKey(ReferData* key);
	ReferData* addkey(ReferData* key);

public:
	ReferSetHeap(const long fields_num=1, const long key_field0=0, const long key_field_flag=0);
	~ReferSetHeap();
	void reset();
};

#endif 


