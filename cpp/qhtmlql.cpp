#include "qhtmlql.h"
#include "htmlql.h"

#include "docbase.h"
#include "htqlexpr.h"
#include "alignment.h"
#include "HtPageModel.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE 1
#endif



HtmlQLParser::HtmlQLParser(){
	FindHtqlPageMark=0; 
	FindHtqlPageMarkLevel=0;

	addHtmlQLFunctions(this);
}

HtmlQLParser::~HtmlQLParser() {
	reset();
	resetHtqlFunctions();
}

void HtmlQLParser::reset() {
	HTQLParser::reset();

	//do not reset these!
	//FindHtqlPageMark=0; 
	//FindHtqlPageMarkLevel=0;
}

int HtmlQLParser::addHtmlQLFunctions(HTQLParser* parser){
	parser->addHtqlFunction("FORM_INPUTS", (void*) functionFormInputs,"FORM_INPUT: tag, name, value, tx(description)");
	parser->addHtqlFunction("TAG_PARENT", (void*) functionTagParent, "goto the parent tag; PARA 1: tag name of the parent tag");
	parser->addHtqlFunction("TAG_NEXT", (void*) functionTagNext, "goto the next tag; PARA 1: tag name of the next tag");
	parser->addHtqlFunction("TAG_PREV", (void*) functionTagPrevious, "goto the previous tag; PARA 1: tag name of the next tag");
	parser->addHtqlFunction("TAG_IEPOS", (void*) functionIEPosTag, "goto the tag of a position return by IE browser; PARA 1: tag index");
	parser->addHtqlFunction("TAG_ATTRS", (void*) functionTagAttrs, "TAG_ATTRS: name, value");	
	parser->addHtqlFunction("TAG_INDEX", (void*) functionTagIndex, "TAG_INDEX: tag, name, index, use_name");	
	parser->addHtqlFunction("TAG_LEAVES", (void*) functionTagLeaves, "TAG_LEAVES");	
	parser->addHtqlFunction("FIND_HTQL", (void*) functionFindHtql, "find htql;");	
	parser->addHtqlFunction("FIND_FOCUS_HTQL", (void*) functionFindFocusHtql, "find focus htql;");	
	parser->addHtqlFunction("FIND_LEAVES_HTQL", (void*) functionFindLeavesHtql, "find leaves htql;");	
	parser->addHtqlFunction("PLAIN_LEAVES_HTQL", (void*) functionPlainLeavesHtql, "PLAIN_LEAVES_HTQL");	
	parser->addHtqlFunction("TAG_LEAVES_HTQL", (void*) functionTagLeavesHtql, "TAG_LEAVES_HTQL");	
	parser->addHtqlFunction("KEY_LEAVES_HTQL", (void*) functionKeyLeavesHtql, "KEY_LEAVES_HTQL");	
	parser->addHtqlFunction("LEAVES_HTQL_SCHEMA", (void*) functionLeavesHtqlSchema, "LEAVES_HTQL_SCHEMA; PARA 1: htql prefix");	
	parser->addHtqlFunction("REPEAT_PATTERNS", (void*) functionRepeatPatterns, "find repeat patterns; PARAS: maximum number of patterns to find");	
	parser->addHtqlFunction("URL_GO", (void*) functionUrlGo, "go to the url; PARAS: name and values for url");	
	parser->addHtqlFunction("OFFSET", (void*) functionOffset, "tag definition by offset;");	
	parser->addHtqlFunction("POSITION", (void*) functionPosition, "position information of tag;");	
	parser->addHtqlFunction("SUBSTR", (void*) functionSubStr, "tag definition by substring;");	
	parser->addHtqlFunction("HTQL_BESTVIEWPOS", (void*) functionHtqlMaxViewPos, "best view of an htql;");	
	parser->addHtqlFunction("FIND_REPEAT_VIEWS", (void*) functionFindRepeatViews, "find views with max repeat; PARA 1: show_num, PARA 2: search_num");	
	parser->addHtqlFunction("FIND_BEST_VIEWS", (void*) functionFindBestViews, "find max items views; PARA 1: show_num, PARA 2: search_num");	
	parser->addHtqlFunction("HTML_TITLE", (void*) functionHtmlTitle, "get the title or header of the page; PARA 1:<H> tag level");	
	parser->addHtqlFunction("HTML_MAIN_DATE", (void*) functionHtmlMainDate, "get the main date of the page");	
	parser->addHtqlFunction("HTML_MAIN_TEXT", (void*) functionHtmlMainText, "get the main text of the page");	
	parser->addHtqlFunction("HTML_KEY_TEXT", (void*) functioHtmlKeyText, "get the main text of the keys");	
	parser->addHtqlFunction("GET_EMAIL", (void*) functionGetEmail, "get emails from text; PARA 1: format such as %n %m");	
	parser->addHtqlFunction("TEXT_WORDS", (void*) functionTextWords, "get words from text; PARA 1: type in 'count/repeat/first/subsentence/text'; PARA 2: option");	

	HTQLFunction* htqlfun=0;
	parser->addHtqlFunction("REFINE_NULL_FIELDS", (void*) 0, "refile null fields; PARA 1: threshold", &htqlfun);
	if (htqlfun) {
		htqlfun->FunPrepare = functionRefineNullFields;
	}
	parser->addHtqlFunction("FORMAT_HTML_RESULTS", (void*) 0, "format results in HTML table format; PARA 1: html encode", &htqlfun);
	if (htqlfun) {
		htqlfun->FunPrepare = functionFormatHtqlResults;
	}

	return 0;
}

int HtmlQLParser::functionUrlGo(char* p, void* call_from, ReferData* tx){
	tx->reset();

	HTQLParser* call = (HTQLParser* )call_from;
	ReferData url;
	call->functionUrl(p, call_from, &url);

	HtmlBuffer html;
	html.setUrl(url.P);
	for (ReferLink* para=call->FunctionParameters; para; ){
		if (para->Name.L==0){
			html.setVariable(para->Value.P, para->Next?para->Next->Value.P:"");
			para = para->Next;
			if (para) para=para->Next;
		}else {
			html.setVariable(para->Name.P, para->Value.P);
			para=para->Next;
		}
	}
	if (call->FunctionParameters){
		html.fetchHtml(0);
	}else{
		html.fetchHtml(url.P);
	}

	if (html.Buffer.DataLen){
		size_t len=0;
		char* data = html.Buffer.GetDataMem(&len);
		if (data && len){
			call->FunctionCurrentItem->Data.Set(data, len, false);
			call->FunctionCurrentItem->Data.setToFree(1);
			call->CurrentSourceUrl.Set(url.P, url.L, true);
		}
	}
	return 0;
}

int HtmlQLParser::functionRefineNullFields(char*p, void* call_from, ReferData* tx){//&refine_nullfields(threshold)
	HTQLParser* call = (HTQLParser* )call_from;

	call->moveFirst();
	int fieldsnum = call->getFieldsCount();
	char* val;
	int i;
	int count;
	double threshold = 0.7;
	ReferLink* para =call->FunctionParameters;
	if (para && !para->Value.isNULL() ){
		sscanf(para->Value.P, "%lf", &threshold);
	}
	double thresnum=threshold*fieldsnum;

	while (!call->isEOF()){
		count=0;
		for (i=1; i<=fieldsnum; i++){
			val = call->getField(i);

			if (!val || !val[0]){
				count++;
			}else{
				int j=0;
				for (j=0; val[j] && tStrOp::isSpace(val[j]) ; j++);
				if (!val[j]) count++;
			}
		}
		if ((double) count >= thresnum){
			call->dropCurrentRecord();
		}else{
			call->moveNext();
		}
	}
	return 0;
}

int HtmlQLParser::functionFormatHtqlResults(char*p0, void* call_from, ReferData* tx){//&FORMAT_HTML_RESULTS(to_encode)
	HTQLParser* call = (HTQLParser* )call_from;
	ReferData text;

	long to_encode=0;
	long max_tuple=0;
	readHtqlParameterLong(call->FunctionParameters, 1, &to_encode);
	readHtqlParameterLong(call->FunctionParameters, 2, &max_tuple);

	text="<html><body>";
	text+="<base href='";
	text += call->getSourceUrl();
	text+= "'>";
	int i=0, fieldsnum=call->getFieldsCount();
	long count=0;

	text+="<table border=1>";
	text+="<tr bgcolor=lightgreen type=schema>";
	char* p=0;
	char buf[20];
	for (i=1; i<=fieldsnum; i++){
		p=call->getFieldName(i);
		text+="<td>";
		if (p&&*p){
			text+=p;
		}else{
			sprintf(buf, "%%%d", i);
			text+=buf;
		}
		text+="</td>";
	}
	text+="</tr>\n";
	
	call->moveFirst();
	while (!call->isEOF()){
		text+="<tr bfcolor=white type=tuple>";
		for (i=1; i<=fieldsnum; i++){
			p= call->getField(i);
			text+="<td valign=top align=left>";
			if (p&&*p){
				if (to_encode){
					ReferData encoded;
					toHtmlPrintable(p,0, &encoded);
					if (encoded.L) text+=encoded;
					else text+="<br>";
				}else{
					text+=p;	 
				}
			}else{
				text+="<br>";
			}
			text+="</td>";
		}
		text+="</tr>\n";
		count++;
		if (max_tuple && count>=max_tuple) break;
		call->moveNext();
	}

	text+="</table>\n";
	text+="<tableinfo Attributes=";
	sprintf(buf, "%d", fieldsnum);
	text+=buf;
	text+=" Tuples=";
	sprintf(buf, "%ld", count);
	text+=buf;
	text+=" Total=";
	sprintf(buf, "%ld", fieldsnum*count);
	text+=buf;
	text+=">\n";
	text+="</body></html>";
	call->resetData();
	call->Data->Data.Set(text.P, text.L, false);
	call->Data->Data.setToFree(true);
	text.setToFree(false);
	return 0;
}


int HtmlQLParser::functioHtmlKeyText(char*p, void* call_from, ReferData* tx){ //&html_key_text(key, key, ...)
		//get the main key text from a page
	tx->Set("", 0, true);

	HtmlQLParser* call = (HtmlQLParser* )call_from;

	ReferLink*link1=0,*link2=0, *link3=0;
	//text positions
	ReferLinkHeap text_positions;
	text_positions.setSortOrder(SORT_ORDER_NUM_INC);
	text_positions.setDuplication(false);

	HTQLTagDataSyntax syntax; 
	syntax.setSentence(p, 0, false); 
	while (syntax.Type != QLSyntax::synQL_END){
		if (syntax.Type == HTQLTagDataSyntax::synQL_DATA){
			link1=text_positions.add((char*) 0, (char*) 0, syntax.Start);
			link1->Value.Set(syntax.Sentence+syntax.Start, syntax.StartLen, false); 
		}

		syntax.match();
	}

	//find all repeats
	PHyperBase hyperbase;
	hyperbase.setStopWords("<option></option>");
	hyperbase.parseWordDoc(p, strlen(p), 0, false);


	ReferLink findlink;

	ReferLinkHeap positions;
	positions.setSortOrder(SORT_ORDER_NUM_INC);
	positions.setDuplication(false);
	//find all matching text
	for (link2=call->FunctionParameters; link2; link2=link2->Next){
		if (!link2->Value.L) continue;

		positions.empty();
		char* p0=p;
		char* p1=tStrOp::strNstr(p0, link2->Value.P, false); 
		while (p1){
			//test if is within a text segment
			findlink.Data=p1-p; 
			link1=(ReferLink*) text_positions.moveFirstLarger((char*) &findlink); 
			if (link1 && link1->Data!=findlink.Data) link1=(ReferLink*) text_positions.movePrevious();
			if (link1 && link1->Data <=findlink.Data && link1->Value.L+link1->Data >findlink.Data){
				//is within a text segment
				link1=positions.add(&link2->Value, 0, findlink.Data); 
				link1->Value.Set(p1, link2->Value.L, false);
			}
			p0=p1+link2->Value.L; 
			p1=tStrOp::strNstr(p0, link2->Value.P, false); 
		}


		findKeyText(p, &hyperbase, &positions, tx);
		if (tx->L) break; 
	}

	return 1;
}


int HtmlQLParser::findKeyText(char*p, PHyperBase* hyperbase, ReferLinkHeap* positions, ReferData* tx){
	ReferLinkHeap found_patterns; 
	found_patterns.setSortOrder(SORT_ORDER_NUM_DEC);
	found_patterns.setDuplication(true);

#ifdef _DEBUG
#define _DEBUG_THIS_FUNCTION
#endif

#ifdef _DEBUG_THIS_FUNCTION
	ReferData debug_msg; 
	char debug_buf[256]; 
#endif

	ReferLink findlink;
	ReferLink*link1=0,*link2=0, *link3=0;
	long count=0;
	for (link1=(ReferLink*) hyperbase->EntriesScores.moveFirst() ;link1; link1 = (ReferLink*) hyperbase->EntriesScores.moveNext()){
		PWordBase* base=(PWordBase*) link1->Value.P;
		if (base->BaseType != PWordBase::LINK) continue;
		PWordLink* link=(PWordLink*) base;

#ifdef _DEBUG_THIS_FUNCTION
		debug_msg+="\r\n==========================\r\n";
#endif
		PWordLink* oldlink=0;
		for (PWordLink* cur=link; cur; cur=(PWordLink*) cur->Next){ //check each sequence
			if (!oldlink) oldlink = cur;
			else if (oldlink == cur) break;

#ifdef _DEBUG_THIS_FUNCTION
			debug_msg+="\r\n--------------------------\r\n";
			long debug_len=cur->LastWord->Word.P-cur->CurWord->Word.P+cur->LastWord->Word.L;
			sprintf(debug_buf, "position=%ld, length=%ld, words=%ld, repeat=%ld, score=%ld\r\n", cur->CurWord->Word.P-p, debug_len, cur->Length, link->Count, link->Score); 
			debug_msg+=debug_buf; 
			debug_msg+="word=";
			debug_msg+="\r\n";
			debug_msg.Cat(cur->CurWord->Word.P, debug_len);
#endif
			
			findlink.Data=cur->CurWord->Word.P-p; 
			long to=cur->LastWord->Word.P-p+cur->LastWord->Word.L;
			link2=(ReferLink*) positions->moveFirstLarger((char*) &findlink);
			if (link2 && link2->Data < to){
				link3=found_patterns.add((ReferData*) 0, (ReferData*) 0, cur->Length*link->Count); //pattern score
				link3->Name.Set(link2->Value.P, link2->Value.L, false); //the substr position pointer
				link3->Value.Set((char*) cur, 0, 0); //the PWordLink entry
				count++; 

#ifdef _DEBUG_THIS_FUNCTION
				debug_msg+="\r\n--------FOUND-------\r\n";
				debug_msg+=link2->Value; 
#endif

#ifndef _DEBUG_THIS_FUNCTION
				break;
#endif
			}
		}
		if (count>5) break; 
	}

	PWordLink* link=0;
	link1=(ReferLink*) found_patterns.moveFirst(); 
	if (link1) {
		link=(PWordLink*)link1->Value.P; 
		//word1 is the first tag larger than the Name
		PWord* word1=link->CurWord; 
		long skip_tags=0;
		while (word1->Word.P <= link1->Name.P && word1->SeqNo<=link->LastWord->SeqNo ){
			word1=(PWord*) word1->Next; 
			skip_tags++; 
		}
		long skip_back=0; 
		PWord* word3=(PWord*) word1->Prev; 
		while (word3->Word.P && (word3->Word.P[0]=='<'  || word3->Word.P >= link1->Name.P) && word3->Prev) {
			if (TagOperation::isTag(word3->Word.P, "<table>")) break;
			word3=(PWord*) word3->Prev; 
			skip_back++;
		}
		if (skip_back>=(link->LastWord->SeqNo - link->CurWord->SeqNo-2)) skip_back = 1;
		//word is the first tag before the next Name
		PWord* word2=word1;
		if (link->Next && ((PWordLink*) link->Next)->CurWord->SeqNo > link->CurWord->SeqNo){ //get the segment between keyword and the next link as value
			word2=((PWordLink*) link->Next)->CurWord; 
			for (long i=0; i<skip_tags-skip_back; i++){
				if ( ((PWord*) word2->Next)->SeqNo > word2->SeqNo)
					word2=(PWord*) word2->Next; 
			}
		}else{
			while ( (word2->Word.P <= link1->Name.P || TagOperation::isTag(word2->Word.P) )
				&& ((PWord*)word2->Next)->SeqNo > word2->SeqNo ){

				word2=(PWord*) word2->Next; 
			}
		}
		while (word1!=word2 && word1->Word.L>=2 && word1->Word.P[0]=='<' && word1->Word.P[1]=='/')
			word1=(PWord *) word1->Next; 
		while (word1!=word2 && word2->Word.L>=2 && word2->Word.P[0]=='<' && word2->Word.P[1]!='/')
			word2=(PWord *) word2->Prev; 


		tx->Set(word1->Word.P, word2->Word.P-word1->Word.P+word2->Word.L, true); 
	}

#ifdef _DEBUG_THIS_FUNCTION
	debug_msg.saveFile("$HtmlQLParser_functioHtmlKeyText.txt");
#endif
	return count;
}


int HtmlQLParser::functionRepeatPatterns(char* p, void* call_from, ReferData* tx){ //&repeat_patterns
	tx->Set("", 0, true);

	char tmp[128];
	long count;

	long maximum_repeat=20;
	long pattern_count=0;

	HtmlQLParser* call = (HtmlQLParser* )call_from;
	readHtqlParameterLong(call->FunctionParameters, 1, &maximum_repeat);


	int* scores=0;
	if (call->FindHtqlPageMark){
		scores=new int[call->SourceData.L];
		for (long i=0; i<call->SourceData.L; i++){
			scores[i]=call->FindHtqlPageMark[i]<call->FindHtqlPageMarkLevel;
		}
	}

	PHyperBase hyperbase;
	hyperbase.parseWordDoc(p, strlen(p), scores);


	//BTreeRecord entryrecord(&hyperbase.EntriesScores);
	ReferLink* link1;
	PWordLink* base; //=(PWordLink*) entryrecord.moveFirst();
	tx->Set("<repeat_patterns>\n", strlen("<repeat_patterns>\n"), true);
	for (link1=(ReferLink*) hyperbase.EntriesScores.moveFirst() ;link1; link1 = (ReferLink*) hyperbase.EntriesScores.moveNext()){
		base=(PWordLink*) link1->Value.P;
		if (base->BaseType != PWordBase::LINK) continue;
		PWordLink* link=(PWordLink*) base;
		sprintf(tmp, "<pattern length=%ld repeat=%ld first_from=%ld first_to=%ld first_length=%ld>\n", 
			link->Length, link->Count, 
			link->CurWord->Pos, link->LastWord->Pos+link->LastWord->Word.L, 
			link->LastWord->Pos+link->LastWord->Word.L-link->CurWord->Pos);
		tx->Cat(tmp, strlen(tmp) );

		ReferData pp;
		count=0;
		for (PWord* word=link->CurWord;word; word=(PWord*) word->Next){
			pp.Cat("<word>", strlen("<word>"));
			word->Word.Seperate();
			pp.Cat(word->Word.P, word->Word.L);
			pp.Cat("</word>", strlen("</word>"));
			count++;
			if (word == link->LastWord) break;
		}

		sprintf(tmp, "\t<pattern_words count=%d>\n\t", count);
		tx->Cat(tmp, strlen(tmp));
		tx->Cat(pp.P, pp.L);
		tx->Cat("\n\t</pattern_words>\n", strlen("\n\t</pattern_words>\n") );

		pp.reset();
		count=0;
		PWordLink* oldlink=0;
		for (PWordLink* cur=link; cur; cur=(PWordLink*) cur->Next){
			if (!oldlink) oldlink = cur;
			else if (oldlink == cur) break;
			if (cur->CurWord) {
				sprintf(tmp, "<position from=%ld to=%ld length=%ld>%ld</position>", cur->CurWord->Pos, cur->LastWord->Pos+cur->LastWord->Word.L, cur->LastWord->Pos+cur->LastWord->Word.L-cur->CurWord->Pos, cur->CurWord->Pos);
				pp.Cat(tmp, strlen(tmp));
				count++;
			}
		}
		sprintf(tmp, "\t<positions count=%d>\n\t", count);
		tx->Cat(tmp, strlen(tmp));
		tx->Cat(pp.P, pp.L);
		tx->Cat("\n\t</positions>\n", strlen("\n\t</positions>\n") );

		tx->Cat("</pattern>\n", strlen("</pattern>\n"));
		pattern_count++;
		if (maximum_repeat && pattern_count>=maximum_repeat) break;
	}
	tx->Cat("</repeat_patterns>\n", strlen("</repeat_patterns>\n") );

	if (scores) delete[] scores;

	return 1;
}

int HtmlQLParser::constructNameFromHyperText(const char* text, long max_len, ReferData* name, int max_words){
	ReferData tx, str;
	if (max_len){
		str.Set((char*) text, max_len, true);
		functionTX((char*) str.P, 0, &tx);
	}else
		functionTX((char*) text, 0, &tx);

	int words_count=0;
	long i=0;
	*name = "";
	while (i<tx.L && words_count < max_words){
		while (i<tx.L && !isalpha(tx.P[i])) i++;
		if (!isalpha(tx.P[i])) break;
		int j=0;
		while (isalnum(tx.P[i+j]) && j<20) j++;
		if (j && !name->isNULL() && name->P[0]) *name += "_";
		name->Cat(tx.P+i, j);
		i+=j;
		words_count++;
	}
	return 0;
}

