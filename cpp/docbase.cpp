#include "docbase.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "referlink.h"
#include "qhtql.h"

#include <math.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void PDocPlainDataSyntax::findNext(){
	if (Next>=Data.L){
		NextType = synQL_END;
		NextLen = 0;

	}else if (isAlpha(Sentence[Next])){
		while (Next+NextLen < Data.L && (isAlpha(Sentence[Next+NextLen])
			||isDigit(Sentence[Next+NextLen])
			||Sentence[Next+NextLen]=='-')){
			NextLen++;
		}
		NextType=synQL_WORD;

	}else if (Sentence[Next]<0){ //multibyte code
		if (Next+NextLen < Data.L && (Sentence[Next+NextLen]<0)){
			NextLen++;
		}
		NextType=synQL_WORD;

	}else if ( isDigit(Sentence[Next])){
		while (Next+NextLen < Data.L && (isDigit(Sentence[Next+NextLen]) || 
			(Sentence[Next+NextLen]=='.' && isDigit(Sentence[Next+NextLen+1]))
			))
			NextLen++;
		NextType=synQL_NUMBER;

	}else{
		switch (Sentence[Next]){
		case '.':
			if (isDigit(Sentence[Next+NextLen])){
				NextLen++;
				NextPrecision++;
				while (isDigit(Sentence[Next+NextLen])) NextLen++;
				NextType=synEXP_NUMBER;
			}else{
				NextType=synEXP_DOT;
			}
			break;
		case ',':
			NextType=synEXP_COMMA;
			break;
		case '?': 
			NextType=synEXP_QUESTION;
			break;
		case '!':
			NextType=synEXP_EXCLAIMER;
			break;
		case ';':
			NextType=synEXP_SEMICOLON;
			break;
		default:
			NextType=synQL_UNKNOW;
			NextLen = 1;
		}
	}
}

PWordBase::PWordBase(){
	Next=0;
	Prev=0;
	Other=0;
	Score=0;
}
PWordBase::~PWordBase(){
	reset();
}
void PWordBase::reset(){
	Prev=0;

	if (Next){
		PWordBase* p=Next;
		while (p && p!=this){
			Next = p->Next;
			p->Next=0;
			delete p;
			p=Next;
		}
		Next=0;
	}
	if (Other){
		delete Other;
		Other=0;
	}
	Score=0;
	Prev=0;
}
int PWordBase::add(PWordBase*p){ //add after this 
	PWordBase* last=p;
	while (last->Next && last->Next!=p) last=last->Next;
	last->Next=Next;
	if (Next) Next->Prev=last;

	Next=p; 
	p->Prev=this;
	return 0;
}
int PWordBase::insert(PWordBase*p){ //add before this 
	PWordBase* last=p;
	while (last->Next && last->Next!=p) last=last->Next;
	p->Prev = Prev;
	last->Next=this;

	if (Prev) Prev->Next=p;
	Prev=last;
	return 0;
}
int PWordBase::remove(){
	if (Prev){
		Prev->Next=Next;
	}
	if (Next){
		Next->Prev=Prev;
	}
	Prev=0; 
	Next=0; 
	delete this; 
	return 0;
}

PWord::PWord(){
	SeqNo=0;
	Pos=0;
	P=0;
	BaseType=WORD;
}
PWord::~PWord(){
	reset();
}
void PWord::reset(){
	PWordBase::reset(); 
	SeqNo=0;
	Pos=0;
	P=0;
	Word.reset();
	BaseType=WORD;
}


int PWord::cmp(PWord* pword){
	return Word.Cmp(pword->Word.P, pword->Word.L, false);
}

int PWord::cmp(char* word, size_t len){
	return Word.Cmp(word, len, false);
}

PWordLink::PWordLink(){
	CurWord=0;
	LastWord=0;
	Length=0;
	BaseType=LINK;
	Count=0;
}

