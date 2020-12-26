#ifndef CLY_DOC_BASE_H_20020921
#define CLY_DOC_BASE_H_20020921

#include "referdata.h"
#include "btree.h"
#include "qlsyntax.h"
#include "referlink.h"
//exact repeat pattern searching from a document

class ReferLink;
class ReferLinkHeap;

//class to find words from document
class PDocPlainDataSyntax: public QLSyntax{
public:
	virtual void findNext();

};


class PWordBase{
public:
	enum {WORD, LINK, DOC};
	int BaseType;
	PWordBase* Next;
	PWordBase* Prev; 
	PWordBase* Other;
	long Score;

	int add(PWordBase*p); //add after this 
	int insert(PWordBase*p); //add before this 
	int remove();
public:
	PWordBase(); 
	virtual ~PWordBase(); 
	void reset(); 
};

class PWord : public PWordBase{ // a list of words
public:
	ReferData Word;
	long SeqNo;
	long Pos;
	void* P;
	virtual int cmp(PWord* pword);
	virtual int cmp(char* word, size_t length);

	PWord();
	virtual ~PWord();
	void reset();
};

class PWordLink: public PWordBase{ //a collection of word lists
public:
	PWord* CurWord;
	PWord* LastWord;
	size_t Length;
	long Count;

	int setLink(PWord*first, PWord* last, size_t len);

	PWordLink();
	virtual ~PWordLink();
	void reset();
};

class PWordTree: public BTree{
	virtual int cmp(char* p1, char* p2);
};

class PLinkTree: public BTree{
	virtual int cmp(char* p1, char* p2);
};

class PLinkScoreTree: public BTree{
	virtual int cmp(char* p1, char* p2);
};


class PDocBase: public PWordBase{
public:
	PWord SingleWords; //link list of words 
	long WordsNum;	//number of words in SingleWords;

	ReferLink* WordPatterns; //link list of word patterns; Data:all pattern lists (PLinkTree);  

	PLinkTree Entries;	//BTree entries to all words or word patterns
			//save all unique words (or words pattern): 
			// including PWords (by SingleWords) and PWordLinks (by WordPatterns);
			// PWords (by SingleWords) is not included after Oct.1, 2002.
			// don't need to free the entries in it;
	long TotalScore;	// the sum of (words_len)*(repeat_num) for all unique PWordLink

	PWord StopWords;
	PWordTree StopWordEntries;	//BTree entries of all stop-words such as A, The, And, ...
	long setStopWords(char* words, long len=0);	//return number of stopwords;

	//PLinkScoreTree EntriesScores;
	ReferLinkHeap EntriesScores; //Value.P: (PWordLink*) entry; Data: sum score (order)
			// sort PWordLink entries in the Entries;
			// stop-words are not included;
			// scored by (words_len)*(repeat_num)
			// don't need to free the entries in it;

	//highest level interface
	long getWordPatterns(char* doc, ReferData* results, long firstn); 

	//second level interface
	int parseWordDoc(char* doc, long len, int* scores=0, int to_delete_shorter=true);
	int getWordPatterns(ReferLinkHeap* patterns); //Data: Len*Count; Value.L:Count; Name: text pattern

	double scoreWord(char* word, long len); //score a word
	double scoreWordLink(PWordLink* wlink); //score a pattern
	int saveIndexFile(char* filename);

	//tools
	virtual long buildWords(char* plain_words, long len, PWord* pwords, long seq_no=0, long offset=0, int* scores=0);
	virtual int setWordScore(PWord* word, int* scores);
	static int getSubText_FirstNWord(ReferData* html, long firstn, ReferData* result);
	static int getSubText_FirstNText(ReferData* html, long firstn, ReferData* result);
	static int getSubSentences(char* doc, ReferLinkHeap* subsentences, int to_count_length=true); 
	static long getSubSentences(char* doc, ReferData* results, long firstn, int to_count_length=true); 
	static int getTextWords(char* text, const char* type, const char* option, ReferData* result); 

	PDocBase();
	~PDocBase();
	void reset();
	void resetStopWords();
};

class PHyperTag: public PWord{
public: 
	virtual int cmp(PWord* pword);
	virtual int cmp(char* word, size_t length);

	PHyperTag();
	virtual ~PHyperTag();
	void reset();
}; 

class PHyperBase: public PDocBase{ //count only hypertext-tags
public:
	virtual long buildWords(char* hyper_tags, long len, PWord* pwords, long seq_no=0, long offset=0, int* scores=0);
};

class PHyperWordBase: public PDocBase{ //count words from hypertext page
public:
	virtual long buildWords(char* hyper_words, long len, PWord* pwords, long seq_no=0, long offset=0, int* scores=0);
};


/*
	SingleWords -> WordBase -> ...
		
	WordPatterns(1) 
		|	|Data
		|	Entry(1) -> -> WordLink -> WordLink -> ...
		|Next
	WordPatterns(2) 
		|	|Data
		|	Entry(2) -> -> WordLink -> WordLink -> ...
		|Next              |(1)    |(2)
		|...               CurWord LastWord
                           |       |
					...... WordBase ...........


	all Entry of repeat>1 is inserted into Entries 
	EntrySort sort all Entries by repeat*length
 */

#endif