int HtmlQLParser::functionFindRepeatViews(char*p, void* call_from, ReferData* tx){ //best_repeat_views;
	tx->Set("", 0, true);
	
	//read parameters of show_view_num and maximum_views;
	HtmlQLParser* call = (HtmlQLParser* )call_from;
	long show_view_num=1;
	long maximum_views=1;
	readHtqlParameterLong(call->FunctionParameters, 1, &show_view_num);
	readHtqlParameterLong(call->FunctionParameters, 2, &maximum_views);
	if (show_view_num ==0) show_view_num = 1;
	if (maximum_views < show_view_num) maximum_views = show_view_num ;

	ReferLinkHeap path_htqls;
	path_htqls.setSortOrder(SORT_ORDER_NUM_DEC);
	path_htqls.setDuplication(true);
	long count=0;

	long source_offset = call->FunctionCurrentItem->SourceOffset;
	HtmlQL ql1;
	ql1.setSourceData(call->SourceData.P, call->SourceData.L, false);
	//ql1.setPageMark(call->FindHtqlPageMark, call->FindHtqlPageMarkLevel);

	char buf[128];
	long offset;
	ReferLinkHeap long_data;
	ReferData key;
	findDistingishedPlainData(p, &long_data, 128);
	//for data segments with length longer than 128
	for (ReferLink* ts=(ReferLink*) long_data.moveFirst(); ts; ts=(ReferLink*) long_data.moveNext() ){
		offset=atoi(ts->Name.P);
		sprintf(buf, "&offset(%ld) &find_htql(%d) ./'\n'/1-%d ",
			source_offset+offset, HtmlQL::heuBEST_VIEW, 2);
		ql1.setQuery(buf);
		while (!ql1.isEOF()){
			char* htql_expr = ql1.getValue(1);
			if (htql_expr && *htql_expr){
				if (strstr(htql_expr, ":xx")){ //remove the last tag
					int e=strlen(htql_expr);
					while (e>0 && htql_expr[e]!='.') e--;
					if (e>0){
						ReferData expr1;
						expr1.Set(htql_expr, e, true);
						path_htqls.add(&expr1, 0, ts->Data);
						//path_htqls.set(ts->Data, expr1.P, "");
					}
				}else{
					key.Set(htql_expr, strlen(htql_expr), false);
					path_htqls.add(&key, 0, ts->Data);
					//path_htqls.set(ts->Data, htql_expr, "");
				}
			}
			ql1.moveNext();
		}
	}

	//for most repetitive tags
	long p_len=strlen(p); 
	HtmlQL ql;
	ql.setSourceData(p, p_len, false);
	int* repeat_mark=0;
	if (call->FindHtqlPageMark){
		repeat_mark=(int*) malloc(sizeof(int)*p_len); 
		for (long i=0; i<p_len; i++){
			repeat_mark[i]=(i+source_offset<call->SourceData.L)?call->FindHtqlPageMark[i+source_offset]<call->FindHtqlPageMarkLevel:0; 
		}
		ql.setPageMark(repeat_mark, 1);
	}
	sprintf(buf, "&repeat_patterns.<pattern>1-%d {from=:first_from; to=:first_to; first_length=:first_length; length=:length; repeat=:repeat}", 
		maximum_views);
	ql.setQuery(buf);
	while(!ql.isEOF())  
	{
		char * tmp = ql.getValue(1);
		if (tmp && isdigit(tmp[0]) )
		{
			offset=atoi(tmp);
			tmp=ql.getValue(3);
			int first_length=tmp?atoi(tmp):0;
			tmp=ql.getValue(4);
			int length=tmp?atoi(tmp):0;
			tmp=ql.getValue(5);
			int repeat=tmp?atoi(tmp):0;
			int tscore=repeat*first_length/2;

			sprintf(buf, "&offset(%ld) &find_htql(%d) ./'\n'/1-%d ",
				source_offset+offset, HtmlQL::heuBEST_VIEW, 2);
			ql1.setQuery(buf);
			while (!ql1.isEOF()){
				char* htql_expr = ql1.getValue(1);
				if (htql_expr && *htql_expr){
					key.Set(htql_expr, strlen(htql_expr), false);
					path_htqls.add(&key, 0, tscore - count);
					//path_htqls.set( tscore - count, htql_expr, "");
					count++;
				}
				ql1.moveNext();
			}
		}
		ql.moveNext();
	}

	*tx += "<find_repeat_views>";
	constructFindViewResult(&ql1, &path_htqls, tx, show_view_num, maximum_views*2+1);
	*tx += "</find_repeat_views>\n";

	if (repeat_mark){
		free(repeat_mark); 
		repeat_mark=0;
	}
	return 1;
}

int HtmlQLParser::functionFindBestViews(char*p, void* call_from, ReferData* tx){ //&find_best_views(show_num, search_num)
	tx->Set("", 0, true);

	//read parameters of show_view_num and maximum_views;
	long show_view_num=1;
	long maximum_views=1;
	HTQLParser* call = (HTQLParser* )call_from;
	HtmlQLParser* call1 = (HtmlQLParser*) call;
	readHtqlParameterLong(call->FunctionParameters, 1, &show_view_num);
	readHtqlParameterLong(call->FunctionParameters, 2, &maximum_views);
	if (show_view_num ==0) show_view_num = 1;
	if (maximum_views < show_view_num) maximum_views = show_view_num ;

	ReferLinkHeap path_htqls;
	path_htqls.setSortOrder(SORT_ORDER_NUM_DEC);
	path_htqls.setDuplication(true);
	long count=0;

	long source_offset = call->FunctionCurrentItem->SourceOffset;
	HtmlQL ql1;
	ql1.setSourceData(call->SourceData.P, call->SourceData.L, false);
	ql1.setPageMark(call1->FindHtqlPageMark, call1->FindHtqlPageMarkLevel);
	
	char buf[128];
	sprintf(buf, "&offset(%ld) &find_htql(%d) ./'\n'/1-%d ",
		source_offset, HtmlQL::heuBEST_VIEW, maximum_views*2);
	ql1.setQuery(buf);
	ReferData key;
	while (!ql1.isEOF()){
		char* htql_expr = ql1.getValue(1);
		if (htql_expr && *htql_expr){
			key.Set(htql_expr, strlen(htql_expr),false);
			path_htqls.add(&key, 0, 100-count);
			//path_htqls.set(100-count, htql_expr, "");
			count++;
		}
		ql1.moveNext();
	}

	*tx += "<find_best_views>";
	constructFindViewResult(&ql1, &path_htqls, tx, show_view_num, maximum_views*2+1);
	*tx += "</find_best_views>\n";
	return 1;
}

int HtmlQLParser::constructFindViewResult(HTQL* ql, ReferLinkHeap* path_htqls, ReferData* tx, int show_view_num, int maximum_views){
	ReferLinkHeap rank_byitems;
	rank_byitems.setSortOrder(SORT_ORDER_NUM_DEC);
	rank_byitems.setDuplication(true);
	rankPathHtqlsToViews(ql, path_htqls, &rank_byitems, show_view_num, maximum_views);

	char buf[128];
	int count=0;
	for (ReferLink* t=(ReferLink*) rank_byitems.moveFirst(); t; t=(ReferLink*) rank_byitems.moveNext() ){
		count++;
		*tx += "<find_view htql=\"";
		*tx += t->Name;
		*tx += "\" candidate=\"";
		*tx += t->Value;
		sprintf(buf, "%ld", t->Data);
		*tx +="\" view_items=\"";
		*tx += buf;
		*tx += "\"></find_view>\n";
	}
	return count;
}
int HtmlQLParser::constructTagDataSyntaxStr(char* p, char* syntax_buf, int max_len, long* data_score){
	long len = strlen(p);
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(p, &len, false);

	int IsSkip=false;
	int cur_pos=0;
	long dscore=0;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA && !IsSkip){
			syntax_buf[cur_pos]='D';
			dscore+= DataSyntax.StartLen;
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
			if (!tStrOp::strNcmp(DataSyntax.Sentence+DataSyntax.Start, "<Script", 7, false)) {
				IsSkip = true;
			}
			syntax_buf[cur_pos]='t';
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (!tStrOp::strNcmp(DataSyntax.Sentence+DataSyntax.Start, "</Script", 8, false)) {
				IsSkip = false;
			}
			syntax_buf[cur_pos]='e';
		}

		cur_pos++;
		if (cur_pos>=max_len) break;
		DataSyntax.match();
	}
	syntax_buf[cur_pos]=0;
	if (data_score) *data_score = dscore;
	return cur_pos;
}


double HtmlQLParser::computeTuplesSimilarity(HTQL* ql, int field_index1, long* tuples_score){
	int max_comp_tuple=10;
	int max_comp_len=10;

	char** syntax_strs=0;
	long total_tuple = ql->getTuplesCount();
	if (total_tuple > max_comp_tuple) total_tuple = max_comp_tuple;
	syntax_strs = (char**) malloc(sizeof(char*)*total_tuple);
	memset(syntax_strs, 0, sizeof(char*)*total_tuple);
	int i,j;
	for (i=0; i<total_tuple; i++){
		syntax_strs[i] = (char*) malloc(sizeof(char) * (max_comp_len+1));
		memset(syntax_strs[i], 0, sizeof(char) * (max_comp_len+1));
	}

	ql->moveFirst();
	int cur_tuple=0;
	long tscore=0, tscore1=0;
	while (!ql->isEOF()){
		char* p=ql->getValue(field_index1);
		if (!p) {
			ql->moveNext();
			continue;
		}

		if (constructTagDataSyntaxStr(p, syntax_strs[cur_tuple], max_comp_len, &tscore1)>0 ){
			cur_tuple++;
			tscore += tscore1;
		}

		if (cur_tuple >= max_comp_tuple) break;
		ql->moveNext();
	}

	StrAlignment align;
	double cost=0;
	long len=0;
	double similarity=0;
	if (cur_tuple) {
		for (i=0; i<cur_tuple-1; i++){
			for (j=i+1; j<cur_tuple; j++){
				align.CompareStrings(syntax_strs[i], syntax_strs[j], &cost, &len, 0, 0);
				similarity += cost / (double)(2*len);
			}
		}
		if (cur_tuple>1)
			similarity /= (cur_tuple*(cur_tuple-1)/2);
		similarity = 1-similarity;

		tscore = tscore*total_tuple/cur_tuple;
	}else{
		similarity = 1.0;
		tscore = 0;
	}

	if (tuples_score) *tuples_score = tscore;

	for (i=0; i<total_tuple; i++){
		free(syntax_strs[i]);
	}
	free(syntax_strs);

	return similarity;
}

double HtmlQLParser::computeNullTuplesSimilarity(HTQL* ql, long** fieldnullcounts){
	//int max_comp_tuple=10;
	//int max_comp_len=10;

	char** syntax_strs=0;
	long total_tuple = ql->getTuplesCount();
	//if (total_tuple > max_comp_tuple) total_tuple = max_comp_tuple;
	syntax_strs = (char**) malloc(sizeof(char*)*total_tuple);
	memset(syntax_strs, 0, sizeof(char*)*total_tuple);
	int i,j;
	int total_fields=ql->getFieldsCount();
	for (i=0; i<total_tuple; i++){
		syntax_strs[i] = (char*) malloc(sizeof(char) * (total_fields+1));
		memset(syntax_strs[i], 0, sizeof(char) * (total_fields+1));
	}

	long* nullcounts = (long*) malloc(sizeof(long)*total_fields);
	memset(nullcounts, 0, sizeof(long)*total_fields);
	ql->moveFirst();
	int cur_tuple=0;
	long null_count=0;
	while (!ql->isEOF()){
		for (i=1; i<=total_fields; i++){
			char* p=ql->getValue(i);
			if (p && p[0]){
				syntax_strs[cur_tuple][i-1]='D';
			}else{
				syntax_strs[cur_tuple][i-1]='V';
				nullcounts[i-1]++;
				null_count++;
			}
		}
		syntax_strs[cur_tuple][total_fields] = 0;

		cur_tuple++;
		if (cur_tuple >= total_tuple) break;
		ql->moveNext();
	}

	StrAlignment align;
	double cost=0;
	long len=0;
	double similarity=0;
	if (cur_tuple) {
		for (i=0; i<cur_tuple-1; i++){
			for (j=i+1; j<cur_tuple; j++){
				align.CompareStrings(syntax_strs[i], syntax_strs[j], &cost, &len, 0, 0);
				similarity += cost / (double)(2*len);
			}
		}
		if (cur_tuple>1)
			similarity /= (cur_tuple*(cur_tuple-1)/2);
		similarity = 1-similarity;
	}else{
		similarity = 1.0;
	}
	similarity *= 1 - null_count/(double)(total_fields*total_tuple);

	for (i=0; i<total_tuple; i++){
		free(syntax_strs[i]);
	}
	free(syntax_strs);

	if (fieldnullcounts){
		*fieldnullcounts = nullcounts;
	}else{
		free(nullcounts);
	}
	return similarity;
}

long HtmlQLParser::computeNullFieldCounts(HTQL* ql, long** fieldnullcounts){
	int total_fields=ql->getFieldsCount();
	long* nullcounts = (long*) malloc(sizeof(long)*total_fields);
	memset(nullcounts, 0, sizeof(long)*total_fields);

	ql->moveFirst();
	long count=0;
	int i,j;
	while (!ql->isEOF()){
		for (i=1; i<=total_fields; i++){
			char* p=ql->getValue(i);
			j=0;
			if (p && p[0]){
				for (j=0; p[j] && tStrOp::isSpace(p[j]) ; j++);
			}
			if (!p || !p[j]){
				nullcounts[i-1]++;
				count++;
			}
		}

		ql->moveNext();
	}
	*fieldnullcounts = nullcounts;
	return count;
}

int HtmlQLParser::functionTagParent(char* p, void* call_from, ReferData* tx){
//	tx->Set("", 0, true);
	tx->reset();

	HTQLParser* call = (HTQLParser* )call_from;
	ReferData tag_name;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		tag_name.Set(call->FunctionParameters->Value.P, call->FunctionParameters->Value.L, true);
	}
	HTQLTagSelection tag_selection;
	tag_selection.Data.Set(call->SourceData.P, call->SourceData.L,false);
	tag_selection.parseHTMLScope();
	tag_selection.Sentence.Set(tag_name.P, tag_name.L, true);
	tag_selection.parseSentence(); 
	HTQLScope* tag = tag_selection.TagScope;
	HTQLScope* found_tag=0;
	while (tag && tag->SourceOffset < call->FunctionCurrentItem->SourceOffset){
		//locate the enclosed tag from the first tag, keep finding the closest tag;
		if (( !tag_name.L || TagOperation::isTag(tag->S.P, tag_name.P)) //has the same tag name
			&& (!tag->E.P || (unsigned long) (tag->E.P - tag_selection.Data.P) >= call->FunctionCurrentItem->SourceOffset + strlen(p) )  //covering the end tag
			){
			//also check any filter conditions
			if (!tag_selection.Operations){ //no filter conditions
				found_tag = tag;
			}else{ //has filter conditions, check condition; also see HTQLTagSelection::parseData()
				int isResult=true;
				ReferLink* var;
				tag_selection.setVariableValues(tag);
				tExprCalc* expr=0;
				for (ReferLink* op=tag_selection.Operations; op; op=op->Next){
					if (op->Data == HTQLTagSelection::opCONDITION){
						expr = (tExprCalc*) op->Value.P;
						for (var=tag_selection.Vars; var; var=var->Next){
							expr->setField(var->Name.P, var->Value.P);
						}
						expr->calculate();
						if (!expr->getBoolean()){
							isResult=false;
							break;
						}
					}else if (op->Data == HTQLTagSelection::opTRANSFORM){
					}
				}
				//above is copied from HTQLTagSelection::parseData()

				if (isResult){
					found_tag = tag;
				}
			}
		}
		tag = tag->NextTag;
	}	
	if (found_tag) {
		call->FunctionCurrentItem->copyTag(found_tag, false);
		if (!found_tag->E.P){
			call->FunctionCurrentItem->E.Set(call->SourceData.P+call->SourceData.L, 0, false);
		}
		call->FunctionCurrentItem->Data.Set(call->FunctionCurrentItem->S.P, call->FunctionCurrentItem->E.P+call->FunctionCurrentItem->E.L-call->FunctionCurrentItem->S.P, false);
		return 0;
	}
	return 1;

}
int HtmlQLParser::functionTagPrevious(char* p, void* call_from, ReferData* tx){ //&tag_previous
	tx->Set("", 0, true);

	HTQLParser* call = (HTQLParser* )call_from;
	ReferData tag_name;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		tag_name.Set(call->FunctionParameters->Value.P, call->FunctionParameters->Value.L, true);
	}
	HTQLTagSelection tag_selection;
	tag_selection.Data.Set(call->SourceData.P, call->SourceData.L,false);
	tag_selection.parseHTMLScope();
	HTQLScope* tag = tag_selection.TagScope;
	HTQLScope* found_tag=0;
	while (tag && tag->SourceOffset < call->FunctionCurrentItem->SourceOffset){
		//keep finding the tag greater or equal to it;
		tag = tag->NextTag;
	}
	//find the next tag 
	if (tag) tag=tag->PreviousTag; 

	if (!tag_name.isNULL() ){
		while (tag && !TagOperation::isTag(tag->S.P, tag_name.P) ) tag=tag->PreviousTag;
	}
	found_tag = tag;
	if (found_tag) {
		call->FunctionCurrentItem->copyTag(found_tag, false);
		if (!found_tag->E.P){
			call->FunctionCurrentItem->E.Set(call->SourceData.P+call->SourceData.L, 0, false);
		}
		call->FunctionCurrentItem->Data.Set(call->FunctionCurrentItem->S.P, call->FunctionCurrentItem->E.P+call->FunctionCurrentItem->E.L-call->FunctionCurrentItem->S.P, false);
		return 0;
	}
	return 1;
}

int HtmlQLParser::functionTagNext(char* p, void* call_from, ReferData* tx){
	tx->Set("", 0, true);

	HTQLParser* call = (HTQLParser* )call_from;
	ReferData tag_name;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		tag_name.Set(call->FunctionParameters->Value.P, call->FunctionParameters->Value.L, true);
	}
	HTQLTagSelection tag_selection;
	tag_selection.Data.Set(call->SourceData.P, call->SourceData.L,false);
	tag_selection.parseHTMLScope();
	HTQLScope* tag = tag_selection.TagScope;
	HTQLScope* found_tag=0;
	while (tag && tag->SourceOffset < call->FunctionCurrentItem->SourceOffset){
		//keep finding the tag greater or equal to it;
		tag = tag->NextTag;
	}
	//find the next tag 
	if (tag && tag->SourceOffset == call->FunctionCurrentItem->SourceOffset){
		if (tag->EnclosedTag) tag = tag->EnclosedTag;
		else tag = tag->NextTag;
	}
	if (!tag_name.isNULL() ){
		while (tag && !TagOperation::isTag(tag->S.P, tag_name.P) ) tag=tag->NextTag;
	}
	found_tag = tag;
	if (found_tag) {
		call->FunctionCurrentItem->copyTag(found_tag, false);
		if (!found_tag->E.P){
			call->FunctionCurrentItem->E.Set(call->SourceData.P+call->SourceData.L, 0, false);
		}
		call->FunctionCurrentItem->Data.Set(call->FunctionCurrentItem->S.P, call->FunctionCurrentItem->E.P+call->FunctionCurrentItem->E.L-call->FunctionCurrentItem->S.P, false);
		return 0;
	}
	return 1;
}

int HtmlQLParser::dumpParentChildLinear(HTQLScope* tag, const char* filename){
	tStack stack;
	tStack* tt;
	HTQLScope* t;
	while (tag){
		tt=stack.add("", "");
		tt->Data=(long) tag;
		tag=get_parentTag(tag);
	}
	FILE* f=fopen(filename, FILE_WRITE);
	ReferData str1, str2, str3;
	if (!f) return -1;
	for (tt=stack.Next; tt; tt=tt->Next){
		t=(HTQLScope*) tt->Data;
		
		if (t->NextTag){
			str1.Set(t->S.P, t->NextTag->S.P-t->S.P, true);
		}else{
			str1.Set(t->S.P, t->S.L, true);
		}
		tStrOp::replaceInplace(str1.P, "\n","");
		tStrOp::replaceInplace(str1.P, "\r","");
		str2.Set(t->E.P, t->E.L, true);
		if (t->EnclosedTag){
			str3.Set(t->EnclosedTag->S.P, t->EnclosedTag->S.L, true);
		}else{
			str3.Set("", 0, true);
		}
		fprintf(f, "%6ld:->%-6ld ## %6ld:->%-6ld *** %s *** %s @@@ %s\n", t->SerialNo, t->EnclosedTag?t->EnclosedTag->SerialNo:0, t->SourceOffset, t->EnclosedTag?t->EnclosedTag->SourceOffset:0, str1.P, str2.P, str3.P);
	}
	fclose(f);
	return 0;
}

