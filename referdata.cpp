#include "log.h"
#include "referdata.h"
#include "stroper.h"
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef WIN32
#include <comdef.h>
#include <atlconv.h>
#endif

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


ReferData::ReferData(){
	Type=0;
	P=0;
	L=0;
	Size=0;
}
ReferData::ReferData(const char* p){
	Type=0;
	P=0;
	L=0;
	Size=0;
	operator=(p);
}
ReferData::ReferData(ReferData &p){
	Type=0;
	P=0;
	L=0;
	Size=0;
	operator=(p);
}

ReferData::~ReferData(){
	reset();
}

void ReferData::reset(){
	if (Type && P){
		free(P);
	}
	Type=0;
	P=0;
	L=0;
	Size=0;
}
ReferData::ReferData(long size){
	Type=0;
	P=0;
	L=0;
	Size=0;
	Malloc(size);
}

char* ReferData::Malloc(long size){
	if (Type && P && Size > size){
		P[0]=0;
		L=0;
		return P;
	}
	if (Type && P){
		free(P);
		P=0;
		L=0;
		Size=0;
	}
	Size=size+1;
	P=(char*) malloc(sizeof(char)*Size);
	Type = 1;
	if (P) P[0]=0;
	L=0;
	return P;
}
char* ReferData::ReMalloc(long size){
	if (size<L) size=L; 
	if (Size<=size || !Type){
		char* p=(char*) malloc(sizeof(char)*(size+1));
		memset(p, 0, sizeof(char)*(size+1));
		if (P) memcpy(p, P, L+1); //in case !Type
		if (Type) free(P); 
		P=p;
		Type=true;
	}
	return P;
}
int ReferData::setToFree(int type){
	int old_type = Type;
	Type = type;
	return old_type;
}

char* ReferData::Cat(char* p, long size){
	if (p == P+L ) {
		L+=size;
		if (Type && L >= Size){
			L = Size-1;
		}
		return P;
	}
	if (Type && P && Size > L+size){
		memcpy(P+L, p, size);
		L+=size;
		P[L]=0;
		return P;
	}
	long S = L + size + 1;
	char* tmp=(char*) malloc(sizeof(char)*S);
	if (!tmp) return 0;

	if (P){
		memcpy(tmp, P, L);
	}
	if (p){
		memcpy(tmp+L, p, size);
	}
	if (Type && P) free(P);
	P=tmp;
	L += size;
	P[L]=0;
	Size = S;
	Type = 1;
	return P;
}

char* ReferData::Set(char* p, long size, int copy){
	if (copy){
		if (Type && size < Size){
			if (p!=P) memcpy(P, p, size);
			L = size;
			P[L]=0;
			return P;
		}else{
			//if (P!=p){
				reset();
				return Cat(p, size);
			//}
		}
	}else{
		if (Type){
			if (p!=P){
				reset();
			}
		}
		P=p;
		L=size;
		return P;
	}
}
int ReferData::borrow(ReferData* from){
	if (from->Type){
		this->Set(from->P, from->L, false); 
		this->setToFree(1);
		from->setToFree(0);
	}else{
		this->Set(from->P, from->L, false); 
	}
	return 0;
}

int ReferData::Cmp(const char* p, long len, int CaseSensitive){
	if (!p && !P) return 0;
	if (!p || !P) return (P)?1:-1;
	if (CaseSensitive) {
		int i=memcmp(P, p, (L>len)?len:L);
		if (i) return i;
		//for (long i=0; i<L; i++){
		//	if (P[i]>p[i] ) return 1;
		//	if (P[i]<p[i] ) return -1;
		//}
		if (len>L) return -1;
		else if (len<L) return 1;
		return 0;
	}else{
#ifdef WIN32
		int i=memicmp(P, p, (L>len)?len:L);
		if (i) return i;
#else
		for (long i=0; i<L && i<len; i++){
			if ((unsigned char) toupper(P[i])>(unsigned char)toupper(p[i]) ) return 1;
			if ((unsigned char) toupper(P[i])<(unsigned char)toupper(p[i]) ) return -1;
		}
#endif
		if (len>L) return -1;
		else if (len<L) return 1;
		return 0;
	}
}

int ReferData::Cmp(ReferData*data, int CaseSensitive){
	return Cmp(data->P, data->L, CaseSensitive);
}

ReferData& ReferData::operator= (const char* p){
	if (p) Set((char*) p, strlen(p), true);
	else Set((char*) p, 0, false);
	return *this;
}

