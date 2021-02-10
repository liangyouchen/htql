#ifndef REFER_LOCK_H_CLY_20061128
#define REFER_LOCK_H_CLY_20061128

	#ifdef WIN32	//windows
	#include <afx.h>
	#include <afxwin.h>         // MFC core and standard components
	#include <afxext.h>         // MFC extensions
	#include <afxmt.h>
	#include <process.h>
	#include <stddef.h>

	#else	//unix
	#include <pthread.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <memory.h>

	#endif


class ReferLock{
protected:
	long ExclusiveLockerID;
	int ExclusiveCount;
	int ShareCount;
#ifdef WIN32
	CMutex Lock;
#else
	pthread_mutex_t Lock;
#endif
	long ThreadID;
public:
	//basic lock
	int IsWindowsWait;
	int wait(double sec);
	bool lock(double max_sec=-1);
	bool unlock();

	//advanced locks
	bool exclusiveLock(double max_sec=-1, long locker_id=0);
	bool exclusiveUnlock(long locker_id=0);

	bool sharedLock(double max_sec=-1);
	bool sharedUnlock();

	ReferLock();
	~ReferLock();
	void reset();
};

#endif 

