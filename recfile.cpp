#include "recfile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


tCode::tCode(){
	strcpy(Version,"code 1.0");
}

unsigned int tCode::code(char *filebuff, unsigned int length){

	return length;

	char char1=0;
	unsigned char char2=0;
	unsigned int i=0;
	while(i<length-i)
	{		
		char1=filebuff[i];
		filebuff[i]=filebuff[length-i-1];
		filebuff[length-i-1]=char1;

		filebuff[i]=~filebuff[i];
		filebuff[length-i-1]=~filebuff[length-i-1];

		char2=filebuff[i]&0xF0;
		filebuff[i]=(filebuff[i]<<4)|(char2>>4);

		char2=filebuff[length-i-1]&0xF0;
		filebuff[length-i-1]=(filebuff[length-i-1]<<4)|(char2>>4);

		i++;
	}
	return length;
}

unsigned int tCode::encode(char *filebuff, unsigned int length){
	return code(filebuff,length);
}
unsigned int tCode::decode(char *filebuff, unsigned int length){
	return code(filebuff,length);
}

tRecFile::tRecFile(){
	LockFileRead[0]='\0';
	LockFileWrite[0]='\0';
	DoReadLock=False;
	DoWriteLock=True;
	RWFlag=0;

	Buffer[0]='\0';
	BufferOff=0;
	BufferPos=0;

	strcpy(RecordSep,RECORD_SEP);
	strcpy(FieldSep,FIELD_SEP);
	Quotation=0;

	FileHandle=-1;
	OpFileName[0]='\0';
	LockTryTimes=LOCKTRY;
	EndFile=True;

	umask(0);
}

tRecFile::~tRecFile(){
	if (FileHandle>=0) closeFile();
}

int tRecFile::lockFile(char * LockFileName,char * suffix){
#ifdef unix
	FILE * ps;
	char c;
	struct stat st;
#endif
	int i,j,pid=0,pid1=0;
	int pid_test=0,old_pid=-1;
	char tmp[80];
	int Lock=-1;

	if (!DoReadLock && !strcmp(suffix,ReadSuf)) 
		return fileSUCCESS;
	if (!DoWriteLock && !strcmp(suffix,WriteSuf)) 
		return fileSUCCESS;

	freeLock(LockFileName);

	strcpy(LockFileName,OpFileName);
	strcat(LockFileName,suffix);
	for (i= 0; i<LockTryTimes ; i++ ){
		Lock=open(LockFileName, O_WRONLY|O_CREAT,0444);
		if ( Lock != -1 ) {
			sprintf(tmp,"%d\n",getpid());
			if (write(Lock,tmp,strlen(tmp))==(int)strlen(tmp) && close(Lock)==0)
				return fileSUCCESS;
			close(Lock);
			Lock=-1;
		}
		if ((i % 30) ==0){
			if (i==0) srand( (unsigned) time(NULL) );

#ifdef ZeroFileNotUnlock
				if (stat(OpFileName,&st)==0){
					if (st.st_size==0) continue; 
					// File length is Zero, not unlock 
					// to avoid file destroying.
				}
#endif

			Lock=open(LockFileName, O_RDONLY);
			if (Lock != -1 ) {
				j=read(Lock,tmp,10);
				close(Lock);
				Lock=-1;
				pid=0;
				sscanf(tmp,"%d",&pid);
#ifdef unix
				sprintf(tmp,"ps -p %d 2>/dev/null",pid);
				if ( j>0 && (ps=popen(tmp,"r"))!=NULL){
					while ((c=fgetc(ps))!=EOF && c!='\n');
					pid1=0;
					if (c!=EOF) fscanf(ps,"%d",&pid1);
					if (pid1<2) j=0; 
					//j>0: process exist; j=0: process not exist; j<0: file not exist;
					pclose(ps);
				}
#endif
				//printf("pid=%d, pid1=%d, old=%d, j=%d \n",pid,pid1,old_pid,j);
				if (j<=0){
					if (pid_test==0) {
						pid_test++;
						old_pid=pid;
					}else {
						if (old_pid==pid && old_pid>=0 )
						UnlinkFile(LockFileName);
						pid_test=0;
					}
					continue;
				}
			}
		}
		usleep((rand()%2000)+2000);
	}
	*LockFileName='\0';
	return fileLOCKFAIL;
}

