#include "alignment.h"
#include "docbase.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#define DEBUG_THIS_FILE
#endif


StrAlignment::StrAlignment(){
	IndelCost[algnMATCH]=0;
	IndelCost[algnINSERT]=1;
	IndelCost[algnDELETE]=1;
	IndelCost[algnREPLACE]=2;
	MatchMismatchPenalty=0.01;
	IsCaseSensitive=true;
	CmpFun=0;
	DebugChar=0;
	CmpCode=0; 
	Translate2Position=0;
}

StrAlignment::~StrAlignment(){
	reset();
}

void StrAlignment::reset(){

}
int StrAlignment::setIndelCost(double match, double insert, double del, double replace){
	IndelCost[algnMATCH]=match; 
	IndelCost[algnINSERT]=insert; 
	IndelCost[algnDELETE]=del; 
	IndelCost[algnREPLACE]=replace; 
	return 0;
}

int StrAlignment::CompareStrings(char* str1, char* str2, double* cost, long* result_len, char** result_str1, char** result_str2){
	//align str1 and str2 using dynamic algorithm.
	//result_str1 and result_str2 are alignments with inserted '-'
	//result_len is the result alignment length
	//cost is the total cost of the alignment
	//insert means insert a '-' in str2; delete means delete a char in str2;
	//use cost/(2*result_len) for cost evaluation, or 1-cost/(2*result_len) for goodness
	long len1=strlen(str1);
	long len2=strlen(str2);
	long cols=len1+1;
	StrAlignmentNode* matrix=(StrAlignmentNode*) malloc(sizeof(StrAlignmentNode)*(len1+1)*(len2+1));
	memset(matrix, 0, sizeof(StrAlignmentNode)*(len1+1)*(len2+1));
	long i,j;
	for (i=1; i<=len1; i++) {
		matrix[i].cost=i;
		matrix[i].prev=2;
	}
	for (j=1; j<=len2; j++) {
		matrix[j*cols].cost=j;
		matrix[j*cols].prev=1;
	}
	//StrAlignmentNode g;
	double cost_insert, cost_delete, cost_replace;
	for (j=1; j<=len2; j++){
		for (i=1; i<=len1; i++){
			if ((IsCaseSensitive && str1[i-1] == str2[j-1]) || (!IsCaseSensitive && toupper(str1[i-1]) == toupper(str2[j-1])) ) {
				//matrix[j*cols+i].cost = matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnMATCH];
				cost_replace=matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnMATCH];
				matrix[j*cols+i].prev = 3;
			} else {
				//take the MINIMAL indel cost;
				cost_replace=matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnREPLACE];
				if (matrix[(j-1)*cols+(i-1)].prev==3) cost_replace+=MatchMismatchPenalty;
			}
			cost_insert=matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT];
			if (matrix[(j-1)*cols+i].prev==3) cost_insert+=MatchMismatchPenalty;

			cost_delete=matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE];
			if (matrix[j*cols+(i-1)].prev==3) cost_delete+=MatchMismatchPenalty;
				
			if (cost_replace<=cost_insert && cost_replace<=cost_delete){
				matrix[j*cols+i].cost=cost_replace; 
				matrix[j*cols+i].prev=3;
			}else if (cost_insert<=cost_delete){
				matrix[j*cols+i].cost=cost_insert; 
				matrix[j*cols+i].prev=1;
			}else{
				matrix[j*cols+i].cost=cost_delete; 
				matrix[j*cols+i].prev=2;
			}
		}
	}
