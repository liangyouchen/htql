#include "dirfunc.h"
#include "referlink.h"
#include "platform.h"
#include "stroper.h"

#undef close
#undef open
//#include <fstream.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#ifdef _WINDOWS
	#include "atlbase.h"
	#include "comutil.h"
	#ifndef VC6
		#include "atlstr.h"
	#endif
#endif

// see http://www.codeproject.com/Articles/76252/What-are-TCHAR-WCHAR-LPSTR-LPWSTR-LPCTSTR-etc
// for unicode handling


DirFunc::DirFunc(){
}
DirFunc::~DirFunc(){
}
void DirFunc::reset(){
}

int DirFunc::saveToFile(char* p, long len, char* filename){
	int f;
	unlink(filename);
	f = open(filename, O_WRONLY | O_CREAT | O_BINARY, S_IREAD | S_IWRITE);
	if (f<0) return false;
	write(f, p, len);
	close(f);
	return true;
}
int DirFunc::getTempPath(ReferData* temppath){
	//temppath ends with '/' or '\\'
#ifdef _UNICODE
	//USES_CONVERSION;
#endif
	temppath->Malloc(255);
#ifdef WIN32
	TCHAR buf[255];
	::GetTempPath(255, buf);
	temppath->Set((char*)buf, _tcslen(buf), true);
	//::GetTempPath(255, temppath->P);
	//temppath->L=strlen(temppath->P);
#else
	*temppath = tmpnam(0);
	char* p = strchr(temppath->P, '/');
	if (!p) p = strchr(temppath->P, '\\');
	if (p) {
		p[1] = 0;
		temppath->L = strlen(temppath->P);
	}
#endif
	return 0;
}
int DirFunc::getTempFile(ReferData* temppath, ReferData* tempname){  //temppath ends with '/' or '\\'
	temppath->Malloc(255);
	tempname->Malloc(255);
#ifdef WIN32
	TCHAR buf[255];
	::GetTempPath(255, buf);
	temppath->Set((char*)buf, _tcslen(buf), true);
	//::GetTempPath(255, temppath->P);
	//temppath->L=strlen(temppath->P);
	char* p = tmpnam(0);
	if (p&&p[0] == '\\' || p[0] == '/'){
		*tempname = p + 1;
	}
	else if (p){
		*tempname = p;
	}
	else{
		*tempname = "tmp";
	}
#else
	*temppath = tmpnam(0);
	char* p = strchr(temppath->P, '/');
	if (!p) p = strchr(temppath->P, '\\');
	if (p) {
		*tempname = p + 1;
		p[1] = 0;
		temppath->L = strlen(temppath->P);
	}
	else{
		*temppath += "/";
		*tempname = "tmp";
	}
#endif
	return 0;
}
int DirFunc::makeDir(const char* dir_path){
	if (!dir_path || !dir_path[0]) return 0;
	char* p = strrchr((char*)dir_path, ':');
	if (!p)	p = (char*)dir_path;
	else p++;
	char* p1 = strstr(p, "\\\\");
	if (p1) p = p1 + 2;

	ReferData dir;
	dir.Set((char*)dir_path, p - dir_path, true);

	return makeSubDir(&dir, p);
}
int DirFunc::makeSubDir(ReferData* dir_path, const char* subdir){ //dir is changed to include subdir
	if (subdir && *subdir){ //make subdir
		//for multiple subdirs
		ReferData subdir_str; subdir_str = subdir;
		tStrOp::replaceInplace(subdir_str.P, "\\", "/");
		ReferLinkHeap subdirs;
		subdirs.setSortOrder(SORT_ORDER_NUM_INC);
		tStrOp::splitString(subdir_str.P, "/", &subdirs);

		for (ReferLink* link = (ReferLink*)subdirs.moveFirst(); link; link = (ReferLink*)subdirs.moveNext()){
			if (!link->Name.L) continue;

			ReferLinkHeap dirlist;
			searchFolder(dir_path->P, &dirlist, folderDIR);

			if (dir_path->P[dir_path->L - 1] != '\\' && dir_path->P[dir_path->L - 1] != '/'){
				*dir_path += "/";
			}
			*dir_path += link->Name;

			if (!dirlist.findName(&link->Name)){
#if defined _WINDOWS || defined WIN32
				mkdir(dir_path->P);
#else
				mkdir(dir_path->P, S_IREAD | S_IWRITE | S_IEXEC);
#endif
			}
		}
	}
	return 0;
}
int DirFunc::getUniqueFileName(const char* dir, const char* filename, ReferData* unique_name, const char* subdir){
	//the unique_name will not include dir, but include subdir
	//this function also make subdir if it is given

	//default path
	ReferData dir_path; dir_path = dir;
	if (dir_path.L == 0) dir_path = ".";

	//subdir path
	makeSubDir(&dir_path, subdir);

	//search for all files in dir_path
	ReferLinkHeap fileslist;
	fileslist.setCaseSensitivity(false);
	searchFolder(dir_path.P, &fileslist);

	//get the filename prefix
	ReferData prefix, suffix;
	if (filename){
		char* p = strrchr((char*)filename, '.');
		if (p){
			prefix.Set((char*)filename, p - filename, true);
			suffix = p;
		}
		else{
			prefix = filename;
		}
	}
	if (!prefix.L){
		prefix = "_";
	}
	removeFilenameSpecial(prefix.P);

	//get a unique filename
	*unique_name = prefix;
	*unique_name += suffix;

	char buf[128];
	long i = 1;
	while (fileslist.findName(unique_name)){
		sprintf(buf, "_%ld", i++);
		*unique_name = prefix;
		*unique_name += buf;
		*unique_name += suffix;
	}

	//to include subdir in the unique filename
	if (subdir && *subdir){
		ReferData keep_name = *unique_name;
		*unique_name = subdir;
		if (unique_name->P[unique_name->L - 1] != '\\' && unique_name->P[unique_name->L - 1] != '/'){
			if (strchr(subdir, '\\')){
				*unique_name += "\\";
			}
			else{
				*unique_name += "/";
			}
		}
		*unique_name += keep_name;
	}

	return 0;
}
int DirFunc::removeFilenameSpecial(char* filename){
	if (!filename) return 0;
	char* p;
	const char* badchar = ":;,'\"|/\\&%@`?<>[]{}()!#$^*~";
	for (p = filename; *p; p++) {
		if (strchr(badchar, *p) || (*p>0 && *p<32) || *p == 127) *p = '_';
		else if (p[0]<0) {
			if (p[1] >= 0)	p[0] = '_';
			else p++;
		}
	}
	return 0;
}

