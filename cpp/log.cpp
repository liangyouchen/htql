// LASTMODIFY CLY20000430
#include "log.h"

#include <stdio.h>
#include "stroper.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char AppCurrentDir[128]="./";

int Log::add(char* FileName,int ErrorCode, char* Message,int line){
	return add(FileName, ErrorCode, Message, 0, line, 0);
}

int Log::add(char *FileName, int ErrorCode, char* Message,int line, char* SourceFile){
	return add(FileName, ErrorCode, Message, 0, line, SourceFile);
}

int Log::add(char *FileName, int ErrorCode, char* Message,char* Message1,int line, char* SourceFile){
	FILE * flog;
	char tmp[50]="";
	tStrOp::DateToChar(time(NULL),"YYYY/MM/DD HH:MI:SS Dy",tmp);

	if ((flog=fopen(FileName,"a+"))==NULL) return ErrorCode;
	fprintf(flog,"%s, CODE: %5d",tmp,ErrorCode);
	if (Message){
		fprintf(flog," --- %s",Message);
	}
	if (Message1){
		fprintf(flog," %s",Message1);
	}
	if (line) {
		fprintf(flog,", LINE:%d",line);
	}
	if (SourceFile){
		fprintf(flog,", FILE:%s",SourceFile);
	}
	fprintf(flog,"\n");
	fclose(flog);
	return ErrorCode;
}

Log::Log(){

}