/*	for (j=0; j<=len2; j++){
		for (i=0; i<=len1; i++){
			printf("%3d", matrix[j*cols+i].cost);
		}
		printf("\n");
	}*/
	char* result1 = (char*) malloc(sizeof(char)*(len1+len2+2));
	char* result2 = (char*) malloc(sizeof(char)*(len1+len2+2));
	int len=len1+len2+1;
	result1[len]=result2[len]=0;
	for(i=len1, j=len2; i || j; ){
		len--;
		if (matrix[j*cols+i].prev==3 ){
			result1[len]=str1[--i];
			result2[len]=str2[--j];
		}else if (matrix[j*cols+i].prev==1 || !i){
			result1[len]='-';
			result2[len]=str2[--j];
		}else if (matrix[j*cols+i].prev==2 || !j){
			result1[len]=str1[--i];
			result2[len]='-';
		}
	}
	if (result_str1){
		*result_str1=(char*) malloc(sizeof(char)*(len1+len2+2-len));
		strcpy(*result_str1, result1+len);
	}
	free(result1);
	if (result_str2){
		*result_str2=(char*) malloc(sizeof(char)*(len1+len2+2-len));
		strcpy(*result_str2, result2+len);
	}
	free(result2);

	if (result_len) *result_len = len1+len2+1-len;
	if (cost) *cost=matrix[len2*cols+ len1].cost;
	free(matrix);
	return 0;
}
int StrAlignment::compareHyperTags(void**sourcetag, long len1, void**strtag, long len2, 
			double* cost, long* result_len, void*** result_tag1, void*** result_tag2, double**result_costs){
	//align sourcetag and strtag using dynamic algorithm.
	//result_tag1 and result_tag2 are alignments with inserted 'NULL'
	//result_len is the result alignment length
	//cost is the total cost of the alignment
	//insert means insert a 'NULL' in strtag; delete means delete a char in strtag;
	//use cost/(2*result_len) for cost evaluation, or 1-cost/(2*result_len) for goodness
	long cols=len1+1;
	//using continuous memory to expedite computation time
	StrAlignmentNode* matrix=(StrAlignmentNode*) malloc(sizeof(StrAlignmentNode)*(len1+1)*(len2+1));
	memset(matrix, 0, sizeof(StrAlignmentNode)*(len1+1)*(len2+1)); 
	long i,j;
	for (i=1; i<=len1; i++) {
		matrix[i].cost=i;
		matrix[i].prev=2;
	}
	for (j=1; j<=len2; j++) {
		matrix[j*cols].cost=j;
		matrix[j*cols].prev=1;
	}
	long* sourcecode=0; 
	long* strcode=0; 
	if (CmpCode) { //translate tags to code to expedite comparison
		sourcecode=(long*) malloc(sizeof(long)*len1); 
		strcode=(long*) malloc(sizeof(long)*len2); 
		for (i=0; i<len1; i++) sourcecode[i]=(*CmpCode)(sourcetag[i]); 
		for (i=0; i<len2; i++) strcode[i]=(*CmpCode)(strtag[i]); 
	}
	//StrAlignmentNode g;
	double cost_insert, cost_delete, cost_replace;
	for (j=1; j<=len2; j++){
		i=1;
		for (i=1; i<=len1; i++){
			if ( (CmpCode && sourcecode[i-1] == strcode[j-1]) 
				|| (!CmpCode && CmpFun && (*CmpFun)(sourcetag[i-1], strtag[j-1])==0 )
				|| (!CmpCode && !CmpFun && sourcetag[i-1] == strtag[j-1] )
				 ) {
				matrix[j*cols+i].cost = matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnMATCH];
				matrix[j*cols+i].prev = 3;
			} else {
				cost_replace=matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnREPLACE];
				if (matrix[(j-1)*cols+(i-1)].prev==3) cost_replace+=MatchMismatchPenalty;

				cost_insert=matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT];
				if (matrix[(j-1)*cols+i].prev==3) cost_insert+=MatchMismatchPenalty;

				cost_delete=matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE];
				if (matrix[j*cols+(i-1)].prev==3) cost_delete+=MatchMismatchPenalty;

				if (cost_replace<=cost_insert && cost_replace<=cost_delete){
					matrix[j*cols+i].cost=cost_replace; 
					matrix[j*cols+i].prev=3;
				}else if (cost_insert<=cost_delete){
					matrix[j*cols+i].cost=cost_insert; 
					matrix[j*cols+i].prev=1;
				}else{
					matrix[j*cols+i].cost=cost_delete; 
					matrix[j*cols+i].prev=2;
				}
				/*g.cost = matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnREPLACE];
				g.prev=3;
				if (matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT] <g.cost){
					g.cost = matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT];
					g.prev=1;
				}
				if (matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE] <g.cost){
					g.cost = matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE];
					g.prev=2;
				}
				matrix[j*cols+i].cost = g.cost;
				matrix[j*cols+i].prev = g.prev;*/
			}
		}
	}
