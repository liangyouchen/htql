#ifndef CLY_SQL_SYSTAX_H
#define CLY_SQL_SYSTAX_H

#ifdef WIN32
#include <afx.h>
#endif
#include "qlsyntax.h"

class IWSqlSyntax: public QLSyntax{
public:
	enum{
		synSQL_TABLE=1000,
		synSQL_SELECT,
		synSQL_FROM,
		synSQL_AS,
		synSQL_WHERE,
		synSQL_UPDATE,
		synSQL_INSERT,
		synSQL_DELETE,
		synSQL_CREATE,
		synSQL_ORDER,
		synSQL_SET,
		synSQL_VALUES,
		synSQL_INTO,
		synSQL_GROUP,
		synSQL_BY,
		synSQL_GROUPBY,
		synSQL_HAVING,
		synSQL_UNIQUE,
		synSQL_ASC,
		synSQL_DESC,
		synSQL_AND,
		synSQL_OR,
		synSQL_NOT,
		synSQL_IN,

		synSQL_MATCHES,
		synSQL_LIKE,
		synSQL_IS,
		synSQL_NULL,

		synSQL_NUMBER,
		synSQL_INTEGER,
		synSQL_CHAR,
		synSQL_LONG,
		synSQL_DATE,

	};
	virtual void findNext();
	virtual int KeyWord();
};

#endif

