#ifndef DIR_FUNC_H_CLY_2010_01_24
#define DIR_FUNC_H_CLY_2010_01_24

class ReferLinkHeap;
class ReferData;

class DirFunc{
public:
	static int saveToFile(char* p, long len, char* filename);
	static int getTempPath(ReferData* temppath);  //temppath ends with '/' or '\\'
	static int getTempFile(ReferData* temppath, ReferData* tempname);  //temppath ends with '/' or '\\'
	static int getUniqueFileName(const char* dir, const char* filename, ReferData* unique_name, const char* subdir=0); 
					//heavy operation for temp dir
	static int removeFilenameSpecial(char* filename); 
	static int makeSubDir(ReferData* dir_path, const char* subdir); //dir_path will be changed to include subdir
	static int makeDir(const char* dir_path); 

	typedef enum {
		folderFILE=0x01, 
		folderDIR=0x02, 
		folderRECURSIVE=0x04, 
		folderLastWriteTime=0x08, 
		folderFileSize=0x10
		}SearchFolderOption;
	static int searchFolder(const char* path, ReferLinkHeap* results, long option=folderFILE);
	static long getFileWriteTime(const char* filename); 
	static long getFileCreateTime(const char* filename); 
	static int loadLibraries(const char* path, const char* entryname, ReferLinkHeap* results); 
	static int freeLibraries(ReferLinkHeap* results); 

	DirFunc();
	~DirFunc(); 
	void reset();
};

#endif

