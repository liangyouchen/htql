#include "htqlexpr.h"
#include "stroper.h"
#include <stdio.h>
#include <string.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void HtqlExpressionSyntax::findNext(){
	if (Next>=Data.L){
		NextType = synQL_END;
		NextLen = 0;

	}else if (Sentence[Next] == '.'){
		NextType = synQL_DOT;

	}else if (isDigit(Sentence[Next]) ){
		while (Next+NextLen < Data.L && (isDigit(Sentence[Next+NextLen]) ||
			Sentence[Next+NextLen] == '-' ||
			Sentence[Next+NextLen] == '#' 
			)){
			NextLen++;
		}
		NextType=synHTEXP_RANGE;

	}else if (Sentence[Next] == '<'){
		while (Next+NextLen < Data.L && Sentence[Next+NextLen] != '>' ){
			NextLen++;
		}
		if (Sentence[Next+NextLen] == '>') NextLen++;
		NextType = synHTEXP_HYPER_TAG;

	}else if (Sentence[Next] == '/'){
		if (Type == synQL_DOT || Type==synQL_UNKNOW){
			int enclosed=false;
			char enclosed_char = 0;

			while (Next+NextLen < Data.L &&  (Sentence[Next+NextLen] != '/' || enclosed ) ){
				if (!enclosed && Sentence[Next+NextLen]=='\''||Sentence[Next+NextLen]=='"'){
					enclosed=true;
					enclosed_char=Sentence[Next+NextLen];
				}else if (enclosed && enclosed_char==Sentence[Next+NextLen]){
					enclosed = false;
					enclosed_char = 0;
				}
				NextLen++;
			}
			if (Sentence[Next+NextLen] == '/') NextLen++;

			NextType = synHTEXP_PLAIN_TAG;

		}else{
			while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '/' || Sentence[Next+NextLen-1] == '\\')){
				NextLen++;
			}
			if (Sentence[Next+NextLen] == '/') NextLen++;
			NextType = synHTEXP_REDUCTION;
		}

	}else if (Sentence[Next] == '{'){
		while (Next+NextLen < Data.L && Sentence[Next+NextLen] != '}' ){
			NextLen++;
		}
		if (Sentence[Next+NextLen] == '}') NextLen++;
		NextType = synHTEXP_SCHEMA;

	}else if (Sentence[Next] == '&'){
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != ')' && 
			Sentence[Next+NextLen] != '.' && 
			Sentence[Next+NextLen] != '<' && 
			Sentence[Next+NextLen] != '/'&&
			Sentence[Next+NextLen] != '{'&&
			Sentence[Next+NextLen] != '&'
			)){
			NextLen++;
		}
		if (Sentence[Next+NextLen] == ')' ) NextLen++; 

		NextType = synHTEXP_FUNCTION;

	}else if (Sentence[Next] == '*'){
		while (Next+NextLen < Data.L && Sentence[Next+NextLen] != '/' ){
			NextLen++;
		}
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != '/' || Sentence[Next+NextLen-1] == '\\')){
			NextLen++;
		}
		if (Sentence[Next+NextLen] == '/') NextLen++;
		NextType = synHTEXP_REDUCTION;

	}else{
		NextType=synQL_UNKNOW;
	}
}


HtqlExpression::HtqlExpression(){
}

HtqlExpression::~HtqlExpression(){
	reset();
}

void HtqlExpression::reset(){
	Expression.reset();
}

int HtqlExpression::setExpr(char* htql, long len){
	if (len) {
		Expression.Set(htql, len, true);
	}else{
		Expression.Set(htql, strlen(htql), true);
	}
	return 0;
}

int HtqlExpression::getPatternsNum(){
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG ||
			expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_SCHEMA || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_REDUCTION || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_FUNCTION 
			){
			count++;
		}
		expr.match();
	}
	return count;
}
int HtqlExpression::getTagSelectionsNum(){
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG ||
			expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG  
			){
			count++;
		}
		expr.match();
	}
	return count;
}

char* HtqlExpression::getTagSelection(int tag_index1){
	GetResult.reset();
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END ){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG
			){
			count++;
			if (count == tag_index1){
				GetResult.Set(expr.Sentence+expr.Start, expr.StartLen, true);
				break;
			}
		}
		expr.match();
	}
	return GetResult.P;
}

int HtqlExpression::getTagSelectionRange(int tag_index1, int* range1, int* range2){
	GetResult.reset();
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END ){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG
			){
			count++;
			if (count == tag_index1){
				GetResult.Set(expr.Sentence+expr.Start, expr.StartLen, true);
				break;
			}
		}
		expr.match();
	}
	expr.match();
	if (expr.Type == HtqlExpressionSyntax::synHTEXP_RANGE){
		getRange(expr.Sentence + expr.Start, range1, range2);
	}

	return 0;
}