#ifdef _WINDOWS
time_t FileTime2Time(FILETIME* ft){
	SYSTEMTIME st;
	FileTimeToSystemTime(ft, &st);
	struct tm t, *t1;
	t.tm_year = st.wYear - 1900;
	t.tm_mon = st.wMonth - 1;
	t.tm_mday = st.wDay;
	t.tm_hour = st.wHour;
	t.tm_min = st.wMinute;
	t.tm_sec = st.wSecond;
	t.tm_isdst = 0;
	long now = time(NULL);
	t1 = localtime((const time_t*)&now);
	if (t1)
		t.tm_isdst = t1->tm_isdst;

	return mktime(&t);
}
#else
//do in Linux

#endif


long DirFunc::getFileWriteTime(const char* filename){
#ifdef _WINDOWS
	USES_CONVERSION;
	_WIN32_FILE_ATTRIBUTE_DATA file_attr;
	if (GetFileAttributesEx(A2T((char*)filename), GetFileExInfoStandard, &file_attr)){
		return FileTime2Time(&file_attr.ftLastWriteTime);
	}
	else{
		return 0;
	}
#else
	//do in Linux

#endif
	return 0;
}
long DirFunc::getFileCreateTime(const char* filename){
#ifdef _WINDOWS
	USES_CONVERSION;
	_WIN32_FILE_ATTRIBUTE_DATA file_attr;
	if (GetFileAttributesEx(A2T((char*)filename), GetFileExInfoStandard, &file_attr)){
		return FileTime2Time(&file_attr.ftCreationTime);
	}
	else{
		return 0;
	}
#else
	//do in Linux

#endif
	return 0;
}

