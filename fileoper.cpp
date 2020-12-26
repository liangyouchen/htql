// LASTMODIFY CLY20000430
#include "fileoper.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


tFileOp::tFileOp(){
	strcpy(OpFileName,"");
	strcpy(LockFileRead,"");
	strcpy(LockFileWrite,"");
	LockTryTimes=LOCKTRY;
	fp=NULL;
	RWFlag=0;
	EndFile=True;
	DoReadLock=False;
	DoWriteLock=True;
	umask(0);
}

tFileOp::~tFileOp(){
	if (fp) closeFile();
}

int tFileOp::lockFile(char * LockFileName,char * suffix){
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
				if ( j>0 && (ps=popen(tmp, FILE_READ))!=NULL){
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
		usleep(10*rand()+2000);
	}
	*LockFileName='\0';
	return fileLOCKFAIL;
}

int tFileOp::freeLock(char * LockFileName){
	if (strcmp(LockFileName,"")){
		UnlinkFile(LockFileName);
		*LockFileName='\0';
	}
	return fileSUCCESS;
}

int tFileOp::UnlinkFile(char * FileName){
#ifndef unix
		chmod(FileName,S_IREAD | S_IWRITE);
#endif
		int i=unlink(FileName);
		return i;
}
	
int tFileOp::openRead(char * FileName){
	int i;
	strcpy(OpFileName,FileName);
	if (fp) if ((i=closeFile())!=fileSUCCESS) return i;
	if ((i=lockFile(LockFileRead,ReadSuf)) != fileSUCCESS) return i;
	fp=fopen(OpFileName,FILE_READ); 
	freeLock(LockFileRead);
	if (fp==NULL) return fileOPENFAIL;
	RWFlag=1;
	EndFile=False;
	return fileSUCCESS;
}

int tFileOp::openWrite(char * FileName){
	int i;
	strcpy(OpFileName,FileName);
	if (fp) if ((i=closeFile())!=fileSUCCESS) return i;
	if ((i=lockFile(LockFileWrite,WriteSuf)) != fileSUCCESS) return i;
	if ((i=lockFile(LockFileRead,ReadSuf)) != fileSUCCESS) return i;
	fp=fopen(OpFileName,FILE_READ);
	freeLock(LockFileRead);
	if (fp==NULL) return fileOPENFAIL;
	RWFlag=2;
	EndFile=False;
	return fileSUCCESS;
}

int tFileOp::startWrite(){
	int i;
	if (fp) if (fclose(fp)!=0) return fileFILECLOSE;
	if ((i=lockFile(LockFileRead,ReadSuf)) != fileSUCCESS) return i;
	if ((fp=fopen(OpFileName, "wt+"))==NULL) return fileOPENFAIL;
	RWFlag=3;
	return fileSUCCESS;
}

int tFileOp::reOpen(){
	int i;
	if (fclose(fp)!=0) return fileFILECLOSE;
	fp=NULL;
	if (RWFlag==1 || RWFlag==2){
		if ((i=lockFile(LockFileRead,ReadSuf)) != fileSUCCESS) return i;
		fp=fopen(OpFileName,FILE_READ);
		freeLock(LockFileRead);
	}
	else if (RWFlag==3){
		 fp=fopen(OpFileName,"wt+");
	}
	if (fp==NULL) return fileOPENFAIL;
	EndFile=False;
	return fileSUCCESS;
}

int tFileOp::closeFile(){
	if (fp) if (fclose(fp)!=0) return fileFILECLOSE;
	freeLock(LockFileRead);
	freeLock(LockFileWrite);
	fp=NULL;
	RWFlag=0;
	EndFile=True;
	return fileSUCCESS;
}

int tFileOp::getStr(char stop, char *result,int MaxChar){
	int length=0;
	char c='\0';
	while (((c=fgetc(fp))!=EOF) && (c!=stop)) {
		result[length++]=c;
		if ((MaxChar>0) && (length>MaxChar)){
			result[--length]='\0';
			return fileDATAERR;
		}
	}
	if (c==EOF) EndFile=True;
	result[length]='\0';
	return length;
}

char tFileOp::getStr(char stop1,char stop2, char *result,int MaxChar){
	int length=0;
	char c='\1';
	while (((c=fgetc(fp))!=EOF) && (c!=stop1) && (c!=stop2)) {
		result[length++]=c;
		if ((MaxChar>0) && (length>MaxChar)){
			result[--length]='\0';
			return fileDATAERR;
		}
	}
	if (c==EOF) EndFile=True;
	result[length]='\0';
	return length;
}

int tFileOp::MatchString(char *Str){
	int length=strlen(Str);
	char c=0;
	int i=0;
	while ((i<length) && ((c=fgetc(fp))!=EOF) && (c==Str[i]) ) i++;
	if (i==length) return fileSUCCESS;
	return fileFILEERR;
}

int tFileOp::putStr(char *Str){
	if (fprintf(fp,"%s",Str)<(int)strlen(Str)) return fileFILEWRITE;
	return fileSUCCESS;
}

int tFileOp::putChar(char ch){
	if (fprintf(fp,"%c",ch)<1) return fileFILEWRITE;
	return fileSUCCESS;
}

