#ifndef HTQL_SYNTAX_H_CLY_2011_03_19
#define HTQL_SYNTAX_H_CLY_2011_03_19

#include "qlsyntax.h"
#include "htmlbuf.h"
#include "stroper.h"

class HTQLSyntax: public QLSyntax{
public:
	enum {
		synQL_MAPPING = 100, synQL_PER, synQL_PARA
	};

	virtual void findNext();

};

class HTQLTagSelSyntax: public QLSyntax{
public:
	enum {
		synQL_MAPPING = 100, synQL_PER
	};

	virtual void findNext();
	virtual int KeyWord();

};

class HTQLTagDataSyntax: public QLSyntax{
public:
	enum {
		synQL_START_TAG=100, synQL_END_TAG, synQL_DATA, synQL_COMMENT, synQL_OTHERS
	};
	enum {
		synQL_XML_DATA, synQL_XML_NULL_TAG, synQL_XML_SINGLE_TAG, synQL_XML_XID_TAG, synQL_XML_DTD_TAG
	};
	int IsXML;
	int NextXMLTagType;
	int XMLTagType;
	ReferData ToFindEnclosedTag;
	virtual void matchNext();
	virtual void findNext();
	virtual int isSpace(char);

	HTQLTagDataSyntax();
	virtual ~HTQLTagDataSyntax();
};

class HTQLNoSpaceTagDataSyntax: public HTQLTagDataSyntax{
public:
	virtual int isSpace(char c){ return tStrOp::isSpace(c);}
};

class HTQLAttrDataSyntax: public QLSyntax{
public:
	virtual void findNext();
};


#endif