int HtmlQLParser::functionFindFocusHtql(char* p, void* call_from, ReferData* tx){
	tx->Set("", 0, true);

	HTQLParser* call = (HTQLParser* )call_from;
	HTQLTagSelection tag_selection;
	tag_selection.Data.Set(call->SourceData.P, call->SourceData.L,false);
	tag_selection.parseHTMLScope();
	//tag_selection.linkUnclosedTag(call->SourceData.P, call->SourceData.L, tag_selection.TagScope, "P");
	//tag_selection.linkUnclosedTag(call->SourceData.P, call->SourceData.L, tag_selection.TagScope, "TR", "TABLE");
	//tag_selection.linkUnclosedTag(call->SourceData.P, call->SourceData.L, tag_selection.TagScope, "TD", "TR");
	//tag_selection.linkUnclosedTag(call->SourceData.P, call->SourceData.L, tag_selection.TagScope, "TABLE", "TD");
	HTQLScope* tag = tag_selection.TagScope;
	HTQLScope* found_tag=0;
	while (tag && tag->SourceOffset < call->FunctionCurrentItem->SourceOffset){
		//keep finding the closest tag;
		if (!tag->E.P || (unsigned long) (tag->E.P - tag_selection.Data.P) >= call->FunctionCurrentItem->SourceOffset + strlen(p) 
			){
			found_tag = tag;
		}
		tag = tag->NextTag;
	}	
	/*while (tag && tag->SourceOffset < call->FunctionCurrentItem->SourceOffset){
		//keep finding the tag greater or equal to it;
		if (tag->NextTag && tag->NextTag->SourceOffset > call->FunctionCurrentItem->SourceOffset) break;
		if (!tag->NextTag) break;
		tag = tag->NextTag;
	}*/
	//find best child 
	const char* rank_tags[]={"A","INPUT","IMG","SPAN",0};
	for (tag=found_tag; tag && tag->SourceOffset<call->FunctionCurrentItem->SourceOffset; tag=tag->NextTag){
		if ( TagOperation::isTags(tag->S.P, rank_tags)>=0 ){
			found_tag=tag;
		}
	}

	//find tag index
	ReferData tag_name, name, value, value1;
	name="NAME";
	get_tagAttribute(found_tag, &name, &value);
	get_tagName(found_tag, &tag_name);
	long use_name=1;
	if (use_name) {
		use_name=value.L?1:0;
	}
	long index=0;
	for (tag = tag_selection.TagScope
		; tag->SourceOffset <= call->FunctionCurrentItem->SourceOffset
		; tag=tag->NextTag
		){
		if (TagOperation::isTag(tag->S.P, tag_name.P) ){
			if (use_name){
				get_tagAttribute(tag, &name, &value1);
				if (!value.Cmp(&value1, true)){
					index++;
				}
			}else{
				index++;
			}
		}
	}

	*tx="<"; *tx+=tag_name; *tx+=" NORECUR"; 
	if (use_name){
		*tx+=" (Name='"; *tx+=value; *tx+="'";
	}
	char buf[128];
	sprintf(buf, ">%ld", index);
	*tx+=buf;
	return 1;
}
int HtmlQLParser::findHtql(ReferData* page, long offset, ReferLinkHeap* results, long heuristic, long find_option, int* mark, int mark_level){
	HTQLTagSelection tag_selection;
	tag_selection.Data.Set(page->P, page->L,false);
	tag_selection.parseHTMLScope();
	//tag_selection.linkUnclosedTag(page->P, page->L, tag_selection.TagScope, "P");
	//tag_selection.linkUnclosedTag(page->P, page->L, tag_selection.TagScope, "TR", "TABLE");
	//tag_selection.linkUnclosedTag(page->P, page->L, tag_selection.TagScope, "TD", "TR");
	//tag_selection.linkUnclosedTag(page->P, page->L, tag_selection.TagScope, "TABLE", "TD");
	HTQLScope* tag = tag_selection.TagScope;
	HTQLScope* found_tag=0;
	while (tag && tag->SourceOffset < (unsigned long) offset){
		//keep finding the tag greater or equal to it;
		if (tag->NextTag && tag->NextTag->SourceOffset > (unsigned long) offset) break;
		if (!tag->NextTag) break;
		tag = tag->NextTag;
	}

#ifdef DEBUG_THIS_FILE
	dumpParentChildLinear(tag, "$find_htql.txt");
#endif

	ReferLinkHeap results0;
	ReferData suffix;
	if (tag && TagOperation::isTag(tag->S.P) ){
		if (tag->SourceOffset+tag->S.L < (unsigned long) offset){
			if (tag->S.P[1]=='/')
				suffix = ":xx";
			else if (tag->S.P[1]!='/' && tag->NextTag && tag->NextTag->S.P[1]!='/'){
				if (find_option & LEAVE_SINGLE_ITEM){
					suffix = ":fx";
				}
			}
		}
		findTagHtql(tag_selection.TagScope, tag, &results0, suffix.P, false, heuristic, mark, mark_level);
		filterHtqlResults(&results0, results, heuristic, "");
	}
	return 0;
}
int HtmlQLParser::functionGetEmail(char*p, void* call_from, ReferData* tx){ //&get_email(fmt) 
	tx->Set("", 0, true);
	HTQLParser* call = (HTQLParser* )call_from;
	ReferData fmt;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		fmt.Set(call->FunctionParameters->Value.P, call->FunctionParameters->Value.L, true);
	}
	tStrOp::GetEmails(p, fmt.P, tx); 

	return 1;
}
int HtmlQLParser::functionTextWords(char*p, void* call_from, ReferData* tx){ //&text_words(type, option) 
	tx->Set("", 0, true);
	HTQLParser* call = (HTQLParser* )call_from;
	ReferData type, option;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		type.Set(call->FunctionParameters->Value.P, call->FunctionParameters->Value.L, true);
	}
	if (call->FunctionParameters && call->FunctionParameters->Next && call->FunctionParameters->Next->Value.P ){
		option.Set(call->FunctionParameters->Next->Value.P, call->FunctionParameters->Next->Value.L, true);
	}

	PDocBase::getTextWords(p, type.P, option.P, tx); 

	return 1;
}

int HtmlQLParser::functionFindHtql(char* p, void* call_from, ReferData* tx){
	tx->Set("", 0, true);

	HTQLParser* call = (HTQLParser* )call_from;
	HtmlQLParser* call1=(HtmlQLParser*) call;
	long heuristic = HtmlQL::heuBEST;
	long find_option = LEAVE_SINGLE_ITEM;
	readHtqlParameterLong(call->FunctionParameters, 1, &heuristic);
	readHtqlParameterLong(call->FunctionParameters, 2, &find_option);

	ReferLinkHeap sort_result;
	findHtql(&call->SourceData, call->FunctionCurrentItem->SourceOffset, &sort_result, heuristic, find_option, call1->FindHtqlPageMark, call1->FindHtqlPageMarkLevel);

	tx->Set("",0,true);
	long count=0;
	for (ReferLink* st = (ReferLink*) sort_result.moveFirst(); !sort_result.isEOF(); st =(ReferLink*) sort_result.moveNext() ){
		st->Name.replaceStr("\n", "\\n");
		*tx+=st->Name;
		*tx+="\n";
		if (++count>100) break; 
	}
	return 1;
}

/* =========USE this script to find a tag from an HTML document  ==========
//set onClick in the <BODY onClick="getClick()"> tag 
<script>
function getClick(){
	var a = document.activeElement;
	var b;
	var s = "";
	var sl = document.selection;
	var r = sl.createRange();
	var tx = r.text;
	if (tx != "") a=r.parentElement();
	var n=a.sourceIndex;

	var t = a.tagName;
	if (t == "TBODY") t="TABLE";
	var d = document.all.tags(t);
	var i=0;
	var n1 = d.item(i).sourceIndex; 
	while (n1 < n){
		i++;
		if (d.item(i) == null) break;
		n1=d.item(i).sourceIndex;
	}
	if (n1==n){
		i++;
		s+= "<"+t+" norecur>"+i;
	}else{
		s+= "<"+t+" norecur>"+i;
	}
	alert(s);
}
</script>
*/

int HtmlQLParser::functionPlainLeavesHtql(char*p, void* call_from, ReferData* tx){ //&plain_leaves_htql
	tx->Set("", 0, true);

	char buf[128];

	HTQLParser* call = (HTQLParser* )call_from;

	*tx+="<leaves_htql>";
	ReferData str;

	HTQLScope* tag_scope=0;
	int count=1000;
	searchPlainPairScope(&call->FunctionCurrentItem->Data, &tag_scope);
	for (HTQLScope* scope = tag_scope; scope; scope=scope->NextTag){
		if (!scope->S.isNULL() && !scope->E.isNULL()){
			*tx+= "<leave_htql name=\"";
			if (scope->S.L>0 && isalpha(scope->S.P[scope->S.L-1])){
				tx->Cat(scope->S.P, scope->S.L);
			}else if (scope->S.L>0) {
				tx->Cat(scope->S.P, scope->S.L-1);
			}
			*tx+="\" htql=\"/'";
			tx->Cat(scope->S.P, scope->S.L);
			if (!scope->E.isNULL()){
				*tx+="'~'";
				str.reset();
				for (long i=0; i<scope->E.L; i++){
					switch (scope->E.P[i]){
					case '\r': str.Cat("\\r", 2);
						break;
					case '\n': str.Cat("\\n", 2);
						break;
					default: 
						if (scope->E.P[i]!='\'' && scope->E.P[i]!='>') 
							str.Cat(scope->E.P+i, 1);
						break;
					}
				}
				tx->Cat(str.P, str.L);
				//tx.Cat(scope->E.P, scope->E.L);
				sprintf(buf, "'/%d\" order=\"%d\"></leave_htql>\n", scope->SerialNo, ++count);
			}else{
				sprintf(buf, "''~/%d\" order=\"%d\"></leave_htql>\n", scope->SerialNo, ++count);
			}
			*tx+=buf;
		}
	}
	if (tag_scope) delete tag_scope;

	*tx+="</leaves_htql>";
	return 1;
}

