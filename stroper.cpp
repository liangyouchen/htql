// LASTMODIFY CLY20000430
#include "stroper.h"
#include <stdlib.h>
#include <stdio.h>
#include "referlink.h"
#include "referlink2.h"
#include "RegExParser.h"
#ifndef NO_ALIGNMENT
#include "alignment.h"
#endif 

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int tStrOp::isBlank(char ch){
	if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') return 1;
	if (ch == '\0' ) return -1;
	return 0;
}
int tStrOp::isSpace(char ch){
	if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') return 1;
	return 0;
}

int tStrOp::isAN_(char ch){    // true: if is alpha-numeric or under score
	return (isalpha(ch) || isdigit(ch) || ch == '_');
}

int tStrOp::isDigit(char ch){
	return (ch >='0' && ch<='9'); 
}

int tStrOp::strNcmp(char* p, char* P, size_t len, int CaseSensitive){
	if (!p && !P) return 0;
	if (!p && P) return -1;
	if (p && !P) return 1;

	if (CaseSensitive){
		return strncmp(p, P, len);
	}else{
		for (size_t i=0; i<len; i++){
			if (toupper(P[i])!=toupper(p[i]) ) return toupper(p[i])-toupper(P[i]);
		}
		return 0;
	}
}
char* tStrOp::strNrstr(char* p, char* P, int CaseSensitive){
	char*p2=strstr(p, P); 
	if (!p2) return 0; 
	else{
		char* p3=p2; 
		while (p3) {
			p3=strstr(p2+1, P);
			if (p3) p2=p3; 
		}
	}
	return p2;
}

char* tStrOp::strNstr(char* p, char* P, int CaseSensitive){
	if (!p) return 0;
	if (!P) return p;

	if (CaseSensitive){
		return strstr(p, P);
	}else{
		size_t len = strlen(p);
		size_t len1= strlen(P);
		for (size_t i=0; i<len; i++){
			int equal=true;
			for (size_t j=0; j<len1; j++){
				if (toupper(p[i+j])!=toupper(P[j]) ) {
					equal=false;
					break;
				}
			}
			if (equal) return p+i;
		}
		return 0;
		
	}
}
int tStrOp::strMcmp(const char* p, const char* P[], int CaseSensitive, int is_nullstr){
	if (!p) return -1;
	for (int i=0; P[i]; i++){
		if (is_nullstr){
			if (StrCmp(p, P[i], CaseSensitive)==0) return i;
		} else {
			if (strNcmp((char*) p, (char*) P[i], strlen(P[i]), CaseSensitive)==0) return i;
		}
	}
	return -1;
}

int tStrOp::DateToChar(time_t Time,char* fmt, char* result){
	tm* t;
	if ((t=localtime(&Time))==NULL) return logDATEFORMAT;
	int year;
	char *p = fmt;
	char *p1 = result;

	if (Time == 0){
		*result='\0';
		return logSUCCESS;
	}
	while (*p!='\0'){
		if (strncmp("SYYYY",p,5)==0){
			sprintf(p1,"%+05d",t->tm_year+1900);
			p+=5;
			p1+=5;
		}else if (strncmp("YYYY",p,4)==0){
			sprintf(p1,"%04d",t->tm_year+1900);
			p+=4;
			p1+=4;
		}else if (strncmp("YYY",p,3)==0){
			year=t->tm_year+1900;
			year= year % 1000;
			sprintf(p1,"%03d",year);
			p+=3;
			p1+=3;
		}else if (strncmp("YY",p,2)==0){
			year=t->tm_year+1900;
			year=year % 100;
			sprintf(p1,"%02d",year);
			p+=2;
			p1+=2;
		}else if (strncmp("YEAR",p,4)==0){
			//not completed
			year=t->tm_year+1900;
			if (year>1999) sprintf(p1,"%s %02d","TWENTY",year % 100);
			else sprintf(p1,"%s %02d","NINETEEN",year % 100);
			p+=4;
			p1+=strlen(p1);
		}else if (strncmp("Year",p,4)==0){
			//not completed
			year=t->tm_year+1900;
			if (year>1999) sprintf(p1,"%s %02d","Twenty",year % 100);
			else sprintf(p1,"%s %02d","Nineteen",year % 100);
			p+=4;
			p1+=strlen(p1);
		}else if (strncmp("Y",p,1)==0){
			year=t->tm_year+1900;
			year=year % 10;
			sprintf(p1,"%01d",year);
			p+=1;
			p1+=1;
		}else if (strncmp("year",p,4)==0){
			//not completed
			year=t->tm_year+1900;
			if (year>1999) sprintf(p1,"%s %02d","twenty",year % 100);
			else sprintf(p1,"%s %02d","nineteen",year % 100);
			p+=4;
			p1+=strlen(p1);
		}else if (strncmp("MONTH",p,5)==0){
			sprintf(p1,"%s",S_MONTH[t->tm_mon]);
			p+=5;
			p1+=strlen(p1);
		}else if (strncmp("MON",p,3)==0){
			sprintf(p1,"%s",S_MON[t->tm_mon]);
			p+=3;
			p1+=strlen(p1);
		}else if (strncmp("month",p,5)==0){
			sprintf(p1,"%s",S_month[t->tm_mon]);
			p+=5;
			p1+=strlen(p1);
		}else if (strncmp("mon",p,3)==0){
			sprintf(p1,"%s",S_mon[t->tm_mon]);
			p+=3;
			p1+=strlen(p1);
		}else if (strncmp("Month",p,5)==0){
			sprintf(p1,"%s",S_Month[t->tm_mon]);
			p+=5;
			p1+=strlen(p1);
		}else if (strncmp("Mon",p,3)==0){
			sprintf(p1,"%s",S_Mon[t->tm_mon]);
			p+=3;
			p1+=strlen(p1);
		}else if (strncmp("MM",p,2)==0){
			sprintf(p1,"%02d",t->tm_mon+1);
			p+=2;
			p1+=2;
		}else if (strncmp("DDD",p,3)==0){
			sprintf(p1,"%03d",t->tm_yday);
			p+=3;
			p1+=3;
		}else if (strncmp("DD",p,2)==0){
			sprintf(p1,"%02d",t->tm_mday);
			p+=2;
			p1+=2;
		}else if (strncmp("DAY",p,3)==0){
			sprintf(p1,"%s",S_DAY[t->tm_wday]);
			p+=3;
			p1+=strlen(p1);
		}else if (strncmp("Day",p,3)==0){
			sprintf(p1,"%s",S_Day[t->tm_wday]);
			p+=3;
			p1+=strlen(p1);
		}else if (strncmp("day",p,3)==0){
			sprintf(p1,"%s",S_day[t->tm_wday]);
			p+=3;
			p1+=strlen(p1);
		}else if (strncmp("DY",p,2)==0){
			sprintf(p1,"%s",S_DY[t->tm_wday]);
			p+=2;
			p1+=strlen(p1);
		}else if (strncmp("Dy",p,2)==0){
			sprintf(p1,"%s",S_Dy[t->tm_wday]);
			p+=2;
			p1+=strlen(p1);
		}else if (strncmp("dy",p,2)==0){
			sprintf(p1,"%s",S_dy[t->tm_wday]);
			p+=2;
			p1+=strlen(p1);
		}else if (strncmp("D",p,1)==0){
			sprintf(p1,"%01d",t->tm_wday);
			p+=1;
			p1+=1;
		}else if (strncmp("WW",p,2)==0){
			sprintf(p1,"%02d",t->tm_yday/7);
			p+=2;
			p1+=2;
		}else if (strncmp("W",p,1)==0){
			sprintf(p1,"%01d",(t->tm_mday-1)/7);
			p+=1;
			p1+=1;
		}else if (strncmp("HH24",p,4)==0){
			sprintf(p1,"%02d",t->tm_hour);
			p+=4;
			p1+=2;
		}else if (strncmp("HH12",p,4)==0){
			sprintf(p1,"%02d",t->tm_hour%12);
			p+=4;
			p1+=2;
		}else if (strncmp("HH",p,2)==0){
			sprintf(p1,"%02d",t->tm_hour%12);
			p+=2;
			p1+=2;
		}else if (strncmp("MI",p,2)==0){
			sprintf(p1,"%02d",t->tm_min);
			p+=2;
			p1+=2;
		}else if (strncmp("SSSSS",p,5)==0){
			sprintf(p1,"%05d",t->tm_hour*60*60+t->tm_min*60+t->tm_sec);
			p+=5;
			p1+=5;
		}else if (strncmp("SS",p,2)==0){
			sprintf(p1,"%02d",t->tm_sec);
			p+=2;
			p1+=2;
		}else if (strncmp("Q",p,1)==0){
			sprintf(p1,"%01d",t->tm_yday/30);
			p+=1;
			p1+=1;
		}else if (strncmp("A.M.",p,4)==0 || strncmp("P.M.",p,4)==0){
			if (t->tm_hour<12) sprintf(p1,"A.M.");
			else sprintf(p1,"P.M.");
			p+=4;
			p1+=4;
		}else if (strncmp("a.m.",p,4)==0|| strncmp("p.m.",p,4)==0){
			if (t->tm_hour<12) sprintf(p1,"a.m.");
			else sprintf(p1,"p.m.");
			p+=4;
			p1+=4;
		}else if (strncmp("AM",p,2)==0|| strncmp("PM",p,2)==0){
			if (t->tm_hour<12) sprintf(p1,"AM");
			else sprintf(p1,"PM");
			p+=2;
			p1+=2;
		}else if (strncmp("am",p,2)==0|| strncmp("pm",p,2)==0){
			if (t->tm_hour<12) sprintf(p1,"am");
			else sprintf(p1,"pm");
			p+=2;
			p1+=2;
		}else if (strncmp("B.C.",p,4)==0|| strncmp("A.D.",p,4)==0){
			year=t->tm_year+1900;
			if (year<0) sprintf(p1,"A.D.");
			else sprintf(p1,"B.C.");
			p+=4;
			p1+=4;
		}else if (strncmp("b.c.",p,4)==0|| strncmp("a.d.",p,4)==0){
			year=t->tm_year+1900;
			if (year<0) sprintf(p1,"a.d.");
			else sprintf(p1,"b.c.");
			p+=4;
			p1+=4;
		}else if (strncmp("BC",p,2)==0|| strncmp("AD",p,2)==0){
			year=t->tm_year+1900;
			if (year<0) sprintf(p1,"AD");
			else sprintf(p1,"BC");
			p+=2;
			p1+=2;
		}else if (strncmp("bc",p,2)==0|| strncmp("ad",p,2)==0){
			year=t->tm_year+1900;
			if (year<0) sprintf(p1,"ad");
			else sprintf(p1,"bc");
			p+=2;
			p1+=2;
		}else{// "J" not completed
			sprintf(p1,"%c",*p);
			p++;
			p1++;
		}
	}
	return logSUCCESS;
}

