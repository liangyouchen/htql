#include "HtTextAlign.h"
#include "alignment.h"
#include "docbase.h"
#include "qhtql.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#define DEBUG_THIS_FILE	
#endif

HtTextAlign::HtTextAlign(){
	TextWords.setSortOrder(SORT_ORDER_KEY_STR_INC);
	TextWords.setCaseSensitivity(false); 
	TextWords.setDuplication(false); 
	//Text1.setSortOrder(SORT_ORDER_NUM_INC); 
	//Text1.setDuplication(false); 
	//Text2.setSortOrder(SORT_ORDER_NUM_INC); 
	//Text2.setDuplication(false); 
	MismatchedRatio=1.0; 
	TextN.setSortOrder(SORT_ORDER_NUM_INC);
	TextN.setDuplication(false); 
	ClusterN=0;
}

HtTextAlign::~HtTextAlign(){
	reset();
}

void HtTextAlign::reset(){
	TextWords.empty(); 
	//Text1.empty(); 
	//Text2.empty(); 
	ReferLink* link; 
	for (link=TextN.getReferLinkHead(); link; link=link->Next){
		ReferLinkHeap* text=(ReferLinkHeap*) link->Value.P; 
		if (text) delete text; 
		link->Value.P=0; 
	}
	TextN.empty(); 
	Clusters.reset(); 
	ClusterN=0;
	ClustersCosts.reset(); 
}

ReferLinkHeap* HtTextAlign::getTextN(long id1_n){
	ReferLinkHeap* text=0; 

	ReferLink* link=TextN.findData(id1_n); 
	if (link){
		text=(ReferLinkHeap*) link->Value.P;
	}else{
		text=new ReferLinkHeap; 
		text->setSortOrder(SORT_ORDER_NUM_INC); 
		text->setDuplication(false); 

		link=TextN.add((ReferData*)0, (ReferData*)0, id1_n); 
		link->Value.P=(char*) text; 
	}

	return text;
}
int HtTextAlign::dropTextN(long id){
	ReferLinkHeap* text=0; 

	ReferLink* link=TextN.findData(id); 
	if (link){
		text=(ReferLinkHeap*) link->Value.P;
		if (text) delete text;
		link->Value.P=0; 
		TextN.remove(link);
	}
	return 0;
}

ReferLink* HtTextAlign::addTextWord(long id1_n, ReferData* word, ReferData* value, long data){
	ReferLinkHeap* text=getTextN(id1_n);

	ReferLink* link1=TextWords.findName(word); 
	if (link1){
		link1->Data++; 
	} else {
		link1=TextWords.add(word, 0, 1); //Name: word; Data: count
	}
	
	ReferLink* link2=text->add((ReferData*) 0, (ReferData*) 0, data); //Data: sorting order
	link2->Name.P=(char*) link1; //Name.P: (ReferLink*)TextWords
	link2->Name.L = text->Total-1; 
	link2->Value.Set(value->P, value->L, value->Type); //Value: by pointer

	return link2; 
}
int HtTextAlign::setPlainText(long id1_n, ReferData* text, long addoffset){
	PDocPlainDataSyntax doc_syntax;
	doc_syntax.setSentence(text->P, &text->L, false);

	ReferData word, value;
	while (doc_syntax.Type != QLSyntax::synQL_END ){
		if (doc_syntax.Type == QLSyntax::synQL_WORD || doc_syntax.Type == QLSyntax::synQL_NUMBER){
			word.Set(doc_syntax.Sentence+doc_syntax.Start, doc_syntax.StartLen, false); //by value
			value.Set(doc_syntax.Sentence+doc_syntax.Start, doc_syntax.StartLen, false); //by pointer
			addTextWord(id1_n, &word, &value, doc_syntax.Start+addoffset); 
		}
		doc_syntax.match();
	}
	return 0; 
}