PWordLink::~PWordLink(){
	reset();
}

void PWordLink::reset(){
	PWordBase::reset();
	CurWord=0;
	LastWord=0;
	Length=0;
	Count=0;
	BaseType=LINK;
}

int PWordLink::setLink(PWord*first, PWord* last, size_t len){
	CurWord=first;
	LastWord=last;
	Length=len;
	return 0;
}


int PLinkTree::cmp(char* p1, char* p2){
	if (!p1 && !p2) return 0;
	if (!p1 || !p2) return p1>p2?1:0;
	
	int i;
//	int i=((PWordBase*)p1)->BaseType - ((PWordBase*)p2)->BaseType;
//	if (i) return i;
//	if (((PWordBase*)p1)->BaseType == PWordBase::WORD){
//		return ((PWord*)p1)->cmp((PWord*)p2);
//	}else 
	if (((PWordBase*)p1)->BaseType == PWordBase::LINK){
		i=((PWordLink*)p1)->Length -  ((PWordLink*)p2)->Length;
		if (i) return i;
		int len=((PWordLink*)p1)->Length;
		int j;
		PWord* w1=((PWordLink*)p1)->CurWord;
		PWord* w2=((PWordLink*)p2)->CurWord;
		for (i=0; i<len; i++){
			if (!w1 || !w2) return w1?1:(w2?-1:0);
			if ((j=w1->cmp(w2))) return j;
			w1=(PWord*) w1->Next;
			w2=(PWord*) w2->Next;
		}
		return 0;
	}else {
		return 1;
	}
}

int PWordTree::cmp(char* p1, char* p2){
	if (!p1 && !p2) return 0;
	if (!p1 || !p2) return p1>p2?1:0;
	return ((PWord*)p1)->cmp((PWord*)p2);
}

int PLinkScoreTree::cmp(char* p1, char* p2){
	if (!p1 && !p2) return 0;
	if (!p1 || !p2) return p1>p2?1:0;
	return (((PWordLink*)p2)->Length) * (((PWordLink*)p2)->Count) - (((PWordLink*)p1)->Length) * (((PWordLink*)p1)->Count);
}


PDocBase::PDocBase(){
	WordPatterns=0;
	WordsNum=0;
	TotalScore=1;
	Entries.NoDuplicate = true;
	EntriesScores.setSortOrder(SORT_ORDER_NUM_DEC);
	EntriesScores.setDuplication(true);
	//EntriesScores.NoDuplicate = false;
	StopWordEntries.NoDuplicate= true;
}

PDocBase::~PDocBase(){
	reset();
	resetStopWords();
}

void PDocBase::resetStopWords(){
	StopWords.reset(); 
	StopWordEntries.reset();
	StopWordEntries.NoDuplicate= true;
}

void PDocBase::reset(){
	SingleWords.reset(); 

	if (WordPatterns){
		for (ReferLink* ref=WordPatterns; ref; ref=ref->Next){
			if (ref->Data){
				PLinkTree* entry = (PLinkTree*) ref->Data;
				BTreeRecord entryrecord(entry);
				while (!entryrecord.isEOF()){
					PWordLink* link = (PWordLink*) entryrecord.getValue();
					delete link;
					entryrecord.moveNext();
				}
				delete entry;
				ref->Data=0;
			}
		}
		delete WordPatterns;
		WordPatterns=0;
	}
	Entries.reset();
	Entries.NoDuplicate = true;
	EntriesScores.empty();
	//EntriesScores.reset();
	//EntriesScores.NoDuplicate = false;
	WordsNum=0;
	TotalScore=1;
}

