#ifndef REFER_DATABASE_H_CLY_2007_01_02
#define REFER_DATABASE_H_CLY_2007_01_02

#include "referset.h"
class ReferSqlParser;

class ReferDatabase{
public:
	enum{ID_TABLE_NAME, ID_TABLE, ID_TABLE_FIELDS_NUM};

	ReferData DatabaseName;
	ReferSet Tables; //Name, TABLE(.P)IsMalloc(.L)
	ReferSqlParser* Parser;

	int addTable(ReferData* tablename, ReferSet* set);

	ReferSet* executeSQL(const char* sql);
public:
	ReferDatabase();
	~ReferDatabase();
	void reset();

protected:
	ReferSet* processQuerySelection();
};

#endif

