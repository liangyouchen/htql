#ifndef LA_WEB_LOCK_H_CLY_2004_07_11
#define LA_WEB_LOCK_H_CLY_2004_07_11
#include "referlock.h"

class CLaWebLocks{
public:
	ReferLock Lock;
	int InitAutoRunning;
	int PathRunning;
	int AppEnding;

	int waitRunning(int* flag, int max_sec);
	int isRunning();
	int waitAll();
	int setRunning(int* flag, int max_sec);
	int clearRunning(int* flag);

	int checkRunning();

	CLaWebLocks();
	~CLaWebLocks();
	void reset();
};

#endif