int printtree(PLinkTree* entry, char* filename){
	BTreeRecord entryrecord(entry);
	char* p = entryrecord.moveFirst();
	ReferData w;
	FILE* f=fopen(filename, FILE_WRITE);
	while (p){
		if (((PWordBase*) p)->BaseType == PWordBase::LINK){
			w.Set(((PWordLink*)p)->CurWord->Word.P, ((PWordLink*)p)->CurWord->Word.L, true);
			fprintf(f, "%s, ", w.P);
		}
		p=entryrecord.moveNext();
	}
	fclose(f);
	return 1;
}
long PDocBase::setStopWords(char* words, long len){
	long num=buildWords(words, len?len:strlen(words), &StopWords);
	for (PWord* curword=(PWord*) StopWords.Next; curword; curword=(PWord*) curword->Next){
		StopWordEntries.insert((char*) curword);
	}
	return num;
}
int PDocBase::setWordScore(PWord* word, int* scores){
	word->Score=0;
	if (scores){
		for (long i=0; i<word->Word.L; i++){
			word->Score+=scores[word->Pos+i]; 
		}
	}else{
		if (StopWordEntries.find((char*) word, 0)){
			word->Score=0; 
		}else{
			word->Score=1; 
		}
	}
	return 0;
}

long PDocBase::buildWords(char* plain_words, long len, PWord* pwords, long seq_no, long offset, int* scores){
	PDocPlainDataSyntax doc_syntax;

	PWord* last_word=pwords;
	while (last_word->Next) last_word = (PWord*) last_word->Next;
	PWord* new_word=0;

	doc_syntax.setSentence(plain_words, &len, false);
	while (doc_syntax.Type != QLSyntax::synQL_END ){
		if (doc_syntax.Type == QLSyntax::synQL_WORD || doc_syntax.Type == QLSyntax::synQL_NUMBER){
			new_word= new PWord;
			new_word->BaseType = PWordBase::WORD;
			new_word->Word.Set(doc_syntax.Sentence+doc_syntax.Start, doc_syntax.StartLen, false);
			new_word->SeqNo = ++seq_no;
			new_word->Pos = offset+doc_syntax.Start;

			setWordScore(new_word, scores);

			last_word->add(new_word);
			last_word=new_word;
		}else if (doc_syntax.Type == synEXP_DOT 
			|| doc_syntax.Type == synEXP_COMMA 
			|| doc_syntax.Type == synEXP_QUESTION 
			|| doc_syntax.Type == synEXP_EXCLAIMER 
			|| doc_syntax.Type == synEXP_SEMICOLON ){

			new_word= new PWord;
			new_word->BaseType = PWordBase::WORD;
			new_word->Word.Set(doc_syntax.Sentence+doc_syntax.Start, doc_syntax.StartLen, false);
			new_word->SeqNo = seq_no;
			new_word->Pos = offset+doc_syntax.Start;

			setWordScore(new_word, scores);

			new_word->Next=last_word->Other; 
			if (last_word->Other) last_word->Other->Prev=new_word; 
			last_word->Other = new_word;
		}
		doc_syntax.match();
	}
	return seq_no;
}