/*	for (j=0; j<=len2; j++){
		for (i=0; i<=len1; i++){
			printf("%3d", matrix[j*cols+i].cost);
		}
		printf("\n");
	}*/
	void** result1 = (void**) malloc(sizeof(void*)*(len1+len2+2));
	void** result2 = (void**) malloc(sizeof(void*)*(len1+len2+2));
	double* costs = (double*) malloc(sizeof(double)*(len1+len2+2));
	long len=len1+len2+1;
	result1[len]=result2[len]=0;
	costs[len]=matrix[len2*cols+len1].cost; 
	for(i=len1, j=len2; i || j; ){
		len--;
		if (matrix[j*cols+i].prev==3 ){
			result1[len]=sourcetag[--i];
			result2[len]=strtag[--j];
		}else if (matrix[j*cols+i].prev==1 || !i){
			result1[len]=0;
			result2[len]=strtag[--j];
		}else if (matrix[j*cols+i].prev==2 || !j){
			result1[len]=sourcetag[--i];
			result2[len]=0;
		}
		costs[len]=matrix[j*cols+i].cost; 
	}
	if (result_tag1){
		*result_tag1=(void**) malloc(sizeof(void*)*(len1+len2+2-len));
		memcpy(*result_tag1, result1+len, sizeof(void*)*(len1+len2+1-len));
	}
	if (result1) free(result1);
	if (result_tag2){
		*result_tag2=(void**) malloc(sizeof(void*)*(len1+len2+2-len));
		memcpy(*result_tag2, result2+len, sizeof(void*)*(len1+len2+1-len));
	}
	if (result2) free(result2);
	if (result_costs){
		*result_costs=(double*) malloc(sizeof(double)*(len1+len2+2-len));
		memcpy(*result_costs, costs+len, sizeof(double)*(len1+len2+1-len));
	}
	if (costs) free(costs);

	if (result_len) *result_len = len1+len2+1-len;
	if (cost) *cost=matrix[len2*cols+ len1].cost;

	if (matrix) free(matrix);
	if (sourcecode) free(sourcecode); 
	if (strcode) free(strcode); 
	return 0;
}

