#ifndef HTQL_EXPR_H_CLY_2003_03_18
#define HTQL_EXPR_H_CLY_2003_03_18

#include "qlsyntax.h"

class HtqlExpressionSyntax: public QLSyntax{
public:
	enum {synHTEXP_HYPER_TAG= 100, 
		synHTEXP_PLAIN_TAG, 
		synHTEXP_SCHEMA, 
		synHTEXP_REDUCTION, 
		synHTEXP_FUNCTION,
		synHTEXP_RANGE
	};
	virtual void findNext();
};

class HtqlExpression{
public:
	ReferData Expression;

	int setExpr(char* htql, long len=0);
	int getPatternsNum();

	int getTagSelectionsNum();
	char* getTagSelection(int tag_index1);
	char* getTagSelectionHeadTo(int tag_index1);
	char* getTagSelectionTailFrom(int tag_index1);
	int getTagSelectionRange(int tag_index1, int* range1, int* range2);
	char* replaceTagSelectionRange(int tag_index1, int range1, int range2);

	char* getTrailAttributes();

	int getSchemasNum();
	char* getSchema(int schema_index1, long* start=0, long* len=0);
	char* getSchemaField(int schema_index1, int field_index1, long* start=0, long* len=0);
	int getSchemaFieldsNum(int schema_index1);
	char* getSchemaPrefix(int schema_index1);
	char* getSchemaFieldName(int schema_index1, int field_index1, long* start=0, long* len=0);
	char* getSchemaFieldHtql(int schema_index1, int field_index1, long* start=0, long* len=0);
	char* replaceSchemaFieldName(int schema_index1, int field_index1, char* new_name);
	char* deleteSchemaField(int schema_index1, int field_index1);

	int getRange(char*expr, int* range1, int* range2);

	static long findSimilarHtqlPos(const char* htql1, const char* htql2);
	static int decomposeSimilarHtql(const char* htql, long similar_pos, ReferData*parent_tag, ReferData*similar_tag, ReferData*suffix);

public:
	HtqlExpression();
	~HtqlExpression();
	void reset();

protected:
	ReferData GetResult;
};

#endif