int PDocBase::parseWordDoc(char* doc, long len, int* scores, int to_delete_shorter){
	reset();

	WordsNum = buildWords(doc, len, &SingleWords, 0, 0, scores);
	PWord* curword = &SingleWords;

	int length=1;
	ReferLink** word_pattern=&WordPatterns; //WordPatterns include all word patterns
	*word_pattern = new ReferLink; //word patterns of length 1, data are entries

	PLinkTree* entry = new PLinkTree; //all unique links are added in entry, only repeat links are added to Entries
	entry->NoDuplicate = true;
	(*word_pattern)->Data = (long) entry; //word patterns of length 1
	
	PWordLink* curr_link, *firstlink;
	//build entries for unique string of length 1
	while (curword){
		//curr_link is single word link, it will be used to search for existing links
		curr_link = new PWordLink;
		curr_link->CurWord = curword;
		curr_link->LastWord = curword;
		curr_link->Length= length;
		curr_link->Score=curword->Score;
		curr_link->Count = 1;
		//printtree(entry, "c:\\tree.txt");
		firstlink = (PWordLink*) entry->find((char*) curr_link, 0);
		if (firstlink){
			//chain the new curr_link to the first link in entry
			firstlink->insert(curr_link);
			//curr_link->Next= firstlink;
			//curr_link->Prev = firstlink->Prev;
			//firstlink->Prev->Next = curr_link;
			//firstlink->Prev = curr_link;
			firstlink->Count ++;

			//is repeat string, add to class Entries
			if (firstlink->Count==2){
				if (!Entries.find((char*)firstlink, 0)){
					Entries.insert((char*)firstlink);
				}
			}
		}else{
			//add the first link to entry
			curr_link->Next = curr_link->Prev = curr_link;  //circle link
			entry->insert((char*)curr_link);
		}
		curword=(PWord*) curword->Next;
	}

	PLinkTree* oldentry;
	BTreeRecord entryrecord;
	long up_bound = (long) sqrt((double) WordsNum)/3+3;
	//build entries for unique string of all length;
	while (entry->total && length<up_bound){
		length ++;
 
		oldentry = entry;
		word_pattern = &(*word_pattern)->Next;  //word patterns of length 2, ..., data are entries
		*word_pattern = new ReferLink;

		entry = new PLinkTree; //create a new entry of length
		entry->NoDuplicate = true;
		(*word_pattern)->Data = (long) entry;  //word patterns of length 2, ...

		entryrecord.tree = oldentry; //search from (length-1)
		PWordLink* firstlink1 = (PWordLink*) entryrecord.moveFirst();
		//for each repeat unique string of length (length-1)
		while (firstlink1){
			if (firstlink1->Count < 2){
				firstlink1 = (PWordLink*)entryrecord.moveNext();
				continue;
			}

			//for each unique string of length (length-1)
			//  look for longer string of length
			PWordLink shorterlink;
			PWordLink* curr_link1, * oldlink=0;
			for (curr_link1=firstlink1;curr_link1; curr_link1=(PWordLink*) curr_link1->Next){//reuse firstlink1 because it is kept in entryrecord
				if (!oldlink) oldlink = curr_link1;
				else if (curr_link1==oldlink) break;

				if (!curr_link1->LastWord->Next || ((PWord*) curr_link1->LastWord->Next)->SeqNo < curr_link1->LastWord->SeqNo) {
					//no longer string
					continue;
				}
				//try a curr_link including the next word
				curr_link = new PWordLink;
				curr_link->CurWord = curr_link1->CurWord;
				curr_link->LastWord = (PWord*) curr_link1->LastWord->Next;
				curr_link->Length= length;
				curr_link->Score = curr_link1->Score+curr_link1->LastWord->Score;
				//find the longer link from the entries
				//printtree(entry, "c:\\tree.txt");
				firstlink = (PWordLink*)entry->find((char*) curr_link, 0);
				if (firstlink){
					//chain the new curr_link to the first link in entry
					firstlink->insert(curr_link);
					//curr_link->Next= firstlink;
					//curr_link->Prev = firstlink->Prev;
					//firstlink->Prev->Next = curr_link;
					//firstlink->Prev = curr_link;
					firstlink->Count ++;
					//is repeat string, add to Entries
					if (firstlink->Count==2){
						if (!Entries.find((char*)firstlink, 0)){
							Entries.insert((char*)firstlink);

							if (to_delete_shorter){
								//mark the next shorter words as deleted.
								shorterlink.CurWord=(PWord*) firstlink->CurWord->Next; //start from the secend word
								shorterlink.LastWord=firstlink->LastWord;
								shorterlink.Length=firstlink->Length-1;
								curr_link = (PWordLink*)Entries.find((char*)&shorterlink, 0);
								if (curr_link) curr_link->Count=0;

								shorterlink.CurWord=firstlink->CurWord;
								shorterlink.LastWord=curr_link1->LastWord; //ends at the last word
								shorterlink.Length=firstlink->Length-1;
								curr_link = (PWordLink*)Entries.find((char*)&shorterlink, 0);
								if (curr_link) curr_link->Count=0;
							}
						}
					}
				}else{
					//create a new string
					curr_link->Next = curr_link->Prev = curr_link;
					curr_link->Count = 1;
					entry->insert((char*) curr_link);
				}
			}

			firstlink1 = (PWordLink*)entryrecord.moveNext();
		}
	}

	//score al Entries, excluding stop words and deleted links
	long score=WordsNum;
	long curscore=0;
	entryrecord.tree = &Entries;
	ReferLink* link;
	for (entryrecord.moveFirst(); !entryrecord.isEOF(); entryrecord.moveNext()){
		PWordLink* firstlink1 = (PWordLink*) entryrecord.getValue();
		if (firstlink1->BaseType == PWord::LINK) {
			//curscore = (firstlink1->Count) * (firstlink1->Length);
			curscore=0;
			for (PWordBase* otherlinks=firstlink1; otherlinks; otherlinks=otherlinks->Next){
				curscore+=otherlinks->Score;
				if (otherlinks!=firstlink1) break;
			}

			if (firstlink1->Count>0 && (firstlink1->Length>1 || !StopWordEntries.find((char*) (firstlink1->CurWord), 0))){
				link=EntriesScores.add((char*) 0, 0, curscore);
				link->Value.P=(char*) firstlink1; 
				//EntriesScores.insert((char*) firstlink1);
			}
			score += curscore;
		}
	}
	TotalScore = score;

	return 0;
}
double PDocBase::scoreWord(char* word, long len){
	if (!WordsNum) return 0.0;
	if (WordsNum == 1) {
		if (((PWord*) SingleWords.Next)->cmp(word, len))
			return 1.0;
		else return 0.0;
	}
	PWord pword;
	pword.Word.Set(word, len, false);
	PWordLink plink;
	plink.setLink(&pword, &pword, 1);
	double score=0.0;
	PWordLink* res= (PWordLink*) Entries.find((char*) &plink, 0);
	if (res){
		score = res->Length * res->Count;
		score /= TotalScore;
	}
	return score;
}