int StrAlignment::computeConsensusStr(char** strs, int n_strs, char** consen_str, double* cost){
	double min_cost=100000;
	int min_i=0;
	int min_j=0;
	int min_len=0;
	char* min_str1=0, *min_str2=0;
	int i, j;
	//ss containt pointers to all strs;  0 when a pointed str is merged with another str
	char** ss=(char**) malloc(sizeof(char*) *n_strs);
	for (i=0; i<n_strs; i++) ss[i]=strs[i];
	//counts reflect the count of merged strs.  If k strs are merged to ss[i], count[i]=k.  count[i] is used as the indel cost.
	int* counts=(int*) malloc(sizeof(int)* n_strs);
	for (i=0; i<n_strs; i++) counts[i]=1;
	//isnewstr indicate if ss[i] is a newly malloced str;
	int* isnewstr=(int*) malloc(sizeof(int)* n_strs);
	memset(isnewstr, 0, sizeof(int)* n_strs);
	//costs contains the cost between alignment of ss[i] and ss[j]
	double* costs =(double*) malloc(sizeof(double)*n_strs*n_strs);
	for (i=0; i<n_strs-1; i++){
		for (j=i+1; j<n_strs; j++){
			costs[i*n_strs+j]=100000;
		}
	}

	int merge_count=0;
	int rest_strs=n_strs;
	StrAlignment align;
	double align_cost=0;
	long result_len=0;
	char* result_str1=0, *result_str2=0;

	while (rest_strs>1){
		min_cost=100000;
		//compare all to all and find the closest alignment with a minimal cost
		for (i=0; i<n_strs-1; i++) {
			if (ss[i]) {
				for (j=i+1; j<n_strs; j++) {
					if (ss[j]) {
						align.IndelCost[algnMATCH]=0;
						align.IndelCost[algnINSERT]=counts[i];
						align.IndelCost[algnDELETE]=counts[j];
						align.IndelCost[algnREPLACE]=(counts[j]>counts[i])?counts[i]:counts[j];
						align.CompareStrings(ss[i], ss[j], &align_cost, &result_len, &result_str1, &result_str2); 
						costs[i*n_strs+j]=align_cost;
						if (align_cost < min_cost){
							min_cost = align_cost;
							min_i=i;
							min_j=j;
							min_len=result_len;
							if (min_str1) free(min_str1);
							min_str1=result_str1;
							result_str1=0;
							if (min_str2) free(min_str2);
							min_str2=result_str2;
							result_str2=0;
						}else{
							free(result_str1);
							free(result_str2);
							result_str1=0;
							result_str2=0;
						}
					}
				}
			}
		}
		//merge min cost strs;
		for (i=0; i<min_len; i++){
			if (min_str1[i]!=min_str2[i]){
				if (min_str1[i]=='-' ){
					min_str1[i]=min_str2[i];
				}else if (min_str2[i]!='-' && counts[min_j]>counts[min_i] ){
					min_str1[i]=min_str2[i];
				}
			}
		}
		counts[min_i]+=counts[min_j];
		merge_count+=counts[min_j];
		counts[min_j]=0;
		if (isnewstr[min_j]) free(ss[min_j]);
		ss[min_j]=0;
		isnewstr[min_j]=false;
		if (isnewstr[min_i]) free(ss[min_i]);
		ss[min_i]=min_str1;
		min_str1=0;
		isnewstr[min_i]=true;
		free(min_str2);
		min_str2=0;
		rest_strs--;

		if (counts[min_i]>n_strs/2) break; //if a group have more than half counts
	}
	
	//find the maximum group as the consensus str
	int max_count=0;
	int max_i=0;
	for (i=0; i<n_strs; i++) {
		if (ss[i] && counts[i] >= max_count) {
			max_count = counts[i];
			max_i=i;
		}
	}
	if (isnewstr[max_i]){
		*consen_str=ss[max_i];
		isnewstr[max_i]=false;
	}else{
		*consen_str=(char*) malloc(sizeof(char)*(strlen(ss[max_i])+1));
		strcpy(*consen_str, ss[max_i]);
	}

	//recompute the cost; by aligning the consensus str to all other strings; indel of '-' is also counted as costs
	align.IndelCost[algnMATCH]=0;
	align.IndelCost[algnINSERT]=1;
	align.IndelCost[algnDELETE]=1;
	align.IndelCost[algnREPLACE]=2;
	*cost=0;
	for (i=0; i<n_strs; i++){
		align.CompareStrings(*consen_str, strs[i], &align_cost, 0, &result_str1,&result_str2); 
		*cost+=align_cost;
//		printf("%s\n%s\n\n", result_str1, result_str2);
		free(result_str1);
		result_str1=0;
		free(result_str2);
		result_str2=0;
	}

	for (i=0; i<n_strs; i++){
		if (isnewstr[i] && ss[i]){
			free(ss[i]);
			ss[i]=0;
		}
	}
	free(ss);
	free(counts);
	free(costs);
	free(isnewstr);
	if (min_str1) free(min_str1);
	if (min_str2) free(min_str2);
	if (result_str1) free(result_str1);
	if (result_str2) free(result_str2);

	return 0;
}
int StrAlignment::alignHyperTags(void**sourcetag, long len1, void**strtag, long len2, 
								 ReferData* results, double* match_score, long*isource1, long*isource2,long*istr1,long*istr2){
	if (results) results->reset(); 
	if (match_score) *match_score=0; 
	int err=0;

	//using matrix will be slowlier
	long cols=len1+1;
	StrAlignmentNode* matrix=(StrAlignmentNode*) malloc(sizeof(StrAlignmentNode)*(len1+1)*(len2+1));
	if (!matrix) return -1; 
	memset(matrix, 0, sizeof(StrAlignmentNode)*(len1+1)*(len2+1)); 
	long i,j;
	for (i=1; i<=len1; i++) {
		matrix[i].cost=i*IndelCost[algnDELETE];
		matrix[i].prev=2;
	}
	for (j=1; j<=len2; j++) {
		matrix[j*cols].cost=j*IndelCost[algnINSERT];
		matrix[j*cols].prev=1;
	}
	long* sourcecode=0; 
	long* strcode=0; 
	if (CmpCode) { //translate tags to code to expedite comparison
		sourcecode=(long*) malloc(sizeof(long)*len1); 
		strcode=(long*) malloc(sizeof(long)*len2); 
		for (i=0; i<len1; i++) sourcecode[i]=(*CmpCode)(sourcetag[i]); 
		for (i=0; i<len2; i++) strcode[i]=(*CmpCode)(strtag[i]); 
	}
	
	double val1;
	long val2;
	double max_cost=0;
	for (i=1; i<=len1; i++){
		for (j=1; j<=len2; j++){
			if ( (CmpCode && sourcecode[i-1] == strcode[j-1]) 
				|| (!CmpCode && CmpFun && (*CmpFun)(sourcetag[i-1], strtag[j-1])==0 )
				|| (!CmpCode && !CmpFun && sourcetag[i-1] == strtag[j-1] )
				 ) {
				matrix[j*cols+i].cost = matrix[(j-1)*cols+(i-1)].cost + IndelCost[algnMATCH]; //match, +1??
				matrix[j*cols+i].prev = 3;
			} else {
				//take the MAXIMAL indel cost
				val1 = matrix[(j-1)*cols+(i-1)].cost + IndelCost[algnREPLACE]; //replacement
				val2 = 3;
				if (matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT] > val1){
					val1 = matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT]; //insert (keep source, insert a space in str) is better than replacement;
					val2 = 1;
				}
				if (matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE] > val1){
					val1 = matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE]; //delete (insert a space in source) is better than replacement;
					val2 = 2;
				}
				if (val1<0) val1=0; //minimal value is 0
				matrix[j*cols+i].cost = val1; 
				matrix[j*cols+i].prev = val2;
			}
			if (matrix[j*cols+i].cost > max_cost){
				max_cost = matrix[j*cols+i].cost;
			}
		}
	}