int HtTextAlign::setHtmlText(long id1_n, ReferData*html, long addoffset, int with_tag){
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(html->P, &html->L, false);

	ReferData word, value;
	char* label; 
	unsigned int len; 
	const char* ignore[]={"<script>", "<style>", 0};
	const char* ignore_end[]={"</script>", "</style>", 0};
	int to_skip=0;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG || DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (with_tag){
				label=TagOperation::getLabel(DataSyntax.Sentence+DataSyntax.Start, &len); 
				word.Set(DataSyntax.Sentence+DataSyntax.Start, label-(DataSyntax.Sentence+DataSyntax.Start)+len, false); //by value
				value.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, false); //by pointer
				addTextWord(id1_n, &word, &value, DataSyntax.Start+addoffset); 
			}
			if (TagOperation::isTags(DataSyntax.Sentence+DataSyntax.Start, ignore)>=0){
				to_skip=1; 
			}else if (TagOperation::isTags(DataSyntax.Sentence+DataSyntax.Start, ignore_end)>=0){
				to_skip=0; 
			}
		}else if (!to_skip && DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA){
			value.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, false); //by pointer
			setPlainText(id1_n, &value, DataSyntax.Start+addoffset); 
		}

		DataSyntax.match();
	}
	return 0; 
}
int HtTextAlign::setCharText(long id1_n, ReferData*str, long addoffset){		//char
	ReferData word, value;
	for (long i=0; i<str->L; i++){
		word.Set(str->P+i, 1, false); 
		value.Set(str->P+i, 1, false); 
		addTextWord(id1_n, &word, &value, i+addoffset); 
	}
	return 0;
}

int HtTextAlign::alignCmpFun(void* link_text1, void* link_text2){
	if (link_text1==link_text2) return 0; //include NULL 
	if (!link_text1 || !link_text2) return 1; //NULL
	//((ReferLink*) link_text1)->Name.P points to TextWords ReferLink*
	return (((ReferLink*) link_text1)->Name.P == ((ReferLink*) link_text2)->Name.P)?0:1;
}
char HtTextAlign::alignGetDebugChar(void* link_text1){
	if (!link_text1 || !((ReferLink*) link_text1)->Name.P) return '-'; 

	ReferLink* wordlink= (ReferLink*) ((ReferLink*) link_text1)->Name.P; 
	return wordlink->Name.P[0]; 
}
long HtTextAlign::translateCmpCode(void* link_text1){
	return link_text1?((long) ((ReferLink*) link_text1)->Name.P):0; 
}
void** HtTextAlign::mallocLinkArray(ReferLinkHeap* text){
	if (!text) return 0;
	void** sourcetag=(void**) malloc(sizeof(void*)*(text->Total+1)); 
	memset(sourcetag, 0, sizeof(void*)*(text->Total+1));
	ReferLink* link; 
	long i=0; 
	for (link=(ReferLink*)text->moveFirst(); link; link=(ReferLink*)text->moveNext()){
		//sourcetag[i]=link->Name.P; 
		sourcetag[i]=link; 
		i++; 
	}
	return sourcetag; 
}
int HtTextAlign::cmpLinks(void* link_text1, void* link_text2){ //0:match;1:insert;2:delete;31:replace;
	if (link_text1 && link_text2 && alignCmpFun(link_text1, link_text2)==0) //matched
		return 0; 
	else if (link_text1 && link_text2) //replace
		return 3; 
	else if (link_text2) //insert
		return 1; 
	else if (link_text1) //delete
		return 2; 
	return 0; 
}


int HtTextAlign::alignMultiple(ReferLinkHeap* aligned_strs, long* aligned_len, double* cost, long start_i, long parent_id, long level){
	//recursively align two sets of strings, see also getClustersHtml
	if (start_i<0) start_i=ClusterN-1; 
	long id1=Clusters(start_i,0); 
	long id2=Clusters(start_i,1); 
	ReferLink* link1=TextN.findData(id1); 
	ReferLink* link2=TextN.findData(id2); 

	//left child
	ReferLinkHeap left_child, left_align;
	double left_cost=0.0, left_align_cost=0.0;
	long left_len=0;

	left_child.setSortOrder(SORT_ORDER_NUM_INC);

	if (id1!=parent_id){
		//set cluster name as link1->Name, at this level;
	}

	//left child clusters
	long i;
	for (i=start_i-1; i>=0; i--){
		if (Clusters(i,0)==id1 || Clusters(i,1)==id1){
			break;
		}
	}
	if (i>=0){ 
		//align left children
		alignMultiple(&left_align, &left_len, &left_cost, i, id1, level+1);
	}


	ReferLinkHeap right_child, right_align;
	double right_cost=0.0, right_align_cost=0.0;
	long right_len=0;

	right_child.setSortOrder(SORT_ORDER_NUM_INC);

	//right child
	if (id2!=parent_id){
		//set cluster name as link2->Name, at level;
	}

	//right child clusters
	for (i=start_i-1; i>=0; i--){
		if (Clusters(i,0)==id2 || Clusters(i,1)==id2){
			break;
		}
	}
	if (i>=0){
		//align right children
		alignMultiple(&right_align, &right_len, &right_cost, i, id2, level+1);
	}

	//merge left & right child clusters
	aligned_strs->setSortOrder(SORT_ORDER_NUM_INC);
	mergeAlignments(&left_align, id1, left_len, &right_align, id2, right_len, aligned_strs, aligned_len);
	return 0;
}