int DirFunc::searchFolder(const char* path, ReferLinkHeap* results, long option){
#ifdef _WINDOWS
	USES_CONVERSION;
	char cur_dir[_MAX_PATH];
	_getcwd(cur_dir, _MAX_PATH);

	long handle;
	struct _finddata_t filestruct;	//表示文件(或目录)的信息
	char path_search[_MAX_PATH];	//表示查找到的路径结果

	//ReferLink* tmp;

	if (_chdir(path) == -1) return -1;
	_getcwd(path_search, _MAX_PATH);

	handle = _findfirst("*", &filestruct);

	if ((handle == -1)) // 目录是空的!?
		return 1;

	do {
		if (!filestruct.name || !strcmp(filestruct.name, ".") || !strcmp(filestruct.name, ".."))
			continue;

		_WIN32_FILE_ATTRIBUTE_DATA file_attr;
		long attr_data = 0;
		if (option&(folderLastWriteTime | folderFileSize)){
			if (GetFileAttributesEx(A2T(filestruct.name), GetFileExInfoStandard, &file_attr)){
				if (option&folderLastWriteTime){
					attr_data = FileTime2Time(&file_attr.ftLastWriteTime);
				}
				else if (option&folderFileSize){
					attr_data = file_attr.nFileSizeLow;
				}
			}
		}

		if (::GetFileAttributes(A2T(filestruct.name)) & FILE_ATTRIBUTE_DIRECTORY){  // 是否为目录?
			if (filestruct.name[0] != '.'){  // 是目录,且非空目录
				if (option&folderDIR){
					results->add(filestruct.name, path_search, (option&(folderLastWriteTime | folderFileSize)) ? attr_data : folderDIR);
					/*
					tmp = new ReferLink;
					tmp->Name = filestruct.name;
					tmp->Value = path_search;
					if (option&(folderLastWriteTime|folderFileSize) ){
					tmp->Data=attr_data;
					}else{
					tmp->Data = folderDIR;
					}
					tmp->Next = *results;
					*results = tmp;
					*/
				}
				if (option&folderRECURSIVE){
					//_chdir(filestruct.name);
					searchFolder(filestruct.name, results, option);
					_chdir("..");
				}
			}
		}
		else{ //file
			if (option&folderFILE){
				results->add(filestruct.name, path_search, (option&(folderLastWriteTime | folderFileSize)) ? attr_data : folderFILE);
				/*
				tmp = new ReferLink;
				tmp->Name = filestruct.name;
				tmp->Value = path_search;
				if (option&(folderLastWriteTime|folderFileSize) ){
				tmp->Data=attr_data;
				}else{
				tmp->Data = folderFILE;
				}
				tmp->Next = *results;
				*results = tmp;
				*/
			}
		}
	} while (!(_findnext(handle, &filestruct)));

	_chdir(cur_dir);

#else
	//do in Linux

#endif

	return 0;
}