#ifdef DEBUG_THIS_FILE
	char* row_str=(char*) malloc(sizeof(char)*(len2+1));
	char* col_str=(char*) malloc(sizeof(char)*(len1+1));
	for (i=0;i<len1;i++) {
		if (DebugChar) col_str[i]=(*DebugChar)(sourcetag[i]); 
		else col_str[i]=((ReferLink*)sourcetag[i])->Name.P[0];
	}
	col_str[i]=0;
	for (i=0;i<len2;i++) {
		if (DebugChar) row_str[i]=(*DebugChar)(strtag[i]); 
		else row_str[i]=((ReferLink*)strtag[i])->Name.P[0];
	}
	row_str[i]=0;
	dumpMatrix("c:\\matrix.txt", __FILE__, matrix, col_str, row_str, len1, len2);
	free(row_str);
	free(col_str);
#endif

	if (results) *results="<max_positions>";
	char buf[128];
	long count=0; 
	for (i=1; i<=len1; i++){
		for (j=1; j<=len2; j++){
			if ((matrix[j*cols+i].cost - max_cost <1e-6) && (matrix[j*cols+i].cost - max_cost >-1e-6)){
				long d=0;
				long k, l;
				for (k=i, l=j; k>=0 && l>=0 && matrix[l*cols+k].cost> 0; ){
					d= matrix[l*cols+k].prev;
					if (d==3) { k--; l--;}
					else if (d==2) k--;
					else if (d==1) l--;
					else {k--; l--;}
				}
				if (d==3) { k++; l++;}
				else if (d==2) k++;
				else if (d==1) l++;
				else {k++; l++;}
				count++; 

				if (results) {
					long sourcepos2=i-1;
					long strpos2=j-1; 
					long sourcepos1=k-1;
					long strpos1=l-1;
					if (Translate2Position){
						sourcepos2=(*Translate2Position)(sourcetag[i-1]);
						strpos2=(*Translate2Position)(strtag[j-1]); 
						sourcepos1=(*Translate2Position)(sourcetag[k-1]); 
						strpos1=(*Translate2Position)(strtag[l-1]); 
					}
					sprintf(buf, "<position source_pos=\"%ld\" str_pos=\"%ld\" cost=\"%lf\" from_pos=\"%ld\" from_str=\"%ld\" />", 
						sourcepos2, strpos2, max_cost, sourcepos1, strpos1 );
					*results+=buf;
				}

				//the last one will be selected
				if (isource1) *isource1=k-1; //from source pos
				if (isource2) *isource2=i-1; //to source pos
				if (istr1) *istr1=l-1; //from str pos
				if (istr2) *istr2=j-1; //to str pos
			}
		}
	}
	if (results) *results+= "</max_positions>";

	if (match_score) *match_score=max_cost;

	if (sourcecode) free(sourcecode); 
	if (strcode) free(strcode); 
	if (matrix) free(matrix); 
	return 0;
}
int StrAlignment::dumpMatrix(const char* dump2file, const char*description, StrAlignmentNode* matrix, const char*col_str, const char*row_str, long cols, long rows){
	FILE* f=fopen(dump2file, "w+");
	long i, v;
	if (description){
		fprintf(f, "%s\n", description);
	}
	for (i=0; i<rows; i++){
		if (i%3==0){
			fprintf(f, "row (i) =%d\n--", i);
			for (v=0; v<cols; v++){
				fprintf(f, "--  %5d%c", v, col_str?col_str[v]:'-');
			}
			fprintf(f, "\n");
		}
		fprintf(f, "(%2d)%c", i, row_str?row_str[i]:' ');
		for (v=0; v<cols; v++){
			fprintf(f, "%8.3lf  ", matrix[i*(cols+1)+v].cost);
		}
		fprintf(f, "\n");
	}
	fclose(f);
	return 0;
}


HyperTagsAlignment::HyperTagsAlignment(){
	SourceTag=0;
	SourcePos=0;
	SourceCount=0;
	StrTag=0;
	StrPos=0;
	StrCount=0;
	AlignType=TYPE_TAG;
	IsCaseSensitive=true;
	CmpFun=0;
	DebugChar=0;

	IndelScore[algnMATCH]=1;	//algnMATCH: (will be added)
	IndelScore[algnINSERT]=-1;//algnINSERT: (will be subtracted) keep source, insert a space into str
	IndelScore[algnDELETE]=-1;//algnDELETE: (will be subtracted) insert a space in source, or keep source and delete a str char
	IndelScore[algnREPLACE]=-2;//algnREPLACE: (will be subtracted)
}

HyperTagsAlignment::~HyperTagsAlignment(){
	reset();
}
void HyperTagsAlignment::reset(){
	STags.reset();
	ETags.reset();
	if (SourceTag) {
		free(SourceTag);
		SourceTag=0;
	}
	if (SourcePos){
		free(SourcePos);
		SourcePos=0;
	}
	SourceCount=0;
	if (StrTag){
		free(StrTag);
		StrTag=0;
	}
	if (StrPos){
		free(StrPos);
		StrPos=0;
	}
	StrCount=0;
}
int HyperTagsAlignment::setAlignType(int type){
	AlignType=type;
	return 0;
}

