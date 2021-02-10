// LASTMODIFY CLY20000430

#include "perlfdb.h"
#include "txtset.h"

TxtSet MyTextSet;


int txtsetDoSQL(char* Sentence, char** Result){
	return MyTextSet.doSQL(Sentence,Result);
}

int txtsetOpenSet(const char* Sql){
	return MyTextSet.openSet(Sql);
}

int txtsetCloseSet(){
	return MyTextSet.closeSet();
}

int txtsetEditRecord(){
	return MyTextSet.editRecord();
}

int txtsetAddRecord(){
	return MyTextSet.addRecord();
}

int txtsetDeleteRecord(){
	return MyTextSet.deleteRecord();
}

int txtsetRequery(){
	return MyTextSet.requery();
}

int txtsetUpdate(){
	return MyTextSet.update();
}


int txtsetMoveNext(){
	return MyTextSet.moveNext();
}

int txtsetMoveFirst(){
	return MyTextSet.moveFirst();
}

int txtsetMovePrev(){
	return MyTextSet.movePrev();
}

int txtsetIsEOF(){
	return MyTextSet.isEOF();
}

int txtsetIsBOF(){
	return MyTextSet.isBOF();
}

int txtsetIsOpen(){
	return MyTextSet.isOpen();
}


int txtsetOpenEx(const char* database){
	return MyTextSet.openEx(database);
}

int txtsetExecuteSQL(const char* sql){
	return MyTextSet.executeSQL(sql);
}


int txtsetGetValueByName(const char* name, char** result){
	*result = MyTextSet.getValue(name);
	return (*result)?0:1;
}

int txtsetGetValueByIndex(int index, char** result){
	*result = MyTextSet.getValue(index);
	return (*result)?0:1;
}

long txtsetGetLongValueByIndex(int index){
	return MyTextSet.getLongValue(index);
}

long txtsetGetLongValueByName(const char* name){
	return MyTextSet.getLongValue(name);
}

double txtsetGetDoubleValueByIndex(int index){
	return MyTextSet.getDoubleValue(index);
}

double txtsetGetDoubleValueByName(const char* name){
	return MyTextSet.getDoubleValue(name);
}

int txtsetSetValue(const char* name, const char* value){
	return MyTextSet.setValue(name, value);
}

int txtsetGetMetaField(int FieldIndex,int FieldType, char** result){
	*result= MyTextSet.getMetaField(FieldIndex, FieldType);
	return (*result)?0:1;
}

int txtsetGetMetaFieldCount(){
	return MyTextSet.getMetaFieldCount();
}

int txtsetGetMetaFieldIndex(const char* FieldName){
	return MyTextSet.getMetaFieldIndex(FieldName);
}

int txtsetSetFilter(const char* filter){
	return MyTextSet.setFilter(filter);
}

int txtseSetSort(const char* SortFields){
	return MyTextSet.setSort(SortFields);
}

int txtsetSetOpenTable(const char* tablename){
	return MyTextSet.setOpenTable(tablename);
}

int txtsetDBCondition(const char* name, const char* value, char** result){
	*result = MyTextSet.DBCondition(name, value);
	return (*result)?0:1;
}

int txtsetDBValue(const char* name, const char* value, char** result){
	*result = MyTextSet.DBValue(name, value);
	return (*result)?0:1;
}