char* HtqlExpression::getTagSelectionHeadTo(int tag_index1){
	GetResult.reset();
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END ){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG
			){
			count++;
			if (count == tag_index1){
				if (expr.NextType == HtqlExpressionSyntax::synHTEXP_RANGE){
					expr.match();
				}
				GetResult.Set(expr.Sentence, expr.Start+ expr.StartLen, true);
				break;
			}
		}
		expr.match();
	}
	return GetResult.P;
}
char* HtqlExpression::getTagSelectionTailFrom(int tag_index1){
	GetResult.reset();
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END ){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG
			){
			count++;
			if (count == tag_index1){
				GetResult.Set(expr.Sentence+expr.Start, expr.Data.L - expr.Start, true);
				break;
			}
		}
		expr.match();
	}
	return GetResult.P;
}

char* HtqlExpression::replaceTagSelectionRange(int tag_index1, int range1, int range2){
	GetResult.reset();
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END ){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG
			){
			count++;
			if (count == tag_index1){
				GetResult.Set(expr.Sentence+expr.Start, expr.StartLen, true);
				break;
			}
		}
		expr.match();
	}
	expr.match();
	GetResult.Set(expr.Sentence, expr.Start, true);

	if (expr.Type == HtqlExpressionSyntax::synHTEXP_RANGE){
		expr.match();
	}
	char buf[20];
	if (range1 || range2){
		sprintf(buf, "%d", range1);
		if ((range2>=0)&&(range2 != range1 || range2 == 0)){
			sprintf(buf, "%d-%d", range1, range2);
		}else{
			sprintf(buf, "%d", range1);
		}
		GetResult.Cat(buf, strlen(buf));
	}

	GetResult.Cat(expr.Sentence+expr.Start, expr.Data.L - expr.Start);
	return GetResult.P;
}

int HtqlExpression::getRange(char*expr, int* range1, int* range2){
	*range1=0;
	*range2=0;
	sscanf(expr, "%d", range1);
	int i=0;
	while (tStrOp::isDigit(expr[i])) i++;
	if (expr[i]=='-') {
		sscanf(expr+i+1, "%d", range2);
	}else{
		range2=range1;
	}
	return 0;
}

int HtqlExpression::getSchemasNum(){
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_SCHEMA 
			){
			count++;
		}
		expr.match();
	}
	return count;
}

char* HtqlExpression::getTrailAttributes(){
	if (strchr(Expression.P, ':')){
		int i;
		char* p=0;
		for (i=Expression.L; i>0; i--){
			if (Expression.P[i]==':') p=Expression.P+i;
			else if (Expression.P[i]=='>'){
				return p;
			}
		}
	}
	return 0;
}

char* HtqlExpression::getSchema(int index1, long* start, long* len){
	GetResult.reset();
	if (start) *start=0;
	if (len) *len=0;

	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	while (expr.Type != QLSyntax::synQL_END ){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_SCHEMA){
			count++;
			if (count == index1){
				GetResult.Set(expr.Sentence+expr.Start, expr.StartLen, true);
				if (start) *start = expr.Start;
				if (len) *len = expr.StartLen;
				break;
			}
		}
		expr.match();
	}
	return GetResult.P;
}

char* HtqlExpression::getSchemaField(int schema_index1, int field_index1, long* start, long* len){
	if (start) *start=0;
	if (len) *len=0;
	long start1, len1;
	getSchema(schema_index1, &start1, &len1);
	GetResult.reset();
	int count=1;
	len1+=start1;
	long offset = start1+1;
	while (offset < len1 && count < field_index1){
		while (Expression.P[offset]==';'||tStrOp::isSpace(Expression.P[offset])) offset++;
		int inquote=0; 
		while (offset < len1 && (Expression.P[offset]!=';'||inquote)) {
			offset++;
			if (Expression.P[offset]=='\'') inquote=!inquote; 
		}
		//while (offset < len1 && Expression.P[offset]!=';' ) offset++;
		count++;
	}
	while (tStrOp::isSpace(Expression.P[offset])|| Expression.P[offset]==';') offset++;
	long offset1 = offset;
	while (offset1 < len1 && Expression.P[offset1]!=';' &&Expression.P[offset1]!='}') 
		offset1++;
	GetResult.Set(Expression.P + offset, offset1-offset, true);
	if (start) *start = offset;
	if (len) *len = offset1-offset;
	return GetResult.P;
}

int HtqlExpression::getSchemaFieldsNum(int schema_index1){
	long start, len;
	getSchema(schema_index1, &start, &len);
	int count=0;
	long offset = 1;
	while (Expression.P[start+offset]==';'||tStrOp::isSpace(Expression.P[start+offset])) offset++;
	while (offset < len){
		int inquote=0; 
		while (offset < len && (Expression.P[start+offset]!=';'||inquote) && Expression.P[start+offset]!='}' ) {
			offset++;
			if (Expression.P[start+offset]=='\'') inquote=!inquote; 
		}
		//while (offset < len && Expression.P[start+offset]!=';' && Expression.P[start+offset]!='}' );
		while (Expression.P[start+offset]==';'||tStrOp::isSpace(Expression.P[start+offset])) offset++;
		count++;
		if (Expression.P[start+offset]=='}' || Expression.P[start+offset]=='|') 
			break;
	}
	return count;
}

