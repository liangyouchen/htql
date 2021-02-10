#ifndef QLSYNTAX_H
#define QLSYNTAX_H

#include "log.h"
#include "referdata.h"

class QLSyntax{
public:
	enum { synQL_UNKNOW = 0, synQL_END,
		synQL_WORD=10, synQL_NUMBER, synQL_STRING, synQL_DATE,
		synQL_PATTERN=20, synQL_DASH,
		synQL_ANY, synQL_ANYSINGLE,
		synQL_DOT, synQL_COLON,
		synQL_MID, synQL_STAR, synQL_REF, synQL_QUES, synQL_TIP, synQL_AT,
		synQL_DOLLAR, synQL_SLASH, synQL_TILE,
		synQL_COMMA, synQL_SEMICOLON,
		synQL_LRECT, synQL_RRECT,
		synQL_LTAG, synQL_RTAG,
		synQL_LBRACE, synQL_RBRACE,
		synQL_LBRACKET, synQL_RBRACKET,
		synQL_EQ, synQL_NE,
		synQL_GT, synQL_LT,
		synQL_GE, synQL_LE,
		synQL_NOT, synQL_ADD,
		synQL_LIKE
	};
	int Type;
	int NextType;


	ReferData Data;
	char *Sentence;
	long Start;
	long StartLen;
	long StartPrecision;
	long Next;
	long NextLen;
	long NextPrecision;

	QLSyntax();
	virtual ~QLSyntax();
	void reset();
	int setSentence(const char *S, long* Length=0, int copy=true);
	int match(const char* word, int case_sensitive=false);
	int match(int);
	int match();
	int matchSyntax(QLSyntax* subparser);
	int takeStartSyntaxString(ReferData* result);
	int takeNextSyntaxString(ReferData* result);

	virtual void findNext();
	virtual void matchNext();
	virtual int KeyWord();
	virtual int takeSyntaxString(int type, long start, long len, ReferData* result);
	virtual int isSpace(char);
	virtual int isComment(char*, long* len);
	virtual int isAlpha(char);
	virtual int isDigit(char);
	static int cmpNoCase(const char* str1, size_t len1, const char* str2, size_t len2);
	virtual int copyFrom(QLSyntax* OtherSyntax);
};



#endif