int HtTextAlign::mergeAlignments(ReferLinkHeap* left_align, long id_left, long left_len, ReferLinkHeap* right_align, long id_right, long right_len, ReferLinkHeap* results, long* result_len){
	
	//
	double indel_cost[4]={-3,left_align->Total,right_align->Total,left_align->Total+right_align->Total}; 
	// win scores for matching 

	ReferLink** source_align_links=0;
	ReferLink** str_align_links=0;
	double cost=0;

	align(id_left, id_right, &cost, result_len, &source_align_links, &str_align_links, indel_cost); 
#ifdef DEBUG_THIS_FILE
	long debug_i=0;
	TRACE("source (%ld): ", id_left);
	for (debug_i=0; debug_i<*result_len; debug_i++){
		if (source_align_links[debug_i]){
			TRACE("%s ", ((ReferLink*)(source_align_links[debug_i]->Name.P ))->Name.P);
		}else{
			TRACE(" - ");
		}
	}
	TRACE("\r\n");
	TRACE("string (%ld): ", id_right);
	for (debug_i=0; debug_i<*result_len; debug_i++){
		if (str_align_links[debug_i]){
			TRACE("%s ", ((ReferLink*)(str_align_links[debug_i]->Name.P ))->Name.P);
		}else{
			TRACE(" - ");
		}
	}
	TRACE("\r\n");
#endif
	
	ReferLink* link=0, **left_links=0, ** right_links=0;
	int has_id_left=1, has_id_right=1; 
	link=left_align->findData(id_left); 
	if (!link){
		link=left_align->add((char*) 0, 0, id_left); 
		link->Value.Set((char*) source_align_links, *result_len, false); 
		left_len=*result_len; 
		has_id_left=0; 
	}
	left_links=(ReferLink**) link->Value.P; 

	link=right_align->findData(id_right); 
	if (!link){
		link=right_align->add((char*) 0, 0, id_right); 
		link->Value.Set((char*) str_align_links, *result_len, false); 
		right_len=*result_len; 
		has_id_right=0;
	}
	right_links=(ReferLink**) link->Value.P; 

	//count gaps for left_align and right_align based on this alignment
	//estimate new alignment lenght
	long new_left_len=left_len, new_right_len=right_len; 
	long i; 
	for (i=0; i<*result_len; i++){
		if (!source_align_links[i]) new_left_len++;
		if (!str_align_links[i]) new_right_len++;
	}
	long* insert_left=(long*) malloc(sizeof(long)*(left_len+2)); 
	memset(insert_left, 0, sizeof(long)*(left_len+2));
	long* insert_right=(long*) malloc(sizeof(long)*(right_len+2)); 
	memset(insert_right, 0, sizeof(long)*(right_len+2));

	//count left and right gaps
	long new_left_i=0, new_right_i=0; 
	long left_gap=0, right_gap=0; 
	new_left_len=left_len, new_right_len=right_len;
	for (i=0; i<*result_len; i++){
		if (source_align_links[i]){
			while (left_links[new_left_i]!=source_align_links[i]) {
				new_left_i++;
				right_gap++;
			}
			right_gap++;
			new_left_i++;
		}else if (!left_links[new_left_i]){
			right_gap++;
			new_left_i++;
		}
		if (str_align_links[i]){
			while (right_links[new_right_i]!=str_align_links[i]) {
				new_right_i++;
				left_gap++;
			}
			left_gap++;
			new_right_i++;
		}else if (!right_links[new_right_i]){
			left_gap++;
			new_right_i++;
		}
		if (str_align_links[i] && right_gap>left_gap ){
			insert_right[new_right_i-1]=right_gap-left_gap; 
			right_gap=0;
			left_gap=0;
			new_right_len+=insert_right[new_right_i-1]; 
		}
		if (source_align_links[i] && left_gap>right_gap ){
			insert_left[new_left_i-1]=left_gap-right_gap; 
			left_gap=0;
			right_gap=0;
			new_left_len+=insert_left[new_left_i-1]; 
		}
	}
	
	*result_len=new_left_len; 
	if (*result_len < new_right_len) *result_len=new_right_len; 

#ifdef DEBUG_THIS_FILE
	TRACE("left (%ld):  ", id_left);
	for (debug_i=0; debug_i<left_len; debug_i++){
		TRACE("%d ", insert_left[debug_i]);
	}
	TRACE("\r\n");
	TRACE("right (%ld): ", id_right);
	for (debug_i=0; debug_i<right_len; debug_i++){
		TRACE("%d ", insert_right[debug_i]);
	}
	TRACE("\r\n");
#endif

	//insert gaps and make new alignment
	//insert left gaps
	for (link=left_align->getReferLinkHead(); link; link=link->Next){
		left_links=(ReferLink**) link->Value.P; 
		ReferLink** new_left_link=(ReferLink**) malloc(sizeof(ReferLink*)*(*result_len+2));
		memset(new_left_link, 0, sizeof(ReferLink*)*(*result_len+2)); 

		new_left_i=0; 
		for (i=0; i<left_len; i++){
			if (insert_left[i]){
				for (long j=0; j<insert_left[i]; j++){
					new_left_link[new_left_i++]=0;
				}
			}
			new_left_link[new_left_i++]=left_links[i]; 
		}

		ReferLink* new_link=(ReferLink*) results->add((char*) 0, 0, link->Data); 
		new_link->Value.Set((char*) new_left_link, *result_len, false); 

		free(left_links); 
		link->Value.P=0;
	}

	//insert right gaps
	for (link=right_align->getReferLinkHead(); link; link=link->Next){
		right_links=(ReferLink**) link->Value.P; 
		ReferLink** new_right_link=(ReferLink**) malloc(sizeof(ReferLink*)*(*result_len+2));
		memset(new_right_link, 0, sizeof(ReferLink*)*(*result_len+2)); 

		new_right_i=0; 
		for (i=0; i<right_len; i++){
			if (insert_right[i]){
				for (long j=0; j<insert_right[i]; j++){
					new_right_link[new_right_i++]=0;
				}
			}
			new_right_link[new_right_i++]=right_links[i]; 
		}

		ReferLink* new_link=(ReferLink*) results->add((char*) 0, 0, link->Data); 
		new_link->Value.Set((char*) new_right_link, *result_len, false); 

		free(right_links); 
		link->Value.P=0;
	}

	if (insert_left) free(insert_left);
	if (insert_right) free(insert_right);

	if (source_align_links && has_id_left) free(source_align_links); 
	if (str_align_links && has_id_right) free(str_align_links); 
	return 0;
}
int HtTextAlign::freeAlignMemory(ReferLinkHeap* aligned_strs){
	for (ReferLink* link=aligned_strs->getReferLinkHead(); link; link=link->Next){
		ReferLink** p=(ReferLink**) link->Value.P; 
		if (p) free(p); 
		link->Value.P=0;
	}
	return 0;
}