double PDocBase::scoreWordLink(PWordLink* wlink){
	double score=0.0;
	PWordLink* res= (PWordLink*) Entries.find((char*) wlink, 0);
	if (res){
		score = res->Length * res->Count;
		score /= TotalScore;
	}else{
		PWord* link, *link1, *link2;
		int match=false;
		for (link = (PWord*) SingleWords.Next; link; link=(PWord*) link->Next){
			match=true;
			for (link1=wlink->CurWord, link2=link
				; link1&&link2; link1=(PWord*) link1->Next, link2=(PWord*) link2->Next){
				if (link1->cmp(link2)) {
					match=false;
					break;
				}
			}
			if (match) break;
		}
		if (match){
			score = wlink->Length;
			score /= TotalScore;
		}else{
			score = 0.0;
		}
	}
	return score;
}

int PDocBase::saveIndexFile(char* filename){
	FILE* f=fopen(filename, FILE_WRITE);
	if (!f) return -1;
	//BTreeRecord entryrecord(&EntriesScores);
	ReferLink* link;
	PWordBase* base;
	for (link=(ReferLink*) EntriesScores.moveFirst() ;link; link = (ReferLink*) EntriesScores.moveNext()){
		base=(PWordBase*) link->Value.P;
		if (base->BaseType != PWordBase::LINK) continue;

		PWordLink* firstlink1=(PWordLink*) base;
		for (PWord* word=firstlink1->CurWord; word; word=(PWord*) word->Next){
			word->Word.Seperate();
			fprintf(f, "%s ", word->Word.P);
			if (word == firstlink1->LastWord) break;
		}
		fprintf(f, ": LEN=%ld, TOTAL=%ld, AT: ",firstlink1->Length, firstlink1->Count);

		PWordLink* oldlink=0;
		for (PWordLink* cur=firstlink1; cur; cur=(PWordLink*) cur->Next){
			if (!oldlink) oldlink = cur;
			else if (oldlink == cur) break;
			if (cur->CurWord) fprintf(f, "%ld ", cur->CurWord->Pos);
		}
		fprintf(f, "\n");
	}
	fclose(f);
	return 0;
}

