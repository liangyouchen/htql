#ifndef TXTSET_H
#define TXTSET_H	CLY20000430

#include "log.h"
#include "filedb.h"
#include "freemem.h"
#include "stack.h"

typedef enum {ofCLOSE, ofOPEN, ofUPDATE, ofINSERT } etOpenFlag;

class TxtSet: public tSQLRequest{
public:
	TxtSet();
	~TxtSet();

	int openSet(const char* Sql=NULL);
	int closeSet();
	int editRecord();
	int addRecord();
	int deleteRecord();
	int requery();
	int update();

	int moveNext();
	int moveFirst();
	int movePrev();
	int isEOF();
	int isBOF();
	int isOpen();

	int openEx(const char* database);
	int executeSQL(const char* sql);

	char* getValue(const char* name);
	char* getValue(int index);
	long getLongValue(int index);
	long getLongValue(const char* name);
	double getDoubleValue(int index);
	double getDoubleValue(const char* name);
	int setValue(const char* name, const char* value);
	char* getMetaField(int FieldIndex,int FieldType);
	int getMetaFieldCount();
	int getMetaFieldIndex(const char* FieldName);
	int setFilter(const char* filter);
	int setSort(const char* SortFields);
	int setOpenTable(const char* tablename);
	char* DBCondition(const char* name, const char* value);
	char* DBValue(const char* name, const char* value);

protected:
	int doOpen();

	tStack m_ResultStack;
	tStack* m_Move;
	etOpenFlag m_OpenFlag; 
	tFreeMem m_GetValueMem;
	tFreeMem m_GetMetaMem;
	tStack m_SetValueStack;
	tFreeMem m_strFilter;
	tStack m_SortFields;
	tFreeMem m_OpenTable;
	tFreeMem m_IndexName;
	tFreeMem m_DBCondition;
	tFreeMem m_DBValue;
	
	long m_RecordCount;
	long m_RecordIndex;
};

#endif
