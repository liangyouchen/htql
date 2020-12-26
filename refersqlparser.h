#ifndef RSQL_PARSER_H_CLY_2006_12_28
#define RSQL_PARSER_H_CLY_2006_12_28

#include "referdata.h"
#include "expr.h"
#include "referset.h" 
#include "iwsqlsyntax.h"

class ReferSqlParser{
public:
	enum{
		ID_TABLE_NAME=0, ID_TABLE_DSNAME, ID_TABLE_ASNAME, ID_TABLE_SUBQUERY, ID_TABLE_FIELDS_NUM,
		ID_GROUPBY_FIELDNAME=0, ID_GROUPBY_TABLENAME, ID_GROUPBY_FIELDS_NUM,
		SQL_SELECT=0, SQL_DELETE, SQL_INSERT, SQL_UPDATE, SQL_CREATE, SQL_DROP
	};
	ReferData Sql;
	IWSqlSyntax SqlSyntax;
	int SqlType;
	ReferSqlParser* Next;
	ReferSqlParser* Parent;
	ReferSet TableNames;//Name, Dsname, AsName, SubQuery(.P)+IsInWhere(.L)
	ReferSqlParser* SubQueries;

	ReferLinkHeap OutputFields; //SELECT fields; Name:fieldname, Value:expression sentence, Data:index
	tExprCalc* OutputExprs; //SELECT fields; StringName, ExprSentence->Sentence, StringValue=type

	int OutputUnique;
	int ToSelectAll;
	ReferData Condition;//WHERE condition
	tExprCalc Expression; //WHERE condition, set context here

	ReferLinkHeap GroupByFields;
	tExprCalc* GroupByExprs; //StringName, ExprSentence->Sentence, StringValue=type
	ReferData HavingCondition;
	tExprCalc HavingExpression;
	ReferLinkHeap OrderByFields;
	tExprCalc* OrderByExprs; //StringName, ExprSentence->Sentence, StringValue=type

	int setQuery(const char* sql);
	static int getSqlErrorMsg(int err, ReferData* errormsg); 
public:
	ReferSqlParser();
	~ReferSqlParser();
	void reset();

protected:
	int parseQuery(IWSqlSyntax* syntax);
	int parseUpdateSetQuery(IWSqlSyntax* syntax, tExprCalc **expr_list); 
	int parseInsertValues(IWSqlSyntax* syntax); 
	int parseWhere(IWSqlSyntax* syntax); 
	int parseTablesList(IWSqlSyntax* syntax);
	int parseFieldsList(IWSqlSyntax* syntax, tExprCalc** expr_list);
	int parseExpr(tExprCalc* calc, IWSqlSyntax* syntax, ReferData* condition, etEXP_SYNTAX pre_op);
	int indexFieldsInHeap(tExprCalc* fields, ReferLinkHeap* heap, char* default_name);
	int deleteFieldExprs(tExprCalc* fieldexprs);
};

#endif

