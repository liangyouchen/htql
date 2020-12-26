#ifndef FILE_OPERATOR_H
#define FILE_OPERATOR_H		CLY20000430

#include "log.h"
#include "stroper.h"
#include <stdio.h>

#ifndef PathLength
#define PathLength			256
#endif
#ifndef True
#define True				1
#endif
#ifndef False
#define False				0
#endif

#define ReadSuf			".RDL"
#define WriteSuf		".WRL"
#define	LOCKTRY			200

#if (!defined(ERRORLOG) || (ERRORLOG == 0))
#define fileSUCCESS			0
#define fileFILEERR			-1
#define fileDATAERR			-2
#define fileLOCKFAIL		-3
#define fileOPENFAIL		-4
#define fileOVERFLOW		-10
#define fileFILEWRITE			-12
#define fileFILECLOSE			-13
#else
#define fileSUCCESS		0
#define fileFILEERR		(Log::add(ERRORLOGFILE,-1,"File operation error.",__LINE__))
#define fileDATAERR		(Log::add(ERRORLOGFILE,-2,"File operation data error.",__LINE__))
#define fileLOCKFAIL		(Log::add(ERRORLOGFILE,-3,"File operation lock error.",__LINE__))
#define fileOPENFAIL		(Log::add(ERRORLOGFILE,-4,"File operation open error.",__LINE__))
#define fileOVERFLOW		(Log::add(ERRORLOGFILE,-10,"File operation overflow.",__LINE__))
#define fileFILEWRITE			(Log::add(ERRORLOGFILE,-12,"File operation write error.",__LINE__))
#define fileFILECLOSE			(Log::add(ERRORLOGFILE,-13,"File operation close error.",__LINE__))
#endif

class tFileOp:public tStrOp{
protected:
	char LockFileWrite[PathLength];
	char LockFileRead[PathLength];
	int DoReadLock;
	int DoWriteLock;
	int RWFlag;
public:
	FILE* fp;
	char OpFileName[PathLength];
	int LockTryTimes;
	int EndFile;

	tFileOp();
	~tFileOp();
	int openRead(char * FileName);
	int openWrite(char * FileName);
	int startWrite();
	int reOpen();
	int closeFile();
	int getStr(char stop, char* result,int MaxChar=0);
	char getStr(char stop1, char stop2, char *result,int MaxChar=0);
	int MatchString(char* Str);
	int putStr(char *str);
	int putChar(char ch);
	int lockFile(char * LockFileName,char * suffix);
	int freeLock(char * LockFileName);
	static int UnlinkFile(char *FileName);
};

#endif
