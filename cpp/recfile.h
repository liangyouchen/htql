#ifndef RECORD_FILE_H
#define RECORD_FILE_H		CLY20000510

#include "log.h"
#include "bindata.h"
#include <stdio.h>

#ifndef NameLength
#define	NameLength		80
#endif
#ifndef PathLength
#define PathLength			256
#endif
#ifndef True
#define True				1
#endif
#ifndef False
#define False				0
#endif
#define BufferSize		1024

#define ReadSuf			".RDL"
#define WriteSuf		".WRL"
#define	LOCKTRY			200
#define RECORD_SEP		"\3\254\4\n"
#define FIELD_SEP		"\2\254\1"

#if (!defined(ERRORLOG) || (ERRORLOG == 0))
#define fileSUCCESS			0
#define fileFILEERR			-1
#define fileDATAERR			-2
#define fileLOCKFAIL		-3
#define fileOPENFAIL		-4
#define fileOVERFLOW		-10
#define fileFILEWRITE			-12
#define fileFILECLOSE			-13
#define fileFILEEOF				-14
#define fileFILEREAD			-15
#define fileMEMORY				-16
#else
#define fileSUCCESS		0
#define fileFILEERR		(Log::add(ERRORLOGFILE,-1,"File operation error.",__LINE__))
#define fileDATAERR		(Log::add(ERRORLOGFILE,-2,"File operation data error.",__LINE__))
#define fileLOCKFAIL		(Log::add(ERRORLOGFILE,-3,"File operation lock error.",__LINE__))
#define fileOPENFAIL		(Log::add(ERRORLOGFILE,-4,"File operation open error.",__LINE__))
#define fileOVERFLOW		(Log::add(ERRORLOGFILE,-10,"File operation overflow.",__LINE__))
#define fileFILEWRITE			(Log::add(ERRORLOGFILE,-12,"File operation write error.",__LINE__))
#define fileFILECLOSE			(Log::add(ERRORLOGFILE,-13,"File operation close error.",__LINE__))
#define fileFILEEOF			(Log::add(ERRORLOGFILE,-14,"File closed.",__LINE__))
#define fileFILEREAD			(Log::add(ERRORLOGFILE,-15,"File operation read error.",__LINE__))
#define fileMEMORY			(Log::add(ERRORLOGFILE,-16,"Memory fail.",__LINE__))
#endif

class tCode{
	char Version[PathLength];
public:
	tCode();
	virtual unsigned int encode(char *filebuff, unsigned int length);  // return result length
	virtual unsigned int decode(char *filebuff, unsigned int length); // return result length
	virtual unsigned int code(char *filebuff, unsigned int length); // return result length
};

class tRecFile{
protected:
	char LockFileWrite[PathLength];
	char LockFileRead[PathLength];
	int DoReadLock;
	int DoWriteLock;
	int RWFlag;

	char Buffer[BufferSize*2+1];
	int BufferOff;
	int BufferPos;
public:

	tCode Code;

	char RecordSep[NameLength];
	char FieldSep[NameLength];
	char Quotation;
	tBinData CurrentRecord;
	tBinData CurrentField;

	int FileHandle;
	char OpFileName[PathLength];
	int LockTryTimes;
	int EndFile;

	tRecFile();
	~tRecFile();
	int openRead(char * FileName);  // open in share read mode 
	int openWrite(char * FileName); // open in share read , excluxive write lock 
	int startWrite(); //open in exclusive read and write mode
	int reOpen();
	int closeFile();

	int readRecord(); // read record, set raw data to CurrentRecord
	int writeRecord(char* Record,size_t Len);  // write raw record data, possible encrypted
	char* getRecordField(int Index, size_t* Len=NULL);  // get (decoded) field value to CurrentField
			// Index: 0 base
	int addRecordField(char* FieldValue, size_t Len);  // encrypt FieldValue to CurrentRecord
	int writeRecord();  // write data in CurrentRecord to file, clear CurrenRecord

	int lockFile(char * LockFileName,char * suffix);
	int freeLock(char * LockFileName);
	static int UnlinkFile(char *FileName);
};

#endif