// Use the sysinternals utility Process Monitor http://technet.microsoft.com/en-us/sysinternals/bb896645 from Microsoft to trace all file access while your dll is trying to load. With this utility, you will see everything that that dll is trying to pull in
int DirFunc::loadLibraries(const char* path, const char* entryname, ReferLinkHeap* results){
#ifdef _WINDOWS
	USES_CONVERSION;
	ReferLinkHeap dllfiles;
	dllfiles.setDuplication(true);
	searchFolder(path, &dllfiles, folderFILE);

	const char* log_filenmae = "loadLibraries.log";

	char buf[_MAX_PATH];
	HINSTANCE hClcltr = 0;
	HMODULE lpfun = 0;

	ReferData logmsg; 
	int log_all = false;
	if (log_all){
		logmsg="[loadLibraries] path=\""; logmsg+=path; logmsg+="\"; entryname=\""; logmsg+=entryname;logmsg+="\"\r\n";
		logmsg.appendFile(log_filenmae);
	}
	for (ReferLink* link = dllfiles.getReferLinkHead(); link; link = link->Next){
		if (log_all){
			logmsg="\t\t\t |--> testing ..."; logmsg+=link->Value.P; logmsg+="/"; logmsg+=link->Name; logmsg+="\r\n";
			logmsg.appendFile(log_filenmae);
		}
		if (link->Name.L>4 && !tStrOp::strNcmp(link->Name.P + link->Name.L - 4, ".dll", 4, false)){
			//if (!entryname || !entryname[0] || tStrOp::StringLike(link->Name.P, entryname)){
				sprintf(buf, "%s/%s", link->Value.P, link->Name.P);

				logmsg="\t\t\t\t loading DLL ..."; logmsg+=buf; 

				hClcltr = LoadLibrary(A2T(buf));
				if (hClcltr){
					if (log_all){
						logmsg+=" OK.\r\n"; 
						logmsg.appendFile(log_filenmae);
					}
					ReferData funcname;
					funcname.Set(link->Name.P, link->Name.L - 4, false);
					lpfun = (HMODULE)GetProcAddress(hClcltr, entryname?entryname:"Main");
					if (lpfun){
						ReferLink* link1 = results->findName(&funcname); 
						if (link1){
							FreeLibrary((HMODULE)link1->Value.P);
						}else{
							link1 = results->add(&funcname, 0, 0);
						}
						if (link1)
							link1->Value.Set((char*)lpfun, 0, false);
					}
					// get all exported function names
					lpfun = (HMODULE)GetProcAddress(hClcltr, "Functions");
					if (lpfun){
						int (*dllfunc)(int, char**, char**, long*);
						dllfunc=(int (*)(int, char**, char**, long*)) lpfun;
						char* pNames=0; 
						long len=0;
						ReferLinkHeap funcnames; 
						dllfunc(0, 0, &pNames, &len); 
						tStrOp::splitString(pNames, ":", &funcnames);
#ifdef _WINDOWS
						//The DLL function needs to use GlobalAlloc() function to allocate memoe
						GlobalFree(pNames);
#else
						free(pNames);
#endif
						sprintf(buf, "%s/%s", link->Value.P, "Functions");
						logmsg="\t\t\t\t found Functions ..."; logmsg+=buf;logmsg+=pNames;logmsg+="\r\n"; 
						if (log_all){
							logmsg.appendFile(log_filenmae);
						}

						// add the other function names to results
						for (ReferLink* link2 = (ReferLink*) funcnames.getReferLinkHead(); link2; link2 = link2->Next){
							funcname = link2->Name; 
							if (funcname.Cmp("Main", 4, false)){
								lpfun = (HMODULE)GetProcAddress(hClcltr, funcname.P);
								if (lpfun){
									ReferLink* link1 = results->findName(&funcname); 
									if (link1){
										FreeLibrary((HMODULE)link1->Value.P);
									}else{
										link1 = results->add(&funcname, 0, 0);
									}
									if (link1)
										link1->Value.Set((char*)lpfun, 0, false);
								}
								sprintf(buf, "%s/%s", link->Value.P, link2->Name.P);

								logmsg="\t\t\t\t loaded Function "; logmsg+=buf; logmsg+=" Done.\r\n";
								if (log_all){
									logmsg.appendFile(log_filenmae);
								}

							}
						}
						

					}
				}else{
					char logbuf[128]; 
					sprintf(logbuf, "%ld", GetLastError());
					logmsg+=" FAILED! error code="; logmsg+=logbuf; logmsg+="\r\n"; 
					logmsg.appendFile(log_filenmae);
				}
			//}
		}
	}
#else
	//do in Linux

#endif

	return 0;
}
int DirFunc::freeLibraries(ReferLinkHeap* results){
#ifdef _WINDOWS
	for (ReferLink* link = results->getReferLinkHead(); link; link = link->Next){
		FreeLibrary((HMODULE)link->Value.P);
	}
	results->empty();
#else
	//do in Linux

#endif

	return 0;
}