int HyperTagsAlignment::setSouceData(ReferData* source){
	if (SourceTag) {
		free(SourceTag);
		SourceTag=0;
	}
	if (SourcePos){
		free(SourcePos);
		SourcePos=0;
	}
	SourceCount=0;

	return builtTagsForAlign(source, &STags, &ETags, &SourceTag, &SourcePos, &SourceCount);
}
int HyperTagsAlignment::setStrData(ReferData* str){
	if (StrTag){
		free(StrTag);
		StrTag=0;
	}
	if (StrPos){
		free(StrPos);
		StrPos=0;
	}
	StrCount=0;
	return builtTagsForAlign(str, &STags, &ETags, &StrTag, &StrPos, &StrCount);
}
int HyperTagsAlignment::builtTagsForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count){
	stags->setCaseSensitivity(IsCaseSensitive);
	etags->setCaseSensitivity(IsCaseSensitive);
	if (AlignType==TYPE_TAG){
		return builtHyperTagsForAlign(source, stags, etags, sourcetag, sourcepos, source_count);
	}else if (AlignType==TYPE_HYPER_CHAR){
		return builtHyperCharStrForAlign(source, stags, etags, sourcetag, sourcepos, source_count);
	}else if (AlignType==TYPE_WORD){
		return builtWordsForAlign(source, stags, etags, sourcetag, sourcepos, source_count);
	}else if (AlignType==TYPE_CHAR){
		return builtPlainCharStrForAlign(source, stags, etags, sourcetag, sourcepos, source_count);
	}
	return -1;
}

int HyperTagsAlignment::alignHyperTagsText(ReferData* source, ReferData* str, ReferData* results, double* align_score, int whole_x){
	setSouceData(source);
	setStrData(str);
	return alignHyperTagsText(results, align_score, whole_x);
}

int HyperTagsAlignment::alignHyperTagsText(ReferData* results, double* align_score, int whole_x){
	return alignHyperTagsText(SourceTag, SourcePos, SourceCount, StrTag, StrPos, StrCount, 
		results, align_score, 0, 0, 0, 0,
		whole_x);
}

#include "hmmMatrix.h"
#include "qhtql.h"