int PDocBase::getWordPatterns(ReferLinkHeap* patterns){ //Data: Score; Name: text pattern
	//BTreeRecord entryrecord(&EntriesScores);
	ReferData pattern;
	ReferLink* link;
	PWordBase* base;
	for (link=(ReferLink*) EntriesScores.moveFirst() ;link; link = (ReferLink*) EntriesScores.moveNext()){
		base=(PWordBase*) link->Value.P;
		if (base->BaseType != PWordBase::LINK) continue;

		PWordLink* firstlink1=(PWordLink*) base;
		pattern.reset();
		for (PWord* word=firstlink1->CurWord; word; word=(PWord*) word->Next){
			if (pattern.L) pattern+=" "; 
			pattern.Cat(word->Word.P, word->Word.L); 
			if (word == firstlink1->LastWord) break;
		}
		ReferLink* link=(ReferLink*) patterns->add(&pattern, 0, firstlink1->Length*firstlink1->Count); 
		if (link) link->Value.L=firstlink1->Count; 
	}
	return 0;
}
long PDocBase::getWordPatterns(char* doc, ReferData* results, long firstn){
	if (StopWordEntries.total==0){
		setStopWords("a an as at be by do if in is it of on or so to us we"
			"and are can for has its the"
			"also from have many much such that this then with"
			"these those where which");
	}

	parseWordDoc(doc, strlen(doc)); 
	ReferLinkHeap patterns; 
	patterns.setSortOrder(SORT_ORDER_NUM_DEC);
	getWordPatterns(&patterns); 
	long count=0; 
	for (ReferLink* link=(ReferLink*) patterns.moveFirst(); link; link=(ReferLink*) patterns.moveNext()){
		if (results->L) *results+="\n"; 
		*results+=link->Name; 
		if (++count>=firstn) break; 
	}
	return count; 
}

