#ifndef PERL_FDB_H
#define PERL_FDB_H	CLY20000430

int txtsetDoSQL(char* Sentence, char** Result);
int txtsetOpenSet(const char* Sql=0);
int txtsetCloseSet();
int txtsetEditRecord();
int txtsetAddRecord();
int txtsetDeleteRecord();
int txtsetRequery();
int txtsetUpdate();

int txtsetMoveNext();
int txtsetMoveFirst();
int txtsetMovePrev();
int txtsetIsEOF();
int txtsetIsBOF();
int txtsetIsOpen();

int txtsetOpenEx(const char* database);
int txtsetExecuteSQL(const char* sql);

int txtsetGetValueByName(const char* name, char** result);
int txtsetGetValueByIndex(int index, char** result);
long txtsetGetLongValueByIndex(int index);
long txtsetGetLongValueByName(const char* name);
double txtsetGetDoubleValueByIndex(int index);
double txtsetGetDoubleValueByName(const char* name);
int txtsetSetValue(const char* name, const char* value);
int txtsetGetMetaField(int FieldIndex,int FieldType, char** result);
int txtsetGetMetaFieldCount();
int txtsetGetMetaFieldIndex(const char* FieldName);
int txtsetSetFilter(const char* filter);
int txtseSetSort(const char* SortFields);
int txtsetSetOpenTable(const char* tablename);
int txtsetDBCondition(const char* name, const char* value, char** result);
int txtsetDBValue(const char* name, const char* value, char** result);

#endif