int HyperTagsAlignment::alignHyperTagsText(void**sourcetag, long*sourcepos, long source_count, void**strtag, long*strpos, long str_count, 
						ReferData* results, double* match_score, long*isource1, long*isource2,long*istr1,long*istr2, 
						int whole_x){
	results->reset(); 
	if (match_score) *match_score=0; 
	int err=0;

	//using matrix will be slowlier
	HmmMatrix costs;
	costs.setCellSize(sizeof(double));
	if ((err=costs.setCol(str_count+1))<0) return err; 
	if ((err=costs.setRow(source_count+1))<0) return err;
	HmmMatrix direction;
	direction.setCellSize(sizeof(long));
	if ((err=direction.setCol(str_count+1))<0) return err;
	if ((err=direction.setRow(source_count+1))<0) return err;
	long i, j;
	double val1;
	long val2;
	double max_cost=0;
	for (i=1; i<=source_count; i++){
		for (j=1; j<=str_count; j++){

			if ( (!CmpFun && sourcetag[i-1] == strtag[j-1]) 
				|| (CmpFun && (*CmpFun)(sourcetag[i-1], strtag[j-1])==0) ) {
				val1 = (*(double*) costs(i-1, j-1)) + IndelScore[algnMATCH]; //match, +1??
				val2 = 3; 
			} else {
				val1 = (*(double*) costs(i-1, j-1)) + IndelScore[algnREPLACE]; //replacement
				val2 = 3;
			}
			if ((*(double*) costs(i, j-1)) + IndelScore[algnINSERT] > val1){
				val1 = (*(double*) costs(i, j-1)) + IndelScore[algnINSERT]; //insert (keep source, insert a space in str) is better than replacement;
				val2 = 1;
			}
			if ((*(double*) costs(i-1, j)) + IndelScore[algnDELETE] > val1){
				val1 = (*(double*) costs(i-1, j)) + IndelScore[algnDELETE]; //delete (insert a space in source) is better than replacement;
				val2 = 2;
			}
			if (val1<0) val1=0; //minimal value is 0
			(*(double*) costs(i, j)) = val1;
			(*(long*) direction(i,j)) = val2;
			
			if ((*(double*) costs(i, j)) > max_cost && (!whole_x || i==source_count) ){
				max_cost = (*(double*) costs(i, j));
			}
		}
	}

#ifdef DEBUG_THIS_FILE
	char* row_str=(char*) malloc(sizeof(char)*(source_count+1));
	char* col_str=(char*) malloc(sizeof(char)*(str_count+1));
	for (i=0;i<source_count;i++) {
		if (DebugChar) row_str[i]=(*DebugChar)(sourcetag[i]); 
		else row_str[i]=((ReferLink*)sourcetag[i])->Name.P[0];
	}
	row_str[i]=0;
	for (i=0;i<str_count;i++) {
		if (DebugChar) col_str[i]=(*DebugChar)(strtag[i]); 
		else col_str[i]=((ReferLink*)strtag[i])->Name.P[0];
	}
	col_str[i]=0;
	costs.dumpMatrix("c:\\matrix.txt", __FILE__, HmmMatrix::DoubleMatrix, row_str, col_str);
	free(row_str);
	free(col_str);
#endif

	if (results) *results="<max_positions>";
	char buf[128];
	long count=0; 
	long i_start=1;
	if (whole_x) i_start=source_count; 
	if (source_count>0){
		for (i=i_start; i<=source_count; i++){
			for (j=1; j<=str_count; j++){
				if (((*(double*) costs(i, j)) - max_cost <1e-6) && ((*(double*) costs(i, j)) - max_cost >-1e-6)){
					long d=0;
					long k, l;
					for (k=i, l=j; k>=0 && l>=0 && (*(double*) costs(k, l)> 0 || (whole_x && k>0)); ){
						d= *(long*) direction(k,l);
						if (d==3 && k>0 && l>0) { k--; l--;}
						else if (d==2 && k>0) k--;
						else if (d==1 && l>0) l--;
						else if (k>0) {k--; }
					}
					if (whole_x){ k++; l++;}
					else if (d==3 || *(double*) costs(k, l) < 1e-6 ) { k++; l++;}
					else if (d==2) k++;
					else if (d==1) l++;
					count++; 

					if (results) {
						sprintf(buf, "<position source_pos=\"%ld\" str_pos=\"%ld\" cost=\"%lf\" from_pos=\"%ld\" from_str_pos=\"%ld\" />", 
							sourcepos[i-1], strpos[j-1], max_cost, sourcepos[k-1], strpos[l-1]  );
						// should use k or (k-1)?  use k-1 for now.
						*results+=buf;
					}

					//the last one will be selected
					if (isource1) *isource1=k-1; //from source pos
					if (isource2) *isource2=i-1; //to source pos
					if (istr1) *istr1=l-1; //from str pos
					if (istr2) *istr2=j-1; //to str pos
				}
			}
		}
	}
	if (results) *results+= "</max_positions>";

	if (match_score) *match_score=max_cost;
	return 0;
}
int HyperTagsAlignment::builtHyperTagsForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count){
	if (!stags->Total){
		stags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		stags->setDuplication(false);
	}
	if (!etags->Total){
		etags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		etags->setDuplication(false);
	}

	*sourcetag=0; //source tags, pointing to a tag index
	*sourcepos=0; //source pos, position of each pointer
	*source_count=0;	//length
	long sourcetag_mal = 128;	//malloc memory size
	remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal); //allocate memory

	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(source->P, &source->L, false);
	
	unsigned int len=0;
	char* p=0;
	ReferData label;
	ReferLink* link;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		link=0;
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
			p=TagOperation::getLabel(DataSyntax.Sentence+DataSyntax.Start, &len);
			label.Set(p, len, false);
			link = stags->findName(&label);
			if (!link) link=stags->add(&label, 0, 0);
			
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			p=TagOperation::getLabel(DataSyntax.Sentence+DataSyntax.Start, &len);
			label.Set(p, len, false);
			link = etags->findName(&label);
			if (!link) link=etags->add(&label, 0, 0);
			
		}

		if (link){
			(*sourcetag)[*source_count]=link;
			(*sourcepos)[(*source_count)++]=DataSyntax.Start;
			if (*source_count >=sourcetag_mal){	
				sourcetag_mal+=128;
				remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal);//increase memory size
			}
		}

		DataSyntax.match();
	}
	return 0;
}
int HyperTagsAlignment::builtWordsForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count){
	if (!stags->Total){
		stags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		stags->setDuplication(false);
	}
	if (!etags->Total){
		etags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		etags->setDuplication(false);
	}

	*sourcetag=0; //source tags, pointing to a tag index
	*sourcepos=0; //source pos, position of each pointer
	*source_count=0;	//length
	long sourcetag_mal = 128;	//malloc memory size
	remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal); //allocate memory

	PDocPlainDataSyntax DataSyntax;
	DataSyntax.setSentence(source->P, &source->L, false);
	
	unsigned int len=0;
	char* p=0;
	ReferData label;
	ReferLink* link;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		link=0;
		if (DataSyntax.Type == QLSyntax::synQL_WORD || DataSyntax.Type == QLSyntax::synQL_NUMBER){
			label.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, false);
			link = stags->findName(&label);
			if (!link) link=stags->add(&label, 0, 0);
		}

		if (link){
			(*sourcetag)[*source_count]=link;
			(*sourcepos)[(*source_count)++]=DataSyntax.Start;
			if (*source_count >=sourcetag_mal){	
				sourcetag_mal+=128;
				remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal);//increase memory size
			}
		}

		DataSyntax.match();
	}
	return 0;
}
int HyperTagsAlignment::builtHyperCharStrForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count){
	if (!stags->Total){
		stags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		stags->setDuplication(false);
	}
	if (!etags->Total){
		etags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		etags->setDuplication(false);
	}

	*sourcetag=0; //source tags, pointing to a tag index
	*sourcepos=0; //source pos, position of each pointer
	*source_count=0;	//length
	long sourcetag_mal = 512;	//malloc memory size
	remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal); //allocate memory

	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(source->P, &source->L, false);
	
	unsigned int len=0;
	char* p=0;
	ReferData label;
	ReferLink* link;
	int space=0;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		link=0;
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
			label="STag";
			link = stags->findName(&label);
			if (!link) link=stags->add(&label, 0, 0);
			
			if (link){
				(*sourcetag)[*source_count]=link;
				(*sourcepos)[(*source_count)++]=DataSyntax.Start;
				if (*source_count >=sourcetag_mal){	
					sourcetag_mal+=512;
					remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal);//increase memory size
				}
			}
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			label="ETag";
			link = stags->findName(&label);
			if (!link) link=stags->add(&label, 0, 0);
			
			if (link){
				(*sourcetag)[*source_count]=link;
				(*sourcepos)[(*source_count)++]=DataSyntax.Start;
				if (*source_count >=sourcetag_mal){	
					sourcetag_mal+=512;
					remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal);//increase memory size
				}
			}
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA){
			space=1;
			for (len=0; len<(unsigned long) DataSyntax.StartLen; len++){
				link=0;
				if (tStrOp::isSpace(DataSyntax.Sentence[DataSyntax.Start+len])){
					if (!space){
						label=" ";
						link = stags->findName(&label);
						if (!link) link=stags->add(&label, 0, 0);
					}
					space=1;
				}else{
					space=0;
					label.Set(DataSyntax.Sentence+DataSyntax.Start+len, 1, false);
					link = stags->findName(&label);
					if (!link) link=stags->add(&label, 0, 0);
				}
				if (link){
					(*sourcetag)[*source_count]=link;
					(*sourcepos)[(*source_count)++]=DataSyntax.Start+len;
					if (*source_count >=sourcetag_mal){
						sourcetag_mal+=512;
						remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal);//increase memory size
					}
				}
			}
		}

		DataSyntax.match();
	}
	return 0;
}
int HyperTagsAlignment::builtPlainCharStrForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count){
	if (!stags->Total){
		stags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		stags->setDuplication(false);
	}
	if (!etags->Total){
		etags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		etags->setDuplication(false);
	}

	*sourcetag=0; //source tags, pointing to a tag index
	*sourcepos=0; //source pos, position of each pointer
	*source_count=0;	//length
	long sourcetag_mal = 512;	//malloc memory size
	remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal); //allocate memory
	
	unsigned int len=0;
	char* p=0;
	ReferData label;
	ReferLink* link;

	for (len=0; len<(unsigned long) source->L; len++){
		label.Set(source->P+len, 1, false);
		link = stags->findName(&label);
		if (!link) link=stags->add(&label, 0, 0);
		if (link){
			(*sourcetag)[*source_count]=link;
			(*sourcepos)[(*source_count)++]=len;
			if (*source_count >=sourcetag_mal){
				sourcetag_mal+=512;
				remallocTagsPos(sourcetag, sourcepos, *source_count, sourcetag_mal);//increase memory size
			}
		}
	}

	return 0;
}

int HyperTagsAlignment::remallocTagsPos(void***sourcetag, long**sourcepos, long len, long newlen){
	void** tmp = (void**) malloc(sizeof(void*)*(newlen) );
	memset(tmp, 0, sizeof(void*)*(newlen));
	if (len>0) memcpy(tmp, *sourcetag, sizeof(void*)*(len));
	if (*sourcetag) free(*sourcetag);
	*sourcetag = tmp;

	long* tmp1 = (long*) malloc(sizeof(long)*(newlen) );
	memset(tmp1, 0, sizeof(long)*(newlen));
	if (len>0) memcpy(tmp1, *sourcepos, sizeof(long)*(len));
	if (*sourcepos) free(*sourcepos);
	*sourcepos = tmp1;
	return 0;
}