int HtmlQLParser::functionTagLeaves(char*p, void* call_from, ReferData* tx){ //&tag_leaves
	tx->Set("", 0, true);
	HTQLScope* tag;
	char buf[256];

	HTQLParser* call = (HTQLParser* )call_from;
	HTQLTagSelection tag_selection;
	tag_selection.Data.Set(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	tag_selection.parseHTMLScope();
	*tx="<tag_leaves>";
	
	long offset=call->FunctionCurrentItem->SourceOffset;
	ReferData tag_name;

	for (tag = tag_selection.TagScope; tag; tag = tag->NextTag){
		if (tag->EnclosedTag && tag->EnclosedTag == tag->NextTag){
			get_tagName(tag, &tag_name);
			sprintf(buf, "<tag_leave name='%s' tag_s_from='%ld' tag_s_len='%ld'; tag_e_from='%ld' tag_e_len='%ld'></tag_leave>\n",
				tag_name.P, tag->SourceOffset+offset, tag->S.L, tag->EnclosedTag->SourceOffset + offset, tag->EnclosedTag->E.L);
			*tx+=buf;
		}
	}

	*tx+="</tag_leaves>";
	return 1;
}

int HtmlQLParser::functionFindLeavesHtql(char*p, void* call_from, ReferData* tx){ //&leaves_htql
	tx->Set("",0,true);

	HTQLParser* call = (HTQLParser* )call_from;
	long leave_options = LEAVE_SINGLE_ITEM;
	readHtqlParameterLong(call->FunctionParameters, 1, &leave_options);
	char buf[256];

	*tx="<leaves_htql>";
	HtmlQL ql;
	ql.setSourceData(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	sprintf(buf,"&tag_leaves_htql(%ld) .<leave_htql>{name=:name; htql=:htql; order=:order}", leave_options);
	ql.setQuery(buf);
	HtmlQL tmpql;
	ReferData str;
	
	ReferLinkHeap all_leaves;
	all_leaves.setCaseSensitivity(true);
	all_leaves.setSortOrder(SORT_ORDER_VAL_STR_INC);
	int count=0;
//	tmpql.setSourceData(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	for (;!ql.isEOF(); ql.moveNext()){
		char* p1=ql.getValue(1);
		char* p2=ql.getValue(2);
		if (!p2 || !*p2) continue;
		if (all_leaves.findValue(p2)) continue;
		//if (all_leaves.search(p2,0)) continue;
		all_leaves.add(0, p2, count);
		//all_leaves.set(0, p2, " ");

		*tx+="<leave_htql name=\"";
		if (p1) *tx+=p1;
		*tx+="\" htql=\"";
		if (p2[0]=='/') *tx+=".";
		*tx += p2;
		char* p3=strrchr(p2, '<');
		if (p3 && !strchr(p3, ':')){
			if (TagOperation::isTag(p3, "TD") ) tx->Cat(":tx", 3);
			else if (TagOperation::isTag(p3, "BR") ) tx->Cat(":tx", 3);
			else if (TagOperation::isTag(p3, "TR") ) tx->Cat("&tx", 3);
			else if (TagOperation::isTag(p3, "INPUT") ) tx->Cat(":st", 3);
		}
		p3=ql.getValue(3); 
		*tx+="\" order=\""; *tx+=p3;
		*tx+="\"></leave_htql>\n";
		count++;
	}
	
	if (count <40){
		//ql.setQuery("&tx &plain_leaves_htql .<leave_htql>{name=:name; htql=:htql}");
		ql.setQuery("&plain_leaves_htql .<leave_htql>{name=:name; htql=:htql}");
		for (;!ql.isEOF(); ql.moveNext()){
			char* p1=ql.getValue(1);
			char* p2=ql.getValue(2);
			char* p3=ql.getValue(3); 
			if (!p2 || !*p2) continue;
			if (all_leaves.findValue(p2)) continue;
			//if (all_leaves.search(p2,0)) continue;
			all_leaves.add(0, p2, count);
			//all_leaves.set(0, p2, " ");

			*tx+="<leave_htql name=\"";
			if (p1) *tx+=p1;
			//*tx+="\" htql=\"&tx. ";
			*tx+="\" htql=\". ";
			*tx+=p2;
			p3=ql.getValue(3); 
			*tx+="\" order=\""; *tx+=p3;
			*tx+="\"></leave_htql>\n";
			count++;
			if (count>45) break;
		}
	}

	*tx+="</leaves_htql>";
	return 1;
}

int HtmlQLParser::functionTagLeavesHtql(char*p, void* call_from, ReferData* tx){ //&tag_leaves_htql
	tx->Set("",0,true);
	HTQLScope* tag;

	HTQLParser* call = (HTQLParser* )call_from;

	long leave_options = LEAVE_SINGLE_ITEM;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		sscanf(call->FunctionParameters->Value.P, "%ld", &leave_options);
	}	
	
	if (call->FunctionCurrentItem->Data.L<1) return 1;
	*tx="<leaves_htql>";

	HTQLTagSelection tag_selection;
	//tag_selection.Data.Set(call->FunctionCurrentItem->Data.P+1, call->FunctionCurrentItem->Data.L-1, false);
	tag_selection.Data.Set(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	tag_selection.parseHTMLScope();
	
	HtmlQL ql;
	ql.setSourceData(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	//ql.setSourceData(call->FunctionCurrentItem->Data.P+1, call->FunctionCurrentItem->Data.L-1, false);
	ql.setSourceUrl(call->SourceUrl.P, call->SourceUrl.L);
	int count=0;
	char *p1, * p2;
	char buf[256];
	ReferLinkHeap all_leaves;
	all_leaves.setCaseSensitivity(true);
	all_leaves.setSortOrder(SORT_ORDER_VAL_STR_INC);
	ReferData name;
	for (tag = tag_selection.TagScope; tag; tag = tag->NextTag){
		//if (tag->NextTag && tag->NextTag->EnclosedTag && (!tag->EnclosedTag || tag->EnclosedTag==tag->NextTag) ){
		if (tag->NextTag && tag->NextTag&&tag->NextTag->SourceOffset>0) {
			for (p2=tag->S.P+tag->S.L; tStrOp::isSpace(*p2) && p2<tag->NextTag->S.P; p2++);
			if (!tag->EnclosedTag || tag->EnclosedTag==tag->NextTag 
				|| TagOperation::isTag(tag->S.P, "A")
				|| (p2<tag->NextTag->S.P && !tStrOp::isSpace(*p2) ) 
				){
				if (count++ > 30) break; //controled in functionFindLeavesHtql
				sprintf(buf, "&offset(%ld) &find_htql(%d, %ld) ./'\n'/", tag->NextTag->SourceOffset-1, HtmlQL::heuBEST_LEAVE, leave_options);
				ql.setQuery(buf);
				p1=ql.getValue(1);
				if (p1 && p1[0] && all_leaves.add(0, p1, count)){
					*tx += "<leave_htql name=\"";
					*tx += name;
					*tx += "\" htql=\"";
					*tx +=p1;
					sprintf(buf, "\" order=\"%d", count);
					*tx +=buf;
					*tx += "\">\n";
				}
				
				sprintf(buf, "&offset(%ld) &find_htql(%d, %ld) ./'\n'/", tag->NextTag->SourceOffset-1, HtmlQL::heuNO_CLASS, leave_options);
				ql.setQuery(buf);
				p1=ql.getValue(1);
				if (p1 && p1[0] && all_leaves.add(0, p1, count)){
					*tx += "<leave_htql name=\"";
					*tx += name;
					*tx += "\" htql=\"";
					*tx +=p1;
					sprintf(buf, "\" order=\"%d", count);
					*tx +=buf;
					*tx += "\">\n";
				}
				
			}
		}
	}
	*tx+="</leaves_htql>";

	return 1;
}
int HtmlQLParser::functionKeyLeavesHtql(char*p, void* call_from, ReferData* tx){ //&key_leaves_htql
	tx->Set("",0,true);
	HTQLScope* tag;
	char buf[256];

	HTQLParser* call = (HTQLParser* )call_from;
	if (call->FunctionCurrentItem->Data.L<1) return 1;
	*tx="<leaves_htql>";

	HTQLTagSelection tag_selection;
	//tag_selection.Data.Set(call->FunctionCurrentItem->Data.P+1, call->FunctionCurrentItem->Data.L-1, false);
	tag_selection.Data.Set(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	tag_selection.parseHTMLScope();
	
	HtmlQL ql;
	ql.setSourceData(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	//ql.setSourceData(call->FunctionCurrentItem->Data.P+1, call->FunctionCurrentItem->Data.L-1, false);
	ql.setSourceUrl(call->SourceUrl.P, call->SourceUrl.L);
	int count=0;
	ReferData str, text;
	for (tag = tag_selection.TagScope; tag; tag = tag->NextTag){
		ReferData name;
		int offset=0;

		if (TagOperation::isTag(tag->S.P, "TD") &&
			tag->PreviousTag && tag->PreviousTag->EnclosedTag && tag->EnclosedTag && 
			TagOperation::isTag(tag->PreviousTag->S.P, "TR")
			){
			//<TR><TD>key:</TD>  <TD>***</TD> </TR>
			text.Set(tag->S.P+tag->S.L, tag->E.P?(tag->E.P-(tag->S.P+tag->S.L)):0, true);
			functionTX(text.P,0, &str);
			if (strchr(str.P, ':')){
				long i=0;
				while (i<str.L && str.P[i]!=':' && !isalpha(str.P[i])) i++;
				if (isalpha(str.P[i])){
					constructNameFromHyperText(str.P+i,0, &name, 3);

					//find the TD with max length;
					long max_len=0;
					i=0;
					for (HTQLScope* tag1=tag->NextTag; tag1 && tag1->SourceOffset<tag->PreviousTag->EnclosedTag->SourceOffset; tag1=tag1->NextTag){
						if (TagOperation::isTag(tag1->S.P, "TD") && tag1->EnclosedTag){
							if (tag1->EnclosedTag->S.P-(tag1->S.P+tag1->S.L) > max_len){
								max_len=tag1->EnclosedTag->S.P-(tag1->S.P+tag1->S.L);
								i=tag1->SourceOffset + tag1->S.L;
							}
						}
					}

					if (max_len && i){
						offset = i;
						tag=tag->EnclosedTag;
					}
				}//end of if (isalpha(str.P[i]))
			} //end of if (strchr(str.P, ':'))
		}else if (TagOperation::isTag(tag->S.P, "B") && tag->EnclosedTag ){
			long i;

			i=tag->EnclosedTag->SourceOffset-1;
			if (':'==TagOperation::firstNonTagCharBefore(tag_selection.Data.P, i, tag->SourceOffset+tag->S.L, &i)){
				//<B>key: </B>
				i= tag->EnclosedTag->SourceOffset + tag->EnclosedTag->S.L;
				if (TagOperation::firstNonTagCharAfter(tag_selection.Data.P, i, tag_selection.Data.L, &i)){
					offset = i;
					constructNameFromHyperText(tag->S.P+tag->S.L, tag->EnclosedTag->S.P-(tag->S.P+tag->S.L), &name, 3);
				}
			}
			if (!offset){
				// ***:<B>text</B>
				i=tag->SourceOffset;
				if (i>0) {
					i--;
					if (':'==TagOperation::firstNonTagCharBefore(tag_selection.Data.P, i, 0, &i)){
						offset = tag->SourceOffset + tag->S.L;
						HTQLScope* tag1=tag;
						while (tag1 && tag1->SourceOffset >=(unsigned long) i) tag1=tag1->PreviousTag;
						if (tag1){
							constructNameFromHyperText(tag1->S.P+tag1->S.L, i-(tag1->SourceOffset+tag1->S.L), &name, 3);
						}
					}
				}
			}
			if (!offset){
				//<B>key</B>: 
				i=tag->EnclosedTag->SourceOffset + tag->EnclosedTag->S.L;
				if (':'==TagOperation::firstNonTagCharAfter(tag_selection.Data.P, i, tag_selection.Data.L, &i)){
					offset = i+1;
					constructNameFromHyperText(tag->S.P+tag->S.L, tag->EnclosedTag->S.P-(tag->S.P+tag->S.L), &name, 3);
				}
			}

		}

		if (offset){
			while (tStrOp::isSpace(tag_selection.Data.P[offset])) 
				offset ++;
			sprintf(buf, "&offset(%ld) &find_htql(%d) ./'\n'/", offset, HtmlQL::heuBEST_LEAVE);
			ql.setQuery(buf);
			if (!ql.isEOF()){
				char* p1=ql.getValue(1);
				if (p1){
					*tx += "<leave_htql name=\"";
					*tx += name;
					*tx += "\" htql=\"";
					*tx +=p1;
					char* p3=strrchr(p1, '<');
					if (p3 && !strchr(p3, ':')){
						if (TagOperation::isTag(p3, "TD")) tx->Cat(":tx", 3);
						else if (TagOperation::isTag(p3, "BR")) tx->Cat(":tx", 3);
						else if (TagOperation::isTag(p3, "TR")) tx->Cat("&tx", 3);
					}
					*tx += "\">\n";
				}
			}
		}
	}
	*tx+="</leaves_htql>";

	return 1;
}

int HtmlQLParser::functionLeavesHtqlSchema(char* p, void* call_from, ReferData* tx){ //&leaves_htql_schema
	tx->Set("", 0, true);

	ReferData prefix;
	HTQLParser* call = (HTQLParser* )call_from;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		prefix.Set(call->FunctionParameters->Value.P, call->FunctionParameters->Value.L, true);
	}

	HtmlQL ql;
	ql.setSourceData(p, strlen(p), false);
	ql.setQuery("<leave_htql>:htql, name");

	ReferData schema;
	tStack exprs;
	exprs.Type = tStack::ordINCKEY;

	schema = "{\n";
	int col=1;
	char buf[20];
	while(!ql.isEOF()) 
	{
		sprintf(buf, "%d",col);
		char* expr = ql.getValue(1);
		char* name = ql.getValue(2);
		if (!expr || !expr[0] ) {
			ql.moveNext();
			continue;
		}
		exprs.set(0, expr, "");

		if (name &&*name){
			schema += name;
			schema += "=" ;
			schema += expr;
			schema += ";\n";
		}else{
			schema += "COLUMN" ;
			schema += buf;
			schema += "=" ;
			schema += expr;
			schema += ";\n";
		}
		col++;

		ql.moveNext();
	}

	schema += "}";
	*tx = prefix;
	*tx += schema;
	return 1;
}

int HtmlQLParser::functionTagAttrs(char*p, void* call_from, ReferData* tx){
	tx->Set("", 0, true);

	HTQLParser* call = (HTQLParser* )call_from;

	ReferData var;
	long len=0;
	char* p1=TagOperation::getLabel(p, (unsigned int*) &len);
	tx->Cat("<tag_attrs tag_name='", strlen("<tag_attrs tag_name='") );
	tx->Cat(p1, len);
	tx->Cat("'>\n", strlen("'>\n") );

	HTQLAttrDataSyntax AttrSyn;
	len=strlen(p);
	AttrSyn.setSentence(p, &len, false);

	while (AttrSyn.Type != QLSyntax::synQL_UNKNOW &&
			AttrSyn.Type != QLSyntax::synQL_RTAG &&
			AttrSyn.Type != QLSyntax::synQL_END 
		){
		while (AttrSyn.Type != QLSyntax::synQL_UNKNOW &&
			AttrSyn.Type != QLSyntax::synQL_END && 
			AttrSyn.Type != QLSyntax::synQL_WORD && AttrSyn.NextType != QLSyntax::synQL_EQ
			){
			AttrSyn.match();
		}
		if (AttrSyn.Type == QLSyntax::synQL_WORD && AttrSyn.NextType == QLSyntax::synQL_EQ){
			var="<tag_attr name='";
			var.Cat(AttrSyn.Sentence + AttrSyn.Start, AttrSyn.StartLen);
			var+="' value=";

			AttrSyn.match();
			AttrSyn.match();// synQL_EQ;
			var.Cat(AttrSyn.Sentence+AttrSyn.Start, AttrSyn.StartLen);
			var+="></tag_attr>\n";
		}
		tx->Cat(var.P, var.L);
		AttrSyn.match();
	}
	tx->Cat("</tag_attrs>", strlen("</tag_attrs>") );
	return 1;
}

int HtmlQLParser::functionFormInputs(char* p, void* call_from, ReferData* tx){
	tx->Set("", 0, true);

	// call the &tag_parent('FORM'); set the parameters first;
	HTQLParser* call = (HTQLParser* )call_from;
	HTQLTagSelection tag_selection;
	if (!TagOperation::isTag(p, "form")){
		call->resetHtqlFunctionParameters();

		call->FunctionParameters = new ReferLink;
		call->FunctionParameters->Value.Set("form", 4, 1);
		
		//call the &tag_parent; get the content of the form
		ReferData text;
		functionTagParent(p, call_from, &text);
		//parse each tag in the form to retrieve attributes and recontruct a new html content;
		//tag_selection.Data.Set(call->SourceData.P, call->SourceData.L,false);
	}else{
		//tag_selection.Data.Set(p, strlen(p), false);
	}
	tag_selection.Data.Set(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	tag_selection.parseHTMLScope();

	HtmlQL form_tag;
	char buf[1024];
	int i;
	HTQLScope* tag;
	HTQLScope* last_input=tag_selection.TagScope;
	int is_new=0;

	form_tag.setSourceData(call->FunctionCurrentItem->Data.P, call->FunctionCurrentItem->Data.L, false);
	form_tag.setSourceUrl(call->CurrentSourceUrl.P, call->CurrentSourceUrl.L);
	sprintf(buf, "<form>1{action=:action &url; method=:method}");
	i=form_tag.setQuery(buf);
	if (!form_tag.isEOF()){
		*tx+="<form_inputs action=\"";
		char* val=form_tag.getValue(1);
		if (val) *tx+=val;
		*tx+="\" method=\"";
		val=form_tag.getValue(2);
		ReferData method;
		if (val) method.Set(val, strlen(val), false);
		if (!method.Cmp("POST", 4, false)) *tx+="POST";
		else *tx+="GET";
		*tx+="\">\n";

	}else{
		tx->Cat("<form_inputs>\n", strlen("<form_inputs>\n") );
	}

	ReferLinkHeap radio_names;
	ReferLink* link=0;
	int is_malloc=0;
	for (tag = tag_selection.TagScope; tag; tag = tag->NextTag){
		form_tag.reset();
		if (TagOperation::isTag(tag->S.P, "input")) {
			sprintf(buf, "<input>1{type=:type; name=:name; id=:id; value=:value; checked=/'checked'~ INCL INSEN/:tn | name is not null } ");
			form_tag.setSourceData(tag->S.P, tag->S.L, false);
			i=form_tag.setQuery(buf);
			if (!form_tag.isEOF()){
				char* type=form_tag.getValue("type");
				char* checked = form_tag.getValue("checked");
				ReferData radio;
				radio.Set("RADIO", 5, true);
				ReferData checkbox;
				checkbox.Set("CHECKBOX", 8, true);
				int is_val=true;
				int check_radio=false;
				if (type && (radio.Cmp(type, 5, false)==0 || checkbox.Cmp(type, 8, false)==0)){
					if (!checked || !*checked) is_val=false;
					check_radio = true;
				}
				char* name=form_tag.getValue("name");
				char* id=form_tag.getValue("id");
				char* value=tStrOp::quoteText(form_tag.getValue("value"), &is_malloc);
				link=radio_names.findName(name);
				if (is_val){
					if (check_radio && !value) value="on";
					*tx+="<form_input tag='input' name='";
					*tx+=name;
					*tx+="' id='";
					*tx+=id;
					*tx+="' value=\"";
					*tx+=value;
					*tx+="\" type='";
					*tx+=type;
					sprintf(buf, "' from='%ld' to='%ld'>", call->FunctionCurrentItem->SourceOffset+tag->SourceOffset, call->FunctionCurrentItem->SourceOffset+tag->SourceOffset+tag->S.L);
					*tx+=buf;
					is_new = 1;

					if (link){
						link->Data=-1;
					}
				}else{
					if (!link){
						link=radio_names.add(name, type, tag->SourceOffset);
					}
				}
				if (is_malloc) free(value);
			}
		}else if (TagOperation::isTag(tag->S.P, "select")) {
			//sprintf(buf, "<select>1{ name=:name; id=:id; value=<option>:st./'selected'~  INSEN/:tn &tag_parent('option').<option>1{val=:value; tx=&txstr./'\n'/1 || res=val/&tx} |name is not null}");
			ReferData name, value, id;
			form_tag.setSourceData(tag->S.P, call->FunctionCurrentItem->Data.P + call->FunctionCurrentItem->Data.L - tag->S.P, false);
			i=form_tag.setQuery("<select>1: name, id");
			if (!form_tag.isEOF()){
				name = form_tag.getValue("name");
				id = form_tag.getValue("id");
				i=form_tag.setQuery("<select>1.<option>:st./'selected'~  INSEN/:tn &tag_parent('option').<option>1");
				if (!form_tag.isEOF()){
					form_tag.dotQuery("{val=:value; tx=&txstr || res=val/&tx}");
				}else{
					form_tag.setQuery("<select>1.<option>1{val=:value; tx=&txstr || res=val/&tx}");
				}
				for (; !form_tag.isEOF(); form_tag.moveNext() ){
					if (value.L) value+=" ";
					value+=form_tag.getValue(1);
				}
				is_new = 1;

				//sprintf(buf, "<select>1.<option> {val=:value; tx=&txstr || res=val/&tx } ");
				//form_tag.setSourceData(tag->S.P, call->FunctionCurrentItem->Data.P + call->FunctionCurrentItem->Data.L - tag->S.P, false);
				i=form_tag.setQuery("<select>1.<option> {val=:value; tx=&txstr || res=val/&tx } ");
				if (!form_tag.isEOF() && !value.L) {
					value = form_tag.getValue(1);
				}

				//sprintf(buf, "<form_input tag='select' name='%s' value=\"",
				//	name.P?name.P:""
				//	);
				//*tx+=buf;
				*tx+="<form_input tag='select' name='";
				*tx+=name; 
				*tx+="' id='"; 
				*tx+=id; 
				*tx+="' value=\""; 
				*tx+=value;
				*tx+="\" type='select";
				sprintf(buf, "' from='%ld' to='%ld'>", call->FunctionCurrentItem->SourceOffset+tag->SourceOffset, call->FunctionCurrentItem->SourceOffset+tag->SourceOffset+(tag->E.P-tag->S.P + tag->E.L));
				*tx+=buf;

				while (!form_tag.isEOF()){
					*tx+="<option value='";
					*tx+=form_tag.getValue(1);
					*tx+="'/>\n";
					form_tag.moveNext();
				}
			}
		}else if (TagOperation::isTag(tag->S.P, "textarea")) {
			sprintf(buf, "<textarea>1{ name=:name; id=:id; value=:tx |name is not null}");
			form_tag.setSourceData(tag->S.P, call->FunctionCurrentItem->Data.P + call->FunctionCurrentItem->Data.L - tag->S.P, false);
			i=form_tag.setQuery(buf);
			if (!form_tag.isEOF()){
				//sprintf(buf, "<form_input tag='textarea' name='%s' value=\"",
				//	form_tag.getValue("name")?form_tag.getValue("name"):""
				//	);
				//*tx+=buf;
				*tx+="<form_input tag='textarea' name='";
				*tx+=form_tag.getValue("name")?form_tag.getValue("name"):""; 
				*tx+="' id='"; 
				*tx+=form_tag.getValue("id")?form_tag.getValue("id"):""; 
				*tx+="' value=\""; 
				char* value=tStrOp::quoteText(form_tag.getValue("value"), &is_malloc);
				*tx+=value;
				if (is_malloc) free(value);
				*tx+="\" type='textarea";
				sprintf(buf, "' from='%ld' to='%ld'>", call->FunctionCurrentItem->SourceOffset+tag->SourceOffset, call->FunctionCurrentItem->SourceOffset+tag->SourceOffset+(tag->E.P-tag->S.P + tag->E.L));
				*tx+=buf;
				is_new = 1;
			}
		}
		if (is_new){
			ReferData description;
			ReferData desc, text;
			char* des=0;
			HTQLScope* desc_from=tag->PreviousTag, * desc_to=tag->NextTag;
			while (desc_from && description.L < 3 ){
				if (tStrOp::strNcmp(desc_from->S.P, "</script>", 9, false)==0){
					desc_from = desc_from->EnclosedTag;
					if (desc_from) desc_from = desc_from->PreviousTag;
					continue;
				}
				if (tStrOp::strNcmp(desc_from->S.P, "<script", 7, false)==0){
					desc_from = desc_from->PreviousTag;
					continue;
				}
				text.Set(desc_from->S.P, desc_from->NextTag->S.P - desc_from->S.P, true);
				desc.reset();
				functionTxStr(text.P, 0, &desc);				
				description+=desc;
				desc_from = desc_from->PreviousTag;
			}
			if (description.L) {
				tStrOp::replaceInplace(description.P, "*", "-");
				tx->Cat(description.P, description.L);
			}
			tx->Cat(" * ", 3);

			des=0;
			description.reset();
			while (desc_to && description.L < 3){
				if (tStrOp::strNcmp(desc_to->S.P, "<script", 7, false)==0){
					desc_to = desc_to->EnclosedTag;
					if (desc_to) desc_to=desc_to->NextTag;
					continue;
				}
				if (tStrOp::strNcmp(desc_to->S.P, "</script>", 9, false)==0){
					desc_to = desc_to->NextTag;
					continue;
				}
				text.Set(desc_to->PreviousTag->S.P, desc_to->S.P - desc_to->PreviousTag->S.P, true);
				desc.reset();
				functionTxStr(text.P, 0, &desc);
				description+=desc;
				desc_to = desc_to->NextTag;
			}
			if (description.L) {
				tStrOp::replaceInplace(description.P, "*", "-");
				tx->Cat(description.P, description.L);
			}

			sprintf(buf, "</form_input>\n");
			tx->Cat(buf, strlen(buf));

			last_input = tag;
			is_new=0;
		}
	}
	for (link=radio_names.getReferLinkHead(); link; link=link->Next){
		if (link->Data>=0){
			*tx+="<form_input tag='input' name='";
			*tx+=link->Name;
			*tx+="' value=\"\" type='";
			*tx+=link->Value;
			sprintf(buf, "' from='%ld' to='%ld'>", call->FunctionCurrentItem->SourceOffset+link->Data, call->FunctionCurrentItem->SourceOffset+link->Data+link->Name.L);
			*tx+=buf;
			*tx+=link->Value; *tx+=": "; 
			*tx+=link->Name; 
			*tx+="</form_input>\n"; 
		}
	}
	tx->Cat("</form_inputs>", strlen("</form_inputs>") );
	return 1;
}

int HtmlQLParser::functionOffset(char* p, void* call_from, ReferData* tx){
	tx->Set("", 0, true);

	long position=0, offset=0;
	HTQLParser* call = (HTQLParser* )call_from;
	if (p && tStrOp::isDigit(*p)){
		sscanf(p, "%ld", &position);
	}
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		sscanf(call->FunctionParameters->Value.P, "%ld", &position);
		if (call->FunctionParameters->Next && call->FunctionParameters->Next->Value.P){
			sscanf( call->FunctionParameters->Next->Value.P, "%ld", &offset);
		}
	}
	if (position < call->SourceData.L) {
		call->FunctionCurrentItem->S.Set(call->SourceData.P + position, 0, false);
		call->FunctionCurrentItem->E.Set(call->SourceData.P + position+offset, 0, false);
		call->FunctionCurrentItem->Data.Set(call->SourceData.P + position, offset, false);
		call->FunctionCurrentItem->SourceOffset = position;
		return 0;
	}
	return 1;
}

int HtmlQLParser::functionSubStr(char* p, void* call_from, ReferData* tx){
	tx->Set("", 0, true);

	long position=0, offset=0;
	HTQLParser* call = (HTQLParser* )call_from;
	if (p && tStrOp::isDigit(*p)){
		sscanf(p, "%ld", &position);
	}
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		sscanf(call->FunctionParameters->Value.P, "%ld", &position);
		if (call->FunctionParameters->Next && call->FunctionParameters->Next->Value.P){
			sscanf( call->FunctionParameters->Next->Value.P, "%ld", &offset);
		}
	}
	if (position < call->FunctionCurrentItem->Data.L) {
		if (position + offset > call->FunctionCurrentItem->Data.L)
			offset = call->FunctionCurrentItem->Data.L - position;
		position += call->FunctionCurrentItem->SourceOffset;
		call->FunctionCurrentItem->S.Set(call->SourceData.P + position, 0, false);
		call->FunctionCurrentItem->E.Set(call->SourceData.P + position + offset, 0, false);
		call->FunctionCurrentItem->Data.Set(call->SourceData.P + position, offset, false);
		call->FunctionCurrentItem->SourceOffset = position;
		return 0;
	}
	return 1;
}

int HtmlQLParser::functionPosition(char* p, void* call_from, ReferData* tx){ //&position
	HTQLParser* call = (HTQLParser* )call_from;
	char tmp[128];
	sprintf(tmp, "<position from=%ld to=%ld length=%ld></position>", 
		call->FunctionCurrentItem->SourceOffset, 
		call->FunctionCurrentItem->SourceOffset + call->FunctionCurrentItem->Data.L,
		call->FunctionCurrentItem->Data.L);
	*tx = tmp;
	return 1;
}
int HtmlQLParser::functionHtmlTitle(char*p, void* call_from, ReferData* tx){ //&html_title
	HTQLParser* call = (HTQLParser* )call_from;
	*tx="";

	int level=0;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		level=call->FunctionParameters->Value.getLong();
	}

	get_Title(p, tx, level); 
	return 1;
}
int HtmlQLParser::get_Title(const char* page, ReferData* title, int level){
	HtmlQL ql;
	ql.setSourceData(page, page?strlen(page):0, false);
	ql.setQuery("<title>");  if (!ql.isEOF()) *title+=ql.getValue(1);
	if (level>=1 || !title->L ) {
		ql.setQuery("<h1> / /");  
		if (ql.isEOF() && (level>=2 || level==0)) ql.setQuery("<h2> / /"); 
		if (ql.isEOF() && (level>=3 || level==0)) ql.setQuery("<h3> / /"); 
		if (!ql.isEOF()) {*title+=" "; *title+=ql.getValue(1);}
	}
	return 0;
}
int HtmlQLParser::functionHtmlMainDate(char*p, void* call_from, ReferData* tx){ //&html_main_date
	*tx="";
	HTQLParser* call = (HTQLParser* )call_from;

	ReferData page, option; 
	page.Set(p, p?strlen(p):0, false); 

	HtPageModel model;
	option="date";
	ReferData results;
	long offset=model.getHtmlMainText(&page, &option, 0, &results);
	if (offset>=0){
		call->FunctionCurrentItem->SourceOffset=call->FunctionCurrentItem->SourceOffset+offset;
		call->FunctionCurrentItem->Data.Set(results.P, results.L, true);
		call->FunctionCurrentItem->S.Set(call->FunctionCurrentItem->Data.P, 0, false);
		call->FunctionCurrentItem->E.Set(call->FunctionCurrentItem->Data.P+call->FunctionCurrentItem->Data.L, 0, false);
	}
	return 0;

}

int HtmlQLParser::functionHtmlMainText(char*p, void* call_from, ReferData* tx){ //&html_main_text
	*tx="";

	HTQLParser* call = (HTQLParser* )call_from;

	ReferData page, page2, option; 
	page.Set(p, p?strlen(p):0, false); 

	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		option.Set(call->FunctionParameters->Value.P, call->FunctionParameters->Value.L, false);
		if (call->FunctionParameters->Next && call->FunctionParameters->Next->Value.P){
			page2.Set(call->FunctionParameters->Next->Value.P, call->FunctionParameters->Next->Value.L, false);
		}
	}
	HtPageModel model;
	if (!option.Cmp("maintext", 8, false)){
		if (page2.L){ //main div tags
			HTQLTagDataSyntax syntax;
			syntax.setSentence(page2.P);
			ReferData tag;
			model.MainDivTags.empty();
			while (syntax.Type!=QLSyntax::synQL_END){
				if (syntax.Type == HTQLTagDataSyntax::synQL_START_TAG || syntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
					tag.Set(syntax.Sentence+syntax.Start, syntax.StartLen, false); 
					model.MainDivTags.add(&tag, 0, 0);
				}
				syntax.match(); 
			}
			if (!model.MainDivTags.Total){
				model.resetMainTags();
			}else{
				HtPageModel model1;
				for (ReferLink* link=model1.MainDivTags.getReferLinkHead(); link; link=link->Next){
					if (!model.MainDivTags.findName(&link->Name)){
						model.MainBlankTags.add(&link->Name, 0, 0); 
					}
				}
			}
		}
	}

	ReferData results;
	long offset=model.getHtmlMainText(&page, &option, &page2, &results); 
	if (offset>=0){
		call->FunctionCurrentItem->SourceOffset=call->FunctionCurrentItem->SourceOffset+offset;
		call->FunctionCurrentItem->Data.Set(results.P, results.L, true);
		call->FunctionCurrentItem->S.Set(call->FunctionCurrentItem->Data.P, 0, false);
		call->FunctionCurrentItem->E.Set(call->FunctionCurrentItem->Data.P+call->FunctionCurrentItem->Data.L, 0, false);
	}
	return 0;
}

