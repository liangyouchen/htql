#ifndef REFER_SCHEDULER_H_CLY_20061128
#define REFER_SCHEDULER_H_CLY_20061128

#include "referlock.h"
#include "referset.h"
#include "platform.h"

class ReferScheduler;
typedef int (*RS_CALLBACK_REFERSET) (ReferScheduler* scheduler, ReferData* thread_id, ReferData* queue_id, ReferSet*dataset, ReferLock*lock); //need to handle the dataset
typedef int (*RS_CALLBACK_TUPLE) (ReferScheduler* scheduler, ReferData* thread_id, ReferData* queue_id, ReferData* tuple); //don't need to free tuple

extern const char* FIELD_QUEUES[];
extern const char* FIELD_THREADS[];


class ReferScheduler{
public:
	enum {
		//Field list following the same order as const char* FIELD_Queues[]
		ID_QueueID=0, ID_Queue, ID_HistoryQueue, ID_QueueLock, ID_QueueType, ID_CallBack, ID_WorkingThread, ID_QueueParent, ID_QueueFieldsNum,

		//Field list following the same order as const char* FIELD_Threads[]
		ID_ThreadID=0, ID_Thread, ID_ThreadLock, ID_ThreadType, ID_AllowedQueueIDs, ID_WorkingQueueID, ID_ThreadFieldsNum,

		//Queue Type, Queues->QueueType.L
		QUEUE_DEFAULT=0, QUEUE_FILO=0x01, QUEUE_PARENT=0x02, QUEUE_CHILD=0x04, 

		//Thread Type, Threads->ThreadType.L
		THREAD_DEFAULT=0, THREAD_FLOATING=0x1, THREAD_WINDOWS=0x2,

		//Callback type, Queue->ID_CallBack.L; also newQueue(callback_type)
		CALLBACK_REFERSET=0, CALLBACK_TUPLE=0x1, CALLBACK_TIMER=0x2,

		//Timer predefined fields 
		ID_TimerType=0, ID_TimerInterval, ID_TimerLastTime, ID_TimerFieldsNum //All in .L
	};

public:
	ReferLock QueuesLock;
	ReferSet Queues; //[QueueID], *Queue, *QueueLock, QueueType(L), *CallBack, WorkingThread, QueueParent
	int QueuesNum;

	ReferLock ThreadsLock;
	ReferSet Threads; //[ThreadID], Thread(L), *ThreadLock, ThreadType(L), AllowedQueueIDs, WorkingQueueID
	int ThreadsNum;

	int ToExit;
	void* Parent;

	static ThreadType serviceThread(void* p);
public:
	//high-level interfaces
	int newQueue(const char* queueid, const char** fields, const char** unqiue_fields, long* unique_fields_type, 
		void* callback_func, int callback_type, int maxthreads=0, long thread_type=THREAD_DEFAULT);
	int addQueueTuple(const char* queueid, ReferLinkHeap* name_values=0, const char** names=0, const char** values=0); //

	//basic interfaces
	ReferData* newQueue(const char* queueid, const char* parentid, long queuetype, 
		void* callback_func, int callback_type, int children_num=0); /*thread safe*/
	int getQueueDataset(const char* queueid, ReferSet** queueset, ReferLock** queuelock); /*thread safe*/
						//caller need to lock the queuelock for any access to the queueset

	int newThread(const char* threadid, long threadtype=ReferScheduler::THREAD_DEFAULT, const char* allowedqueues=0, ReferLock** threadlock=0); /*thread safe*/
						//allowedqueues: list of queue names separated by ':'
						//threadlock: used for waiting/sleeping when threads fetch queue tuples (not used for locking now)
	int waitActiveThreads();

	//construction
	ReferScheduler();
	~ReferScheduler();
	void reset();
};

#endif 