int tRecFile::freeLock(char * LockFileName){
	if (strcmp(LockFileName,"")){
		UnlinkFile(LockFileName);
		*LockFileName='\0';
	}
	return fileSUCCESS;
}

int tRecFile::UnlinkFile(char * FileName){
#ifndef unix
		chmod(FileName,S_IREAD | S_IWRITE);
#endif
		int i=unlink(FileName);
		return i;
}
	
int tRecFile::openRead(char * FileName){
	int i;
	strcpy(OpFileName,FileName);
	if (FileHandle>=0) if ((i=closeFile())!=fileSUCCESS) return i;
	if ((i=lockFile(LockFileRead,ReadSuf)) != fileSUCCESS) return i;
	FileHandle=open(OpFileName,O_RDONLY); 
	freeLock(LockFileRead);
	if (FileHandle<0) return fileOPENFAIL;
	RWFlag=1;
	BufferOff=0;
	BufferPos=0;
	EndFile=False;
	return fileSUCCESS;
}

int tRecFile::openWrite(char * FileName){
	int i;
	strcpy(OpFileName,FileName);
	if (FileHandle>=0) if ((i=closeFile())!=fileSUCCESS) return i;
	if ((i=lockFile(LockFileWrite,WriteSuf)) != fileSUCCESS) return i;
	if ((i=lockFile(LockFileRead,ReadSuf)) != fileSUCCESS) return i;
	FileHandle=open(OpFileName,O_RDONLY); 
	freeLock(LockFileRead);
	if (FileHandle<0) return fileOPENFAIL;
	RWFlag=2;
	BufferOff=0;
	BufferPos=0;
	EndFile=False;
	return fileSUCCESS;
}

int tRecFile::startWrite(){
	int i;
	if (FileHandle >=0 ) if (close(FileHandle)!=0) return fileFILECLOSE;
	if ((i=lockFile(LockFileRead,ReadSuf)) != fileSUCCESS) return i;
	FileHandle=open(OpFileName,O_WRONLY|O_CREAT,0666); 
	if (FileHandle<0) return fileOPENFAIL;
	RWFlag=3;
	BufferOff=0;
	BufferPos=0;
	EndFile=False;
	return fileSUCCESS;
}

int tRecFile::reOpen(){
	int i;
	if (close(FileHandle)!=0) return fileFILECLOSE;
	FileHandle=-1;
	if (RWFlag==1 || RWFlag==2){
		if ((i=lockFile(LockFileRead,ReadSuf)) != fileSUCCESS) return i;
		FileHandle=open(OpFileName,O_RDONLY);
		freeLock(LockFileRead);
	}
	else if (RWFlag==3){
		FileHandle=open(OpFileName,O_WRONLY|O_CREAT|O_EXCL,0666); 
	}
	if (FileHandle<0) return fileOPENFAIL;
	BufferOff=0;
	BufferPos=0;
	EndFile=False;
	return fileSUCCESS;
}

int tRecFile::closeFile(){
	if (FileHandle>=0) if (close(FileHandle)!=0) return fileFILECLOSE;
	freeLock(LockFileRead);
	freeLock(LockFileWrite);
	FileHandle=-1;
	RWFlag=0;
	BufferOff=0;
	BufferPos=0;
	EndFile=True;
	CurrentRecord.Free();
	return fileSUCCESS;
}