ReferData& ReferData::operator= (ReferData& p){
	if (p.P==P && p.L==L) return *this;
	Set(p.P, p.L, p.Type);
	return *this;
}
ReferData& ReferData::operator+= (const char* p){
	if (p) Cat((char*)p, strlen(p));
	return *this;
}
ReferData& ReferData::operator+= (ReferData& p){
	Cat(p.P, p.L);
	return *this;
}

int ReferData::saveFile(const char* filename, int binary){
	if (!filename || !filename[0]) return -1;
	FILE* fw;
#ifdef WIN32
	if (binary) fw= fopen(filename, FILE_WRITE);
	else fw=fopen(filename, FILE_TWRITE);
	if (!fw){
		wchar_t buff[_MAX_PATH];
		MultiByteToWideChar(CP_THREAD_ACP, MB_COMPOSITE, filename, strlen(filename), buff, _MAX_PATH);
		wchar_t permission[_MAX_PATH];
		if (binary) MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, FILE_WRITE, strlen(FILE_WRITE), permission, _MAX_PATH);
		else		MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, FILE_TWRITE, strlen(FILE_TWRITE), permission, _MAX_PATH);
		fw=_wfopen(buff, permission);
	}
#else
	if (binary) fw= fopen(filename, FILE_WRITE);
	else fw=fopen(filename, FILE_TWRITE);
#endif
	if (!fw) return -1;
	if (P) fwrite(P, 1, L, fw);
	fclose(fw);
	return 0;

}
int ReferData::appendFile(const char* filename, int binary){
	if (!filename || !filename[0]) return -1;
	FILE* fw=0;
	if (binary) fw= fopen(filename, FILE_APPEND);
	else fw=fopen(filename, FILE_TAPPEND);
	if (!fw ) {
#ifdef WIN32
		wchar_t buff[_MAX_PATH];
		MultiByteToWideChar(CP_THREAD_ACP, MB_COMPOSITE, filename, strlen(filename), buff, _MAX_PATH);
		wchar_t permission[_MAX_PATH];
		if (binary) MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, FILE_APPEND, strlen(FILE_APPEND), permission, _MAX_PATH);
		else MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, FILE_TAPPEND, strlen(FILE_TAPPEND), permission, _MAX_PATH);
		fw=_wfopen(buff, permission);
		if (!fw) return -1;
#else
		return -1;
#endif
	}
	if (P) fwrite(P, 1, L, fw);
	fclose(fw);
	return 0;
}

int ReferData::readFile(const char* filename, int binary){
	if (!filename || !filename[0]) return -1;
	char buf[1024];
	FILE* fw=0;
	if (binary) fw= fopen(filename, FILE_READ);
	else  fw= fopen(filename, FILE_TREAD);
	if (!fw ) return -1;
	reset();
	int len;
	while ((len=fread(buf, 1, 1024, fw))>0){
		Cat(buf, len);
	}
	fclose(fw);
	return 0;

}

int ReferData::isNULL(){
	return (!this || !P );
}

char* ReferData::Seperate(){
	if (!Type && P){
		char* p=P;
		long l=L;
		reset();
		if (l>=0)
			Set(p, l, true);
		else 
			Set(p, 0, true);
	}
	return P;
}

double ReferData::getDouble(){
	double d=0;
	if (P) sscanf(P, "%lf", &d);
	return d;
}
long ReferData::getLong(){
	long d=0;
	if (P) sscanf(P, "%ld", &d);
	return d;
}
int ReferData::setDouble(double d, const char* fmt){
	Malloc(128);
	sprintf(P, (fmt&&fmt[0])?fmt:"%lf", d);
	L=strlen(P);
	return 0;
}
int ReferData::setLong(long d, const char* fmt){
	Malloc(128);
	sprintf(P, (fmt&&fmt[0])?fmt:"%ld", d);
	L=strlen(P);
	return 0;
}

int ReferData::replaceStr(const char*pattern, const char* dest){
	if (!P || !L || !pattern) return 0; 
	if (!Type){
		P=tStrOp::replaceMalloc(P, (char*) pattern, (char*) dest, L);
		L=strlen(P); 
		Type=1;
	}else{
		if (!dest || strlen(dest)<=strlen(pattern)){
			tStrOp::replaceInplace(P, (char*) pattern, (char*) dest);
			L=strlen(P);
		}else{
			char* p1=tStrOp::replaceMalloc(P, (char*) pattern, (char*) dest, L); 
			free(P); 
			P=p1; 
			L=strlen(P); 
		}
	}
	return 0;
}