char* HtqlExpression::getSchemaPrefix(int schema_index1){
	GetResult.reset();
	HtqlExpressionSyntax expr;
	expr.setSentence(Expression.P, &Expression.L, false);
	int count=0;
	int start=0;
	while (expr.Type != QLSyntax::synQL_END ){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_SCHEMA){
			count++;
			if (count == schema_index1-1){
				start = expr.Start + expr.StartLen;
			}else if (count == schema_index1){
				break;
			}
		}
		expr.match();
	}
	GetResult.Set(expr.Sentence+start, expr.Start - start, true);
	return GetResult.P;
}

char* HtqlExpression::getSchemaFieldName(int schema_index1, int field_index1, long* start1, long* len1){
	if (start1) *start1=0;
	if (len1) *len1=0;
	long start, len;
	getSchemaField(schema_index1, field_index1, &start, &len);
	GetResult.reset();
	len+=start;
	//if (field_index1>1 && Expression.P[start]==';') start++;
	while (tStrOp::isSpace(Expression.P[start])||Expression.P[start]==';') start++;
	long offset=1;
	while (offset<len && Expression.P[start+offset] !='=') offset++;
	if (Expression.P[start+offset]=='='){
		GetResult.Set(Expression.P+start, offset, true);
		if (start1) *start1=start;
		if (len1) *len1=offset;
	}
	return GetResult.P;
}
char* HtqlExpression::getSchemaFieldHtql(int schema_index1, int field_index1, long* start1, long* len1){
	if (start1) *start1=0;
	if (len1) *len1=0;
	long start, len;
	getSchemaField(schema_index1, field_index1, &start, &len);
	len+=start;
	//if (Expression.P[len]==';' || Expression.P[len]=='}') len--;
	GetResult.reset();
	//if (field_index1>1 && Expression.P[start]==';') start++;
	while (start<len && Expression.P[start]!='=') start++;
	if (Expression.P[start]=='='){
		start++;
		GetResult.Set(Expression.P+start, len-start, true);
		if (start1) *start1=start;
		if (len1) *len1=len-start;
	}
	return GetResult.P;
}

char* HtqlExpression::replaceSchemaFieldName(int schema_index1, int field_index, char* new_name){
	long start, len;
	char*p=getSchemaFieldName(schema_index1, field_index, &start, &len);
	GetResult.reset();
	if (!p||!isalpha(Expression.P[start])) {
		GetResult = Expression;
	}else{
		GetResult.Set(Expression.P, start, true);
		GetResult += new_name;
		GetResult.Cat(Expression.P + start+len, Expression.L - start-len);
	}
	return GetResult.P;
}

char* HtqlExpression::deleteSchemaField(int schema_index1, int field_index){
	long start, len;
	char*p=getSchemaField(schema_index1, field_index, &start, &len);
	if (Expression.P[start+len]==';') len++;
	if (!p && len==0){
		GetResult=Expression;
	}else{
		GetResult.Set(Expression.P, start, true);
		GetResult.Cat(Expression.P + start+len, Expression.L - start-len);
	}
	return GetResult.P;
}

long HtqlExpression::findSimilarHtqlPos(const char* htql1, const char* htql2){
	long len1=strlen(htql1), len2=strlen(htql2); 
	long start, end, i;
	for (start=0; start<len1 && start<len2 && htql1[start]==htql2[start]; start++); 
	for (end=1; end<len1 && end<len2 && htql1[len1-end]==htql2[len2-end]; end++); 
	//in the middle, test if the two htqls differ only by a number
	int isnumberlink=true, isnumberlast=true;
	for (i=start; i<=len1-end; i++) if (!tStrOp::isDigit(htql1[i])) isnumberlink=false;
	for (i=start; i<=len2-end; i++) if (!tStrOp::isDigit(htql2[i])) isnumberlast=false;
	//if only differ with a number, the two htqls are similar
	if (isnumberlink && isnumberlast){
		return start;
	}else{
		return -1;
	}
}
int HtqlExpression::decomposeSimilarHtql(const char* htql, long similar_pos, ReferData*parent_tag, ReferData*similar_tag, ReferData*suffix){
	HtqlExpressionSyntax expr;
	expr.setSentence(htql, 0, false);

	//find parent_tag, rest_parent, last_tag, similar_tag, and pattern_tag
	parent_tag->reset(); 
	similar_tag->reset();
	suffix->reset();
	//ReferData parent_tag, rest_parent, last_tag, child_tag, pattern_tag, pvalue_tag;
	while (expr.Type != QLSyntax::synQL_END ){
		if (expr.Type == HtqlExpressionSyntax::synHTEXP_HYPER_TAG || 
			expr.Type == HtqlExpressionSyntax::synHTEXP_PLAIN_TAG
			){
			if (expr.Start+expr.StartLen<=similar_pos){
				parent_tag->Set(similar_tag->P, similar_tag->L, false);
				similar_tag->Set(expr.Sentence+expr.Start, expr.StartLen, false);
			}else if (expr.Start>=similar_pos){
				suffix->Set(expr.Sentence+expr.Start, strlen(expr.Sentence+expr.Start), false);
				break;
			}
		}
		expr.match();
	}
	return 0;
}