int functionHtmlMainText_OLD(char*p, void* call_from, ReferData* tx){ //&html_main_text
	*tx="";

	const char* blanks[]={"<P>", "</P>","<br>", "</br>", "<a>", "</a>", 
		"<b>", "</b>", "<strong>", "</strong>", "<i>", "</i>", "<font>", "</font>", 
		"<h1>", "</h1>", "<h2>", "</h2>", "<h3>", "</h3>", "<img>", "<td>", "</td>", 
		"<ul>","</ul>","<ol>","</ol>","<li>","</li>", 
		0};
	const char* ignore[]={"<textarea>", "<script>", "<style>", "<span>", "</span>", 
			"<head>", "<title>", "<meta>",
		0};
	const char* divs[]={"<div>", "</div>", "<table>", "</table>", 0};
	long best_len=0, newlen=0, last_len=0;

	int is_valid_tag=0;
	char* tag=0, *lasttag=p, *best_tag=0, *best_lasttag=0;
	long lastpos=0, best_lastpos=0, best_offset=0;
	HTQLTagDataSyntax syntax;
	syntax.setSentence(p, 0, false);
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG || syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isTags(syntax.Sentence+syntax.Start, blanks)>=0 ){
				is_valid_tag=1;
			}else if (TagOperation::isTags(syntax.Sentence+syntax.Start, ignore)>=0 ){
				is_valid_tag=-1;
			}else if (TagOperation::isTags(syntax.Sentence+syntax.Start, divs)>=0 ){
				last_len=0;
				lastpos=syntax.Start;
				is_valid_tag=0;
			}else{
				is_valid_tag=0;
			}
		}else if (syntax.Type==HTQLTagDataSyntax::synQL_DATA && is_valid_tag>=0){
			newlen=syntax.StartLen;
			if (is_valid_tag){
				newlen+=last_len;
			}else{
				lastpos=syntax.Start;
			}
#ifdef _DEBUG
#if 0
			ReferData a;
			a.Set(syntax.Sentence+syntax.Start, newlen>100?100:newlen, true);
			TRACE("%ld, %ld: %s\n", best_len, newlen, a.P);
#endif
#endif
			if (newlen>best_len){
				best_len=newlen;
				best_lastpos=lastpos;
				best_offset=syntax.Start+syntax.StartLen-best_lastpos;
			}
			last_len=newlen;
		}else{
			if (is_valid_tag>0)
				is_valid_tag=0;
		}

		syntax.match();
	}

	HTQLParser* call = (HTQLParser* )call_from;
	call->FunctionCurrentItem->SourceOffset=call->FunctionCurrentItem->SourceOffset+best_lastpos;
	call->FunctionCurrentItem->Data.Set(p+best_lastpos, best_offset, true);
	return 0;
	//tx->Set(p+best_lastpos, best_len, true);
	//return 1;

/*
	HTQLTagSelection tag_selection;
	tag_selection.Data.Set(p, strlen(p), false);
	tag_selection.parseHTMLScope();
	HTQLScope* tag, *lasttag, *best_tag, *best_lasttag;
	tag = lasttag = best_tag = best_lasttag = tag_selection.TagScope;
	for (tag = tag_selection.TagScope; tag; tag = tag->NextTag){
		if (!tag->NextTag || TagOperation::isTags(tag->S.P, ignore)>=0) continue; //ignore the text at the tail for now.

		newlen=tag->NextTag->S.P-tag->S.P-tag->S.L;
		if (TagOperation::isTags(tag->S.P, blanks)>=0 ){
			newlen+=last_len;
		}else{
			lasttag=tag;
		}
#if 0
		ReferData a;
		a.Set(tag->S.P, newlen>100?100:newlen, true);
		TRACE("%ld, %ld: %s\n", best_len, newlen, a.P);
#endif
		if (newlen>best_len){
			best_len=newlen;
			best_lasttag=lasttag;
			best_tag=tag;
		}
		last_len=newlen;
	}
	tx->Set(best_lasttag->S.P, best_tag->NextTag->S.P-best_lasttag->S.P, true); //html format
	//for (tag = best_lasttag; tag; tag = tag->NextTag){
	//	newlen=tag->NextTag->S.P-tag->S.P-tag->S.L;
	//	tx->Cat(" ", 1); tx->Cat(tag->S.P+tag->S.L, newlen);
	//	if (tag==best_tag) break;
	//}
	*/
	return 1;
}

int HtmlQLParser::functionTagIndex(char* p, void* call_from, ReferData* tx){ //&tag_index(source0/scope1, no_name)
		// <tag_index>:tag, name, index, use_name
	*tx="";
	HTQLParser* call = (HTQLParser* )call_from;
	long source_scope=0;
	long use_name=1;
	ReferData tag_name;
	ReferData name, value, value1;
	readHtqlParameterLong(call->FunctionParameters, 1, &source_scope);
	readHtqlParameterLong(call->FunctionParameters, 2, &use_name);
	char* names[]={"NAME", "ID", 0};
	int i;
	for (i=0; names[i]; i++){
		name=names[i];
		get_tagAttribute(call->FunctionCurrentItem, &name, &value);
		if (value.L) break;
	}
	get_tagName(call->FunctionCurrentItem, &tag_name);
	if (use_name) {
		use_name=value.L?1:0;
	}

	char tmp[128];
	long index=1;
	long in_iframe=0;
	const char* ignored_start[]={"<iframe>", "<noscript>", 0};
	const char* ignored_end[]={"</iframe>", "</noscript>", 0};
	HTQLScope* tag;
	if (source_scope==0){//from source
		HTQLTagSelection tag_selection;
		//tag_selection->Options = call->Options;
		tag_selection.Data.Set(call->SourceData.P, call->SourceData.L, false);
		tag_selection.parseHTMLScope(true);
		index=1;
		for (tag = tag_selection.TagScope
			; tag->SourceOffset < call->FunctionCurrentItem->SourceOffset
			; tag=tag->NextTag
			){
			if (TagOperation::isTags(tag->S.P, ignored_start)>=0 && tag->EnclosedTag!=tag){
				in_iframe++;
			}else if (TagOperation::isTags(tag->S.P, ignored_end)>=0 ){
				in_iframe--;
			}else if (TagOperation::isTag(tag->S.P, tag_name.P) && in_iframe==0 ){
				if (use_name){
					get_tagAttribute(tag, &name, &value1);
					if (!value.Cmp(&value1, true)){
						index++;
					}
				}else{
					index++;
				}
			}
		}
	}else{
		HTQLItem* first_item;
		index=0;
		for (first_item= call->Data; first_item; first_item=first_item->NextItem){
			if (first_item->ItemType == HTQLItem::itemSCHEMA_DEF) continue;
			if (TagOperation::isTag(first_item->S.P, tag_name.P) ){
				if (use_name){
					get_tagAttribute(first_item, &name, &value1);
					if (!value.Cmp(&value1, true)){
						index++;
					}
				}else{
					index++;
				}
			}
			if (first_item==call->FunctionCurrentItem) break;
		}
	}
	*tx="<tag_index tag=\"";
	*tx+=tag_name;
	*tx+="\" nameid=\"";
	*tx+=name;
	*tx+="\" namevalue=\"";
	*tx+=value;
	sprintf(tmp, "\" index=\"%ld\" use_name=\"%ld\"></tag_index>", index, use_name);
	*tx+=tmp;
	return 1;
}

int HtmlQLParser::functionIEPosTag(char* p, void* call_from, ReferData* tx){ //&position
	// <position>: from, to, lenght
	*tx = "";
	HTQLParser* call = (HTQLParser* )call_from;
	long tagindex = 0;
	readHtqlParameterLong(call->FunctionParameters, 1, &tagindex);

	long tagcount=0;
	if (tagindex ==0) tagindex=1;

	/*
	ReferData tag;
	HTQLTagDataSyntax DataSyntax;
	DataSyntax.setSentence(call->SourceData.P, &call->SourceData.L, false);
	for (;
		DataSyntax.Type != QLSyntax::synQL_END && tagcount<tagindex; 
		DataSyntax.match())
		{
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA) continue;
		tag.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, false);
		if (!strncmp(tag.P, "</", 2)) continue;
		
		if (TagOperation::isTag(tag.P, "SCRIPT")) {
			while (DataSyntax.Type != QLSyntax::synQL_END &&
				!TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "SCRIPT"))
				DataSyntax.match();
		}
		if (TagOperation::isTag(tag.P, "TITLE")) continue;
		if (TagOperation::isTag(tag.P, "TBODY")) continue;
		if (TagOperation::isTag(tag.P, "TABLE")) tagcount++;
		tagcount++;

		if (tagcount>=tagindex) {
			//set: call->FunctionCurrentItem
			return 0;
		}
	}
	*/
	
	HTQLTagSelection tag_selection;
	tag_selection.Data.Set(call->SourceData.P, call->SourceData.L, false);
	tag_selection.parseHTMLScope(true);
	HTQLScope* tag = tag_selection.TagScope;
	int in_tb=0, in_tb_data=0;

	for (; tag && tagcount<tagindex; tag = tag->NextTag){
		//keep finding the closest tag;
		if (!tag->S.P || !tag->S.L) continue;
		if (!strncmp(tag->S.P, "</", 2)) continue;
		if (TagOperation::isTag(tag->S.P, "SCRIPT") && tag->EnclosedTag) {
			tag = tag->EnclosedTag; 
		}
		if (TagOperation::isTag(tag->S.P, "TITLE")) continue;
		if (TagOperation::isTag(tag->S.P, "TBODY")) continue;

		if (TagOperation::isTag(tag->S.P, "TABLE")) {
			tagcount++;
			in_tb =1;in_tb_data=0;
		}else if (TagOperation::isTag(tag->S.P, "TR")) {
			if (in_tb_data && in_tb!=5)	tagcount++;
			in_tb =2;in_tb_data=0;
		}else if (TagOperation::isTag(tag->S.P, "TD")) {
			if (in_tb_data && in_tb!=2 && in_tb!=4)	tagcount++;
			in_tb =0;in_tb_data=0;
		}else if (TagOperation::isEndTag(tag->S.P, "TD")) {
			if (in_tb_data)	tagcount++;
			in_tb =4;in_tb_data=0;
		}else if (TagOperation::isEndTag(tag->S.P, "TR")) {
			if (in_tb_data && in_tb!=4 && in_tb!=2)	tagcount++;
			in_tb =5;in_tb_data=0;
		}else if (TagOperation::isEndTag(tag->S.P, "Table")) {
			if (in_tb_data && in_tb!=5 && in_tb!=1)	tagcount++;
			in_tb =0;in_tb_data=0;
		}else if (in_tb && TagOperation::isTag(tag->S.P)) {
			in_tb_data =1;
		}else{
			in_tb_data = 0;
		}
		tagcount++;

		if (tagcount>=tagindex) break;
	}
	if (!tag) tag=tag_selection.TagScope;
	if (tag) {
		call->FunctionCurrentItem->copyTag(tag, false);
		if (!tag->E.P){
			call->FunctionCurrentItem->E.Set(tag->S.P+tag->S.L, 0, false);
		}
		call->FunctionCurrentItem->Data.Set(call->FunctionCurrentItem->S.P, call->FunctionCurrentItem->E.P+call->FunctionCurrentItem->E.L-call->FunctionCurrentItem->S.P, true);
		return 0;
	}

	return 1;
}

int HtmlQLParser::functionHtqlMaxViewPos(char* p, void* call_from, ReferData* tx){//&htql_bestview
	tx->Set("", 0, true);
	if (!p) return 1;

	HTQLParser* call = (HTQLParser* )call_from;

	int max_pos=0, se_max_pos=0;
	long max=0, se_max=0;
	double similarity=1.0;
	long tscore=0;
	findBestViewTagPos(p, &call->SourceData, &max_pos, &max, &se_max_pos, &se_max, &similarity, &tscore);

	char buf[20];
	*tx = "<htql_max_view_pos Htql=\"";
	*tx += p;
	*tx += "\" MaxTagPos=\"";
	sprintf(buf, "%d", max_pos);
	*tx += buf;
	*tx += "\" MaxTagNum=\"";
	sprintf(buf, "%ld", max);
	*tx += buf;
	*tx += "\" SecMaxTagPos=\"";
	sprintf(buf, "%d", se_max_pos);
	*tx += buf;
	*tx += "\" SecMaxTagNum=\"";
	sprintf(buf, "%ld", se_max);
	*tx += buf;
	*tx += "\" Similarity=\"";
	sprintf(buf, "%lf", similarity);
	*tx += buf;
	*tx += "\" TupleScore=\"";
	sprintf(buf, "%ld", tscore);
	*tx += buf;
	*tx += "\"></htql_max_view_pos>";

	return 1;
}

HTQLScope* HtmlQLParser::get_parentTag(HTQLScope* target_tag, char* parent_name){
	HTQLScope* tag=target_tag->PreviousTag;
	//if (target_tag && target_tag->SourceOffset==9453){
	//	printf("test");
	//}
	while (tag && 
		(!tag->E.P
		|| (target_tag->E.P && tag->E.P < target_tag->E.P) 
		|| (!target_tag->E.P && tag->E.P <= target_tag->S.P)
		) ){
		//keep finding the closest tag;
		tag = tag->PreviousTag;
	}
	return tag;
}

long HtmlQLParser::get_tagName(HTQLTag* element, ReferData* tag_name){
	long len=0;
	char* p1=TagOperation::getLabel(element->S.P, (unsigned int*) &len);
	tag_name->Set(p1, len, true);
	return len;
}

long HtmlQLParser::get_tagAttribute(HTQLTag* element, ReferData* attr_name, ReferData* attr_value){
	if (!attr_name->Cmp("tx", 2, false)){
		long len=element->E.P-(element->S.P+element->S.L);
		attr_value->Set(element->S.P+element->S.L, (len>0)?len:0, true);
		return element->SourceOffset+element->S.L;
	}
	HTQLAttrDataSyntax AttrSyn;
	ReferData name;
	AttrSyn.setSentence(element->S.P, &element->S.L, false);
	while (AttrSyn.Type != QLSyntax::synQL_UNKNOW &&
		AttrSyn.Type != QLSyntax::synQL_END 
		){
		if (AttrSyn.Type == QLSyntax::synQL_WORD ){
			name.Set(AttrSyn.Sentence+AttrSyn.Start, AttrSyn.StartLen, true);
			tStrOp::replaceInplace(name.P, ":", "_");
			tStrOp::replaceInplace(name.P, "-", "_");
			if (!attr_name->Cmp(name.P, name.L, false)) break;
		}
		AttrSyn.match();
	}
	long offset=element->SourceOffset;
	if (AttrSyn.Type == QLSyntax::synQL_WORD){
		AttrSyn.match();
		AttrSyn.match();// synQL_EQ;
		if (AttrSyn.Type == QLSyntax::synQL_WORD){
			attr_value->Set(AttrSyn.Sentence+AttrSyn.Start, AttrSyn.StartLen, true);
			offset = element->SourceOffset + AttrSyn.Start;
		}else if (AttrSyn.Type == QLSyntax::synQL_STRING){
			if (AttrSyn.Sentence[AttrSyn.Start] == '\'' || AttrSyn.Sentence[AttrSyn.Start] == '"'){
				attr_value->Set(AttrSyn.Sentence+AttrSyn.Start+1, AttrSyn.StartLen-2, true);
				offset = element->SourceOffset + AttrSyn.Start+1;
			}else{
				attr_value->Set(AttrSyn.Sentence+AttrSyn.Start, AttrSyn.StartLen, true);
				offset = element->SourceOffset + AttrSyn.Start;
			}
		}else{
			attr_value->Set(AttrSyn.Sentence+AttrSyn.Start, 0, true);
			offset = element->SourceOffset + AttrSyn.Start;
		}
	}else{
		attr_value->Set(AttrSyn.Sentence+AttrSyn.Start, 0, true);
		offset = element->SourceOffset + AttrSyn.Start;
	}
	return offset;
}

HTQLScope* HtmlQLParser::get_nextChildTag(HTQLScope* parent_tag, HTQLScope* tag, char* tag_name){
	if (!tag) tag=parent_tag;
	tag=tag->NextTag;
	while (tag){
		while ( tag && (!tag_name || !TagOperation::isTag(tag->S.P, tag_name)) ){
			tag = tag->NextTag;
		}
		if (!tag || tag->S.P >= parent_tag->E.P) break;
		if (!tag->E.P || tag->E.P < parent_tag->E.P) return tag;
		tag=tag->NextTag;
	}
	return 0;
}

int HtmlQLParser::tag_contain(HTQLScope* parent_tag, HTQLScope* tag){
//	return (parent_tag->S.P < tag->S.P) && (!tag->E.P || parent_tag->E.P > tag->E.P);

	return (parent_tag->S.P < tag->S.P) && (parent_tag->E.P > tag->E.P);
}

int HtmlQLFindLevel=9; //this is only a lower limit for algFindTagHtqlParent2Child, so set smaller; 
						//using pubmed, this should be 9
						//set =10~13 if HtmlQLFindAlgorithm=algFindTagHtqlChild2Parent
int HtmlQLFindAlgorithm=algFindTagHtqlParent2Child;  

/*int HtmlQLParser::findTagHtql(HTQLScope* lpElementCollection, HTQLScope* lpSelElement, tStack* results, char* suffix, int EnclosedSelItem, int HeuristicMethod){
	ReferLinkHeap result1;
	findTagHtql(lpElementCollection, lpSelElement, &result1, suffix, EnclosedSelItem, HeuristicMethod);
	for (ReferLink* ts=(ReferLink*) result1.moveFirst(); !result1.isEOF() ; ts=(ReferLink*) result1.moveNext() ){
		results->set(ts->Data, ts->Name.P, " ");
	}
	return 0;
}*/
int HtmlQLParser::findTagHtql(HTQLScope* lpElementCollection, HTQLScope* lpSelElement, ReferLinkHeap* results, char* suffix, int EnclosedSelItem, int HeuristicMethod, int* mark, int mark_level){
	if (!results->Total){
		results->setSortOrder(SORT_ORDER_KEY_STR_INC);
		results->setCaseSensitivity(false);
		results->setDuplication(false);
	}

	if (!lpSelElement->E.P && lpSelElement->S.P && lpSelElement->S.P[1]=='/'){
		if (lpSelElement->EnclosedTag) //has the corresponding starting tag
			lpSelElement = lpSelElement->EnclosedTag;
		else if (lpSelElement->PreviousTag){ //suppose the previous tag is not the end tag again, fix it later
			lpSelElement = lpSelElement->PreviousTag;
		}
	}

	if (EnclosedSelItem && !lpSelElement->E.P){
		lpSelElement = get_parentTag(lpSelElement);
	}

	switch (HtmlQLFindAlgorithm){
	case algFindTagHtqlChild2Parent:
		return findTagHtqlChild2Parent(lpElementCollection, lpSelElement, results, suffix, 0, HeuristicMethod);
	case algFindTagHtqlParent2Child:
	default:
		return findTagHtqlParent2Child(lpElementCollection, lpSelElement, results, suffix, HeuristicMethod, mark, mark_level);
	}
	return 0;
}

