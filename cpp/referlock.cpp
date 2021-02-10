#include "referlock.h"

#include "platform.h"
#if defined (_WINDOWS) && !defined (NO_WINDOWS)
#include "MyFunction.h"
#endif 

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


ReferLock::ReferLock(){
	ExclusiveCount=0;
	ExclusiveLockerID=0;
	ShareCount=0;
	IsWindowsWait=0;
	ThreadID=0;
#ifndef WIN32
	pthread_mutex_init(&Lock, 0);
#endif
}

ReferLock::~ReferLock(){
	reset();
}

void ReferLock::reset(){
	ExclusiveLockerID=0;
	ExclusiveCount=0;
	ShareCount=0;
	IsWindowsWait=0;
	ThreadID=0;
#ifndef WIN32
	pthread_mutex_init(&Lock, 0);
#endif
}


int ReferLock::wait(double sec){
	if (IsWindowsWait){
#if defined (_WINDOWS) && !defined (NO_WINDOWS)
		if (sec>5){
			WindowsWait((long) sec, 0);
		}else{
			WindowsWaitM((long) (sec*1000), 0);
		}
#else
		if (sec>5){
			sleep((long) sec);
		}else{
			usleep((long) (sec*1000));
		}
#endif
	}else{
		if (sec>5){
			sleep((long) sec);
		}else{
			usleep((long) (sec*1000));
		}
	}
	return 0;
}

bool ReferLock::lock(double max_sec){
	bool infinite=(max_sec<0.001);
	bool success=true;
#ifdef WIN32
	success=(bool) Lock.Lock(infinite?INFINITE:(long)(max_sec*1000));

	if (0){
		long thread_id = ::GetCurrentThreadId();
		long count=0;
		while(ThreadID!=thread_id)
		{
			// keep trying until successfully completed the operation
#ifdef VC6
			::InterlockedCompareExchange((void**)&ThreadID, (void*)thread_id, 0);
#else
			::InterlockedCompareExchange((long*)&ThreadID, (long)thread_id, 0);
#endif
			if(ThreadID==thread_id) break;
			::Sleep(1);

			count++; 
			if (!infinite && count>=max_sec) return false;
		}
	}
#else
	success=pthread_mutex_lock(&Lock)==0;
#endif
	return success;
}
bool ReferLock::unlock(){
	bool success=true;
#ifdef WIN32
	success=(bool) Lock.Unlock();

	if (0){
#ifdef VC6
			::InterlockedCompareExchange((void**)&ThreadID, 0, (void*)::GetCurrentThreadId());
#else
			::InterlockedCompareExchange((long*)&ThreadID, 0, (long)::GetCurrentThreadId());
#endif
	}
#else
	success=pthread_mutex_unlock(&Lock);
#endif
	return success;
}


bool ReferLock::exclusiveLock(double max_sec, long locker_id){
	int infinite=(max_sec<0.00001);
	double rest_sec=max_sec;
	int my_lock=0, is_locked=0;
	while (infinite || rest_sec>0){
		is_locked=lock(1);
		if (is_locked && !my_lock && (ExclusiveCount==0 || (locker_id && locker_id==ExclusiveLockerID) ) ){
			ExclusiveCount++;
			ExclusiveLockerID=locker_id;
			my_lock=true;
		}
		if (ShareCount==0 && my_lock && ExclusiveCount>0){
			unlock();
			return true;
		}else{
			unlock();
		}

		if (rest_sec>1){
			wait(1);
		}else if (rest_sec>0.001){
			wait(rest_sec);
		}else if (!infinite) {
			break;
		}
		rest_sec=rest_sec-1;
	}
	if (my_lock){
		lock(1);
		ExclusiveCount--;
		ExclusiveLockerID=0;
		unlock();
	}
	return false;
}
bool ReferLock::exclusiveUnlock(long locker_id){
	lock();
#ifdef _WINDOWS
	if (locker_id && locker_id!=ExclusiveLockerID){
		ASSERT(0);
	}
#endif
	ExclusiveCount--;
	if (ExclusiveCount==0) ExclusiveLockerID=0;
	unlock();
	return true;
}

bool ReferLock::sharedLock(double max_sec){
	int infinite=(max_sec<0.001);
	double rest_sec=max_sec;
	while (infinite || rest_sec>0){
		lock(1);
		if (ExclusiveCount==0){
			ShareCount=ShareCount+1;
			unlock();
			return true;
		}else{
			unlock();
		}


		if (rest_sec>1){
			wait(1);
		}else if (rest_sec>0.001){
			wait(rest_sec);
		}else if (!infinite) {
			return false;
		}
		rest_sec=rest_sec-1;
	}
	return false;
}
bool ReferLock::sharedUnlock(){
	lock();
	ShareCount=ShareCount-1;
	unlock();
	return true;
}
