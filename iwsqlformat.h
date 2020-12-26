#ifndef CLY_SQL_FORMAT_H
#define CLY_SQL_FORMAT_H

#ifdef WIN32
#include <afx.h>
#endif

#include "referdata.h"
#include "referlink.h"

typedef enum {iwSQL_MSSQL, iwSQL_MSACCESS, iwSQL_ORACLE, iwSQL_MYSQL} IwSqlCategory;

class IWSqlFormat{
public:
	enum {NUMBER, DATE, STRING};
	ReferData TableName;
	IwSqlCategory DBType; //IwSqlCategory
	ReferLinkHeap FieldValues; //select condition, insert(1), create field+data len(1), update condition(1)
	ReferLinkHeap SelectFields; //select fields
	int SelectFieldsNum;
	ReferLinkHeap UpdateFields; //insert(2), update value (2), create type+data prec(2)
	int UniqueSelect;

	int setTable(ReferData* table_name);
	int setTable(const char* table_name);
	int setFieldValue(ReferData* field_name, ReferData* field_value, int type); //type: {NUMBER, DATE, STRING}
	int setFieldValue(const char* field_name, const char* field_value, int type);
	int setUpdateField(ReferData* field_name, ReferData* field_value, int type);
	int setUpdateField(const char* field_name, const char* field_value, int type);
	int setSelectField(ReferData* field_name);
	int setSelectField(const char* field_name);
	int getInsertSql(ReferData* sql);
	int getSelectSql(ReferData* sql);
	int getDeleteSql(ReferData* sql);
	int getUpdateSql(ReferData* sql);
	int getCreateSql(ReferData* sql);

	ReferData* getDBString(ReferData* field_value, int type);
	ReferData GetDBString;
	static int getType(const char* type_str);

	IWSqlFormat();
	~IWSqlFormat();
	void reset();
};



#endif