int HtmlQLParser::findTagHtqlChild2Parent(HTQLScope* lpElementCollection, HTQLScope* lpSelElement, ReferLinkHeap* results, char* suffix, int level, int HeuristicMethod){
	if (level>HtmlQLFindLevel) return 0;
	ReferData tagname, parentname, str1, str2;
	ReferData bstr, estr;
	long len=0;
	long index;
	HTQLScope *lpHtmlElm, *lpHtmlElm0;
	HTQLScope *lpParentElm;
	HTQLScope * lpElementCollection0=lpElementCollection;
	char tmp[128];

	lpParentElm = get_parentTag(lpSelElement);
	if (lpParentElm){
		lpElementCollection = lpParentElm;
	}

	int max_id_length=256; 
	int circle=0;
	char* attributes[]={"Name", "ID", "CLASS", 0};
	while (lpElementCollection){
		circle++;
		get_tagName(lpSelElement, &tagname);

		if (lpParentElm){
			get_tagName(lpParentElm, &parentname);
		}else{
			parentname.Set("",0,true);
		}
		lpHtmlElm0=0;

		ReferData n1, v;
		int use_name=0, k;
		if (!tagname.Cmp("input", 5, false)||
			!tagname.Cmp("form", 4, false)||
			!tagname.Cmp("div", 3, false)||
			!tagname.Cmp("span", 4, false)||
			!tagname.Cmp("frame", 5, false)||
			!tagname.Cmp("a", 1, false)||
			!tagname.Cmp("table", 5, false)){
			if (HeuristicMethod==HtmlQL::heuBEST || HeuristicMethod==HtmlQL::heuDEPTH_SHORT){
				for (k=0; attributes[k]; k++){
					n1.Set(attributes[k], strlen(attributes[k]), false);
					get_tagAttribute(lpSelElement, &n1, &v);
					if (v.L && ((k==0 && v.L<max_id_length*1.5) || (k==1 && v.L<max_id_length) || (k==2 && v.L<max_id_length*2) ) ) {
						use_name=k+1;
						break;
					}
				}
			}
		}
		index=0;
		for (lpHtmlElm=lpElementCollection
			; lpHtmlElm && lpHtmlElm->S.P < lpElementCollection->E.P && lpHtmlElm != lpSelElement
			; lpHtmlElm = HTQLTagSelection::searchNextTag(lpHtmlElm, &tagname, true, false)
			) {
			if (!use_name) index++;
			else {
				str1.reset();
				n1.Set(attributes[use_name-1], strlen(attributes[use_name-1]), false);
				get_tagAttribute(lpHtmlElm, &n1, &str1);
				if (str1.L && !str1.Cmp(&v, false)) 
					index++;
			}
		}

		if (lpHtmlElm == lpSelElement){
			if (use_name){
				sprintf(tmp, " (%s='%s')", attributes[use_name-1], v.P);
				v=tmp;
				index++;
			}else{
				v="";
			}
			if (lpSelElement->S.P[0] == '<'){
				sprintf(tmp, "<%s%s>%d",tagname.P, v.P, index);
			}else{
				bstr.Set(lpSelElement->S.P, lpSelElement->S.L, true);
				estr.Set(lpSelElement->E.P, lpSelElement->E.L, true);
				if (estr.isNULL()){
					sprintf(tmp, "<'%s'>%d", bstr.P, index+1);
				}else{
					sprintf(tmp, "<'%s'~'%s'>%d", bstr.P, estr.P, index);
				}
			}
			str1.Set(tmp, strlen(tmp), true);
			ReferLinkHeap result1;
			ReferLink* ts, *ts1;
			long data=0; 
			//int isFormat = (!tagname.CompareNoCase("TABLE") || !tagname.CompareNoCase("DIV"));
			if (lpParentElm && lpElementCollection0 != lpElementCollection){
				findTagHtql(lpElementCollection0, lpParentElm, &result1, str1.P, level+1, HeuristicMethod);
				for (ts=(ReferLink*) result1.moveFirst(); !result1.isEOF() ; ts=(ReferLink*) result1.moveNext() ){
					str2 = ts->Name;
					data = ts->Data;
					if (suffix && *suffix){
						if (suffix[0]=='<' || suffix[0]=='/') str2.Cat(".", 1);
						str2+= suffix;
						data++;
					}
					if (!results->findName(&str2)){
						ts1=results->add(&str2, 0, data);
					}
				}
			}else{
				data=1;
				str2.Set(str1.P, str1.L, true);
				if (suffix && *suffix){
					if (suffix[0]=='<' || suffix[0]=='/') str2.Cat(".", 1);
					str2+=suffix;
					data ++;
				}
				if (!results->findName(&str2)){
					results->add(&str2, 0, data);
				}
			}
		}

		if (!lpParentElm) break;
		get_tagName(lpParentElm, &parentname);
		if ( !parentname.Cmp(&tagname,false) ) break;
		lpParentElm = get_parentTag(lpParentElm);
		if (lpParentElm){
			get_tagName(lpParentElm, &str2);
			if ( !str2.Cmp("HTML", 4, false) || !parentname.Cmp("HTML", 4, false)){
				//break;
				lpParentElm=0;
				lpElementCollection = lpElementCollection0;
			}else{
				lpElementCollection = lpParentElm;
			}
		}else {
			lpElementCollection = lpElementCollection0;
		}
	}
	return 0;
}

//-----------new algorithm from here------------
struct FindTagHtqlStruct{
	int map_index;
	int tag_i;
	int use_name;
	ReferData name;
	ReferData result;
	int tag_name_i;

	FindTagHtqlStruct(){
		map_index=tag_i=use_name=tag_name_i=0;
	}
};

int HtmlQLParser::findTagHtqlParent2Child(HTQLScope* lpElementCollection, HTQLScope* lpSelElement, ReferLinkHeap* results, char* suffix, int HeuristicMethod, int* mark, int mark_level){
	ReferLink* head=0, *tt;
	HTQLScope* tag;
	int h=1;
	for (tag=lpSelElement; tag; tag=get_parentTag(tag) ){
		h++;
#ifdef DEBUG_THIS_FILE
		if (h>20){
			TRACE("big h=%d\n", h);
		}
#endif
		tt=head;
		head=new ReferLink;
		head->Next=tt;
		head->Value.Set((char*) tag, 0, false);
		get_tagName(tag, &head->Name);
	}

	int i, j, k, l;
	int max_id_length=256; 
	i=1;
	HTQLScope** tags=new HTQLScope*[h+1];
	tags[0]=lpElementCollection;
	for (tt=head; tt; tt=tt->Next){
		//printf("%s\n", tt->Name.P);
		HTQLScope* scope=(HTQLScope*) tt->Value.P;
		if ((!tt->Name.Cmp("HTML", 4, false)||!tt->Name.Cmp("BODY", 4, false)||!tt->Name.Cmp("TBODY", 5, false))&&tt->Next){
			tt->Data=-1;
			h--;
		}else if (mark &&scope && mark[scope->SourceOffset]<mark_level){
			tt->Data=-1;
			h--;
		}else{
			tags[i]=(HTQLScope*) tt->Value.P;
			tt->Data=i++; //index_map's index
		}
	}

	FindTagHtqlStruct** index_map=new FindTagHtqlStruct*[h+1];
	for (i=0; i<h+1; i++){
		index_map[i]=new FindTagHtqlStruct[h];
		for (j=0; j<h; j++) index_map[i][j].tag_i=0;
	}

	char tmp[256];
	char* attributes[]={"Name", "ID", "CLASS",  0,   "tx",   0,   "TITLE",  0}; //don't change the order now, it is prone to break!!
	enum			{I_Name, I_ID, I_CLASS, I_NONE1, I_TX, I_NONE2, I_TITLE};
	for (i=0; i<h+1; i++){
		for (j=i+1; j<h; j++){
			ReferData tagname;
			get_tagName(tags[j], &tagname);
			ReferData n1, v, val_title;
			int use_name=0;
			if (!tagname.Cmp("input", 5, false)||
				!tagname.Cmp("form", 4, false)||
				!tagname.Cmp("div", 3, false)||
				!tagname.Cmp("span", 4, false)||
				!tagname.Cmp("frame", 5, false)||
				!tagname.Cmp("a", 1, false)||
				!tagname.Cmp("table", 5, false)){
				if (HeuristicMethod==HtmlQL::heuBEST || HeuristicMethod==HtmlQL::heuDEPTH_SHORT || HeuristicMethod==HtmlQL::heuBEST_LEAVE
					|| HeuristicMethod==HtmlQL::heuNO_CLASS){
					for (k=0; attributes[k]; k++){
						if (HeuristicMethod==HtmlQL::heuBEST_LEAVE && (k==I_ID || k==I_Name)){ 
							//no id and name
							continue; 
						}
						n1.Set(attributes[k], strlen(attributes[k]), false);
						get_tagAttribute(tags[j], &n1, &v);
						if (v.L && ((k==I_Name && v.L<max_id_length*1.5) || (k==I_ID && v.L<max_id_length) || (k==I_CLASS && v.L<max_id_length*2) ) ) {
							use_name=k+1; //from 1
							break;
						}
						if (HeuristicMethod==HtmlQL::heuNO_CLASS) break; //only name
					}
					if (HeuristicMethod!=HtmlQL::heuBEST_VIEW  && HeuristicMethod!=HtmlQL::heuDEPTH_LONG
						){ //try title
						k=I_TITLE; 
						n1.Set(attributes[k], strlen(attributes[k]), false);
						get_tagAttribute(tags[j], &n1, &val_title);
						if (val_title.L && val_title.L<max_id_length*1.5) {
							//has title
							int has_special=0;
							for (int ki=0; ki<val_title.L; ki++){
								if ( val_title.P[ki] == '\'' || val_title.P[ki] == '"' || val_title.P[ki] < ' ' || val_title.P[ki] > 126){
									has_special=1; break;
								}
							}
							if (!has_special){
								use_name=k+1; //from 1
								v=val_title;
							}
						}
					}
				}
			}

			// this is the last tag
			int like_expr=false;
			int is_last_tag=false;
			ReferData val_tx;
			if (HeuristicMethod!=HtmlQL::heuBEST_VIEW && HeuristicMethod!=HtmlQL::heuDEPTH_LONG
				&& j==h-1 && tags[j]->NextTag==tags[j]->EnclosedTag){ //this is the last tag
				is_last_tag=true; 
				//for special next query
				n1="tx";
				get_tagAttribute(tags[j], &n1, &val_tx);
				char* nextchar[]={"Next", "Prev", "Previous", "下一页","上一页","下页","上页", ">>", "<<", "&gt;&gt;", "&lt;&lt;",0};
				//find exact match
				//for (k=0; !attributes[k] || n1.Cmp(attributes[k], strlen(attributes[k]),false); k++);
				for (l=0; nextchar[l]; l++){
					if (!val_tx.Cmp(nextchar[l], strlen(nextchar[l]),false)){
						use_name=I_TX+1; //from 1
						v=val_tx;
						break;
					}
				}
				//find like match
				if (!use_name && val_tx.L<max_id_length){
					val_tx.Seperate();
					for (l=0; nextchar[l]; l++){
						if (tStrOp::strNstr(val_tx.P, nextchar[l], false)){
							use_name=I_TX+1; //from 1
							v="%"; v+=nextchar[l]; v+="%";
							like_expr=true;
							break;
						}
					}
				}
			}

			int index = 1; 
			if (!like_expr) index=get_tagChildIndex(tags[i], tags[j], (use_name)?attributes[use_name-1]:0, v.P);
			//ASSERT((index>0) || (j>i+1) || tags[i]->SourceOffset==tags[j]->SourceOffset); //must be able to query the immediate child
			//for extremely large index, try to use tx
			if (index>8 && is_last_tag && !tagname.Cmp("a", 1, false)){
				use_name=I_TX+1; //from 1
				v=val_tx;
				index=get_tagChildIndex(tags[i], tags[j], "tx", val_tx.P);
			}

			if (index>0){
				if (use_name && v.L<128){ //why sometimes it has long value??? check later
					if (HeuristicMethod==HtmlQL::heuBEST_LEAVE){
						//int like_expr=false; //defined previously
						if (use_name==I_Name+1 || use_name==I_ID+1){ //Name or ID
							for (k=0; k<v.L; k++){
								if (isdigit(v.P[k])) {
									v.P[k]='%'; like_expr=true;
								}
							}
						}
					}

					if (v.P && strchr(v.P, '\'')){
						char* v1=tStrOp::replaceMalloc(v.P, "'", "''");
						v.reset();
						v.Set(v1, strlen(v1), false); 
						v.setToFree(true); 
					}
					if (like_expr) sprintf(tmp, " (%s like '%s')", attributes[use_name-1], v.P);
					else sprintf(tmp, " (%s='%s')", attributes[use_name-1], v.P);
					v=tmp;
				}else{
					v="";
				}
				if (tags[j]->S.P[0] == '<'){
					sprintf(tmp, "<%s%s>%d",tagname.P, v.P, index);
				}else{
					ReferData bstr, estr;
					bstr.Set(tag->S.P, tag->S.L, true);
					estr.Set(tag->E.P, tag->E.L, true);
					if (estr.isNULL()){
						sprintf(tmp, "<'%s'>%d", bstr.P, index);
					}else{
						sprintf(tmp, "<'%s'~'%s'>%d", bstr.P, estr.P, index-1);
					}
				}
				index_map[i][j].use_name = use_name;
				if (use_name){
					index_map[i][j].tag_name_i=index;
				}else{
					index_map[i][j].tag_i=index;
				}
				index_map[i][j].result=tmp;
				index_map[i][j].map_index=index;
			}
		}
	}

#ifdef DEBUG_THIS_FILE
	for (i=0; i<h+1; i++){
		for (j=0; j<=i; j++){
			TRACE("%8s", " ");
		}
		for (j=i+1; j<h; j++){
			TRACE("%8s", (index_map[i][j].result.P)?index_map[i][j].result.P:"-");
		}
		TRACE("\n");
	}
#endif
	
	int err=searchTagHtqlFromParentChildBits(results, index_map, h, suffix);

	delete head;
	delete[] tags;
	for (i=0; i<h+1; i++){
		delete[] index_map[i];
	}
	delete[] index_map;
	return err;
}
int HtmlQLParser::searchTagHtqlFromParentChildBits(ReferLinkHeap* results, FindTagHtqlStruct**index_map, int depth, char* suffix){
	int h=depth;
	unsigned long bits=1; //assume 64 bits
	int half1=0, half2=0;
	int len=62;
	if (HtmlQLFindLevel && (2*HtmlQLFindLevel)<len){
		len=HtmlQLFindLevel*2;
	}
	if (len>h-1 && h>1) {
		len=h-1; //last tag is always selected
	}else{ // ... half1 and half2 is not used
		half1=len/2;
		half2=h-1-len/2;
	}
	bits<<=len; // bits = total number of combinations

	int HtmlQLFindLevel1=HtmlQLFindLevel;
	while (results->Total==0 ){ 
		//increase the upper bound of expression length until results can be found
		int j;
		for (unsigned long array=0; array<bits; array++){
			//check if the expression is too long -- bigger than HtmlQLFindLevel
			int count=1;
			for (j=len-1; j>=1; j--){
				//array&( ((unsigned long)1)<<(j-1)) is the (j-1)'th bit
				if (array&( ((unsigned long)1)<<(j-1)) ) count++;
			}
			if (count > HtmlQLFindLevel1) continue;

			//check if we can index the target tag from the last parent;
			for (j=len-1; j>=1 && !(array&( ((unsigned long)1)<<(j-1)) ); j--);
			if (index_map[j][h-1].map_index<=0) continue; 
			
			//printf("%s\n", index_map[j][h-1].result.P);
				
			ReferData v;
			v="";
			int last=0;
			int valid=true;
			long data=1;
			for (j=1; j<len; j++){
				//get the represented htql expression
				int test=( (array&( ((unsigned long)1)<<(j-1)) ) !=0 );
				if (test && index_map[last][j].map_index>0){
					if (v.L) {
						v+=".";
					}
					//from last to j, the index in index_map[last][j].result
					v+=index_map[last][j].result;
					last=j;
					data++;
				}else if (test){
					//the bit is set, but there is no expression from the last to this bit (j)
					valid=false;
					break;
				}
			}
			if (!valid) continue;

			//last
			if (v.L) {
				v+=".";
			}
			v+=index_map[last][h-1].result;
			if (suffix){
				v+=suffix;
			}

			if (v.L && !results->findName(&v)){
				results->add(&v, 0, data);

				//printf("%05X\t%s\n", array, v.P);
			}
		}
		//increase the upper bound of expression length until results can be found
		if (++HtmlQLFindLevel1>len) break;
	}

	return 0;
}

int HtmlQLParser::get_tagChildIndex(HTQLScope* parent, HTQLScope* child, const char* attrname, const char* attrvalue){ //use attrname is attrname is not null
	ReferData tagname;
	get_tagName(child, &tagname);
	ReferData n, v;
	n = attrname; //"Name";

	int index=0;
	HTQLScope* tag=parent;
	if (tag && TagOperation::isTag(tag->S.P, tagname.P)) tag=tag->NextTag; //skip itself
	if (tag && !TagOperation::isTag(tag->S.P, tagname.P)) 
		tag=HTQLTagSelection::searchNextTag(tag, &tagname, true, false); //find the first child tag

	for (; tag && tag->S.P < parent->E.P && tag->SerialNo <= child->SerialNo
		; tag = HTQLTagSelection::searchNextTag(tag, &tagname, true, false)
		) {
		if (!attrname || !attrname[0]) index++;
		else {
			v.reset();
			get_tagAttribute(tag, &n, &v);
			if (attrvalue && v.L && !v.Cmp(attrvalue, strlen(attrvalue), false)) 
				index++;
			else if (!attrvalue && !v.L) 
				index++;
		}
		if (tag==child) return index;
	}

	return -1;
}

int HtmlQLParser::filterHtqlResults(ReferLinkHeap* Source, ReferLinkHeap* Result, int HeuristicMethod, char* MatchString){
//	*Result = 0;
	int len=0;
	if (MatchString) len=strlen(MatchString);

	ReferLinkHeap* BestItem = Result;
	int Depth=0, FormatDepth=0, SecFormatDepth=0, BadDepth=0, maxidx=0, GoodDepth=0, RecurDepth=0;
	long data;
	BestItem->setSortOrder(SORT_ORDER_NUM_DEC);
	BestItem->setDuplication(true);
	ReferLink* ts;
	int count=0;
	for (ts=(ReferLink*) Source->moveFirst(); !Source->isEOF() ; ts=(ReferLink*) Source->moveNext() ){
		if ((!len || matchItemKeys(ts->Name.P, MatchString)) && ts->Name.L ){
			count ++;
			findItemInfor(ts->Name.P, &Depth, &FormatDepth, &SecFormatDepth, &BadDepth, &GoodDepth, &RecurDepth, &maxidx);
			if (HeuristicMethod == HtmlQL::heuBEST || HeuristicMethod == HtmlQL::heuNO_CLASS){
				data = + FormatDepth* 6 + SecFormatDepth* 4 - Depth*5 - BadDepth*4 + GoodDepth*3 - RecurDepth/2;
				if (maxidx>6) {
					data -= (maxidx-6)*5; //if (maxidx>10) data -= (maxidx-10)/3;
				}
				if (BadDepth ==0) 
					BestItem->add(&ts->Name, &ts->Value, data);
			}else if (HeuristicMethod == HtmlQL::heuDEPTH_SHORT){
				data = - Depth;
				if (BadDepth ==0)
					BestItem->add(&ts->Name, &ts->Value, data);
			}else if (HeuristicMethod == HtmlQL::heuDEPTH_LONG){
				data = Depth;
					BestItem->add(&ts->Name, &ts->Value, data);
			}else if (HeuristicMethod == HtmlQL::heuBEST_LEAVE){ 
				data = + FormatDepth* 6 + SecFormatDepth* 5 - Depth*2 - BadDepth*4 + GoodDepth*3;
					BestItem->add(&ts->Name, &ts->Value, data);
			}else if (HeuristicMethod == HtmlQL::heuBEST_VIEW){
				data = + FormatDepth* 6 + SecFormatDepth* 5 - Depth*2 - BadDepth*4 ;
					BestItem->add(&ts->Name, &ts->Value, data);
			}else{
				data = + FormatDepth* 6 + SecFormatDepth* 4 - Depth*5 - BadDepth*4 + GoodDepth*3;
				if (maxidx>6) {
					data -= (maxidx-6)*5; //if (maxidx>10) data -= (maxidx-10)/3;
				}
				if (BadDepth ==0)
					BestItem->add(&ts->Name, &ts->Value, data);
			}
		}
	}
//	*Result= BestItem;
	return count;
}
/*
int HtmlQLParser::filterHtqlResults(tStack* Source, tStack* Result, int HeuristicMethod, char* MatchString){
//	*Result = 0;
	int len=0;
	if (MatchString) len=strlen(MatchString);

	tStack* BestItem = Result;
	int Depth=0, FormatDepth=0, SecFormatDepth=0, BadDepth=0, maxidx=0;
	long data;
	BestItem->Type=tStack::ordDECDATA;
	tStack* ts;
	int count=0;
	for (ts=Source->Next; ts; ts=ts->Next){
		if (!len || matchItemKeys( ts->Key, MatchString)){
			count ++;
			findItemInfor(ts->Key, &Depth, &FormatDepth, &SecFormatDepth, &BadDepth, &maxidx);
			if (HeuristicMethod == heuBEST){
				data = + FormatDepth* 6 + SecFormatDepth* 4 - Depth*3 - BadDepth*4;
				if (maxidx>10) data -= (maxidx-10)/3;
				if (BadDepth ==0) 
					BestItem->set(data,ts->Key, " ");
			}else if (HeuristicMethod == heuDEPTH_SHORT){
				data = - Depth;
				if (BadDepth ==0)
					BestItem->set(data,ts->Key, " ");
			}else if (HeuristicMethod == heuDEPTH_LONG){
				data = Depth;
				BestItem->set(data,ts->Key, " ");
			}else if (HeuristicMethod == heuBEST_LEAVE){
				data = + FormatDepth* 6 + SecFormatDepth* 5 - Depth*2 - BadDepth*4;
				BestItem->set(data,ts->Key, " ");
			}else{
				data = + FormatDepth* 6 + SecFormatDepth* 4 - Depth*3 - BadDepth*4;
				if (maxidx>10) data -= (maxidx-10)/3;
				if (BadDepth ==0)
					BestItem->set(data,ts->Key, " ");
			}
		}
	}
//	*Result= BestItem;
	return count;
}
*/
int HtmlQLParser::evaluateHtqlExpr(HTQL* htql, const char* expr, ReferData* eval_result){
	*eval_result = "<htql_expr_eval>";
	int i;
	char* p;

	htql->setQuery(expr);
	int fieldsnum=htql->getFieldsCount();
	long* null_fields=(long*) malloc(sizeof(long)*fieldsnum);
	memset(null_fields, 0, sizeof(long)*fieldsnum);

	long tuples=0;
	while (!htql->isEOF()){
		for (i=1; i<=fieldsnum; i++){
			p=htql->getValue(i);
			if (!p || !p[0]) null_fields[i-1]++;
		}
		tuples++;
		htql->moveNext();
	}

	char buf[128];
	ReferData tx;
	tx = "<expr_eval htql=\"";
	tx += expr;
	sprintf(buf, "\" tuples=\"%ld\" fields=\"%d\" ", tuples, fieldsnum);
	tx += buf;
	tx+=" >";

	for (int field=1; field<=fieldsnum; field++){
		sprintf(buf, "<field name=\"%s\" index=\"%d\" null_ratio=\"%lf\" model=\"%s\" model_support=\"%lf\">");
	}

	free(null_fields);

	return 0;
}