int HtTextAlign::align(long id1, long id2, double*cost, long* result_len, ReferLink*** source_align_links, ReferLink*** str_align_links, double* indel_cost){
	//align progressively in each window 
	//copy data to arrays
	ReferLinkHeap* text1=getTextN(id1); 
	ReferLinkHeap* text2=getTextN(id2); 
	if (!text1 || !text2) return -1; 

	//estimate alignment window size
	long win_size=estimateWindowSize(text1, text2); 
	if (win_size>1000) win_size=1000; 
	if (win_size<50) win_size=50; 

	void** sourcetag=mallocLinkArray(text1); 
	void** strtag=mallocLinkArray(text2); 

	long i=0; 

	//allocate global alignment positions
	void** source_align_tag=(void**) malloc(sizeof(void*)*(text1->Total+text2->Total+1)); 
	void** str_align_tag=(void**) malloc(sizeof(void*)*(text1->Total+text2->Total+1)); 
	long aligned_len=0; 
	double aligned_cost=0; 

	//align data progressively
	//HyperTagsAlignment align; 
	StrAlignment align; 
	unsigned long isource=0, istr=0, sourcecount=win_size, str_count=win_size; 
	double match_score;
	ReferData results; 
	long isource1=0, isource2=0,istr1=0,istr2=0; //offset
	long iteration=0; 
	while (isource<text1->Total || istr<text2->Total){
		iteration++; 
		//set the data length to match
		if (isource+sourcecount>text1->Total) sourcecount=text1->Total-isource; 
		if (istr+str_count>text2->Total) str_count=text2->Total-istr; 

		//find local matches
		isource1=isource2=istr1=istr2=0; //offsets
		align.setIndelCost(1.0, -0.5/MismatchedRatio, -0.5/MismatchedRatio, -1.0/MismatchedRatio); 
		if (indel_cost){
			align.setIndelCost(indel_cost[StrAlignment::algnREPLACE]-indel_cost[StrAlignment::algnMATCH], 
				-indel_cost[StrAlignment::algnINSERT]/MismatchedRatio, 
				-indel_cost[StrAlignment::algnDELETE]/MismatchedRatio, 
				-(indel_cost[StrAlignment::algnREPLACE]-indel_cost[StrAlignment::algnMATCH])/MismatchedRatio); 
		}
		align.DebugChar=alignGetDebugChar; 
		//align.CmpFun=alignCmpFun;
		align.CmpCode=translateCmpCode; //istr1 and istr2 are translated codes
		int err=align.alignHyperTags(sourcetag+isource, sourcecount, strtag+istr, str_count, 
					&results, &match_score, &isource1, &isource2, &istr1, &istr2); 
		if (err<0){
			if (sourcecount>str_count && sourcecount>100) sourcecount/=2; 
			else if (str_count>sourcecount && str_count>100) str_count/=2; 
			else { //cannot align, failed
				break; 
			}
			continue; 
		}

		if (isource2==0 || istr2==0) {
			isource2=sourcecount; 
			istr2=str_count;
		}else{
			isource2++; istr2++; //alignment returns the positions, here we use counts
		}

		//do global alignment between isource->isource+isource2 and istr->istr+istr2;
		StrAlignment match; 
		match.setIndelCost(0.0, 1.0, 1.0, 2.0); 
		if (indel_cost)
			match.setIndelCost(indel_cost[StrAlignment::algnMATCH],indel_cost[StrAlignment::algnINSERT],indel_cost[StrAlignment::algnDELETE],indel_cost[StrAlignment::algnREPLACE]);
		match.CmpFun=alignCmpFun; 
		match.DebugChar=alignGetDebugChar; 
		//result_tag1 and result_tag2 are the original codes
		double match_cost=0; 
		long match_len=0; 
		void** result_tag1=0, **result_tag2=0; 
		match.compareHyperTags(sourcetag+isource, isource2, strtag+istr, istr2, 
			&match_cost, &match_len, &result_tag1, &result_tag2); 

#ifdef DEBUG_THIS_FILE
		ReferData debug_str1, debug_str2; 
		char debug_file[256]; 
		sprintf(debug_file, "HtTextAlign_align%ld.txt", iteration); 
		FILE* debug_f=fopen(debug_file, "w+"); 
		for (i=0; i<match_len; i++){
			debug_str1.reset(); 
			if (result_tag1[i] && ((ReferLink*)(result_tag1[i]))->Name.P) 
				debug_str1.Set(((ReferLink*)((ReferLink*)(result_tag1[i]))->Name.P)->Name.P, ((ReferLink*)((ReferLink*)(result_tag1[i]))->Name.P)->Name.L, false);
			if (debug_str1.L>60) debug_str1.L=60; 
			debug_str1.Seperate(); 

			debug_str2.reset(); 
			if (result_tag2[i] && ((ReferLink*)(result_tag2[i]))->Name.P) 
				debug_str2.Set(((ReferLink*)((ReferLink*)(result_tag2[i]))->Name.P)->Name.P, ((ReferLink*)((ReferLink*)(result_tag2[i]))->Name.P)->Name.L, false);
			if (debug_str2.L>60) debug_str2.L=60; 
			debug_str2.Seperate(); 

			fprintf(debug_f, "%ld:%d:%s:%s\r\n", iteration, i, debug_str1.P?debug_str1.P:"NULL", debug_str2.P?debug_str2.P:"NULL"); 
		}
		fclose(debug_f); 
#endif

		memcpy(source_align_tag+aligned_len, result_tag1, match_len*sizeof(void*)); 
		memcpy(str_align_tag+aligned_len, result_tag2, match_len*sizeof(void*)); 
		aligned_len+=match_len; 
		aligned_cost+=match_cost; 

		isource+=isource2; 
		istr+=istr2; 

		if (result_tag1) free(result_tag1); 
		if (result_tag2) free(result_tag2); 
	}
	
	//copy to results 
	if (cost) *cost=aligned_cost; 
	if (result_len) *result_len=aligned_len; 
	if (source_align_links) {
		*source_align_links=(ReferLink **) source_align_tag; 
		source_align_tag=0; 
	}
	if (str_align_links){
		*str_align_links=(ReferLink **) str_align_tag; 
		str_align_tag=0; 
	}

	if (sourcetag) free(sourcetag); 
	if (strtag) free(strtag); 
	if (source_align_tag) free(source_align_tag);
	if (str_align_tag) free(str_align_tag); 

	return 0; 
}
long HtTextAlign::addToClusters(long id, int allow_dupplicate){
	long closest_id=-1; 

	double cost1=0, cost2=0, cost12=0;
	long len1=0, len2=0, len12=0; 
	double score1=0, score2=0, score12=0; 

	long i=ClusterN-1; 
	if (i<0){
		insertClusterRow(0, id, -1, -1); 
		return id;
	}else if (i==0 && Clusters(i, 1)<0){
		long id1=Clusters(i,0);
		score1=getClustersCost(id1,id); 
		if (!allow_dupplicate && score1<1e-10){
			return -id1; //use minus score, avoid using 0 id
		}
		Clusters(i,1)=id; 
		Clusters(i,2)=(long) score1;
		return id1;
	}

	//compare the distance of the top two nodes; 
	while (i>=0){
		long id1=Clusters(i,0); 
		long id2=Clusters(i,1);
		//we can save 1/3 time if reuse alignment score in each iteration
		score1=getClustersCost(id1,id); 
		score2=getClustersCost(id2,id); 
		score12=getClustersCost(id1,id2); 
#ifdef DEBUG_THIS_FILE
		TRACE("id (%ld)--id1 (%ld) = %.4f, id (%ld)--id2 (%ld) = %.4f, id1 (%ld)--id2 (%ld) = %.4f\n", id, id1, score1, id, id2, score2, id1, id2, score12);
#endif

		if (!allow_dupplicate){
			if (score1<1e-10){
				return -id1; //use minus score, avoid using 0 id
			}else if (score2<1e-10){ //exact matched? 
				return -id2; //use minus score, avoid using 0 id
			}
		}

		if (score1>score12 && score2>score12){ //different from existing clusters
			/*if (score1>score2){
				closest_id=id2;
				insertClusterRow(i+1, id2, id, score2); 
			}else{
				closest_id=id1;
				insertClusterRow(i+1, id1, id, score1); 
			} */
			insertClusterRow(i+1, id1, id, score1); 
			closest_id=(score1>score2)?id2:id1;
			break;
		}else if (score1>score2){ //belongs to id2 cluster
			long j; 
			for (j=i-1; j>=0; j--){
				if (Clusters(j,0)==id2 || Clusters(j,1)==id2){
					break;
				}
			}
			if (j>=0) i=j;
			else{
				insertClusterRow(i, id2, id, score2); //how about id,id2? need to check later
				closest_id=id2;
				break;
			}
		}else{ //belongs to id1 cluster
			long j;
			for (j=i-1; j>=0; j--){
				if (Clusters(j,0)==id1 || Clusters(j,1)==id1){
					break;
				}
			}
			if (j>=0) i=j;
			else{
				insertClusterRow(i, id1, id, score1); //how about id,id1? need to check later
				closest_id=id1;
				break;
			}
		}
	}
#ifdef DEBUG_THIS_FILE
	ReferData Debug_page;
	getClustersHtml(&Debug_page);
	char Debug_tmp[256];
	sprintf(Debug_tmp, "$HtTextAlign_addToClusters_%ld.html", id); 
	Debug_page.saveFile(Debug_tmp);
#endif
	return closest_id;
}
int HtTextAlign::insertClusterRow(long i, long id1, long id2, double cost){
	if (ClusterN==0){
		ClustersCosts.reset(); 
	}
	if (ClusterN>=Clusters.Rows-1){
		Clusters.expandDim(ClusterN+128, Clusters.Cols?Clusters.Cols:3); 
	}

	long j,k;
	for ( j=ClusterN-1; j>=i; j--){
		for (k=0; k<Clusters.Cols; k++){
			Clusters(j+1,k)=Clusters(j,k);
		}
	}
	Clusters(i,0)=id1;
	Clusters(i,1)=id2;
	Clusters(i,2)=(long)(cost*10e4);
	ClusterN++;
	return 0;
}
double HtTextAlign::getClustersCost(long id1, long id2){
	double cost=-1;
	long len;
	char buf[256]; 
	sprintf(buf, "(%ld)(%ld)", id1, id2);
	ReferLink*link=ClustersCosts.findName(buf);
	if (link) cost=(double)link->Data/10.0e6; 
	else{
		align(id1, id2, &cost, &len, 0, 0); 
		if (cost<0.5) return cost; 

		cost/=(2.0*len);

		sprintf(buf, "(%ld)(%ld)", id1, id2);
		ClustersCosts.add(buf, 0, (long)(cost*10e6));
		sprintf(buf, "(%ld)(%ld)", id2, id1);
		ClustersCosts.add(buf, 0, (long)(cost*10e6));
	}
	return cost;
}
long HtTextAlign::isInCluster(long id){
	long i;
	for ( i=0; i<ClusterN; i++){
		if (Clusters(i,0)==id || Clusters(i,1)==id) return i+1; 
	}
	return 0;
}
long HtTextAlign::getClosestNeighbor(long id){
	long i;
	for ( i=0; i<ClusterN; i++){
		if (Clusters(i,0)==id) return Clusters(i,1);
		if (Clusters(i,1)==id) return Clusters(i,0);
	}
	return -1;
}