int tRecFile::readRecord(){
	if (EndFile) return fileFILEEOF;
	if (!CurrentRecord.Malloc(BufferSize)) return fileMEMORY;
	CurrentRecord.DataLen=0;
	unsigned int LastCompare=0;
	int RecordSepLen=strlen(RecordSep);
	int Found=false;
	int byteread=0;
	unsigned int i=0;
	while (!Found){
		if ( (BufferOff==0) || ( BufferPos > 0 && BufferOff < BufferPos && BufferOff < BufferSize) ){
			memcpy(Buffer,Buffer+BufferPos, BufferOff);
			BufferPos=0;
			byteread=read(FileHandle, Buffer+BufferOff,BufferSize);
			if (byteread<0) return fileFILEREAD;
			if (byteread == 0 && BufferOff == 0) {
				EndFile=true;
				return 0;
			}
			BufferOff += byteread;
			Buffer[BufferOff] = '\0';
		}
		if (! CurrentRecord.Cat(Buffer+BufferPos,BufferOff) ) return fileMEMORY;
		BufferPos += BufferOff;
		BufferOff =0;
		for (i=LastCompare; i<=(unsigned int)(CurrentRecord.DataLen-RecordSepLen) && CurrentRecord.DataLen>=(unsigned int)RecordSepLen; i++){
			if (CurrentRecord.Data[i] == RecordSep[0]){
				LastCompare =i;
				if (memcmp(CurrentRecord.Data+i, RecordSep, RecordSepLen)==0){
					BufferOff = CurrentRecord.DataLen-i-RecordSepLen;
					BufferPos -= BufferOff;
					CurrentRecord.DataLen = i+ RecordSepLen;
					Found = true;
					break;
				}
			}
		}
	}
	CurrentRecord.Data[CurrentRecord.DataLen]='\0';
	return CurrentRecord.DataLen;
}

int tRecFile::writeRecord(char* Record, size_t Len){
	if (write(FileHandle, Record, Len) < (int) Len) return fileFILEWRITE;
	return fileSUCCESS;
}

char* tRecFile::getRecordField(int Index, size_t * Len){
	CurrentField.SetValue("",0);
	unsigned int start=0, end=0;
	int FieldSepLen=strlen(FieldSep);
	int RecordSepLen=strlen(RecordSep);
	for (int i=0; i< Index; i++){
		while (start<CurrentRecord.DataLen){
			if (CurrentRecord.Data[start] == FieldSep[0]){
				if (memcmp(CurrentRecord.Data+start, FieldSep, FieldSepLen)==0){
					start+= FieldSepLen;
					break;
				}
			}
			start++;
		}
	}
	if (start >= CurrentRecord.DataLen) return NULL;
	end = start;
	int Found=false;
	while (end<CurrentRecord.DataLen){
		if (CurrentRecord.Data[end] == FieldSep[0] && memcmp(CurrentRecord.Data+end, FieldSep, FieldSepLen)==0 ){
			end+= FieldSepLen;
			Found=true;
			CurrentField.SetValue(CurrentRecord.Data+start, end-start-FieldSepLen);
			break;
		}
		if (CurrentRecord.Data[end] == RecordSep[0] && memcmp(CurrentRecord.Data+end, RecordSep, RecordSepLen)==0) {
			end+= RecordSepLen;
			Found=true;
			CurrentField.SetValue(CurrentRecord.Data+start, end-start-RecordSepLen);
			break;
		}
		end++;
	}
	
	if (!Found ) return NULL;

	//TODO: decode CurrentField
	CurrentField.DataLen= Code.encode(CurrentField.Data,CurrentField.DataLen);  
	// the result has same length as the source.  if different, memery must be realloc;

	if (Len) *Len=CurrentField.DataLen;
	return CurrentField.Data;
}

int tRecFile::addRecordField(char* FieldValue, size_t Len){
	unsigned int FieldSepLen=strlen(FieldSep);
	CurrentField.SetValue(FieldValue, Len);

	//TODO: encode CurrentField
	CurrentField.DataLen= Code.decode(CurrentField.Data,CurrentField.DataLen);
	// the result has same length as the source.  if different, memery must be realloc;

	if (!CurrentRecord.Cat(CurrentField.Data, CurrentField.DataLen)) return fileMEMORY;
	if (!CurrentRecord.Cat(FieldSep, FieldSepLen)) return fileMEMORY;
	return fileSUCCESS;
}

int tRecFile::writeRecord(){
	unsigned int RecordSepLen=strlen(RecordSep);
	if (!CurrentRecord.Cat(RecordSep, RecordSepLen)) return fileMEMORY;
	int i= writeRecord(CurrentRecord.Data, CurrentRecord.DataLen);
	CurrentRecord.Clear();
	return i;
}

