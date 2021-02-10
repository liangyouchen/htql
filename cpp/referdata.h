#ifndef REFER_DATA_H
#define REFER_DATA_H

class ReferData{
public:
	int Type; //0: refer; 1: buffer;
	char* P;
	long L;
	long Size;

	void reset();
	char* Malloc(long size);
	char* ReMalloc(long size);
	char* Cat(char* p, long size);
	char* Set(char* p, long size, int copy=false);
	int borrow(ReferData* from); 
	int Cmp(const char* p, long len, int CaseSensitive=true);
	int Cmp(ReferData*, int CaseSensitive=true);
	ReferData& operator= (const char* p);
	ReferData& operator= (ReferData& p);
	ReferData& operator+= (const char* p);
	ReferData& operator+= (ReferData& p);
	int saveFile(const char* filename, int binary=true);
	int appendFile(const char* filename, int binary=true);
	int readFile(const char* filename, int binary=true);
	double getDouble();
	long getLong();
	int setDouble(double d, const char* fmt=0);
	int setLong(long d, const char* fmt=0);
	int replaceStr(const char*pattern, const char* dest);

	int isNULL();
	char* Seperate();
	int setToFree(int type);
	ReferData();
	ReferData(const char* p);
	ReferData(ReferData &p);
	ReferData(long size);
	~ReferData();
};


#endif
