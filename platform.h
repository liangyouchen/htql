#ifndef UNIX2WIN_DEF_H
#define UNIX2WIN_DEF_H  	CLY20001023

#ifdef WIN32
	//#include <afx.h>
	#include <fcntl.h>
	#include <string.h>
	#include <ctype.h>
	#include <io.h>
	#include <direct.h>

	#ifndef WIN_TIME_DEF
		#define WIN_TIME_DEF
		#include <afx.h>
		#include <time.h>

		//#if !defined(Sleep)
		//	static long usleep_loop;
		//	static long usleep_time0;
		//	static long usleep_time1;
		//	#define usleep(A)	{for (usleep_loop=0;usleep_loop<(A)*100;usleep_loop++);}
		//	#define sleep(A) {for (usleep_time0=time(0), usleep_time1=time(0); usleep_time1-usleep_time0<=(long)(A); usleep_time1=time(0));}
		//#else
		#ifndef sleep
		#define sleep(A) (Sleep(A*1000))
		#endif
		#define usleep(A) (Sleep(A))
		//#endif

	#endif

	#ifndef WIN_FILE_DEF
	#define WIN_FILE_DEF
		//#ifndef errno
		//#define errno		0
		//#endif
		#define getpid		_getpid
		#define umask		_umask
		#define chmod		_chmod
		#define unlink		_unlink
		#define read		_read
		#define write		_write 
		#define tempnam		_tempnam 
		//#define stat		_stat
		//#define mkdir		_mkdir
		#define open		_open
		#define close		_close
		#define commit		_commit
		#define popen		_popen
		#define pclose		_pclose
		#define O_CREAT		_O_CREAT
		#define O_RDWR		_O_RDWR
		#define O_EXCL		_O_EXCL
		#define O_RDONLY	_O_RDONLY
		#define O_WRONLY	_O_WRONLY
		#define S_READ		_S_READ
		#define S_IREAD		_S_IREAD
		#define S_WRITE		_S_WRITE
		#define S_IWRITE	_S_IWRITE
		#define S_EXEC		_S_EXEC
		#define S_IEXEC		_S_IEXEC
	#endif

	#define LINE_BREAK	"\r\n"
	#define FILE_READ	"rb"
	#define FILE_WRITE	"w+bc"
	#define FILE_APPEND	"a+bc"
	#define FILE_TREAD	"r"
	#define FILE_TWRITE	"w+c"
	#define FILE_TAPPEND	"a+c"

	#ifndef WIN_PROCESS_DEF
	#define WIN_PROCESS_DEF
		#include <process.h>
		#define getpid		_getpid
		#define beginthread		_beginthread
	#endif


	//#define Thread __declspec(thread) 
	#define ThreadType	void 

#else  // unix
	#define _ftime	ftime
	#define _timeb	timeb

	//#define Thread 
	#define LINE_BREAK	"\n"
	#define FILE_READ	"r"
	#define FILE_WRITE	"w+"
	#define FILE_APPEND	"a+"
	#define FILE_TREAD	"r"
	#define FILE_TWRITE	"w+"
	#define FILE_TAPPEND	"a+"

	#define O_BINARY		O_CREAT

	#ifndef false
		#define false 0
	#endif
	#ifndef ture
		#define ture 1
	#endif

	#include <unistd.h>
	#include <stdio.h>
	#include <fcntl.h>
	#include <string.h>
	#include <ctype.h>
	#include <sys/stat.h>

	#define ThreadType	void*

	#define stricmp		strcasecmp
	#define strnicmp	strncasecmp

#endif  //unix

#endif  //UNIX2WIN_DEF_H  	
