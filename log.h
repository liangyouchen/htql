#ifndef LOG__H
#define LOG__H		CLY19991028

#include "platform.h"


// ERRORLOG 0: no log, 1:error log, 2: debug log
#define ERRORLOG	0
#define ERRORLOGFILE 	"error.log"

static char LOGFILE[256]=ERRORLOGFILE;

extern char AppCurrentDir[];

#define logSUCCESS		0

#if (!defined(ERRORLOG) || (ERRORLOG == 0))
#define logOVERFLOW		-1000
#define logDATEFORMAT	-1001
#else
#define logOVERFLOW	(Log::add(LOGFILE,-1000,"data overflow",__LINE__,__FILE__))
#define logDATEFORMAT	(Log::add(LOGFILE,-1001,"date format error",__LINE__,__FILE__))
#endif

#ifndef True
#define True		1
#endif

#ifndef False
#define False		0
#endif

class Log{
public:
	Log();
	static int add(char* Logfile,int ErrorCode, char* Message,int line=0);
	static int add(char* Logfile,int ErrorCode, char* Message,int line,char* SourceFile);
	static int add(char* Logfile,int ErrorCode, char* Message,char* Message1, int line=0,char* SourceFile=0);
};

#endif