int HtTextAlign::getClustersHtml(ReferData* page, long start_i, long parent_id, long level){
	if (start_i<0) start_i=ClusterN-1; 
	char buf[256]; 
	//left
	*page="<Table border=2 cellSpacing=0 cellspadding=1 borderColor=yellow borderColorLight=firebrick borderColorDark=white bgColor=white ><tr><td>\n"; 
	long id1=Clusters(start_i,0); 
	long id2=Clusters(start_i,1); 
	ReferLink* link1=TextN.findData(id1); 
	ReferLink* link2=TextN.findData(id2); 

	if (id1!=parent_id){
		for (long i=0; i<level; i++) 
			*page+="+ ";
		if (link1 && link1->Name.P){
			*page+=link1->Name;
		}else{
			sprintf(buf, "%ld", id1);
			*page+=buf;
		}
		sprintf(buf, " (%ld)", Clusters(start_i,2));
		*page+=buf;
	}

	//left child clusters
	long i;
	for (i=start_i-1; i>=0; i--){
		if (Clusters(i,0)==id1 || Clusters(i,1)==id1){
			break;
		}
	}
	if (i>=0){
		ReferData page1; 
		getClustersHtml(&page1, i, id1, level+1); 
		*page+=page1; 
	}

	//right
	*page+="</td></tr><tr><td>\n"; 
	if (id2!=parent_id){
		for (long i=0; i<level; i++) 
			*page+="+ ";
		if (link2 && link2->Name.P){
			*page+=link2->Name;
		}else{
			sprintf(buf, "%ld", id2);
			*page+=buf;
		}
		sprintf(buf, " (%ld)", Clusters(start_i,2));
		*page+=buf;
	}

	//right child clusters
	for (i=start_i-1; i>=0; i--){
		if (Clusters(i,0)==id2 || Clusters(i,1)==id2){
			break;
		}
	}
	if (i>=0){
		ReferData page2; 
		getClustersHtml(&page2, i, id2, level+1); 
		*page+=page2; 
	}


	*page+="</td></tr></table>\n"; 
	
	return 0;
}
int HtTextAlign::setIdInfo(long id, const char* info, int copy){ //set to TextN->Name
	ReferLink* link=TextN.findData(id); 
	if (link){
		link->Name.Set((char*) info, info?strlen(info):0, copy);
	}
	return 0;
}
int HtTextAlign::moveNextLink(ReferLink** link, long* index, char* search_P, ReferLinkHeap* text){
	while ((*link) && (*link)->Name.P != search_P) {
		(*link)=(ReferLink*) text->moveNext(); 
		if (index) index++; //use link->Name.L instead
	}
	return 0; 
}