int PDocBase::getSubText_FirstNText(ReferData* html, long firstn, ReferData* result){
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(html->P, &html->L, false);
	long count=0; 
	int isskip=false;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA && !isskip){
			if (result->L) *result+="\n";
			result->Cat(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen);

			if (++count>=firstn) break;

		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG){
			if (TagOperation::isTag(DataSyntax.Sentence+DataSyntax.Start, "Script")) {
				isskip = true;
			}
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isEndTag(DataSyntax.Sentence+DataSyntax.Start, "Script")) {
				isskip = false;
			}
		}

		DataSyntax.match();
	}
	return 0;
}
int PDocBase::getSubText_FirstNWord(ReferData* html, long firstn, ReferData* result){
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(html->P, &html->L, false);

	ReferData text, tx;
	const char* ignore[]={"<script>", "<style>", 0};
	const char* ignore_end[]={"</script>", "</style>", 0};
	int to_skip=0;
	long count=0; 
	int is_end=0;
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG || DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isTags(DataSyntax.Sentence+DataSyntax.Start, ignore)>=0){
				to_skip=1; 
			}else if (TagOperation::isTags(DataSyntax.Sentence+DataSyntax.Start, ignore_end)>=0){
				to_skip=0; 
			}
		}else if (!to_skip && DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA){
			text.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, false); //by pointer
			PDocPlainDataSyntax doc_syntax;
			doc_syntax.setSentence(text.P, &text.L, false);

			while (doc_syntax.Type != QLSyntax::synQL_END ){
				if (doc_syntax.Type == QLSyntax::synQL_WORD || doc_syntax.Type == QLSyntax::synQL_NUMBER)
					count++;

				if (tx.L+doc_syntax.Start+doc_syntax.StartLen+1>=firstn) {
					is_end=1;
					break; 
				}
				doc_syntax.match();

				//if (count>=firstn) break; 
			}
			if (tx.L) tx+=" ";
			tx.Cat(doc_syntax.Sentence, doc_syntax.Start); 
		}

		if (is_end) break;
		//if (count>=firstn) break; 
		DataSyntax.match();
	}

	tStrOp::trimCRLFSpace(tx.P, result, 0); 
	if (DataSyntax.Type!=QLSyntax::synQL_END){
		*result+="$";
	}

	return 0;
}
int PDocBase::getSubSentences(char* doc, ReferLinkHeap* subsentences, int to_count_length){ 
	PDocPlainDataSyntax doc_syntax;
	doc_syntax.setSentence(doc, 0, false);

	const char* connectors[]={"'", 0};
	long from;
	ReferData subsentence;
	long in_subsentence=0; //longest
	long subsentence_count=0; //first
	while (doc_syntax.Type != QLSyntax::synQL_END ){
		if (doc_syntax.Type==QLSyntax::synQL_WORD || doc_syntax.Type==QLSyntax::synQL_NUMBER 
			|| tStrOp::strMcmp(doc_syntax.Sentence+doc_syntax.Start, connectors, true, false)>=0){
			if (in_subsentence==0){
				from=doc_syntax.Start; 
			}
			in_subsentence++; 
		}else{
			if (in_subsentence>0){
				subsentence.Set(doc_syntax.Sentence+from, doc_syntax.Start-from, false); 
				subsentence_count++;
				subsentences->add(&subsentence, 0, to_count_length?in_subsentence:subsentence_count); 
			}
			in_subsentence=0; 
		}

		doc_syntax.match();
	}
	if (in_subsentence>0){
		subsentence.Set(doc_syntax.Sentence+from, doc_syntax.Start-from, false); 
		subsentence_count++;
		subsentences->add(&subsentence, 0, to_count_length?in_subsentence:subsentence_count); 
	}

	return 0;
}

long PDocBase::getSubSentences(char* doc, ReferData* results, long firstn, int to_count_length){
	ReferLinkHeap subsentences; 
	subsentences.setSortOrder(to_count_length?SORT_ORDER_NUM_DEC:SORT_ORDER_NUM_INC); 
	getSubSentences(doc, &subsentences); 
	long count=0; 
	for (ReferLink* link=(ReferLink*) subsentences.moveFirst(); link; link=(ReferLink*) subsentences.moveNext()){
		if (results->L) *results+="\n"; 
		*results+=link->Name; 
		if (++count>=firstn) break; 
	}
	return count; 

}
int PDocBase::getTextWords(char* text, const char* type, const char* option, ReferData* result){
	if (!tStrOp::strNcmp((char*) type, "first",5,false)){
		//get the first n words, n in option
		ReferData html; 
		if (text){
			html.Set(text, strlen(text), false); 
			long firstn=0;
			if (option) sscanf(option, "%ld", &firstn);
			getSubText_FirstNWord(&html, firstn, result); 
		}

	}else if (!tStrOp::strNcmp((char*) type,"repeat",6,false)){
		long firstn=0;
		if (option) sscanf(option, "%ld", &firstn);
		PDocBase doc;
		doc.getWordPatterns(text, result, firstn); 

	}else if (!tStrOp::strNcmp((char*) type,"subsentence",strlen("subsentence"),false)){
			long firstn=0;
			if (option) sscanf(option, "%ld", &firstn);
		getSubSentences(text, result, firstn, true); 

	}else if (!tStrOp::strNcmp((char*) type,"text",strlen("text"),false)){
		//get the first n text, n in option
		ReferData html; 
		if (text){
			html.Set(text, strlen(text), false); 
			long firstn=0;
			if (option) sscanf(option, "%ld", &firstn);
			getSubText_FirstNText(&html, firstn, result); 
		}

	}else{ //"count"
		long count=tStrOp::countWords(text); 
		result->setLong(count); 
	}
	return 0;
}