int tStrOp::DateToLong(char* Time, char* fmt, time_t* result){
	//*result=0;
	if (!Time) return logSUCCESS;

	struct tm t;
	time_t now=*result;
	char *p = fmt;
	char *p1 = Time;
	char *end= Time+strlen(Time);
	int i,j;

	tm* t1=localtime(&now);
	memcpy(&t,t1,sizeof(tm));
	t.tm_mday = 1;

	while (*p!='\0'){
		if (strncmp("SYYYY",p,5)==0){
			now=0;
			sscanf(p1,"%5d",&now);
			if (now>0) t.tm_year=now-1900;
			p+=5;
			for (j=0; j<5;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("YYYY",p,4)==0){
			now=0;
			sscanf(p1,"%4d",&now);
			if (now>0) t.tm_year=now-1900;
			p+=4;
			for (j=0; j<4;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("YYY",p,3)==0){
			now=0;
			sscanf(p1,"%3d",&now);
			if (now<100) t.tm_year=now+100;
			else t.tm_year=now%100;
			p+=3;
			for (j=0; j<3;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("YY",p,2)==0){
			now=0;
			sscanf(p1,"%2d",&now);
			if (now<73) t.tm_year=now+100;
			else t.tm_year=now;
			p+=2;
			for (j=0; j<2;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("YEAR",p,4)==0){
			//not completed
			now=0;
			if (strncmp("TWENTY",p1,6)==0){
				now+=100;
				p1+=6;
			}
			t.tm_year=now;
			now=0;
			if (tStrOp::isDigit(p1[2])) 
				sscanf(p1,"%4d",&now);
			else sscanf(p1,"%2d",&now);
			t.tm_year+=now;
			if (t.tm_year>1900) t.tm_year-=1900;
			p+=4;
			if (tStrOp::isDigit(p1[2])) 
				p1+=4;
			else p1+=2;
			if (p1>=end) break;
		}else if (strncmp("Year",p,4)==0){
			//not completed
			now=0;
			if (strncmp("Twenty",p1,6)==0){
				now+=100;
				p1+=6;
			}
			t.tm_year=now;
			now=0;
			if (tStrOp::isDigit(p1[2])) 
				sscanf(p1,"%4d",&now);
			else sscanf(p1,"%2d",&now);
			t.tm_year+=now;
			if (t.tm_year>1900) t.tm_year-=1900;
			p+=4;
			if (tStrOp::isDigit(p1[2])) 
				p1+=4;
			else p1+=2;
			if (p1>=end) break;
		}else if (strncmp("Y",p,1)==0){
			now=0;
			sscanf(p1,"%1d",&now);
			t.tm_year=(t.tm_year/10)*10+now;
			p+=1;
			p1+=1;
			if (p1>=end) break;
		}else if (strncmp("year",p,4)==0){
			//not completed
			now=0;
			if (strncmp("twenty",p1,6)==0){
				now+=100;
				p1+=6;
			}
			t.tm_year=now;
			now=0;
			if (tStrOp::isDigit(p1[2])) 
				sscanf(p1,"%4d",&now);
			else sscanf(p1,"%2d",&now);
			t.tm_year+=now;
			if (t.tm_year>1900) t.tm_year-=1900;
			p+=4;
			if (tStrOp::isDigit(p1[2])) 
				p1+=4;
			else p1+=2;
			if (p1>=end) break;
		}else if (strncmp("MONTH",p,5)==0){
			for (i=0;i<12;i++){
				if (strncmp(S_MONTH[i],p1,strlen(S_MONTH[i]))==0){
					t.tm_mon=i;
					break;
				}
			}
			p+=5;
			if (i<12) p1+=strlen(S_MONTH[i]);
			else p1+=1;
			if (p1>=end) break;
		}else if (strncmp("MON",p,3)==0){
			for (i=0;i<12;i++){
				if (strncmp(S_MON[i],p1,strlen(S_MON[i]))==0){
					t.tm_mon=i;
					break;
				}
			}
			p+=3;
			if (i<12) p1+=strlen(S_MON[i]);
			else p1+=3;
			if (p1>=end) break;
		}else if (strncmp("month",p,5)==0){
			for (i=0;i<12;i++){
				if (strncmp(S_month[i],p1,strlen(S_month[i]))==0){
					t.tm_mon=i;
					break;
				}
			}
			p+=5;
			if (i<12) p1+=strlen(S_month[i]);
			else p1+=1;
			if (p1>=end) break;
		}else if (strncmp("mon",p,3)==0){
			for (i=0;i<12;i++){
				if (strncmp(S_mon[i],p1,strlen(S_mon[i]))==0){
					t.tm_mon=i;
					break;
				}
			}
			p+=3;
			if (i<12) p1+=strlen(S_mon[i]);
			else p1+=3;
			if (p1>=end) break;
		}else if (strncmp("Month",p,5)==0){
			for (i=0;i<12;i++){
				if (strncmp(S_Month[i],p1,strlen(S_Month[i]))==0){
					t.tm_mon=i;
					break;
				}
			}
			p+=5;
			if (i<12) p1+=strlen(S_Month[i]);
			else p1+=1;
			if (p1>=end) break;
		}else if (strncmp("Mon",p,3)==0){
			for (i=0;i<12;i++){
				if (strncmp(S_Mon[i],p1,strlen(S_Mon[i]))==0){
					t.tm_mon=i;
					break;
				}
			}
			p+=3;
			if (i<12) p1+=strlen(S_Mon[i]);
			else p1+=3;
			if (p1>=end) break;
		}else if (strncmp("MM",p,2)==0){
			now=0;
			sscanf(p1,"%2d",&now);
			if (now>0) t.tm_mon=now-1;
			p+=2;
			for (j=0; j<2;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("DDD",p,3)==0){
			now=0;
			sscanf(p1,"%3d",&now);
			if (now>0) t.tm_yday=now;
			p+=3;
			for (j=0; j<3;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("DD",p,2)==0){
			now=0;
			sscanf(p1,"%2d",&now);
			if (now>0) t.tm_mday=now;
			p+=2;
			for (j=0; j<2;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("DY",p,2)==0||strncmp("Dy",p,2)==0||strncmp("dy",p,2)==0){
			p+=2;
			p1+=strlen(S_DY[0]);
			if (p1>=end) break;
		}else if (strncmp("DAY",p,3)==0){
			for (i=0;i<7;i++){
				if (strncmp(S_DAY[i],p1,strlen(S_DAY[i]))==0){
					break;
				}
			}
			p+=3;
			if (i<7) p1+=strlen(S_DAY[i]);
			else p1+=1;
			if (p1>=end) break;
		}else if (strncmp("Day",p,3)==0){
			for (i=0;i<7;i++){
				if (strncmp(S_Day[i],p1,strlen(S_Day[i]))==0){
					break;
				}
			}
			p+=3;
			if (i<7) p1+=strlen(S_Day[i]);
			else p1+=1;
			if (p1>=end) break;
		}else if (strncmp("day",p,3)==0){
			for (i=0;i<7;i++){
				if (strncmp(S_day[i],p1,strlen(S_day[i]))==0){
					break;
				}
			}
			p+=3;
			if (i<7) p1+=strlen(S_day[i]);
			else p1+=1;
			if (p1>=end) break;
		}else if (strncmp("D",p,1)==0){
			now=0;
			sscanf(p1,"%1d",&now);
			if (now>0) t.tm_wday=now;
			p+=1;
			p1+=1;
			if (p1>=end) break;
		}else if (strncmp("WW",p,2)==0){
			p+=2;
			p1+=2;
			if (p1>=end) break;
		}else if (strncmp("W",p,1)==0){
			p+=1;
			p1+=1;
			if (p1>=end) break;
		}else if (strncmp("HH24",p,4)==0){
			now=0;
			sscanf(p1,"%2d",&now);
			//if (now>0) 
				t.tm_hour=now;
			p+=4;
			for (j=0; j<2;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("HH12",p,4)==0){
			now=0;
			sscanf(p1,"%2d",&now);
			//if (now>0) 
				t.tm_hour=now;
			p+=4;
			for (j=0; j<2;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("HH",p,2)==0){
			now=0;
			sscanf(p1,"%2d",&now);
			//if (now>0) 
				t.tm_hour=now;
			p+=2;
			for (j=0; j<2;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("MI",p,2)==0){
			now=0;
			sscanf(p1,"%2d",&now);
			if (now>0) t.tm_min=now;
			p+=2;
			for (j=0; j<2;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("SSSSS",p,5)==0){
			p+=5;
			p1+=5;
			if (p1>=end) break;
		}else if (strncmp("SS",p,2)==0){
			now=0;
			sscanf(p1,"%2d",&now);
			if (now>0) t.tm_sec=now;
			p+=2;
			for (j=0; j<2;j++) if (tStrOp::isDigit(*p1)) p1++;
			if (p1>=end) break;
		}else if (strncmp("Q",p,1)==0){
			p+=1;
			p1+=1;
			if (p1>=end) break;
		}else if (strncmp("A.M.",p,4)==0 || strncmp("P.M.",p,4)==0){
			if (strncmp(p1,"P.M.",4)==0) {
				if (t.tm_hour<12) t.tm_hour+=12;
			}
			p+=4;
			p1+=4;
			if (p1>=end) break;
		}else if (strncmp("a.m.",p,4)==0|| strncmp("p.m.",p,4)==0){
			if (strncmp(p1,"p.m.",4)==0) {
				if (t.tm_hour<12) t.tm_hour+=12;
			}
			p+=4;
			p1+=4;
			if (p1>=end) break;
		}else if (strncmp("AM",p,2)==0|| strncmp("PM",p,2)==0){
			if (strncmp(p1,"PM",2)==0) {
				if (t.tm_hour<12) t.tm_hour+=12;
			}
			p+=2;
			p1+=2;
			if (p1>=end) break;
		}else if (strncmp("am",p,2)==0|| strncmp("pm",p,2)==0){
			if (strncmp(p1,"pm",2)==0) {
				if (t.tm_hour<12) t.tm_hour+=12;
			}
			p+=2;
			p1+=2;
			if (p1>=end) break;
		}else if (strncmp("B.C.",p,4)==0|| strncmp("A.D.",p,4)==0){
			p+=4;
			p1+=4;
			if (p1>=end) break;
		}else if (strncmp("b.c.",p,4)==0|| strncmp("a.d.",p,4)==0){
			p+=4;
			p1+=4;
			if (p1>=end) break;
		}else if (strncmp("BC",p,2)==0|| strncmp("AD",p,2)==0){
			p+=2;
			p1+=2;
			if (p1>=end) break;
		}else if (strncmp("bc",p,2)==0|| strncmp("ad",p,2)==0){
			p+=2;
			p1+=2;
			if (p1>=end) break;
		}else{// "J" not completed
			p++;
			p1++;
			if (p1>=end) break;
		}
	}
	now=time(NULL);
	t1=localtime(&now);
	if (t1->tm_year == t.tm_year){
		t.tm_isdst = t1->tm_isdst;
	}else if (t.tm_year==69){
		t.tm_year=t1->tm_year; 
		t.tm_isdst = t1->tm_isdst;
		if (t1->tm_mon < t.tm_mon){
			t.tm_year--;
		}
	}

	*result=mktime(&t);
	return logSUCCESS;
}
int tStrOp::getNumber(const char*p, int index1, ReferData* value){
	int is_reverse=false;
	if (index1<0) {
		is_reverse=true;
		index1=-index1;
	}else if (index1==0){
		index1=1;
	}
	
	ReferLink2 links;
	ReferLink2* link=0;

	int total=0;
	char* p1=(char*) p; 
	while (p1 && *p1 && (is_reverse || total<index1)){
		while (*p1 && !tStrOp::isDigit(*p1)) p1++;
		if (!*p1) break; //no more numbers
		
		//p1 is number
		int has_dot=false, has_e=false;
		
		char*p2=p1+1; 
		if (p1>p && (*(p1-1)=='.')) {p1--; has_dot=true;}
		if (p1>p && (*(p1-1)=='+' || *(p1-1)=='-')) p1--;

		while (*p2 && (tStrOp::isDigit(*p2)||*p2=='.'||*p2=='E'||*p2=='e')){
			if (*p2=='.'){
				if (!has_dot) has_dot=true; 
				else break;
			}
			if (*p2=='e' || *p2=='E'){
				if (!has_e) {has_e=true; has_dot=false;}
				else break;
			}
			p2++;
		}
		link=links.insert();
		link->Value.Set(p1, p2-p1, false); 
		link->Data=total++;

		p1=p2;
	}

	int count;
	if (!total){
		return false;
	}else if (is_reverse){
		link=links.Prev; 
		for (count=1; count<index1; count++){
			link=link->Prev; 
			if (link==&links) link=link->Prev; 
		}
	}else{
		link=(ReferLink2*) links.Next; 
		for (count=0; count<index1-1; count++){
			link=(ReferLink2*) link->Next; 
			if (link==&links) link=(ReferLink2*) link->Next; 
		}
	}
	value->Set(link->Value.P, link->Value.L, false); 
	return true;
}
long tStrOp::searchDateStr(const char* start, long start_len, ReferLinkHeap* dates, long start_offset){ //dates: Name=found date; Value=YYYY/MM/DD; Data=offset
	long pos=0; 

	time_t now=time(NULL);
	tm* t1=localtime(&now);
	int thisyear=t1->tm_year+1900;

	long i,j,k;
	long nextlen;
	char* datestart, *dateend;
	long count=0;
	while (pos<start_len){
		nextlen=1; datestart=(char*) start+pos; 
		if (tStrOp::isDigit(start[pos])){
			//find any separator
			int d[3]={-1,-1,-1};
			int year=-1,mon=-1,day=-1; 

			sscanf(start+pos, "%d", &d[0]); 
			char sep=0;
			int is_ddyyyy=0;
			for (i=0; i<20; i++){
				//YYYY/MM/DD; YY/MM/DD; MM/DD/YY; DD/MM/YY; MM/DD; MM/YY; 
				//YYYY-MM-DD; YY-MM-DD; MM-DD-YY; DD-MM-YY; MM-DD; MM-YY;
				if ((start[pos+i]=='/' || start[pos+i]=='-') && tStrOp::isDigit(start[pos+i+1]) ){
					if (d[1]<0){sep=start[pos+i]; sscanf(start+pos+i+1, "%d", &d[1]); }
					else if (sep==start[pos+i]) sscanf(start+pos+i+1, "%d", &d[2]); 
					else break;
				}else if (!tStrOp::strNcmp((char*) start+pos+i, "Äê",2)){
					for (int j=i-1; j>=0; j--){
						if (!tStrOp::isDigit(start[pos+j]) ){
							sscanf(start+pos+(j+1), "%d", &d[0]); 
							break;
						}
					}
					i++;
				}else if (!tStrOp::strNcmp((char*) start+pos+i, "ÔÂ",2)){
					for (int j=i-1; j>=0; j--){
						if (!tStrOp::isDigit(start[pos+j]) ){
							sscanf(start+pos+(j+1), "%d", &d[1]); 
							break;
						}
					}
					i++;
				}else if (!tStrOp::strNcmp((char*) start+pos+i, "ÈÕ",2)){
					for (int j=i-1; j>=0; j--){
						if (!tStrOp::isDigit(start[pos+j]) ){
							sscanf(start+pos+(j+1), "%d", &d[2]); 
							break;
						}
					}
					i+=2;
					break;
				}else if (start[pos+i]==',' && (tStrOp::isDigit(start[pos+i+1]) || tStrOp::isDigit(start[pos+i+2])) ){
					sscanf(start+pos+i+1, "%d", &d[1]); //DD, YYYY
					i+=2; 
					for (int j=0; j<4; j++){
						if (tStrOp::isDigit(start[pos+i])) i++;
						else break;
					}
					is_ddyyyy=1;
					break;
				}else if (!tStrOp::isDigit(start[pos+i])){
					break;
				}
			}
			if (d[1]>=0 && d[2]<0){ //find MON-DD-YYYY
				j=pos-1; 
				while (j>0 && (isspace(start[j])||start[j]=='-'||start[j]=='/'||start[j]=='.')) j--; //last char of mon
				int i1;
				for (i1=j; i1>=0 && isalpha(start[i1]); i1--); 
				i1++; //first char of mon
				ReferData monstr;
				monstr.Set((char*) start+i1, j-i1+1, true);

				k=tStrOp::strMcmp(monstr.P, S_MONTH, false,false);
				if (k<0) k=tStrOp::strMcmp(monstr.P, S_MON, false,false);

				if (k>=0){
					mon=k+1; 
					datestart=(char*) start+i1;
				}else if (is_ddyyyy){ //but no month
					d[0]=d[1]=-1;
				}
 
			}
			if (d[2]<0 && (d[1]>2030 || d[1]<1900) && (d[0]>2030 || d[0]<1900)) {
				d[0]=d[1]=-1;
			}
			int daymonth=false, monthyear=true, dayyear=true; //order
			if (d[1]<0 && i==6){//YYYYMM; YYMMDD; 
				if (!strncmp(start+pos, "19",2) || !strncmp(start+pos, "20",2)){
					sscanf(start+pos, "%4d%2d", &year, &mon); 
				}else{
					sscanf(start+pos, "%2d%2d%2d", &year, &mon, &day); 
				}
			}else if (d[1]<0 && i==8){//YYYYMMDD;
				sscanf(start+pos, "%4d%2d%2d", &year, &mon, &day); 
			}else if (d[1]<0 && i<=4){
				//search for month
				const char* dayth[]={"th","st","nd","rd",0};
				int dayth_i=tStrOp::strMcmp(start+pos+i,dayth,false,false);
				if (dayth_i>=0) i+=strlen(dayth[dayth_i]); 

				while (pos+i<start_len && isspace(start[pos+i])) i++; 
				if (pos+i<start_len && (start[pos+i]=='/' || start[pos+i]=='-')) i++; 
				if (pos+i<start_len && isalpha(start[pos+i])){
					for (j=0; pos+i+j<start_len; j++){
						if (!isalpha(start[pos+i+j]) ) break; 
					}
					ReferData monstr;
					monstr.Set((char*) start+pos+i, j, true);

					k=tStrOp::strMcmp(monstr.P, S_MONTH, false,false);
					if (k<0) k=tStrOp::strMcmp(monstr.P, S_MON, false,false);

					if (k>=0){
						mon=k+1;
						sscanf(start+pos,"%d", &d[0]);
						if (!tStrOp::isDigit(start[pos+i+j])) j++;
						if (tStrOp::isDigit(start[pos+i+j])) {
							sscanf(start+pos+i+j,"%d", &d[2]);
							while (tStrOp::isDigit(start[pos+i+j])) j++;
						}
						if (dayth_i>=0){
							day=d[0]; d[0]=-1; 
							year=d[2]; d[2]=-1;
						}
						i+=j; //to skip to nextlen
					}
				}

				if (d[1]<0 && pos>0) { 
					//search if month is before pos
					int di=-1; 
					while (di>=-pos && isspace(start[pos+di])) di--; 
					if (di>=-pos && start[pos+di]=='/' || start[pos+di]=='-') di--; 
					if (di>=-pos && isalpha(start[pos+di])){
						for (j=0; pos+di+j>=0; j--){
							if (!isalpha(start[pos+di+j]) ) break; 
						}

						ReferData monstr;
						monstr.Set((char*) start+pos+di+j+1, -j, true);

						k=tStrOp::strMcmp(monstr.P, S_MONTH, false,false);
						if (k<0) k=tStrOp::strMcmp(monstr.P, S_MON, false,false);

						if (k>=0){
							datestart=(char*) start+pos+di+j+1; 
							mon=k+1;
							sscanf(start+pos,"%d", &d[0]);

							if (start[pos+i]=='-' || start[pos+i]=='/'  || start[pos+i]==',') i++;
							while (pos+i<start_len && isspace(start[pos+i])) i++; 
							if (pos+i<start_len && tStrOp::isDigit(start[pos+i])) {
								sscanf(start+pos+i,"%d", &year);
								if (year>2100) year=-1;
								else{
									while (tStrOp::isDigit(start[pos+i])) i++; 
								}
								day=d[0]; d[0]=-1;
							}
						}
					}
				}
			}
			nextlen=i;
			dateend=(char*) start+pos+nextlen;

			for (i=0; i<3; i++){
				if ((d[i]>31 && d[i]<2100) || d[i]==0){
					year=d[i]; d[i]=-1; 
					if (year<100 && year>31) year+=1900;
					if (year==0) year=2000;
				}
			}
			for (i=0; i<3; i++){
				if (d[i]>12 && d[i]<=31 && year>=0){
					day=d[i]; d[i]=-1; 
				}
			}
			if (year>=0 && mon>=0 && day>=0){
				//no need
			}else if (year<0 && day<0){ 
				if (d[2]<0){ //MM-YY; YY-MM
					if (mon<0) {
						if (monthyear){ year=d[0]; mon=d[1];} else {year=d[1]; mon=d[0];}
					}else{
						year=(d[0]>=0)?d[0]:d[1]; 
					}
				}else{
					if (dayyear && monthyear) {
						year=d[2]; 
						if (mon<0){
							if (daymonth) {day=d[0]; mon=d[1]; }else {day=d[1]; mon=d[0]; }
						}else{
							day=(d[0]>=0)?d[0]:d[1];
						}
					}else {
						year=d[0]; 
						if (mon<0){
							if (daymonth) {day=d[1]; mon=d[2]; }else {day=d[2]; mon=d[1]; }
						}else{
							day=(d[1]>=0)?d[1]:d[2];
						}
					}
				}
			}else if (year>=0 && day>=0){
				for (i=0; i<3; i++) {
					if (d[i]>0) mon=d[i]; 
				}
			}else {//year>=0 or day>=0, only one of them>=0
				if (d[0]<0) {d[0]=d[1]; d[1]=d[2]; d[2]=-1;}
				if (year>=0){
					if (d[0]>=0 && d[1]>=0){
						if (daymonth) {day=d[0]; mon=d[1]; }else{ day=d[1]; mon=d[0]; }
					}else if (d[0]){
						if (mon<0) mon=d[0];
						else day=d[0];
					}else if (d[1]){
						if (mon<0) mon=d[1]; 
						else day=d[1];
					}
				}else if (day>=0){
					if (d[0]>=0 && d[1]>=0){
						if (monthyear) {mon=d[0]; year=d[1]; }else{ mon=d[1]; year=d[0]; }
					}else if (d[0]){
						if (mon<0) mon=d[0];
						else year=d[0];
					}else if (d[1]){
						if (mon<0) mon=d[1]; 
						else year=d[1];
					}
				}
			}
			if (year<100 && year>31) year+=1900;
			if (year==0) year=2000;
			if (year<0){
				year=thisyear;
			}
			if (year<=31 && day<0){
				day=year; year=thisyear;
			}
			if (year<=31) year+=2000; 

			if (day<0) {
				day=0;
			}

			int t3[3]={0,0,0};
			if (mon>0 && mon<=12 && day<=31){//valid date
				//find time
				long nextspace=nextlen; 
				while (start[pos+nextspace] && tStrOp::isSpace(start[pos+nextspace])) nextspace++;
				for (i=0;i<30;i++) {
					if (!start[pos+nextspace] || tStrOp::isDigit(start[pos+nextspace])) break;
					nextspace++;
				}

				if (tStrOp::isDigit(start[pos+nextspace])){
					//HH24
					sscanf(start+pos+nextspace, "%d", &t3[0]); 
					if (t3[0]<24){
						while (start[pos+nextspace] && tStrOp::isDigit(start[pos+nextspace])) nextspace++;
						if (start[pos+nextspace]==':'){
							nextspace++; 

							if (tStrOp::isDigit(start[pos+nextspace])){
								//MI
								sscanf(start+pos+nextspace, "%d", &t3[1]); 
								if (t3[1]<=60){
									while (start[pos+nextspace] && tStrOp::isDigit(start[pos+nextspace])) nextspace++;
									if (start[pos+nextspace]==':'){
										nextspace++; 

										if (tStrOp::isDigit(start[pos+nextspace])){
											//SS
											sscanf(start+pos+nextspace, "%d", &t3[2]); 
											if (t3[2]<=61){
												while (start[pos+nextspace] && tStrOp::isDigit(start[pos+nextspace])) nextspace++;
											}else{
												t3[2]=0;
											}
										}
									}
								}else{
									t3[1]=0; 
								}
							}					
						}
						//skip space
						while (start[pos+nextspace] && tStrOp::isSpace(start[pos+nextspace])) nextspace++;
						//PM
						if (!tStrOp::strNcmp((char*) start+pos+nextspace, "PM", 2, false) && !tStrOp::isAN_(start[pos+nextspace+2])){
							if (t3[0]<12){
								t3[0]+=12; 
							}
							nextspace+=2; 
						}else if (!tStrOp::strNcmp((char*) start+pos+nextspace, "AM", 2, false) && !tStrOp::isAN_(start[pos+nextspace+2])){
							if (t3[0]==12) t3[0]=0; 
							nextspace+=2; 
						}
						nextlen=nextspace;
						dateend=(char*) start+pos+nextlen;
					}else{
						t3[0]=0;
					}
				}

				struct tm t;
				memset(&t, 0, sizeof(t));
				t.tm_year=year-1900;
				t.tm_mon=mon-1;
				t.tm_mday=day?day:1;
				if (t1->tm_year == t.tm_year){
					t.tm_isdst = t1->tm_isdst;
				}
				if (t3[0]<24) t.tm_hour=t3[0];
				if (t3[1]<60) t.tm_min=t3[1]; 
				if (t3[2]<=61) t.tm_sec=t3[2]; 

				//mktime(&t)

				ReferData datestr;
				datestr.Set(datestart, dateend-datestart,true);

				char buf[128]; 
				sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d", year, mon, day, t3[0], t3[1], t3[2]); //"YYYY/MM/DD"

				//add to result
				dates->add(datestr.P, buf, datestart-start+start_offset);
				count++;
			}
		}

		pos+=nextlen;
	}
	return count;
}
long tStrOp::searchDateText(const char* start, long start_len, ReferLinkHeap* dates, long start_offset){ //dates: Name=found date; Value=YYYY/MM/DD; Data=offset
	if (!start) return 0;
	char* search_patterns[]={
			"\\d+\\s+(days|day|ds)\\s+ago", 
			"\\d+\\s+(hours|hour|hrs|hr|hs)\\s+ago",
			"\\d+\\s+(minutes|minute|min|ms)\\s+ago",
			"\\d+\\s+(seconds|second|sec|ss)\\s+ago",
			"\\d+\\s+(years|year|yrs|yr|ys)\\s+ago",
			"yesterday|yestoday",
			"today",
			0
			}; 
	long lessdays[]={24*60*60, 
		60*60,  
		60, 
		1,
		365*24*60*60,
		24*60*60,
		0,
		0};
	int count_pos=5; 
	ReferLinkHeap results;
	results.setSortOrder(SORT_ORDER_NUM_INC);
	ReferLink* link;
	RegExParser regparser;
	int i=0;
	for (i=0; search_patterns[i]; i++){
		regparser.CaseSensitive=false;
		regparser.Overlap=false;
		regparser.searchRegExText(start, search_patterns[i], &results);
		if (results.Total) break;
		regparser.reset();
	}

	
	long count=0;
	for (link=(ReferLink*) results.moveFirst(); link; link=(ReferLink*) results.moveNext()){
		time_t t=time(0); 
		if (i<count_pos){
			long n=0; 
			sscanf(link->Value.P, "%d", &n); 
			t -= lessdays[i]*n; 
		}else
			t -= lessdays[i]; 

		ReferData buf(128); 
		DateToChar(t, "YYYY/MM/DD HH24:MI:SS", buf.P); //"YYYY/MM/DD"
		buf.L=strlen(buf.P);

		//add to result
		dates->add(&link->Value, &buf, link->Value.P-start+start_offset);
		count++;
	}
	return count; 
}

int tStrOp::MonthDays(int year, int month){//month=0..11
	int i=month+1;
	int n=0;
	if (i==1||i==3||i==5||i==7||i==8||i==10||i==12)
		n=31;
	else if (i==4||i==6||i==9||i==11)
		n=30;
	else if (year%4==0 && (year%100!=0 || year%400==0))
		n=29;
	else n=28;
	return n;
}

int tStrOp::SubString(char *str,char sep, int pos, char *result){
	int i,j,k=strlen(str);
	for (i=pos, j=0;(j<k)&&(i>0); j++)
		if (*(str+j)==sep) i--;
	if ((i==0)&&(j<k)) {
		for (;(*(str+j)!=sep)&&(j<k);j++){
			result[i++]=*(str+j);
		}
		result[i]='\0';
		return i;
	} 
	result[0]='\0';
	return -1;
}

int tStrOp::StringPrintf(char *str, double DoubleValue, int length, int precision){
	char tmp[40];
	int i=0,j=0,k;
	sprintf(tmp,"%-25.10lf",DoubleValue);
	while( tmp[i]!='\0' && tmp[i]!='.') i++;
	if ( tmp[i]=='.'){
		while ( tmp[i+j+1]!='\0' && tmp[i+j+1]!=' ') j++;
	}
	if ( i>length ) return logOVERFLOW;
	strncpy(str,tmp,i);
	str[i]='\0';
	if (precision>0 && j>0 ){
		k= (precision>j) ? j : precision;
		strncpy(str+i,tmp+i,++k);
		str[i+k]='\0';
	}
	return logSUCCESS;
}
int tStrOp::StrCmp(const char* p1, const char* p2, int CaseSensitive){
	if (!p1) {
		if (!p2) return 0;
		else if (CaseSensitive) return -1;
		else return 0;
	}
	if (!p2) {
		if (CaseSensitive) return 1;
		else return 0;
	}
	if (CaseSensitive) return strcmp(p1, p2);
	else{
		long len=strlen(p1)+1;
		for (long i=0; i<len; i++){
			if (toupper(p1[i]) != toupper(p2[i])) return (toupper(p1[i])-toupper(p2[i]));
		}
	}
	return 0;
}

int tStrOp::StringLike(const char *str, const char* like){
	if (!str) return (!like || !like[0]);
	int i=0,j=0;
	int k;
	char tmp[256];
	char *s;
	while(1){
		if (like[j]=='_' || like[j]=='?'){
			j++;
			if (str[i]=='\0') return False;
			i++;
			continue;
		}else if (like[j]=='%' || like[j]=='*'){
			j++;
			while (like[j]=='%' || like[j]=='_' || like[j]=='*' || like[j]=='?') j++; 
			k=0;
			while (like[j]!='%' && like[j]!='_' && like[j]!='*' && like[j]!='?' && like[j]!='\0' && k<255){
				tmp[k++]=like[j++];
			}
			tmp[k]='\0';
			if (!k) return True;
			/*for (s=(char*) (str+i); *s; s++){
				if (!tStrOp::StrCmp(s, tmp, false)) break;
			}
			if (!s[0]) return False;
			*/
			//s=strstr(str+i,tmp);
			s=strNstr((char*) (str+i), tmp, false);
			if (s==NULL) return False;
			i=s-str+strlen(tmp);
		}else{
			if (like[j]=='\0') {
				if (str[i]=='\0') return True;
				else return False;
			}
			if (toupper(str[i])!=toupper(like[j])) return False;
			i++;
			j++;
		}
	}
	return False;
}
int tStrOp::StringMatch(const char *str,const char* match){
	if (!str) return (!match || !match[0]);
	int i=0,j=0;
	int k;
	char tmp[256];
	char *s;
	while(1){
		if (match[j]=='_' || match[j]=='?'){
			j++;
			if (str[i]=='\0') return False;
			i++;
			continue;
		}else if (match[j]=='%' || match[j]=='*'){
			j++;
			while (match[j]=='%' || match[j]=='_' || match[j]=='*' || match[j]=='?') j++; 
			k=0;
			while (match[j]!='%' && match[j]!='_' && match[j]!='*' && match[j]!='?' && match[j]!='\0' && k<255){
				tmp[k++]=match[j++];
			}
			tmp[k]='\0';
			if (!k) return True;
			s=strNstr((char*) (str+i), tmp, false);
			if (s==NULL) return False;
			i=s-str+strlen(tmp);
		}else{
			if (match[j]=='\0') {
				if (str[i]=='\0') return True;
				else return False;
			}
			if (toupper(str[i])!=toupper(match[j])) return False;
			i++;
			j++;
		}
	}
	return False;
}

long tStrOp::trimCRLFSpace(const char* p, ReferData* tx, long max_len){
	long lastpos=0;
	int issp=true;
	long pos=0;
	long first_pos=-1;
	for (pos=0; p[pos] ;pos++){
		if (p[pos]==' ' || p[pos]=='\t' || p[pos]=='\r' || p[pos]=='\n' || p[pos]=='<' || p[pos]=='>'){
			if (!issp) {
				if (!max_len || pos-lastpos+tx->L<max_len){
					if (tx->L) *tx+=" ";
					tx->Cat((char*)p+lastpos, pos-lastpos);
					issp=true;
					if (first_pos<0) first_pos=lastpos;
				}else{
					break;
				}
			}
		}else{
			if (issp){
				lastpos=pos;
				issp=false;
			}
		}
	}
	if (!issp){
		if (!max_len || pos-lastpos+tx->L<max_len){
			if (tx->L) tx->Cat(" ", 1);
			tx->Cat((char*)p+lastpos, pos-lastpos);
			if (first_pos<0) first_pos=lastpos;
		}else{
			if (tx->P && *tx->P && max_len>tx->L) tx->Cat(" ", 1);
			if (max_len>tx->L) tx->Cat((char*)p+lastpos, max_len-tx->L);
			tx->Cat("$",1);
		}
	}
	return first_pos; 
}

long tStrOp::countWords(const char* text){
	long count=0;
	int in_word=false;
	for (char* p1=(char*) text; p1&&*p1; p1++){
		if (isalpha(*p1)||tStrOp::isDigit(*p1)||*p1=='_'){
			if (!in_word) count++;
			in_word=true;
		}else{
			in_word=false;
		}
	}
	return count;
}

int tStrOp::countChar(char *str,char sep){
	int i,j=0;
	for (i=strlen(str)-1;i>=0;i--) 
		if (*(str+i)==sep) j++;
	return j;
}
int tStrOp::isValidPhoneNum(const char* phone){
	int len=strlen(phone); 
	if (len>30) len=30; 
	int i=0; 
	while (i<len && isspace(phone[i])) i++; 
	if (phone[i]=='+') i++; //+
	while (i<len && (isspace(phone[i]) || (phone[i]>='0' && phone[i]<='9'))) i++; 
	if (phone[i]=='(') i++; //(
	while (i<len && (isspace(phone[i]) || (phone[i]>='0' && phone[i]<='9') || phone[i]=='-' || phone[i]=='.' )) i++; 
	if (phone[i]==')') i++; //(
	while (i<len && (isspace(phone[i]) || (phone[i]>='0' && phone[i]<='9') || phone[i]=='-' || phone[i]=='.' )) i++; 
	if (i>=6 && i<=20) return true;
	return false; 
}
int tStrOp::IsValidEmail(const char*Email){
	int len = strlen(Email);
	int Now=0;
	char ch;
	int LeftBrace=false;
	int i=0;
	while (i< len){
		ch=Email[i];
		if ((ch == ' ' || ch == '\t') && (Now<3 || Now >6 ) ) {
			i++;
			continue;
		}
		if (ch == '\r' || ch == '\n') return false;
		switch (Now){
		case 0: // start 
			if (ch == '"' || ch == '\''){
				Now = 1;
				i++;
				continue;
			}else {
				Now=2;
				continue;
			}
			break;
		case 1: // name
			if (ch == '"' || ch == '\''){
				Now=2;
				i++;
				continue;
			}
			i++;
			break;
		case 2: // email start
			if (ch == '<') {
				LeftBrace = true;
				i++;
				Now=3;
				continue;
			}
			Now=3;
		case 3: // email account
			if (!(isalpha(ch) || tStrOp::isDigit(ch) || ch == '_' || ch == '-' || ch =='/' || ch == '.' )) 
				return false;
			while ( tStrOp::isDigit(ch) || tStrOp::isDigit(ch) || ch == '_' || ch == '-' || ch =='/' || ch == '.'){
				i++;
				if (i>=len) return false;
				ch=Email[i];
			}
			Now=4;
		case 4:
			if (ch != '@') return false;
			i++;
			Now=5;
			continue;
		case 5: //email domain
			if (!(isalpha(ch) || tStrOp::isDigit(ch) || ch == '_' || ch == '-' )) 
				return false;
			while ( isalpha(ch) || tStrOp::isDigit(ch) || ch == '_' || ch == '-' || ch =='.' ) {
				i++;
				if (i>=len) return (!LeftBrace);
				ch=Email[i];
			}
			Now=6;
		case 6:
			if (LeftBrace){
				if (ch != '>') return false;
				else {
					i++;
					if (i>=len ) return true;
					ch=Email[i];
				}
			}
			Now=7;
		case 7:
			if (ch == '\0') return true;
			if (ch != ' ' && ch != '\t') return false;
			i++;
			continue;
		default:
			return false;
		}
	}

	return (Now==7);
}

bool tStrOp::IsEmailChar( char p )
{
	return ( ( p>='0' && p<='9' ) ||
		p =='.' ||
		( p>='A' && p<='Z' ) ||
		( p>='a' && p<='z' ) ||
		p =='_' || p == '-' 
		);
}
char* tStrOp::EmailLeftChar(const char *start, const char *head)
{
//	ASSERT( start && head );	
	char *pdest=0;

	if (start >= head){
		char *p = (char*) start;
		while (p>head && IsEmailChar(*p)) p--;
		if (p<start && !IsEmailChar(*p)) p++;
		while (*p=='.') p++; 

		int two_dot=false;
		for (char* p1=p; p1<start; p1++){
			if (*p1=='.') {
				if (p1[1]=='.') {
					two_dot=true;
					break;
				}
			}
		}


		if (p<=start && !two_dot) {
			pdest=p;
		}
	}

	return pdest;
}
char* tStrOp::EmailRightChar(const char *start)
{
//	ASSERT( start );
	char *pdest =0;

	char *p = (char*) start;
	while( *p &&  IsEmailChar( *p )  )
		p ++;
	while (p>start && *(p-1)=='.') p--;

	int has_dot=false;
	int two_dot=false;
	for (char* p1=(char*) start; p1<p; p1++){
		if (*p1=='.') {
			has_dot=true;
			if (p1[1]=='.') {
				two_dot=true;
				break;
			}
		}
	}

	if( p>start && has_dot && !two_dot) {
		pdest = p;
	}

	return pdest;
}

char* tStrOp::GetEmail(const char *p, ReferData* email, ReferData* name)
{
	email->reset();
	name->reset();
	char *pdest = strchr((char*) p, '@');
	if( pdest ){
		// get start of email
		char *start = EmailLeftChar(pdest-1, p);
		if (start){
			// get the end of email
			char *end = EmailRightChar(pdest+1);
			if (end){
				int len = end - start;
//				ASSERT( len <200 );
				email->Set( (char*)start, len, true );
				if (strstr(email->P, "..")){
					email->reset();
				}else{
					EmailName(start, p, name);
				}
				pdest = end;
			}else
				pdest++;
		}else
			pdest++;
	}

	return pdest;
}
char* tStrOp::EmailName(const char*emailstart, const char* head, ReferData* name){
//	ASSERT( emailstart && head );
	if (!emailstart || !head) return 0;
	char* p=(char*) emailstart-1;
	if (p>head && *p=='<') p--;
	while (p>head && isSpace(*p) ) p--;
	char quote=0;
	if (*p=='"' || *p=='\'') quote=*p;
	char* dest=p;
	if (quote && p>head){ //has qutation
		p--;
		char* p1=p-1;
		while (p1>head && *p1!=quote && p-p1<100) p1--;
		dest=p1;
		if (*p1==quote){
			p1++;
			name->Set(p1, p-p1+1, true);
		}
	}else if (p>head) { //no qutation
		if (p>head && *p==':') p--;
		name->reset(); 
		ReferData word, keep_name, keep; 
		int count=0, keep_count=0, word_count=0, this_type=0; //type=0: small case; 1: one Cap-word; 2: two Cap-word
		p++; 
		const char* ignores[]={"at", "on", "of", "to", 0}; 
		while (p>=head){
			word.reset(); 
			p=EmailNameWordLeft(p-1, head, &word);
			if (word.L && tStrOp::strMcmp(word.P, ignores, false)<0 ) { //not at,
				if (word.P[0]>='A' && word.P[0]<='Z'){
					this_type++;
				} else 
					this_type=0; 

				if (this_type==0 && keep_count>0) break; 

				keep=word;
				if (this_type<=1 && count<4 && keep_count==0){ //regular name
					if (name->L) {
						word+=" "; 
						word+=*name; 
					}
					*name=word; 
					count++; 
				}
				if (this_type>=1 && this_type<4){ //capped name
					if (this_type==1){
						keep_name=keep;
					}else{
						keep+=" "; 
						keep+=keep_name; 
						keep_name=keep; 
					}
				}

				if (this_type>=2 && this_type>keep_count){ //at least two capped word
					*name=keep_name;
					keep_count=this_type;
				}

				if (keep_count>=4) break; //long capped word

				/*
				if ((word.P[0]<'A' || word.P[0]>'Z') && count>=2) break; //small case word and have name already
					if (name->L) {
						word+=" "; 
						word+=*name; 
					}
					*name=word; 
					count++; 
				*/
			} else if (!word.L && keep_count==0){
				p--;
				if (p>head && *p=='>'){ 
					//dirty method to skip a tag
					while (p>=head && *p!='<') p--; 
				}
			}else if (keep_count>0) { //already have a name
				break;
			}
			
			if (++word_count>100) break; 
			//if (count>=4) break; 
		}
		dest=p;

		/*
		char* p1=p;
		const char* stop_chars="\"'=<>";

		while (p1>head && !isSpace(*p1) && !strchr(stop_chars, *p1) && p-p1<50) p1--;
		if (isSpace(*p1) || strchr(stop_chars, *p1) ) p1++;
		dest=p1;
		name->Set(p1, p-p1+1, true);*/
	}
	return dest;
}
char* tStrOp::EmailNameWordLeft(const char*start, const char* head, ReferData* name){
	char* p=(char*) start;
	while (p>head && isSpace(*p) ) p--;
	char* p1=p;
	const char* stop_chars="\"'=<>";

	while (p1>head && !isSpace(*p1) && !strchr(stop_chars, *p1) && p-p1<50) p1--;
	if (isSpace(*p1) || strchr(stop_chars, *p1) ) p1++;
	name->Set(p1, p-p1+1, true);
	return p1;
}
int tStrOp::sepStrPara(char* fmt, ReferLink2* links){
	const char* flags="+- 0#";
	const char* lengths="hlL";
	char* p1=fmt;
	ReferData buf, shortfmt;
	long width=0, prec=0;
	ReferData result(128);
	ReferLink2* link; 
	while (p1 && *p1){
		char* p2=strchr(p1, '%'); 
		while (p2 && p2[1]=='%') p2=strchr(p2+2, '%'); //%%
		if (p2){
			buf.Set(p1, p2-p1, true);
			convertSpaceSpecial(buf.P); 
			buf.L=strlen(buf.P); 

			link=links->insert(); 
			link->Name=buf; 

			char* p3=p2+1;
			//flags	
			if (strchr(flags, *p3)) p3++; 
			//width
			width=0; 
			sscanf(p3, "%ld", &width); 
			while (*p3 && tStrOp::isDigit(*p3) ) p3++; 

			//prec
			prec=0;
			if (*p3=='.') {
				p3++; 
				sscanf(p3, "%ld", &prec); 
				while (*p3 && tStrOp::isDigit(*p3) ) p3++; 
			}

			//length
			if (strchr(lengths, *p3)) p3++;
			
			//format
			if (*p3) p3++; 

			shortfmt.Set(p2, p3-p2, true); 

			link=links->insert(); 
			link->Name=shortfmt; 
				
			p1=p3; 
		}else{//no %
			buf=p1;
			convertSpaceSpecial(buf.P); 
			buf.L=strlen(buf.P); 
			link=links->insert(); 
			link->Name=buf; 
			p1=0;
		}
	}
	return 0;
}

long tStrOp::GetEmails(const char* start, ReferLinkHeap *emails )
{
	if( !start ) return false;

	char* p=(char*) start;
	ReferData email, name;
	long count=0;
	while( p && *p != 0 ) {
		p = GetEmail(p, &email, &name);
		if (email.L){
			count++;
			emails->add(&email, &name, emails->Total); //emails->Total may not be the same as count
		}
	}
	return count;
}
long tStrOp::GetEmails(const char*text, const char* fmt, ReferData* result){
	ReferLinkHeap emails; 
	emails.setCaseSensitivity(false);
	emails.setDuplication(false);

	tStrOp::GetEmails(text, &emails); 

	emails.setSortOrder(SORT_ORDER_NUM_INC); 
	emails.resort(); 

	ReferLink2 links; 
	if (fmt && *fmt) tStrOp::sepStrPara((char*) fmt, &links); 

	for (ReferLink* link=(ReferLink*) emails.moveFirst(); link; link=(ReferLink*) emails.moveNext()){
		if (fmt && *fmt){
			for (ReferLink2* link2=(ReferLink2*) links.Next; link2!=&links; link2=(ReferLink2*) link2->Next){
				if (!link2->Name.Cmp("%n", 2, false)) {
					*result+=link->Value; //name; 
				}else if (!link2->Name.Cmp("%m", 2, false)) {
					*result+=link->Name; //email
				}else{
					*result+=link2->Name; 
				}
			}
		}else{
			if (result->L) *result+=";";
			if (link->Value.L){
				*result += "\""; 
				*result += link->Value; //name
				*result += "\" "; 
			}
			*result += "<"; 
			*result += link->Name; //email
			*result += ">\n"; 
		}
	}
	return emails.Total;
}
int tStrOp::trimRight(char* str, char trim){
	if (!str) return 0;
	int i=strlen(str);
	if (trim){
		for ( ; i>0 && str[i-1]==trim; i--) 
			str[i-1]=0;
	}else{
		for ( ; i>0 && (str[i-1]==' ' || str[i-1]=='\n' || str[i-1]=='\r' || str[i-1]=='\f' || str[i-1]=='\t'); i--)
			str[i-1]=0;
	}
	return i; //length
}

int tStrOp::trimLeft(char* str, char trim){
	if (!str) return 0;
	int i=0,j=strlen(str);
	if (trim){
		for (i=0; (str[i]==trim); i++);
	}else{
		for (i=0 ; (str[i-1]==' ' || str[i-1]=='\n' || str[i-1]=='\r' || str[i-1]=='\f' || str[i-1]=='\t'); i++);
	}
	if (i){
		for (j=0;str[i]!='\0'; i++,j++){
			str[j]=str[i];
		}
		str[j]='\0';
	}
	return j;
}
long tStrOp::trimText(const char*text, ReferData* tx, const char* pattern, const char* mark){
				//mark: ="1010": first, middle, last, remove all spaces; 0=no trim; 1=trim; 2=trim CRLF; 3=trim all;
				//		="121"; trim, no CRLF
	if (!text) return 0; 
	if (!pattern){
		pattern=" \t\n\r"; 
	}
	if (!mark){
		mark="1010";
	}
	long marklen=mark?strlen(mark):0; 

	char* p1=(char*) text; 
	if (mark[0]>='1'){ //trim left
		while (p1 && *p1 && strchr(pattern, *p1)) p1++;
	}
	char* p2=p1+strlen(p1); 
	if (marklen>2 && mark[2]>='1'){ //trim right
		while (p2>p1 && strchr(pattern, *(p2-1))) p2--;
	}
	if (marklen>1 && mark[1]>='1'){
		tx->Malloc(p2-p1+2);
		long i=0, j=0;
		int inpattern=0;
		int count_r=0;
		int count_n=0;
		for (i=0; i<p2-p1; i++){
			if (strchr(pattern, p1[i])){
				if (inpattern==0) inpattern=1;
				if (p1[i]=='\r') count_r++;
				if (p1[i]=='\n') count_n++; 
			}else{
				if (inpattern){ //next to pattern
					if (mark[1]>='3' || (marklen>3 && mark[3]>='1')){ //remove any space in the results
					}else if (mark[1]>='2'){//remove any \r\n\t
						tx->P[j++]=pattern[0];
					}else{
						if (count_r==0 && count_n==0){tx->P[j++]=pattern[0];} //leave one space in the result string
						else if (count_r<2 && count_n<2){tx->P[j++]='\n';} //leave one \n in the result string
						else {tx->P[j++]='\n';tx->P[j++]='\n'; }; //leave two \n in the result string
					}
				}
				tx->P[j++]=p1[i];
				inpattern=count_r=count_n=0;
			}
		}
		tx->P[j]=0;
		tx->L=j;
	}else{
		tx->Set(p1, p2-p1, true);
	}
	return tx->L;
}

int tStrOp::toLowerCase(char* str){
	int i;
	for ( i=0; str[i] != '\0'; i++){
		if (str[i] >='A' && str[i]<='Z')
			str[i]=str[i] - 'A' + 'a';
	}
	return i;
}

int tStrOp::toUpperCase(char* str){
	int i;
	for ( i=0; str[i] != '\0'; i++){
		if (str[i] >='a' && str[i]<='z')
			str[i]=str[i] - 'a' + 'A';
	}
	return i;
}

int tStrOp::replaceInplace(char* str, char* source, char* dest, int case_sensitive){
	if (!str) return 0;
	int lensource=strlen(source);
	int lendest=strlen(dest);
	int lendiff=lendest-lensource;
	char* p=(char*) malloc(sizeof(char) * (strlen(str)+lendest) );
	if (!p) return -1;

	char* p1=strNstr(str, source, case_sensitive);
	int ReplaceNum=0;
	while (p1){
		strcpy(p,p1+lensource);
		strcpy(p1,dest);
		strcat(p1,p);
		ReplaceNum++;
		p1= strNstr(p1+lendest, source, case_sensitive);
	}
	free(p);
	return ReplaceNum;
}

char* tStrOp::replaceMalloc(char* str, char* source, char* dest, long maxstrlen, int case_sensitive){
	if (!str) return 0;
	if (maxstrlen<=0) maxstrlen=strlen(str);
	int lensource=strlen(source);
	int lendest=strlen(dest);
	char* str1=(char*) malloc(sizeof(char) * (strlen(str)+1) );
	if (!str1) return NULL;
	strncpy(str1,str,maxstrlen); str1[maxstrlen]=0;
	char* p=NULL;
	char* p1=strNstr(str1, source, case_sensitive);
	int ReplaceNum=0;
	while (p1){
		p=(char*) malloc(sizeof(char) * (strlen(str1)+lendest) );
		if (!p) {
			free(str1);
			return NULL;
		}
		strncpy(p,str1,p1-str1);
		strcpy(p + (p1-str1), dest);
		strcat(p, p1+lensource);
		p1=p+(p1-str1);
		free(str1);
		str1=p;
		p=NULL;
		ReplaceNum++;
		p1= strNstr(p1+lendest, source, case_sensitive);
	}
	return str1;
}
int tStrOp::replaceChars(char* str, char* chars, char* replace){
	unsigned long i;
	for (i=0; i<strlen(chars); i++){
		if (i<strlen(replace)){
			//do replacement
			char*p1=str;
			while (*p1){
				if (*p1==chars[i]) *p1=replace[i];
				p1++;
			}
		}else{
			//just delete the corresponding chars
			char*p1=str;
			char*p2=str;
			while (*p1){
				if (*p1!=chars[i]) {
					*p2=*p1;
					p2++;
				}
				p1++;
			}
			*p2=0;
		}
	}
	return 0;
}

int tStrOp::convertSpaceSpecial(char* str){
	if (!str) return 0;

	long i=0, j=0; 
	long len=strlen(str);

	while (str[i]){
		if (str[i]=='\\'){
			if (str[i+1]=='n'){
				str[j++]='\n';
				i+=2;
			}else if (str[i+1]=='r'){
				str[j++]='\r';
				i+=2;
			}else if (str[i+1]=='t'){
				str[j++]='\t';
				i+=2;
			}else if (str[i+1]=='b'){
				str[j++]=' ';
				i+=2;
			}else if (str[i+1]=='\\'){
				str[j++]='\\';
				i+=2;
			}else{
				str[j++]=str[i++];
			}
		}else{
			str[j++]=str[i++];
		}
	}
	str[j]=0;
	return 0;
}
int tStrOp::dequoteString(char* str){
	if (!str) return 0;

	long i=0, j=0; 
	long len=strlen(str);

	while (str[i]){
		if (str[i]=='\\'){
			if (str[i+1]=='n'){
				str[j++]='\n';
				i+=2;
			}else if (str[i+1]=='r'){
				str[j++]='\r';
				i+=2;
			}else if (str[i+1]=='t'){
				str[j++]='\t';
				i+=2;
			}else if (str[i+1]=='b'){
				str[j++]=' ';
				i+=2;
			}else if (str[i+1]=='\''){
				str[j++]='\'';
				i+=2;
			}else if (str[i+1]=='"'){
				str[j++]='"';
				i+=2;
			}else if (str[i+1]=='\\'){
				str[j++]='\\';
				i+=2;
			}else{
				str[j++]=str[i++];
			}
		}else if (str[i]=='\'' && str[i+1]=='\''){
			str[j++]='\'';
			i+=2;
		}else{
			str[j++]=str[i++];
		}
	}
	str[j]=0;
	return 0;
}
char* tStrOp::quoteString(const char* str, ReferData* result){
	result->reset(); 
	result->P=quoteString(str, &result->Type, &result->Size, &result->L);
	return result->P;
}

char* tStrOp::quoteString(const char* str, int*is_malloc, long* size, long* length){
	*is_malloc=false;
	if (size) *size=0;
	if (length) *length=0;
	if (!str) return 0;
	size_t i, j, len=strlen(str), added=0;
	const char* quoted_chars="\\\"'\r\n";
	const char* escaped_chars="\\\"'rn";
	char* p;
	for (i=0; i<len; i++){
		if (strchr(quoted_chars, str[i])) added+=1;
	}
	if (!added) {
		*is_malloc=false;
		if (length) *length=strlen(str);
		return (char*) str;
	}

	char* res=(char*) malloc(sizeof(char)*(added+len+1));
	res[0]=0;
	j=0;
	for (i=0; i<len; i++){
		if ((p=strchr((char*) quoted_chars, str[i]))){
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "\\");
			strncat(res, escaped_chars+(p-quoted_chars), 1);
		}
	}
	strcat(res, str+j);

	*is_malloc=true;
	if (size) *size=added+len+1;
	if (length) *length=strlen(res);
	return res;
}

char* tStrOp::quoteText(const char* str, int*is_malloc, long* size, long* length){
	*is_malloc=false;
	if (size) *size=0;
	if (length) *length=0;
	if (!str) return 0;
	size_t i, j, len=strlen(str), added=0;
	for (i=0; i<len; i++){
		if (str[i]=='"' || str[i]=='\\') added+=10;
	}
	if (!added) {
		*is_malloc=false;
		if (length) *length=strlen(str);
		return (char*) str;
	}

	char* res=(char*) malloc(sizeof(char)*(added+len+1));
	res[0]=0;
	j=0;
	for (i=0; i<len; i++){
		if (str[i]=='"'){
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "&quot;");
		}else if (str[i]=='\\'){
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "&bslash;");
		}
	}
	strcat(res, str+j);

	*is_malloc=true;
	if (size) *size=added+len+1;
	if (length) *length=strlen(res);
	return res;
}
int tStrOp::dequoteText(char* str){
	replaceInplace(str, "&bslash;", "\\");
	return replaceInplace(str, "&quot;", "\"");
}
char* tStrOp::encodeHtml(const char* str, int*is_malloc, long* size, long* length){
	*is_malloc=false;
	if (size) *size=0;
	if (length) *length=0;
	if (!str) return 0;
	size_t i, j, len=strlen(str), added=0;
	for (i=0; i<len; i++){  
		//keep consistent with the following transformation block
		if (str[i]=='<' || str[i]=='>'|| str[i]=='"' || str[i]=='\'' || str[i]=='\\' || str[i]=='&') added+=10;
	}
	if (!added) {
		*is_malloc=false;
		if (length) *length=strlen(str);
		if (size) *size=strlen(str);
		return (char*)str;
	}

	char* res=(char*) malloc(sizeof(char)*(added+len+1));
	res[0]=0;
	j=0;
	for (i=0; i<len; i++){
		if (str[i]=='>'){ //keep consistent with the above malloc counting sentence
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "&gt;");
		}else if (str[i]=='<'){
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "&lt;");
		}else if (str[i]=='"'){
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "&quot;");
		}else if (str[i]=='\''){
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "&#39;"); //use Decimal 39? Octal 047; Hex 027;
		}else if (str[i]=='\\'){
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "&#92;"); //use Decimal 39? Octal 047; Hex 027;
		}else if (str[i]=='&'){
			strncat(res, str+j, i-j);
			j=i+1;
			strcat(res, "&amp;");
		}else if (str[i]=='\r' && str[i+1]=='\n'){
			strncat(res, str+j, i-j);
			j=i+1;
		}
	}
	strcat(res, str+j);

	*is_malloc=true;
	if (size) *size=added+len+1;
	if (length) *length=strlen(res);
	return res;
}
char* tStrOp::encodeHtml(const char* str, ReferData* result){
	result->reset(); 
	result->P=encodeHtml(str, &result->Type, &result->Size, &result->L);
	return result->P;
}
int tStrOp::decodeHtml(char* str){
	if (!str) return 0;
	long len=str?strlen(str):0;
	long i=0, j=0; 
	int num=0;
	for (i=0; i<len; i++){
		if (str[i]=='&'){
			if (!strncmp(str+i+1, "nbsp;", 5)) {str[j++]=' '; i+=5; num++;}
			else if (!strncmp(str+i+1, "lt;", 3)) {str[j++]='<'; i+=3; num++;}
			else if (!strncmp(str+i+1, "gt;", 3)) {str[j++]='>'; i+=3; num++;}
			else if (!strncmp(str+i+1, "quot;", 5)) {str[j++]='"'; i+=5; num++;}
			else if (!strncmp(str+i+1, "bslash;", 7)) {str[j++]='\\'; i+=7; num++;}
			else if (!strncmp(str+i+1, "amp;", 4)) {str[j++]='&'; i+=4; num++;}
			else if (i+3<=len && str[i+1]=='#'){
				unsigned int code=' ';
				int skip=2;
				if (str[i+2]=='0'){
					sscanf(str+i+2, "%d", &code); //Octet? %o?  No. It should be %d 
					skip=3;
					while (skip<8 && tStrOp::isDigit(str[i+skip])) skip++;
				}else if (str[i+2]=='x' || str[i+2]=='X'){
					sscanf(str+i+3, "%x", &code); //Octet? 
					skip=3;
					while (skip<8 && (tStrOp::isDigit(str[i+skip]) || (str[i+skip]>='A' && str[i+skip]<='F') || (str[i+skip]>='a' && str[i+skip]<='f'))) skip++;
				}else{
					sscanf(str+i+2, "%d", &code); //Decimal? 
					while (skip<8 && tStrOp::isDigit(str[i+skip])) skip++;
				}
				if (str[i+skip]==';') {
					ReferData udf; 
					encodeUTF8(code, &udf); 
					for (int k=0; k<udf.L; k++){
						str[j++]=udf.P[k]; 
					}
					i+=skip; num++;
				}else{
					str[j++]=str[i];
				}
			/*}else if (i+4<=len && str[i+1]=='#' && str[i+4]==';') { //only 2 digits? 
				int code=' ';
				sscanf(str+i+2, "%d", &code); //Decimal? 
				str[j++]=(char) code;
				i+=4; num++;
			}else if (i+5<=len && str[i+1]=='#' && str[i+2]=='0' && str[i+5]==';') { //only 2 digits?
				int code=' ';
				sscanf(str+i+2, "%o", &code);
				str[j++]=(char) code;
				i+=5; num++; */
			}else{
				str[j++]=str[i];
			};
		}else{
			str[j++]=str[i];
		}
	}
	str[j]=0;
#ifdef _WINDOWS
	ASSERT(j<=len);
#endif
/*
	int num=replaceInplace(str, "&nbsp;", " ");
	num+=replaceInplace(str, "&lt;", "<");
	num+=replaceInplace(str, "&gt;", ">");
	num+=replaceInplace(str, "&quot;", "\"");
	num+=replaceInplace(str, "&bslash;", "\\");
	num+=replaceInplace(str, "&amp;", "&");
	*/
	return num;
}
int tStrOp::decodeHtml(ReferData* str){
	str->Seperate();
	int num=decodeHtml(str->P); 
	str->L=str->P?strlen(str->P):0; 
	return num;
}

char* tStrOp::encodeUrl(char* url){ //caller need to free the result str
	//also see HtmlBuffer::encodeUrl();
	ReferData EncodeUrl;
	char buf[10], buf1[10];
	size_t len=strlen(url);
	EncodeUrl.Malloc(2*len);

	size_t i;
	for (i=0; i<len; i++){
		if (0 && i<len-1 && (url[i]&0x80) && (url[i+1]&0x80) ){
			buf1[0]=url[i];
			buf1[1]=url[i+1];
			buf1[2]=0; 

			char*p=encodeUTF8(buf1);
			EncodeUrl+=p; 
			i++;
			free(p); 
		}else{
			switch (url[i]){
			case '\r':
			case '\n':
			case '%':
			case '&':
			case '?':
			case '#':
			case '\t':
			case ':':
			case '~':
			case '(':
			case ')':
			case '<':
			case '>':
			case '|':
			case ',':
			case '"':
			case '\'':
			case '\\':
				sprintf(buf, "%%%02X", url[i]);
				EncodeUrl.Cat(buf, 3);
				break;
			case ' ':
				EncodeUrl.Cat("+", 1);
				break;
			default:
				if (url[i]<=0){
					sprintf(buf, "%%%02X", (unsigned char) url[i]);
					EncodeUrl.Cat(buf, 3);
				}else{
					EncodeUrl.Cat(url+i, 1);
				}
			}
		}
		if (EncodeUrl.L+3 >= EncodeUrl.Size) EncodeUrl.ReMalloc(EncodeUrl.Size+len);
	}
	EncodeUrl.setToFree(false); //caller free
	return EncodeUrl.P;
}
char* tStrOp::encodeUrlSpecial(char* url){ //caller need to free the result str
	//also see encodeUrl();
	ReferData EncodeUrl;
	char buf[10];
	size_t len=strlen(url);
	EncodeUrl.Malloc(2*len);

	size_t i;
	for (i=0; i<len; i++){
		if (url[i]<=0){
			sprintf(buf, "%%%02X", (unsigned char) url[i]);
			EncodeUrl.Cat(buf, 3);
		}else{
			EncodeUrl.Cat(url+i, 1);
		}
		if (EncodeUrl.L+3 >= EncodeUrl.Size) EncodeUrl.ReMalloc(EncodeUrl.Size+len);
	}
	EncodeUrl.setToFree(false); //caller free
	return EncodeUrl.P;
}
char* tStrOp::decodeUrl(char* url){ //caller need to free the result str
	//also refer: CGI::convertSpecial()
	ReferData EncodeUrl;
	char buf[10];
	size_t len=strlen(url);
	EncodeUrl.Malloc(len);

	size_t i=0;
	while (i<len){
		switch (url[i]){
		case '%':
			buf[0]=buf[1]=0;
			sscanf(url+i+1, "%02X", buf);
			if (buf[0]){
				EncodeUrl.Cat(buf,1);
				i+=2;
			}else{
				EncodeUrl.Cat(url+i, 1);
			}
			break;
		case '+':
			EncodeUrl.Cat(" ", 1);
			break;
		default:
			EncodeUrl.Cat(url+i, 1);
		}
		if (EncodeUrl.L+3 >= EncodeUrl.Size) EncodeUrl.ReMalloc(EncodeUrl.Size+=len);
		i++;
	}
	EncodeUrl.setToFree(false); //caller free
	return EncodeUrl.P;
}
int tStrOp::splitUTF8(const char* text, ReferLinkHeap* results){
	if (!text) return false; 
	long len=strlen(text);  
	long i=0, j=0, k=0, count=results->Total;
	ReferData word;
	while (i<len){
		if ((text[i] & 0xC0) == 0xC0) {
			if (i>j){
				word.Set((char*) text+j, i-j, false); 
				results->add(&word, 0, count++);
			}
			k=1;
			if (!(text[i] & 0x20) && i+1<len){ //110x,xxxx 10xx,xxxx
				k=2; 
			}else if (!(text[i] &0x10) && i+2<len){ //1110,xxxx 10xx,xxxx 10xx,xxxx
				k=3; 
			}else if (!(text[i] &0x08) && i+3<len){ //1111,0xxx 10xx,xxxx 10xx,xxxx 10xx,xxxx
				k=4; 
			}
			word.Set((char*) text+i, k, false);
			results->add(&word, 0, count++);
			i+=k;
			j=i;
		}else if (tStrOp::isSpace(text[i]) || !tStrOp::isAN_(text[i]) ){
			if (i>j){
				word.Set((char*) text+j, i-j, false); 
				results->add(&word, 0, count++);
			}
			i+=1;
			j=i;
		}
	}
	if (i>j){
		word.Set((char*) text+j, i-j, false); 
		results->add(&word, 0, count++);
	}
	return 0;
}
int tStrOp::decodeUTF8(char*text, ReferData* result){
	result->reset(); 
	if (!text) return false; 
	long len=strlen(text);  
	long i, j;
	result->Set(text, len, false); 
	for (i=0; i<len; i++) {
		if ((text[i] & 0xC0)==0xC0) {
			result->Seperate(); break; 
		}
	}
	if (result->Type){
		i=0; j=0;
		while (i<len){
			if ((text[i] & 0xC0) == 0xC0) {
				if (!(text[i] & 0x20) && i+1<len){ //110x,xxxx 10xx,xxxx
					result->P[j]=(text[i] & 0x1F) >> 2; 
					result->P[j+1]=(text[i+1] & 0x3F) + ((text[i] & 0x03)<<6);
					i+=2; j+=2;
				}else if (!(text[i] &0x10) && i+2<len){ //1110,xxxx 10xx,xxxx 10xx,xxxx
					result->P[j]=((text[i] & 0x0F)<<4) + ((text[i+1] & 0x3F)>>2); 
					result->P[j+1]=((text[i+1] & 0x3F)<<6) + (text[i+2] & 0x3F);
					i+=3; j+=2;
				}else if (!(text[i] &0x08) && i+3<len){ //1111,0xxx 10xx,xxxx 10xx,xxxx 10xx,xxxx
					result->P[j]=((text[i] & 0x07)<<2) + ((text[i+1] & 0x3F)>>4); 
					result->P[j+1]=((text[i+1] & 0x0F)<<4) + ((text[i+2] & 0x3F)>>2);
					result->P[j+2]=((text[i+2] & 0x03)<<6) + ((text[i+3] & 0x3F));
					i+=4; j+=3;
				}
			}else{
				result->P[j++]=text[i++];
			}
		}
		result->P[j]=0;
		result->L=j;
		return true;
	}

	return false;
}

char* tStrOp::encodeUTF8(char*text){ //caller need to free the result str
	ReferData EncodeUrl;
	unsigned char d[3];
	char buf[20];
	size_t len=strlen(text);
	EncodeUrl.Malloc(2*len);

	size_t i;
	for (i=0; i<len; i++){
		unsigned char c=text[i];
		if (c>=0 && c<=0x7F){
			EncodeUrl.Cat(text+i, 1); 
		}else if (c>=0x80 && i<len-1){ // && c<=0xBF 
			d[2]=0x80 + (text[i+1] & 0x3F); 
			d[1]=0x80 + (((unsigned char)(text[i] & 0x0F))<<2) + (((unsigned char)(text[i+1] & 0xC0))>>6); 
			d[0]=0xE0 + (((unsigned char)(text[i] & 0xF0))>>4); 
			sprintf(buf, "%%%02X%%%02X%%%02X", d[0], d[1], d[2]);
			EncodeUrl.Cat(buf, 9); 
			i++;
		}else{
			sprintf(buf, "%%%02x", text[i]);
			EncodeUrl.Cat(buf, 3); 
		}
	}
	EncodeUrl.setToFree(false); //caller free
	return EncodeUrl.P;
}
int tStrOp::encodeUTF8(unsigned	int ch, ReferData* result){
	result->Malloc(5); 
	if (ch<0x80){
		result->P[0]=ch; 
		result->P[1]=0; 
		result->L=1;
	}else if (ch>=0x80 && ch<0x0800){
		result->P[0]=((ch>>6) & 0x1F) + 0xC0; 
		result->P[1]=(ch & 0x3F) + 0x80; 
		result->P[2]=0; 
		result->L=2;
	}else if (ch>=0x800 && ch<0x10000){
		result->P[0]=((ch>>12) & 0x0F) + 0xE0; 
		result->P[1]=((ch>>6) & 0x3F) + 0x80; 
		result->P[2]=(ch & 0x3F) + 0x80; 
		result->P[3]=0; 
		result->L=3;
	}else if (ch>=0x10000 && ch<0x110000){
		result->P[0]=((ch>>18) & 0x07) + 0xF0; 
		result->P[1]=((ch>>12) & 0x3F) + 0x80; 
		result->P[2]=((ch>>6) & 0x3F) + 0x80; 
		result->P[3]=(ch & 0x3F) + 0x80; 
		result->P[4]=0; 
		result->L=4;
	}
	return result->L;
}

int tStrOp::splitString(const char* str, char* sep, ReferLinkHeap* results){
	if (!str) return 0;
	char* p=(char*) str, *p1=0, *p2;
	ReferData substr;
	long count=results->Total;
	while (p && *p) {
		if (sep) p1=strstr(p, sep);
		if (!p1) p1=p+strlen(p); //separator position

		while (p<p1 && isSpace(*p)) p++; //first nonspace char
		if (p1-p>0){
			p2=p1-1;
			while (p2>=p && isSpace(*p2)) p2--; //last nonspace char
			if (p2>=p){
				substr.Set(p, p2-p+1, false);
				results->add(&substr, 0, count++);
			}
		}

		if (p1 && *p1 && sep) p=p1+strlen(sep);
		else p=p1;
	}
	return 0;
}
int tStrOp::parseCookiesString(const char* cookies_str, ReferLinkHeap* cookies){
	char* p0=(char*) cookies_str, *p1, *p2, *p3, *p4;
	ReferData name, value;
	int count=0;
	for (p1=p0; *p1; ){
		while (*p1 && (*p1==' '||*p1=='\t'||*p1=='\r'||*p1=='\n')) p1++; //p1: the start pos of cookie name
		for (p2=p1; *p2 && *p2!=';' && *p2!='=' && *p2!='\r' && *p2!='\n'; p2++); //p2: '='
		if (*p2=='=' || *p2==';'){
			name.Set(p1, p2-p1, true); 
			p3=p2; if (*p3=='=') p3++; 

			while (*p3 && *p3!=';' && *p3!='\r' && *p3!='\n') p3++;
			p4=p3; //end of cookie
			while (p3>p2 && (*p3==0 || *p3==';' || *p3==' '||*p3=='\t'||*p3=='\r'||*p3=='\n')) p3--; //remove trailing spaces
			value.Set(p2+1, p3-p2, true);

			cookies->add(&name, &value, ++count);

			p1=p4+1; //p1 skip to p3
		}else{
			p1=p2;
		}
	}
	return 0;
}

int tStrOp::splitCsvString(const char* str, char* sep, ReferLinkHeap* results){
	if (!str) return 0;
	char* p=(char*) str, *p1, *p2;
	ReferData substr;
	ReferData quotation;
	long count=0;
	while (p && *p) {
		p1=strstr(p, sep);
		if (!p1) p1=p+strlen(p); //separator position

		while (p<p1 && isSpace(*p)) p++; //first nonspace char
		if (p==p1){
			results->add("", "", count++);
		}else{
			while (1){
				p2=p1-1;
				while (p2>=p && isSpace(*p2)) p2--; //last nonspace char
				if ((*p!='\'' && *p!='"' && *p!='#') || *p2==*p || p2==p || !*p1 ) break; //not ' and not "

				//find the next separator
				p1=strstr(p1+1, sep);
				if (!p1) p1=p+strlen(p); //separator position
			}

			if (p2>=p){
				if ((*p=='\'' || *p=='"' || *p=='#') && *p2==*p && p2>p){
					substr.Set(p+1, p2-p-1, false);
					quotation.Set(p, 1, true);
				}else{
					substr.Set(p, p2-p+1, false);
					quotation.reset();
				}
				
				results->add(&substr, &quotation, count++);
			}
		}

		if (p1 && *p1) p=p1+strlen(sep);
		else p=p1;
	}
	return 0;
}
int tStrOp::getCSVFields(const char* csv_source, ReferLink** fields, ReferData* field_sep, ReferData* line_sep, int* quoted_field){
	//char quot=0;
	if (!csv_source || !csv_source[0]) {
		if (!field_sep->L) {
			*field_sep="\t";
			*quoted_field = '"';
		}
		if (!line_sep->L) *line_sep="\n";
		return 0;
	}

	ReferData field_name;
	ReferData field_value;
	ReferData field_assign;
	char buf[256];

	char* p1=(char*) csv_source, *p2=0, *p3, *p4, *p5;
	//find line end;
	char* line_end=0;
	if (line_sep->L){ //use the existing line_sep
		line_end=strstr((char*) csv_source, line_sep->P);
		if (!line_end) line_end=(char*)csv_source+strlen(csv_source);
	}else{ //find new line seperator
		for (line_end=(char*)csv_source; *line_end && *line_end!='\r' && *line_end!='\n'; line_end++);
		for (p2=line_end; *p2=='\r'; p2++); //ignore the leading \r
		for (p3=p2; *p3=='\r' || *p3=='\n'; p3++);
		if (*p2) line_sep->Set(p2, p3-p2, true);
		else *line_sep="\n";
	}

	//find field separator
	char* separators="\t|:;,";
	int fields_count=0;
	if (!field_sep->L){ //find field separator
		char* separator=separators;
		while (*separator){
			int quot=0;
			for (p2=(char*)csv_source; *p2 && p2<line_end; p2++) {
				if (!quot && (*p2=='"' || *p2=='\'') ){
					quot=*p2; 
				}else if (quot && *p2==quot){
					quot=0; 
				}
				if (!quot && *p2==*separator) fields_count++; //separator count
			}
			if (fields_count>0) break;
			separator++;
		}
		if (*separator) field_sep->Set(separator, 1, true);
		else *field_sep="\t";

		//find quotation mark
		if (csv_source[0]=='\'' || csv_source[0]=='"'){
			*quoted_field=csv_source[0];
		}else{
			*quoted_field=0;
		}
	}

	ReferLink** new_field=fields;
	fields_count=0; //recount fields from 0
	p1=(char*) csv_source;
	while (*p1 && p1<line_end){ //beginning of the field
		while (*p1==' ') p1++; //skip leading blanks
		p2=strstr(p1, field_sep->P);
		if (!p2) p2=line_end; //ending of the field
		for (p5=p2-1; p5>p1 && *p5==' '; p5--); //trailing blanks;
		for (p3=p1; p3<=p5 && !isalpha(*p3) && *p3!='_'; p3++); //begining of name;
		for (p4=p3; p4<=p5 && tStrOp::isAN_(*p4); p4++); //ending of name;
		
		if (*quoted_field){ //csv format, use /'\n'/
			sprintf(buf, "&get_csvfield('%s',%d) ", field_sep->P, ++fields_count);
		}else{
			sprintf(buf, "/'%s'/%d ", field_sep->P, ++fields_count); 
		}
		field_value=buf;
		if ((*p1=='\'' || *p1=='"') && *p5==*p1 && p5>p1){
			sprintf(buf, "&trim(' \\r\\n&#%d;', '101')", *p1);
		}else{
			sprintf(buf, "&trim(' \\r\\n', '101')");
		}
		field_value+=buf;

		field_name.Set(p3, p4-p3, true);

		*new_field=new ReferLink;
		(*new_field)->Name=field_name;
		(*new_field)->Value=field_value;
		new_field=&(*new_field)->Next;

		p1=p2+field_sep->L;
	}

	/*while (p1 && *p1 !='\r' && *p1 !='\n'){
		quot=0; 
		if (*p1=='\'' || *p1=='"') quot=*p1;

		//field name
		while (*p1 && !isalpha(*p1)) p1++;//first alphabet
		p2=p1;
		while (TagOperation::isAN_(*p2)) p2++;
		if (p2==p1) break;

		field_count++;
		field_name.Set(p1, p2-p1, false);

		if (quot && *p1==quot) p1++;
		else quot=0;

		//separator
		p1=p2;
		while (*p2 && *p2!='\r' && *p2!='\n' && !TagOperation::isAN_(*p2) ) p2++;
		if (p2-p1>1 && (*(p2-1)=='\'' || *(p2-1)=='"') ) p2--;
		if (p1==p2) break; 

		if (!field_sep->L && p2!=p1){
			field_sep->Set(p1, p2-p1, true); //separator
			if (!strchr(field_sep->P, '\'')){
				field_assign="/'";
				field_assign+=*field_sep;
				field_assign+="'/";
			}else if (!strchr(field_sep->P, '"')){
				field_assign="/\"";
				field_assign+=*field_sep;
				field_assign+="\"/";
			}else{
				field_assign="/\"";
				char* p3=tStrOp::replaceMalloc(field_sep->P, "\"", "\\\"");
				field_assign+=p3;
				if (p3) free(p3);
				field_assign+="\"/";
			}
		}

		if (field_assign.L){
			field_value=field_assign;
			sprintf(buf, "%d", field_count);
			field_value+=buf;
		}else{
			field_value="%1";
		}
		if (quot=='\'') {
			sprintf(buf, "&trim('&#39;', '101')"); //Single quote; Decimal:39; Octal:47; Hex:27
		}else if (quot=='"') {
			sprintf(buf, "&trim('&#34;', '101')"); //Double quote; Decimal:34; Octal:42; Hex:22
		}

		*new_field=new ReferLink;
		(*new_field)->Name=field_name;
		(*new_field)->Value=field_value;
		new_field=&(*new_field)->Next;

		p1=p2;
	}
	*/

	//p2=p1;
	//while (p2 && *p2 && (*p2=='\r' || *p2=='\n')) p2++;
	
	//line_sep->Set(p1, p2-p1, true);
	return fields_count;
}

int tStrOp::getCSVHtqlExpr(const char* csv_source, ReferData* htql_result, int first_line_isname, const char* fieldsep, const char* linesep, int quoted_field){
	char* p1=0, *p2=0;
	p1=(char*) csv_source;

	ReferData field_sep;
	ReferData line_sep;
	if (fieldsep) field_sep=fieldsep;
	if (linesep) line_sep=linesep;

	ReferData htql_expr;
	ReferLink* fields=0, *link=0;
	int field_count=getCSVFields(p1, &fields, &field_sep, &line_sep, &quoted_field);
	if (field_count>0){
		char buf[128];
		field_count=0;
		for (link=fields; link; link=link->Next){
			field_count++;
			if (!first_line_isname){//text, do not use name from data
				sprintf(buf, "Column%d", field_count);
				link->Name=buf;
			}
			htql_expr+=link->Name; 
			htql_expr+="=";
			htql_expr+=link->Value;
			htql_expr+="; ";
		}
	}else{
		htql_expr="Column1=%1";
		field_count=1;
	}
	if (fields) {
		delete fields;
		fields=0;
	}

	//if (first_line_isname) *htql_result = "/'\\n'/2-0 {";
	//else *htql_result = "/'\\n'/ {";
	*htql_result="/'"; *htql_result+=line_sep; *htql_result+="'/";
	if (first_line_isname) *htql_result += "2-0";
	*htql_result += " {";
	*htql_result+=htql_expr;
	*htql_result+="}";
	return field_count;
}
int tStrOp::getUrlHostName(const char* url, char** start, long* len){
	if (!url){
		if (start) *start=0;
		if (len) *len=0;
		return 0;
	}
	//find start
	char *p=strstr((char*) url, "://");
	if (!p) p=(char*) url;
	else p+=3;

	//find end
	char* p1=strchr(p, '/');
	if (!p1) p1=p+strlen(p);

	//results
	if (start) *start=p;
	if (len) *len=p1-p;
	return 1;
}

int tStrOp::getUrlDomainName(const char* url, char** start, long* len){
	if (!url){
		if (start) *start=0;
		if (len) *len=0;
		return 0;
	}
	//find start
	char *p=strstr((char*) url, "://"); 
	if (!p) p=(char*) url;
	else p+=3;

	if (!tStrOp::strNcmp(p, "www.",4,false)) p+=4;

	//find end
	char* p1=strchr(p, '/');
	if (!p1) p1=p+strlen(p);

	//results
	if (start) *start=p;
	if (len) *len=p1-p;
	return 1;
}
int tStrOp::getUrlPathName(const char* url, char** start, long* len){
	if (!url){
		if (start) *start=0;
		if (len) *len=0;
		return 0;
	}
	//find start
	char *p=strstr((char*) url, "://"); 
	if (!p) p=(char*) url;
	else p+=3;

	//find path start
	char* p1=strchr(p, '/');
	if (!p1) p1=p+strlen(p);
	else p1+=1;

	//find end
	char* p2=strrchr(p1, '/');
	if (!p2) p2=p1+strlen(p1); 

	if (start) *start=p1; 
	if (len) *len=p2-p1; 
	return 1; 
}

int tStrOp::getUrlProtocol(const char* url, ReferData* protocol){
	char* p=(char*) strstr(url, "://");
	if (p) {
		protocol->Set((char*) url, p-url, true);
		return (p-url);
	}else{
		protocol->reset();
		return 0;
	}
}
int tStrOp::getUrlHost(const char* url, ReferData* host){
	char* p=(char*) strstr(url, "://");
	if (!p) p=(char*) url;
	else p=p+3;

	char* p1=strchr(p, '/');
	if (!p1) p1=strchr(p, '\\');
	if (p1){
		host->Set(p, p1-p, true);
		return (p1-p);
	}else{
		host->reset();
		return 0;
	}
}

int tStrOp::getUrlAction(const char* url, ReferData* action){
	char* p=(char*) strstr(url, "://");
	if (!p) p=(char*) url;
	else p=p+3;

	char* p1=strchr(p, '/');
	if (!p1) p1=strchr(p, '\\');
	if (p1) p=p1+1;

	p1=strchr(p, '#');
	char* p2=strchr(p, '?');
	if (!p1 || (p2 && p2<p1)) p1=p2;
	
	if (p1){
		action->Set(p, p1-p, true);
	}else{
		action->Set(p, strlen(p), true);
	}
	return (int) action->L;
}



#ifndef NO_ALIGNMENT
ReferLink* tStrOp::bestMatch(ReferLinkHeap* heap, const char* matchname, double* best_score){
	*best_score=0;
	if (heap->Total==0 || !matchname) return 0;

	double *scores=new double[heap->Total+1]; 
	double max_score=0, score=0;
	long i=0, best_i=0, result_len=0;
	double cost=0;
	StrAlignment align; //global alignment! local alignment uses HyperTagsAlignment
	align.IsCaseSensitive=false;
	align.MatchMismatchPenalty=2; //ensure continuous matches

	ReferLink* result_link=0;
	i=0;
	for (ReferLink* link=heap->getReferLinkHead(); link; link=link->Next){
		align.CompareStrings(link->Name.P, (char*) matchname, &cost, &result_len);
		score=1-cost/(double)(2*result_len);
		scores[i]=score;
		if (scores[i]>max_score) {
			max_score=scores[i]; 
			best_i=i;
			result_link=link;
		}
		i++;
	}
	delete [] scores;

	if (best_score) *best_score=max_score;

	return result_link; //best_i+1;
}
#endif 

int tStrOp::getFilePath(const char* filename, ReferData* filepath, ReferData* name){
	filepath->reset(); 
	name->reset();
	if (!filename) return 0; 

	char* p=strrchr((char*) filename, '/'); 
	if (!p) p=strrchr((char*) filename, '\\'); 

	if (p){
		filepath->Set((char*) filename, p-filename, true); //not include '/' or '\\'
		name->Set(p+1, (filename+strlen(filename))-p-1, true); 
	}else{
		filepath->Set((char*) filename, 0, true); //not include '/' or '\\'
		name->Set((char*) filename, strlen(filename), true); 
	}
	return 0;
}

