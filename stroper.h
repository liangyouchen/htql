#ifndef STR_OPER_H
#define STR_OPER_H CLY20000430

#include "platform.h"
#include "log.h"

#include <time.h>

#ifdef StrCmp
#undef StrCmp
#endif

class ReferData;
class ReferLink;
class ReferLinkHeap;
class ReferLink2; 

static const char* S_MONTH[]={"JANUARY","FEBRUARY","MARCH","APRIL",
		"MAY","JUNE", "JULY","AUGUST",
		"SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER",0};

static const char* S_month[]={"january","february","march","april",
		"may","june", "july","august",
		"september", "october","november","december",0};

static const char* S_Month[]={"January","February","March","April",
		"May","June", "July","August",
		"September","October","November","December",0};

static const char* S_MON[]={"JAN","FEB","MAR","APR","MAY","JUN",
		"JUL","AUG","SEP","OCT","NOV","DEC",0};

static const char* S_mon[]={"jan","feb","mar","apr","may","jun",
		"jul","aug","sep","oct","nov","dec",0};

static const char* S_Mon[]={"Jan","Feb","Mar","Apr","May","Jun",
		"Jul","Aug","Sep","Oct","Nov","Dec",0};

static const char* S_DAY[]={"SUNDAY","MONDAY","TUESDAY",
		"WEDNESDAY","THURSDAY","FRIDAY","SATURDAY",0};

static const char* S_Day[]={"Sunday","Monday","Tuesday",
		"Wednesday","Thursday","Friday","Saturday",0};

static const char* S_day[]={"sunday","monday","tuesday",
		"wednesday","thursday","friday","saturday",0};

static const char* S_DY[]={"SUN","MON","TUE","WED","THU","FRI","SAT",0};

static const char* S_Dy[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat",0};

static const char* S_dy[]={"sun","mon","tue","wed","thu","fri","sat",0};

class tStrOp{
public:
		static int isBlank(char ch);	//| > 0: if is blank; 
					//| < 0: if is eol or eof; 
					//| ==0: if is not blank
					//-----------------------

		static int isSpace(char ch);	//| > 0: if is space; 
					//| ==0: if is not space
					//-----------------------

		static int isAN_(char ch); 
					//| true: if is alpha-cumeric or under score
					//| false: if is not
					//-------------------------
		static int isDigit(char ch);
		static int strNcmp(char* p, char* P, size_t len, int CaseSensitive=true);
		static int strMcmp(const char* p, const char* P[], int CaseSensitive=true, int is_nullstr=true);
		static char* strNstr(char* p, char* P, int CaseSensitive=true);
		static char* strNrstr(char* p, char* P, int CaseSensitive=true);
        static int countChar(char *str,char sep);
        static int SubString(char *str,char sep, int pos, char *result);
        static int StringPrintf(char *str,double DoubleValue, int length, int precision);
		static int sepStrPara(char* fmt, ReferLink2* links); 
		static int StrCmp(const char* p1, const char* p2, int CaseSensitive=true);
        static int StringLike(const char *str,const char* like);
        static int StringMatch(const char *str,const char* match);
        static int DateToChar(time_t Time,char* fmt, char* result);
        static int DateToLong(char* Time, char* fmt, time_t* result);
		static long searchDateStr(const char* start, long start_len, ReferLinkHeap* dates, long start_offset=0); //dates: Name=found date; Value=YYYY/MM/DD; Data=offset
		static long searchDateText(const char* start, long start_len, ReferLinkHeap* dates, long start_offset=0); //dates: Name=found date; Value=YYYY/MM/DD; Data=offset
        static int MonthDays(int year, int month);
		static int IsValidEmail(const char*Email);
		static int isValidPhoneNum(const char* phone);
		static bool IsEmailChar(char p);
		static char* EmailLeftChar(const char *start, const char *head);
		static char* EmailRightChar(const char *start);
		static char* EmailName(const char*emailstart, const char* head, ReferData* name);
		static char* EmailNameWordLeft(const char*start, const char* head, ReferData* name);
		static char* GetEmail(const char *p, ReferData* email, ReferData* name);
		static long GetEmails(const char* p, ReferLinkHeap *emails);
		static long GetEmails(const char*text, const char* fmt, ReferData* results);
		static int getNumber(const char*p, int index, ReferData* value); //value.P pointing to the text in p
		static int trimRight(char* str, char trim=0);
		static int trimLeft(char* str, char trim=0);
		static long trimCRLFSpace(const char* text, ReferData* tx, long max_len=0);
		static long trimText(const char*text, ReferData* tx, const char* pattern, const char* mark);
				//mark: ="1010" (default): first, middle, last, remove all spaces; 0=no trim; 1=trim; =2; trim CRLF; 3=trim all;
				//		="121"; trim, no CRLF
		static int toLowerCase(char* str);
		static int toUpperCase(char* str);
		static int replaceInplace(char* str, char* source, char* dest, int case_sensitive=true);
		static char* replaceMalloc(char* str, char* source, char* dest, long maxstrlen=0, int case_sensitive=true);
		static int replaceChars(char* str, char* chars, char* replace);
		static char* quoteText(const char* str, int*is_malloc, long* size=0, long* len=0);
		static int dequoteText(char* str);
		static char* encodeHtml(const char* str, int*is_malloc, long* size=0, long* len=0);
		static char* encodeHtml(const char* str, ReferData* result);
		static int decodeHtml(char* str);
		static int decodeHtml(ReferData* str);
		static char* encodeUrlSpecial(char* url);
		static char* encodeUrl(char* url); //caller need to free the result str
		static char* decodeUrl(char* url); //caller need to free the result str
		static int splitUTF8(const char* text, ReferLinkHeap* results);
		static int decodeUTF8(char*text, ReferData* result);
		static char* encodeUTF8(char*text); //caller need to free the result str
		static int encodeUTF8(unsigned int ch, ReferData* result); 
		static int convertSpaceSpecial(char* str);
		static int dequoteString(char* str);
		static char* quoteString(const char* str, int*is_malloc, long* size=0, long* len=0);
		static char* quoteString(const char* str, ReferData* result);
		static int splitString(const char* str, char* sep, ReferLinkHeap* results);
		static int splitCsvString(const char* str, char* sep, ReferLinkHeap* results);
		static int getCSVHtqlExpr(const char* csv_source, ReferData* htql_expr, int first_line_isname, const char* fieldsep=0, const char* linesep=0, int quoted_field=0); //if fieldsep or linesep==0, find them from data
		static int getCSVFields(const char* csv_source, ReferLink** fields, ReferData* field_sep, ReferData* line_sep, int* quoted_field=0);
		static int parseCookiesString(const char* cookies_str, ReferLinkHeap* cookies); 
#ifndef NO_ALIGNMENT
		static ReferLink* bestMatch(ReferLinkHeap* heap, const char* matchname, double* best_score=0);
#endif 
		static int getUrlHostName(const char* url, char** start, long* len); 
		static int getUrlDomainName(const char* url, char** start, long* len); 
		static int getUrlPathName(const char* url, char** start, long* len); 
		static int getUrlProtocol(const char* url, ReferData* protocol);
		static int getUrlHost(const char* url, ReferData* host);
		static int getUrlAction(const char* url, ReferData* action);
		static long countWords(const char* text); 
		static int getFilePath(const char* filename, ReferData* filepath, ReferData* name); 
};

#endif

