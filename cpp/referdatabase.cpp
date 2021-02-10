#include "referdatabase.h"

#include "refersqlparser.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


ReferDatabase::ReferDatabase(){
	Tables.setFieldsNum(ID_TABLE_FIELDS_NUM);
	Tables.setFieldName(ID_TABLE_NAME, "TableName");
	Tables.newIndexField(ID_TABLE_NAME, 0);

	Parser=0;
}

ReferDatabase::~ReferDatabase(){
	reset();
}

void ReferDatabase::reset(){
	ReferData* tuple;
	for (tuple=Tables.moveFirst(); tuple; tuple=Tables.moveNext()){
		if (tuple[ID_TABLE].P && tuple[ID_TABLE].L){
			delete (ReferSet*) tuple[ID_TABLE].P;
			tuple[ID_TABLE].reset();
		}
	}
	Tables.empty();

	if (Parser){
		delete Parser;
		Parser=0;
	}
}

int ReferDatabase::addTable(ReferData* tablename, ReferSet* set){
	ReferData* tuple=0;
	if ((tuple=Tables.findFieldString("TableName", tablename))){
		if (tuple[ID_TABLE].P && tuple[ID_TABLE].L){
			delete (ReferSet*) tuple[ID_TABLE].P;
			tuple[ID_TABLE].reset();
		}
	}else{
		tuple=Tables.newTuple();
	}
	tuple[ID_TABLE].Set((char*) set, 0, false);
	return 0;
}
ReferSet* ReferDatabase::executeSQL(const char* sql){
	if (Parser){
		delete Parser;
		Parser=0;
	}

	Parser=new ReferSqlParser;
	Parser->setQuery(sql);
	if (Parser->SqlType==ReferSqlParser::SQL_SELECT){
		return processQuerySelection();
	}else if (Parser->SqlType==ReferSqlParser::SQL_DELETE){

	}else if (Parser->SqlType==ReferSqlParser::SQL_CREATE){

	}else if (Parser->SqlType==ReferSqlParser::SQL_DROP){

	}
	return 0;
}
ReferSet* ReferDatabase::processQuerySelection(){
	return 0;
}