PHyperTag::PHyperTag(){
}
PHyperTag::~PHyperTag(){
	reset();
}
void PHyperTag::reset(){
	PWord::reset();
}

int PHyperTag::cmp(PWord* pword){
	if (!Word.P && !pword) return 0;
	if (!Word.P || !pword) return Word.P?1:0;
	return cmp(pword->Word.P, pword->Word.L); 
}
int PHyperTag::cmp(char* word, size_t length){
	if (!Word.P && !word) return 0;
	if (!Word.P || !word) return Word.P?1:0;
	if (Word.P[0]=='<' &&word[0]=='<'){
		if (TagOperation::isTag(Word.P, word)) 
			return 0; 
		else 
			return Word.Cmp(word, length, false);
	}else if (Word.P[0]!='<' &&word[0]!='<'){
		return 0;
	}else {
		return Word.Cmp(word, length, false);
	}
	return 0;
}

long PHyperBase::buildWords(char* hyper_tags, long len, PWord* pwords, long seq_no, long offset, int* scores){
	HTQLNoSpaceTagDataSyntax DataSyntax;
	DataSyntax.setSentence(hyper_tags, &len, false);

	PWord* last_word=pwords;
	while (last_word->Next) last_word = (PWord*) last_word->Next;
	PWord* new_word;

	while (DataSyntax.Type != QLSyntax::synQL_END){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG 
			|| DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG
			|| DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA){
			new_word= new PHyperTag;
			new_word->BaseType = PWordBase::WORD;
			new_word->Word.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, false);
			new_word->SeqNo = ++seq_no;
			new_word->Pos = offset+DataSyntax.Start;

			setWordScore(new_word, scores);

			last_word->add(new_word);
			last_word=new_word;
		}

		DataSyntax.match();
	}
	return seq_no;

}

long PHyperWordBase::buildWords(char* hyper_words, long len, PWord* pwords, long seq_no, long offset, int* scores){
	HTQLNoSpaceTagDataSyntax syntax;
	syntax.setSentence(hyper_words, &len, false);

	PWord* last_word=pwords;
	while (last_word->Next) last_word = (PWord*) last_word->Next;

	const char* ignore_tags[]={"<script>", "<style>", "<meta>", 0};
	int is_valid_tag=0;
	while (syntax.Type!=QLSyntax::synQL_END){
		if (syntax.Type==HTQLTagDataSyntax::synQL_START_TAG || syntax.Type==HTQLTagDataSyntax::synQL_END_TAG){
			if (TagOperation::isTags(syntax.Sentence+syntax.Start, ignore_tags)>=0 ){
				is_valid_tag=-1;
			}else{
				is_valid_tag=0;
				
				PWord* new_word= new PWord;
				new_word->BaseType = PWordBase::WORD;
				new_word->Word.Set(syntax.Sentence+syntax.Start, syntax.StartLen, false);
				new_word->SeqNo = seq_no;
				new_word->Pos = offset+syntax.Start;

				setWordScore(new_word, scores);

				new_word->Next=last_word->Other; 
				if (last_word->Other) last_word->Other->Prev=new_word; 
				last_word->Other = new_word;
			}
		}else if (syntax.Type==HTQLTagDataSyntax::synQL_DATA && is_valid_tag>=0){
			//count text segment
			seq_no=PDocBase::buildWords(syntax.Sentence+syntax.Start, syntax.StartLen, last_word, seq_no, offset+syntax.Start, scores);
			while (last_word->Next) last_word = (PWord*) last_word->Next;
			
		}else{ //ignore
			is_valid_tag=0;
		}

		syntax.match();
	}
	return seq_no;
}