int HtmlQLParser::matchItemKeys(char* Source, char* Keys){
	ReferData OneKey;
	int i=0, j=0;
	char* t;
	int len=strlen(Keys);
	while (i < len){
		while (i<len && Keys[i] == ' ' || Keys[i] == '\t') i++;
		if (i >= len) return true;
		for (j=i; j<len && Keys[j] != ' ' && Keys[j] != '\t'; j++);
		if (Keys[i]=='^')
			OneKey.Set(Keys+i+1, j-i-1);
		else
			OneKey.Set(Keys+i, j-i);
		t=tStrOp::strNstr(Source, OneKey.P, false);

		if ((Keys[i]=='^' && t) || (Keys[i]!='^' && !t)) return false;
		i=j;
	}
	return true;
}


int HtmlQLParser::findItemInfor(char* Source, int* Depth, int* FormatDepth, int* SecondFormatDept, int* BadDepth, int*GoodDepth, int*RecurDepth, int* maxidx){
	if (Depth) *Depth=0;
	if (FormatDepth) *FormatDepth=0;
	if (SecondFormatDept) *SecondFormatDept=0;
	if (BadDepth) *BadDepth=0; //<HTML>, <BODY>
	if (GoodDepth) *GoodDepth=0; //having conditions
	if (RecurDepth) *RecurDepth=0; //having recursive tags
	if (maxidx) *maxidx=0; 
	ReferData str, last_str;
	int len=strlen(Source);
	int i=0, j=0;
	int turn=0;
	int tagidx=0;
	while (i<len){
		if (RecurDepth && str.L && !str.Cmp(&last_str, false)){
			(*RecurDepth)++;
		}
		if (!str.Cmp("FORM", 4, false) || !str.Cmp("TABLE", 5, false) 
			||!str.Cmp("DIV", 3, false) || !str.Cmp("DL", 2, false) 
			|| !str.Cmp("A", 1, false) || !str.Cmp("FRAME", 5, false)  ){
			if (FormatDepth) (*FormatDepth)++;
			if (!str.Cmp("FORM", 4, false)) turn = 1;
			else if (!str.Cmp("TABLE", 5, false)) turn = 2;
			else if (!str.Cmp("DIV", 3, false)) turn = 4;
		}else if ((turn==1||turn ==0) && (!str.Cmp("INPUT",5,false) ||!str.Cmp("SELECT",6,false)|| !str.Cmp("TEXTAREA", 8, false))){
			if (SecondFormatDept) (*SecondFormatDept)++;
		//}else if (((turn ==2||turn ==0) && (!str.Cmp("TR", 2, false) || !str.Cmp("TD",2,false) )){
		//	if (SecondFormatDept) (*SecondFormatDept)++;
		}else if ((turn ==2||turn ==0) && !str.Cmp("TR", 2, false)){ 
			if (SecondFormatDept) (*SecondFormatDept)++;
			turn=3;
		}else if ((turn ==3||turn ==0) && !str.Cmp("TD", 2, false)){
			if (SecondFormatDept) (*SecondFormatDept)++;
			turn=5;
		}else if (!str.Cmp("HTML", 4, false) || !str.Cmp("BODY", 4, false)  ){
			if (BadDepth) (*BadDepth)++;
			turn=5;
		}else if (str.P){
			turn = 5;
		}
		if (tagidx > *maxidx) *maxidx=tagidx;
		/*
		if (turn==0 || turn==1 ){
			if (!str.Cmp("FORM", 4, false) || !str.Cmp("TABLE", 5, false) ||!str.Cmp("DIV", 3, false)  ){
				if (FormatDepth) (*FormatDepth)++;
				turn = 1;
			}else if (!str.isNULL()){
				turn = 2;
			}
		}
		if (turn==3){
			if (!str.Cmp("TD",2,false) || !str.Cmp("TR", 2, false)){
				if (SecondFormatDept) (*SecondFormatDept)++;
			}
			turn = 4;
		}
		if (turn==2){
			if (!str.Cmp("INPUT",5,false) || !str.Cmp("TEXTAREA", 8, false)){
				if (SecondFormatDept) (*SecondFormatDept)++;
				turn = 3;
			}else if (!str.Cmp("TABLE", 5, false) || !str.Cmp("DIV", 3, false) || !str.Cmp("DL", 2, false)){
				turn = 1;
				continue;
			}else{
				turn = 4;
			}
		}
		if (turn==4){
			if (!str.Cmp("FORM", 4, false) || !str.Cmp("TABLE", 4, false) || !str.Cmp("DIV", 3, false) || !str.Cmp("DL", 2, false)){
				if (FormatDepth) (*FormatDepth)++;
				turn = 1;
			}else{
				turn = 5;
			}
		}
		*/

		i=j;
		int have_condition=0;
		while (i<len && Source[i] != '<') { 
			if (Source[i]=='(') have_condition=1;
			i++;
		}
		if (have_condition && GoodDepth) *GoodDepth++; //previous condition
		if (i>=len) break;
		if (Depth) (*Depth)++;
		i++;
		for (j=i; j<len && Source[j]!='>' && Source[j]!=' ' && Source[j]!='\t' && Source[j]!='\r' && Source[j]!='\n'; j++);
		tagidx=0;
		if (j<len) sscanf(Source+j+1, "%d", &tagidx);
		last_str=str;
		str.Set(Source+i, j-i, true);
	}
	return true;
}

int HtmlQLParser::searchPlainPairScopePreviousTag(ReferData* data, long pos, HTQLScope* PreviousTag){
	long k=pos-1;
	long j=0;
	while (k>=0 && data->P[k]==' ') k--;
	j=k+1;
	while (k>=0 && data->P[k]!=' ' && !isdigit(data->P[k]) && !isalpha(data->P[k])) k--;
	k++;
	ReferData str;
	char* p1;
	if (k<j){
		str.Set(data->P+k, j-k, true);
		p1 = strstr(data->P+PreviousTag->SourceOffset, str.P);
		if (p1 && p1-data->P >= k){
			PreviousTag->E.Set(data->P+k, (j-k>=0)?(j-k):0, false);
		}
	}
	if (PreviousTag->E.isNULL() && PreviousTag->NextTag){
		str.Set(PreviousTag->NextTag->S.P, PreviousTag->NextTag->S.L, true);
		p1 = strstr(data->P+PreviousTag->SourceOffset, str.P);
		if (p1 && p1-data->P >= pos){
			PreviousTag->E.Set(PreviousTag->NextTag->S.P, PreviousTag->NextTag->S.L, false);
		}
	}

	searchPlainPairScopePreviousSameTag(PreviousTag);

	/* create a new end-tag??
	if (!PreviousTag->E.isNULL()){
		HTQLScope* newTag1=new HTQLScope;
		if (!newTag1) return tagMEMORY;
		newTag1->S.Set(PreviousTag->E.P, PreviousTag->E.L, false);
		newTag1->SourceOffset = PreviousTag->E.P - data->P;
		newTag1->TagType = HTQLTagDataSyntax::synQL_END_TAG; // synQL_END_TAG or synQL_START_TAG
		newTag1->SerialNo = PreviousTag->SerialNo; // SerialNo;
		// newTag->SerialNo = ++SerialNo;
		newTag1->PreviousTag = PreviousTag;
		PreviousTag->NextTag = newTag1;
		newTag1->EnclosedTag = PreviousTag;
		PreviousTag->EnclosedTag = newTag1;
		newTag1->NextTag = newTag;
		PreviousTag = newTag1;
	}
	*/
	return 0;
}

int HtmlQLParser::searchPlainPairScopePreviousSameTag(HTQLScope* tag_scope){
	HTQLScope* newTag1;
	for (newTag1 = tag_scope->PreviousTag; newTag1; newTag1=newTag1->PreviousTag){
		if (!newTag1->S.Cmp(&tag_scope->S, true) 
			&& newTag1->E.Cmp(&tag_scope->E,true) ) {
			tag_scope->SerialNo = newTag1->SerialNo + 1;
			break;
		}
	}
	return 0;
}

int HtmlQLParser::searchPlainPairScope(ReferData* data, HTQLScope** tag_scope){
	HTQLScope ** pTags = tag_scope;
	HTQLScope*  newTag;
	HTQLScope* PreviousTag = NULL;
	HTQLScope* LastNoEndTag = NULL;
	int SerialNo=0;

	*tag_scope = 0;

	long SourceOffset = 0;

	long pos=0;
	long newpos;
	char delimit_char[]="\t:|-=\n\r";
	char newline_delimit[]=":|=";
//	char newrec_delimit[]=":";
	ReferData str;
	int IsTag=false;
	int IsNewLine=true;
//	int IsNewRec=false;
//	int IsBold=false;
	int IsPlainTag = false;
	while (pos < data->L){
		if (data->P[pos] == '<'){
			IsTag=true;
			newpos=pos+1;
			while (newpos<data->L && data->P[newpos]!='>') newpos++;
			newpos++;
			//str.Set(data->P+pos, newpos-pos, false);
			//if (!tStrOp::strNcmp(str.P, "<BR",3,false) || !tStrOp::strNcmp(str.P, "<P",2,false) || !tStrOp::strNcmp(str.P, "<TD",3,false) )
			if (TagOperation::isTag(data->P+pos, "BR") || TagOperation::isTag(data->P+pos, "P") || TagOperation::isTag(data->P+pos, "TD") )	
				IsNewLine = true;
/*
			if (TagOperation::isTag(data->P+pos, "TR") ){
				IsNewRec = true;
			}else if (data->P[pos+1]=='/' && TagOperation::isAN_Str(data->P+pos+2, "TD")){
				IsNewRec = false;
			}else if (TagOperation::isTag(data->P+pos, "B") )
				IsBold = true;
			else if (data->P[pos+1]=='/' && TagOperation::isAN_Str(data->P+pos+2, "B")){
				IsBold = false;
			}
*/
			pos=newpos;
		}else if (data->P[pos] == '\r' || data->P[pos]=='\n'){
			pos++;
			IsNewLine=true;
		}else if ((IsTag||IsNewLine) && isalpha(data->P[pos]) && isupper(data->P[pos])){
			IsPlainTag = false;
			newpos=pos+1;
			while (isalpha(data->P[newpos]) && isupper(data->P[newpos])) newpos++;
			if (IsNewLine && isalpha(data->P[newpos])){
				while (isalpha(data->P[newpos])) newpos++;
				if (strchr(newline_delimit, data->P[newpos]))
					IsPlainTag = true;
			}else if (newpos-pos>1 && strchr(delimit_char, data->P[newpos])){
					IsPlainTag = true;
			}
			if (IsPlainTag){
				newTag=new HTQLScope;
				if (!newTag) return tagMEMORY;
				newTag->S.Set(data->P + pos, newpos-pos+1, false);
				newTag->SourceOffset = SourceOffset + pos;
				newTag->TagType = HTQLTagDataSyntax::synQL_START_TAG; // synQL_END_TAG or synQL_START_TAG
				newTag->SerialNo = 1; //++ SerialNo;

				//searching for the end-tag for the previous tag;
				if (LastNoEndTag && LastNoEndTag->E.isNULL() ){
					if (PreviousTag){
						PreviousTag->NextTag = newTag;
						newTag->PreviousTag = PreviousTag;
					}

					searchPlainPairScopePreviousTag(data, pos, PreviousTag);
				}
				LastNoEndTag = newTag;

				newTag->PreviousTag = PreviousTag;
				PreviousTag = newTag;

				*pTags = newTag;
				pTags = &(newTag->NextTag);
			}
			pos = newpos;
			IsNewLine = false;
			IsTag=false;
/*		}else if ((IsNewRec||IsBold) && strchr(data->P[pos], newrec_delimit) ){
			IsPlainTag = false;
			newpos=pos+1;
			while (isalpha(data->P[newpos]) && isupper(data->P[newpos])) newpos++;
			if (IsNewLine && isalpha(data->P[newpos])){
				while (isalpha(data->P[newpos])) newpos++;
				if (strchr(newline_delimit, data->P[newpos]))
					IsPlainTag = true;
			}else if (strchr(delimit_char, data->P[newpos])){
					IsPlainTag = true;
			}
			if (IsPlainTag){
				newTag=new HTQLScope;
				if (!newTag) return tagMEMORY;
				newTag->S.Set(data->P + pos, newpos-pos+1, false);
				newTag->SourceOffset = SourceOffset + pos;
				newTag->TagType = HTQLTagDataSyntax::synQL_START_TAG; // synQL_END_TAG or synQL_START_TAG
				newTag->SerialNo = 1; //++ SerialNo;

				//searching for the end-tag for the previous tag;
				if (LastNoEndTag && LastNoEndTag->E.isNULL() ){
					if (PreviousTag){
						PreviousTag->NextTag = newTag;
						newTag->PreviousTag = PreviousTag;
					}

					searchPlainPairScopePreviousTag(data, pos, PreviousTag);
				}
				LastNoEndTag = newTag;

				newTag->PreviousTag = PreviousTag;
				PreviousTag = newTag;

				*pTags = newTag;
				pTags = &(newTag->NextTag);
			}
			pos = newpos;
			IsNewLine = false;
			IsTag=false;
			*/
		}else if (isdigit(data->P[pos])){
			newpos = pos+1;
			if (pos>0 && ( data->P[pos-1] == '$' ) ){ // is currency;
				while (isdigit(data->P[newpos])||data->P[newpos]=='.'||data->P[newpos]==',') newpos++;
				//while (!isalpha(data->P[newpos])) newpos++;
				//while (!isdigit(data->P[newpos]) && data->P[newpos] != '$' ) newpos--;
				//newpos++;
				if (!isalpha(data->P[newpos])){
					newTag=new HTQLScope;
					if (!newTag) return tagMEMORY;
					newTag->S.Set(data->P + pos-1, 1, false);
					newTag->SourceOffset = SourceOffset + pos-1;
					newTag->TagType = HTQLTagDataSyntax::synQL_START_TAG; // synQL_END_TAG or synQL_START_TAG
					newTag->SerialNo = 1; //++ SerialNo;

					newTag->E.Set(data->P + newpos, 1, false);

					newTag->PreviousTag = PreviousTag;
					PreviousTag = newTag;

					*pTags = newTag;
					pTags = &(newTag->NextTag);

					searchPlainPairScopePreviousSameTag(newTag);
				}
			}
			pos = newpos;
			IsNewLine = false;
//			IsNewRec = false;
			IsTag=false;
		}else{
			if (!tStrOp::isSpace(data->P[pos])){
				IsNewLine = false;
				IsTag=false;
			}
			pos++;
		}
	}
	if (PreviousTag && PreviousTag->TagType == HTQLTagDataSyntax::synQL_START_TAG 
		&& PreviousTag->E.isNULL() ){
		searchPlainPairScopePreviousTag(data, data->L, PreviousTag);
	}

	return 0;
}

int HtmlQLParser::createHtqlTailView(char* htql, ReferData* data, int max_pos, int se_max_pos, ReferData* view_htql, int* cols, long* rows){
	HtqlExpression expr1, expr2;
	expr1.setExpr(htql);

	HtmlQL ql;
	ql.setSourceData(data->P, data->L, false);
	if (max_pos>1) { //replace prehead with the best htql (not for best view)
		ReferData prehead; prehead=expr1.getTagSelectionHeadTo(max_pos-1);
		ReferData rest; rest=expr1.getTagSelectionTailFrom(max_pos);

		//change prehead and max_pos if there is a better head
		ql.setQuery(prehead.P);
		if (!ql.isEOF()){ //find the best head htql (not for best view)
			char buf[128];
			sprintf(buf, "&find_htql(%d)./'\n'/", HtmlQL::heuBEST);
			ql.dotQuery(buf);
			if (!ql.isEOF()){
				prehead=ql.getValue(1);
				expr2.setExpr(prehead.P);
				max_pos=expr2.getPatternsNum()+1;
				//expr2.getTagSelectionRange(max_pos, &range1, &range2);
			}
		}
		prehead+=".";
		prehead+=rest;
		expr1.setExpr(prehead.P, prehead.L);
	}

	int range1=0, range2=0;
	expr1.getTagSelectionRange(max_pos, &range1, &range2);

	ReferData head;
	head=expr1.getTagSelectionHeadTo(max_pos);
	ReferData tail;
	tail = expr1.getTagSelectionTailFrom(max_pos+1);
	expr2.setExpr(head.P, head.L);

	long rows_count=0;
	int cols_count;
//	char* p;
	ReferData new_htql;
	view_htql->reset();
	int last_cols=0;
	long last_rows=0;
	ReferData last_htql;
	int look_ahead = 0;
	int max_look_ahead = 2;
	while (look_ahead<max_look_ahead){
		new_htql = expr2.replaceTagSelectionRange(max_pos, look_ahead? (range1 + look_ahead):1, 0);
		head = expr2.replaceTagSelectionRange(max_pos, range1 + look_ahead, -1);
		int col=appendHtqlLeaveViewSchema(&head, &tail, &ql, &new_htql);
		new_htql += "&refine_null_fields(0.7)"; //if has COLUMN0: 0.65, else 0.7
		ql.setQuery(new_htql.P);
		cols_count= ql.getFieldsCount();
		/*rows_count= 0;
		int i=0,j=0;
		while (!ql.isEOF()){
			j=0;
			for (i=0; i<cols_count; i++){
				if ((p=ql.getValue(i)) && *p) j++; 
			}
			if (j/(float)cols_count > 0.8) rows_count++; 
			ql.moveNext();
		}*/
		rows_count = ql.getTuplesCount();
		if (cols &&  cols_count < *cols){
			look_ahead++;
			if (cols_count > last_cols){
				last_htql.Set(new_htql.P, new_htql.L, true);
				last_cols = cols_count;
				last_rows = rows_count;
			}
			continue;
		}
		if (rows && rows_count < *rows){
			look_ahead++;
			if (rows_count > last_rows){
				last_htql.Set(new_htql.P, new_htql.L, true);
				last_cols = cols_count;
				last_rows = rows_count;
			}
			continue;
		}else
			break;
	}
	if (look_ahead<max_look_ahead){
		view_htql->Set(new_htql.P, new_htql.L, true);
		if (rows) *rows = rows_count;
		if (cols) *cols = cols_count;
	}else{
		view_htql->Set(last_htql.P, last_htql.L, true);
		if (rows) *rows = last_rows;
		if (cols) *cols = last_cols;
	}

	return 0;
}

int HtmlQLParser::appendHtqlLeaveViewSchema(ReferData* htql_head, ReferData* htql_tail, HTQL* ql, ReferData* view_htql, int* cols){
	ReferData schema;
	ReferData col0;
	ReferLinkHeap htqls;
	htqls.setSortOrder(SORT_ORDER_VAL_STR_INC);
	htqls.setDuplication(true);
	ReferLinkHeap fields_index;
	//htqls.Type = tStack::ordINCKEY;

	int col=1;
	char buf[128];
	char* name, *tmp, *p3;
	ReferData htql;
	htql.Set(htql_head->P, htql_head->L, false);
	htql += ":tx &find_leaves_htql .<leave_htql>{name=:name; htql=:htql; order=:order}";
	ql->setQuery(htql.P);
	schema = "{\n";
	while(!ql->isEOF()) 
	{
		name = ql->getValue(1);
		tmp = ql->getValue(2);
		p3 = ql->getValue(3); 
		if (!tmp || !tmp[0] || htqls.findValue(tmp)) {
			ql->moveNext();
			continue;
		}
		int order=0; 
		if (p3 && isdigit(*p3)) order=atoi(p3); 
		if (!name || !name[0] || fields_index.findName(name)) {
			sprintf(buf, "COLUMN%d", col); 
			name=buf;
		}

		htqls.add(name, tmp, order); //order of field
		fields_index.add(name, tmp, col-1); //index of field

		schema += name;
		schema += "=" ;
		schema += tmp;
		schema += ";\n";

		ql->moveNext();
		col++;
	}
	schema += "}";

	//select the best field with the same order 
	htqls.setSortOrder(SORT_ORDER_NUM_INC);
	htqls.resort();
	ReferLink* link, *last_link=0; 
	ReferLink* link1, *last_link1=0;
	int has_duplicate=false; //check if there are duplicate order fields
	for (link=(ReferLink*) htqls.moveFirst(); link; link=(ReferLink*) htqls.moveNext()){
		if (last_link && link->Data==last_link->Data) has_duplicate=true; 
		last_link=link; 
	}
	if (has_duplicate && view_htql->L){
		ql->setQuery(view_htql->P); 
		ql->dotQuery(schema.P); 
		int field=0;
		//count non-empty fields
		long* fields_count=new long[col]; 
		for (field=0; field<col; field++) fields_count[field]=0; 
		for (ql->moveFirst(); !ql->isEOF(); ql->moveNext()){
			for (field=0; field<col; field++){
				char* val=ql->getValue(field+1);
				if (val && val[0]) fields_count[field]++;
			}
		}

		//compare fields of the same order
		last_link=0; 
		for (link=(ReferLink*) htqls.moveFirst(); link; link=(ReferLink*) htqls.moveNext()){
			if (last_link && last_link->Data==link->Data){
				last_link1=fields_index.findName(&last_link->Name); 
				link1=fields_index.findName(&link->Name); 
				if (fields_count[link1->Data]<fields_count[last_link1->Data]){
					link->Value.reset(); //clear the value 
				}else if (fields_count[link1->Data]==fields_count[last_link1->Data]
					&& link->Value.L < last_link->Value.L){
					link->Value.reset(); //clear the value 
				}else{
					last_link->Value.reset(); 
					last_link=link; 
				}
			}else{
				last_link=link; 
			}
		}
		//regenerate schema
		col=1; 
		schema = "{\n";
		for (link=(ReferLink*) htqls.moveFirst(); link; link=(ReferLink*) htqls.moveNext()){
			if (!link->Value.L) continue; 
			
			if (!tStrOp::strNcmp(link->Name.P, "COLUMN", 6, true)){
				sprintf(buf, "COLUMN%d", col); 
				name=buf;
			}else{
				name=link->Name.P; 
			}
			schema += name;
			schema += "=" ;
			schema += link->Value;
			schema += ";\n";

			col++;
		}
		schema += "}";
		if (fields_count) delete[] fields_count; 
	}

	*view_htql += schema;
	if (cols) *cols=col;

	return col;
}