long HtTextAlign::estimateWindowSize(ReferLinkHeap* text1, ReferLinkHeap* text2, long max_size){ 
	//max_size not used now

	//find the most frequent word
	ReferLink* link=0, *link_max=0; 
	for (link=TextWords.getReferLinkHead(); link; link=link->Next){
		if (!link_max || link_max->Data < link->Data){ //max count
			link_max=link; 
		}
	}
	
	//find the largest gap between Text1 and Text2 for the most frequent word
	ReferLink* link1_last=(ReferLink*) text1->moveLast(); 
	ReferLink *link2_last=(ReferLink*) text2->moveLast(); 
	ReferLink* link1=(ReferLink*) text1->moveFirst(); 
	ReferLink *link2=(ReferLink*) text2->moveFirst(); 
	if (!link1_last || !link2_last) return 2; 

	//moveNextLink(&link1, 0, (char*) link_max, text1); //use link->Name.L instead
	//moveNextLink(&link2, 0, (char*) link_max, text2); //use link->Name.L instead
	while (link1 && link1->Name.P != (char*) link_max) link1=(ReferLink*) text1->moveNext(); 
	while (link2 && link2->Name.P != (char*) link_max) link2=(ReferLink*) text2->moveNext(); 
	long gap=1; 
	if (link1) gap=link1->Name.L; 
	if (link2 && link2->Name.L > gap) gap=link2->Name.L; 

	while (!text1->isEOF() || !text1->isEOF()){
		if (link1 && link2 && link1->Name.L >= link2->Name.L){
			if (gap < link1->Name.L-link2->Name.L) 
				gap=link1->Name.L-link2->Name.L; 
			link2=(ReferLink*) text2->moveNext();
			while (link2 && link2->Name.P != (char*) link_max) link2=(ReferLink*) text2->moveNext(); 
		}else if (link1 && link2 && link1->Data < link2->Data){
			if (gap < link2->Name.L-link1->Name.L) 
				gap=link2->Name.L-link1->Name.L; 
			link1=(ReferLink*) text1->moveNext();
			while (link1 && link1->Name.P != (char*) link_max) link1=(ReferLink*) text1->moveNext(); 
		}else if (link1){
			if (link1->Name.L >= link2_last->Name.L) {
				if (gap < link1->Name.L - link2_last->Name.L) 
					gap=link1->Name.L - link2_last->Name.L; 
			}else{
				if (gap < link2_last->Name.L - link1->Name.L) 
					gap=link2_last->Name.L - link1->Name.L; 
			}
			link1=(ReferLink*) text1->moveNext();
			while (link1 && link1->Name.P != (char*) link_max) link1=(ReferLink*) text1->moveNext(); 
		}else if (link2){
			if (link2->Name.L >= link1_last->Name.L) {
				if (gap < link2->Name.L - link1_last->Name.L) 
					gap=link2->Name.L - link1_last->Name.L; 
			}else{
				if (gap < link1_last->Name.L - link2->Name.L) 
					gap=link1_last->Name.L - link2->Name.L; 
			}
			link2=(ReferLink*) text2->moveNext();
			while (link2 && link2->Name.P != (char*) link_max) link2=(ReferLink*) text2->moveNext(); 
		}
	}

	return gap*2;
}