int HtmlQLParser::findDistingishedPlainData(char* p, ReferLinkHeap* results, int min_data_len){
	results->setSortOrder(SORT_ORDER_NUM_DEC);
	results->setDuplication(true);
	//results->Type = tStack::ordDECDATA;
	long len=strlen(p);

	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(p, &len, false);

	char tmp[30];
	int count=0;
	unsigned long length=0;
	unsigned long length1=0;
	int IsSkip=false;
	int IsBody=true;
	if (!tStrOp::strNcmp(DataSyntax.Sentence, "HTTP:", 5, false))
		IsBody=false;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA && !IsSkip && IsBody){
			if (DataSyntax.StartLen>min_data_len) {
				sprintf(tmp, "%ld", DataSyntax.Start);
				ReferData data, key;
				data.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, true);
				key.Set(tmp, strlen(tmp), false);
				results->add(&key, &data, DataSyntax.StartLen);
				//results->set(DataSyntax.StartLen, tmp, data.P);
				//printf("%d: offset=%ld, type=TEXT, L=%ld\n", ++count, DataSyntax.Start, DataSyntax.StartLen);
				//printf("%s\n", data.P);
			}
			length += DataSyntax.StartLen;
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG || 
			DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (!tStrOp::strNcmp(DataSyntax.Sentence+DataSyntax.Start, "<Script", 7, false)) {
				IsSkip = true;
			}else if (!tStrOp::strNcmp(DataSyntax.Sentence+DataSyntax.Start, "</Script", 8, false)) {
				IsSkip = false;
			}else if (!IsBody && !tStrOp::strNcmp(DataSyntax.Sentence+DataSyntax.Start, "<body", 5, false)) {
				IsBody = true;
			}
			//if (DataSyntax.StartLen>128) 
			//printf("%d: offset=%ld, type=TAG, L=%ld, tag=%s\n", ++count, DataSyntax.Start, DataSyntax.StartLen, data.P);
			length1 += DataSyntax.StartLen;
		}

		DataSyntax.match();
	}
	//printf("\nlength=%ld, taglength=%ld\n", length, length1);
	return 0;
}

int HtmlQLParser::findBestViewTagPos(char* htql, ReferData* data, int* max_pos, long* max, int* se_max_pos, long* se_max, double* similarity, long* tuple_score){
	HtqlExpression expr;
	expr.setExpr(htql);
	int tag_member_number = expr.getTagSelectionsNum();
	
	//get item count and max and second_max and their index
	*max=0;
	*se_max=0;
	*max_pos=0;
	*se_max_pos=0;
	if (similarity) *similarity=1.0;
	if (tuple_score) *tuple_score=0;
	int pos;
	char* p1;
	int range1=0, range2=0;
	long count=0;
	long tscore=0;
	double best_eval=0.0;
	double se_eval=0.0;

	HtmlQL ql;
	ql.setSourceData(data->P, data->L, false);

	for (pos=tag_member_number; pos>=1; pos--){
		expr.getTagSelectionRange(pos, &range1, &range2);
		p1 = expr.replaceTagSelectionRange(pos, 1, 0);
		if (!p1) continue;

		ql.setQuery(p1);
		count = ql.getTuplesCount();
		double simil = computeTuplesSimilarity(&ql, 1, &tscore);
		double eval = count*simil*simil*simil;

		//if (count > *max){
		if (eval > best_eval || (eval==best_eval && eval>1.5) ){
			se_eval=best_eval;
			best_eval = eval;

			*se_max = *max;
			*se_max_pos = *max_pos;
			if (similarity) *similarity=simil;
			if (tuple_score) *tuple_score = tscore;
			*max = count;
			*max_pos = pos;
		//}else if (count > *se_max){
		}else if (eval > se_eval){
			se_eval=eval;
			*se_max = count;
			*se_max_pos = pos;
		}
	}
	return 0;
}

int HtmlQLParser::rankPathHtqlsByViewItems(HTQL* ql, ReferLinkHeap* path_htqls, ReferLinkHeap* rank_htqls, int max_htql_num){
	rank_htqls->setSortOrder(SORT_ORDER_NUM_DEC);
	rank_htqls->setDuplication(true);
	//rank_htqls->Type=tStack::ordDECDATA;
	ReferLink* t;	
	int count=0;
	char buf[128];
	ReferData value;
	for ( t = (ReferLink*) path_htqls->moveFirst(); t; t= (ReferLink*) path_htqls->moveNext() )
	{
		ql->setQuery(t->Name.P);

		if (ql->isEOF())
			continue;

		int max_pos=0, se_max_pos=0;
		long max=0, se_max=0;
		long val = 0;
		double similarity=1.0;
		long tscore;
		findBestViewTagPos(t->Name.P, &ql->Parser->SourceData, &max_pos, &max, &se_max_pos, &se_max, &similarity, &tscore);
		//t->Data is the evaluation of repeats or quality of path
		if (max_pos < se_max_pos){
			if (se_max > 1){
				val = (long) ((max+1)*(se_max+1)*(tscore+1)*similarity*similarity);
				//val = (long) ((max+1)*(se_max+1)*(tscore+1)/max/se_max); 
			}else{
				val = (long) ((max+1)*(tscore+1)*similarity*similarity);
				//val = (long) (tscore/2); // max
			}
		}else{
			val = (long) ((max+1)*(tscore+1)*similarity*similarity);
			//val = (long) (tscore);  //max;
		}
		ReferLink* t1=(ReferLink*) rank_htqls->moveFirst();
		if (t1 && t1->Data == val){
			val --;
		}
		sprintf(buf, "%d:%ld:%d:%ld:%lf:%ld", max_pos, max, se_max_pos, se_max, similarity, tscore);
		value.Set(buf, strlen(buf),false);
		rank_htqls->add(&t->Name, &value, val);
		//rank_htqls->set(val, t->Key, buf);
		//createHtqlTailView(t->Key, &ql->Parser->SourceData, int max_pos, int se_max_pos, ReferData* view_htql, int* cols, long* rows);

		if(count++>=max_htql_num) 
			break;
	}
	return 0;
}

#include "htwrapper.h" 

int HtmlQLParser::rankPathHtqlsToViews(HTQL* ql, ReferLinkHeap* path_htqls, ReferLinkHeap* view_htqls, int show_view_num, int maximum_views){
	ReferLinkHeap rank_byitems;
	rankPathHtqlsByViewItems(ql, path_htqls, &rank_byitems, maximum_views);
	int max_pos=0, se_max_pos=0, cols=0;
	long rows=0;
	long max=0, se_max=0, val=0;
	ReferLink* t;
	view_htqls->setSortOrder(SORT_ORDER_NUM_DEC);
	view_htqls->setDuplication(true);
	ReferData view_htql;
	int i;
	ReferLinkHeap exist_head;
	exist_head.setSortOrder(SORT_ORDER_KEY_STR_INC);
	//exist_head.Type = tStack::ordINCKEY;
	HtqlExpression expr1;
	for (i=0, t=(ReferLink*) rank_byitems.moveFirst(); t && i<show_view_num; i++, t=(ReferLink*) rank_byitems.moveNext() ){
		max_pos=se_max_pos=0;
		max=se_max=0;
		cols=rows=2;
		sscanf(t->Value.P, "%d:%ld:%d:%ld", &max_pos, &max, &se_max_pos, &se_max);

		expr1.setExpr(t->Name.P);
		ReferData head; head = expr1.getTagSelectionHeadTo(max_pos);
		if (!exist_head.findName(&head)){
			exist_head.add(&head, 0, 0);
			//exist_head.set(0, head, "");

			createHtqlTailView(t->Name.P, &ql->Parser->SourceData, max_pos, se_max_pos, &view_htql, &cols, &rows);

			//expr1.setExpr(head);
			//expr1.replaceTagSelectionRange(max_pos, 1, 0);
			//if (!view_htql.P) continue;
			ql->setQuery(view_htql.P);
			long count = ql->getTuplesCount();
			long tscore=0, tscore1;
			int fields = ql->getFieldsCount();

			int j;
			double simil;
			for (j=1; j<=fields && j<10; j++){
				simil=computeTuplesSimilarity(ql, j, &tscore1);
				tscore += (long) (tscore1*simil*simil);
			}
			simil=computeNullTuplesSimilarity(ql);
			tscore = (long) (tscore*simil*simil);

			val = (long) (rows*cols+cols-rows-cols/(rows-0.5))*(tscore); // + t->Data;

			//compute new htql expression
//			ql->Parser->formatHtmlResult("c:\\111.html");
			/*
			long* fieldnullcounts=0;
			computeNullFieldCounts(ql, &fieldnullcounts);
			for (j=fields; j>0; j--){
				if (fieldnullcounts[j-1]/(double)count > 0.65) {
					expr1.setExpr(view_htql.P);
					view_htql = expr1.deleteSchemaField(1, j);
				}
			}*/
			/*
			ReferData hyper_con;
			ReferData text_con;
			long cost=0;
			for (j=fields; j>0; j--){
				int to_delete = false;
				HtWrapper::computeFieldHyperConStr(ql, j, &hyper_con, &cost, 10);
				if (hyper_con.isNULL() || !hyper_con.P[0]) to_delete = true;
				else if (cost/(double)hyper_con.L/2.0 >0.65) to_delete = true;
				else if (!strchr(hyper_con.P, dhyconPLAIN) && !strchr(hyper_con.P, dhyconS_TAG_IMAGE)) to_delete = true;
				else if (strlen(hyper_con.P)==1) {
					if (hyper_con.P[0] == dhyconS_TAG || hyper_con.P[0] == dhyconE_TAG) to_delete = true;
					else if (hyper_con.P[0] == dhyconPLAIN ) {
						HtWrapper::computeFieldTextConStr(ql, j, &text_con, &cost, 10);
						if (text_con.isNULL() || !text_con.P[0]) to_delete = true;
						else if (cost/(double)text_con.L/2.0 >0.65) to_delete=true;
					}
				}
				if (to_delete){
					expr1.setExpr(view_htql.P);
					view_htql = expr1.deleteSchemaField(1, j);
				}
			}

			ql->setQuery(view_htql.P);
			fields = ql->getFieldsCount();
			ql->Parser->formatHtmlResult("c:\\222.html");
			*/

			view_htqls->add(&view_htql, &t->Name, val);
			//view_htqls->set(val, view_htql.P, t->Key);
		}
	}

	return 0;
}
/*
#include "hmmMatrix.h"

int HtmlQLParser::builtHyperTagsForAlign(ReferData* source, ReferLinkHeap* stags, ReferLinkHeap* etags, void***sourcetag, long**sourcepos, long* source_count){
	if (!stags->Total){
		stags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		stags->setDuplication(false);
	}
	if (!etags->Total){
		etags->setSortOrder(SORT_ORDER_KEY_STR_INC);
		etags->setDuplication(false);
	}

	*sourcetag = (void**) malloc(sizeof(void*)*128);
	memset(sourcetag, 0, sizeof(void*)*128);
	*sourcepos = (long*) malloc(sizeof(long)*128);
	memset(sourcepos, 0, sizeof(void*)*128);
	long sourcetag_mal = 128;
	*source_count=0;

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
				void** tmp = (void**) malloc(sizeof(void*)*(sourcetag_mal+128) );
				memset(tmp, 0, sizeof(void*)*(sourcetag_mal+128));
				memcpy(tmp, *sourcetag, sizeof(void*)*(sourcetag_mal));
				free(*sourcetag);
				*sourcetag = tmp;

				long* tmp1 = (long*) malloc(sizeof(long)*(sourcetag_mal+128) );
				memset(tmp1, 0, sizeof(long)*(sourcetag_mal+128));
				memcpy(tmp1, *sourcepos, sizeof(long)*(sourcetag_mal));
				free(*sourcepos);
				*sourcepos = tmp1;

				sourcetag_mal+=128;
			}
		}

		DataSyntax.match();
	}
	return 0;
}

int HtmlQLParser::alignHyperTagsText(void**sourcetag, long* sourcepos, long source_count, void**strtag, long* strpos, long str_count, ReferData* results){
	HmmMatrix costs;
	costs.setCellSize(sizeof(int));
	costs.setCol(str_count+1);
	costs.setRow(source_count+1);
	HmmMatrix direction;
	direction.setCellSize(sizeof(int));
	direction.setCol(str_count+1);
	direction.setRow(source_count+1);
	long i, j;
	int val1, val2;
	int max_cost=0;
	for (i=1; i<=source_count; i++){
		for (j=1; j<=str_count; j++){
			if (sourcetag[i-1] == strtag[j-1]) {
				(*(int*) costs(i, j)) = (*(int*) costs(i-1, j-1)) + 1;
				(*(int*) direction(i,j)) = 3;
			} else {
				val1 = (*(int*) costs(i-1, j-1)) - 2; //replacement
				val2 = 3;
				if ((*(int*) costs(i, j-1)) -1 > val1){
					val1 = (*(int*) costs(i, j-1)) - 1; //insert;
					val2 = 1;
				}
				if ((*(int*) costs(i-1, j)) -1 > val1){
					val1 = (*(int*) costs(i-1, j)) - 1; //insert;
					val2 = 2;
				}
				if (val1<0) val1=0;
				(*(int*) costs(i, j)) = val1;
				(*(int*) direction(i,j)) = val2;
			}
			if ((*(int*) costs(i, j)) > max_cost){
				max_cost = (*(int*) costs(i, j));
			}
		}
	}

	costs.dumpMatrix("c:\\matrix.txt", __FILE__, HmmMatrix::IntMatrix);

	*results="<max_positions>";
	char buf[128];
	for (i=1; i<=source_count; i++){
		for (j=1; j<=str_count; j++){
			if ((*(int*) costs(i, j)) == max_cost){
				int d=0;
				long k, l;
				for (k=i, l=j; k>=0 && l>=0 && *(int*) costs(k, l)> 0; ){
					d= *(int*) direction(k,l);
					if (d==3) { k--; l--;}
					else if (d==2) k--;
					else if (d==1) l--;
					else {k--; l--;}
				}
				if (d==3) { k++; l++;}
				else if (d==2) k++;
				else if (d==1) l++;
				else {k++; l++;}
				sprintf(buf, "<position source_pos=\"%ld\" str_pos=\"%ld\" cost=\"%d\" from_pos=\"%ld\" />", 
					sourcepos[i-1], strpos[j-1], max_cost, sourcepos[k-1] );
				*results+=buf;
			}
		}
	}
	*results+= "</max_positions>";
	return 0;
}


int HtmlQLParser::alignHyperTagsText(ReferData* source, ReferData* str, ReferData* results){
	ReferLinkHeap stags;
	ReferLinkHeap etags;
	void** sourcetag=0;
	long* sourcepos=0;
	long source_count=0;
	void** strtag=0;
	long* strpos=0;
	long str_count=0;
	builtHyperTagsForAlign(source, &stags, &etags, &sourcetag, &sourcepos, &source_count);
	builtHyperTagsForAlign(str, &stags, &etags, &strtag, &strpos, &str_count);
	alignHyperTagsText(sourcetag, sourcepos, source_count, strtag, strpos, str_count, results);
	if (sourcetag) free(sourcetag);
	if (sourcepos) free(sourcepos);
	if (strtag) free(strtag);
	if (strpos) free(strpos);

	return 0;
	*/
/*
	stags.setSortOrder(SORT_ORDER_KEY_STR_INC);
	stags.setDuplication(false);
	etags.setSortOrder(SORT_ORDER_KEY_STR_INC);
	etags.setDuplication(false);

	void** sourcetag = (void**) malloc(sizeof(void*)*128);
	memset(sourcetag, 0, sizeof(void*)*128);
	long* sourcepos = (long*) malloc(sizeof(long)*128);
	memset(sourcepos, 0, sizeof(void*)*128);
	long sourcetag_mal = 128;
	long source_count=0;

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
			link = stags.findName(&label);
			if (!link) link=stags.add(&label, 0, 0);
			
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			p=TagOperation::getLabel(DataSyntax.Sentence+DataSyntax.Start, &len);
			label.Set(p, len, false);
			link = etags.findName(&label);
			if (!link) link=etags.add(&label, 0, 0);
			
		}

		if (link){
			sourcetag[source_count]=link;
			sourcepos[source_count++]=DataSyntax.Start;
			if (source_count >=sourcetag_mal){
				void** tmp = (void**) malloc(sizeof(void*)*(sourcetag_mal+128) );
				memset(tmp, 0, sizeof(void*)*(sourcetag_mal+128));
				memcpy(tmp, sourcetag, sizeof(void*)*(sourcetag_mal));
				free(sourcetag);
				sourcetag = tmp;

				long* tmp1 = (long*) malloc(sizeof(long)*(sourcetag_mal+128) );
				memset(tmp1, 0, sizeof(long)*(sourcetag_mal+128));
				memcpy(tmp1, sourcepos, sizeof(long)*(sourcetag_mal));
				free(sourcepos);
				sourcepos = tmp1;

				sourcetag_mal+=128;
			}
		}

		DataSyntax.match();
	}

	void** strtag = (void**) malloc(sizeof(void*)*128);
	memset(strtag, 0, sizeof(void*)*128);
	long* strpos = (long*) malloc(sizeof(long)*128);
	memset(strpos, 0, sizeof(long)*128);
	long strtag_mal = 128;
	long str_count=0;

	DataSyntax.setSentence(str->P, &str->L, false);
	
	while (DataSyntax.Type != QLSyntax::synQL_END){
		link=0;
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
			p=TagOperation::getLabel(DataSyntax.Sentence+DataSyntax.Start, &len);
			label.Set(p, len, false);
			link = stags.findName(&label);
			if (!link) link=stags.add(&label, 0, 0);
			
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			p=TagOperation::getLabel(DataSyntax.Sentence+DataSyntax.Start, &len);
			label.Set(p, len, false);
			link = etags.findName(&label);
			if (!link) link=etags.add(&label, 0, 0);
			
		}

		if (link){
			strtag[str_count]=link;
			strpos[str_count++]=DataSyntax.Start;
			if (str_count >=strtag_mal){
				void** tmp = (void**) malloc(sizeof(void*)*(strtag_mal+128) );
				memset(tmp, 0, sizeof(void*)*(strtag_mal+128));
				memcpy(tmp, sourcetag, sizeof(void*)*(strtag_mal));
				free(strtag);
				strtag = tmp;

				long* tmp1 = (long*) malloc(sizeof(long)*(strtag_mal+128) );
				memset(tmp1, 0, sizeof(long)*(strtag_mal+128));
				memcpy(tmp1, strpos, sizeof(long)*(strtag_mal));
				free(strpos);
				strpos = tmp1;

				strtag_mal+=128;
			}
		}

		DataSyntax.match();
	}

	HmmMatrix costs;
	costs.setCellSize(sizeof(int));
	costs.setCol(str_count+1);
	costs.setRow(source_count+1);
	HmmMatrix direction;
	direction.setCellSize(sizeof(int));
	direction.setCol(str_count+1);
	direction.setRow(source_count+1);
	long i, j;
	int val1, val2;
	int max_cost=0;
	for (i=1; i<=source_count; i++){
		for (j=1; j<=str_count; j++){
			if (sourcetag[i-1] == strtag[j-1]) {
				(*(int*) costs(i, j)) = (*(int*) costs(i-1, j-1)) + 1;
				(*(int*) direction(i,j)) = 3;
			} else {
				val1 = (*(int*) costs(i-1, j-1)) - 2; //replacement
				val2 = 3;
				if ((*(int*) costs(i, j-1)) -1 > val1){
					val1 = (*(int*) costs(i, j-1)) - 1; //insert;
					val2 = 1;
				}
				if ((*(int*) costs(i-1, j)) -1 > val1){
					val1 = (*(int*) costs(i-1, j)) - 1; //insert;
					val2 = 2;
				}
				if (val1<0) val1=0;
				(*(int*) costs(i, j)) = val1;
				(*(int*) direction(i,j)) = val2;
			}
			if ((*(int*) costs(i, j)) > max_cost){
				max_cost = (*(int*) costs(i, j));
			}
		}
	}

	costs.dumpMatrix("c:\\matrix.txt", __FILE__, HmmMatrix::IntMatrix);

	*results="<max_positions>";
	char buf[128];
	for (i=1; i<=source_count; i++){
		for (j=1; j<=str_count; j++){
			if ((*(int*) costs(i, j)) == max_cost){
				int d=0;
				long k, l;
				for (k=i, l=j; k>=0 && l>=0 && *(int*) costs(k, l)> 0; ){
					d= *(int*) direction(k,l);
					if (d==3) { k--; l--;}
					else if (d==2) k--;
					else if (d==1) l--;
					else {k--; l--;}
				}
				if (d==3) { k++; l++;}
				else if (d==2) k++;
				else if (d==1) l++;
				else {k++; l++;}
				sprintf(buf, "<position source_pos=\"%ld\" str_pos=\"%ld\" cost=\"%d\" from_pos=\"%ld\" />", 
					sourcepos[i-1], strpos[j-1], max_cost, sourcepos[k-1] );
				*results+=buf;
			}
		}
	}
	*results+= "</max_positions>";

	free(sourcetag);
	free(sourcepos);
	free(strtag);
	free(strpos);

	return 0;
}
*/
