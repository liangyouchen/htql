#include "referscheduler.h"
#include "referlock.h"
#include "referset.h"
#include "stroper.h"


#if defined(_WINDOWS) && !defined(NO_WINDOWS)
	//for Window threading
#include "MiniWin.h"
#endif

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define TRACING_SERVICE_THREADS	2	//1:everything; 2:no lock and unlock; 
#endif

const char* FIELD_QUEUES[]={
	"QueueID", "Queue", "HistoryQueue", "QueueLock", "QueueType", "CallBack", "WorkingThread", "QueueParent", 0
};
const char* FIELD_THREADS[]={
	"ThreadID", "Thread", "ThreadLock", "ThreadType", "AllowedQueueIDs", "WorkingQueueID", 0
};

ReferScheduler::ReferScheduler(): Queues("ReferScheduler::Queues"), Threads("ReferScheduler::Threads"){
	int i=0;
#ifdef _WINDOWS
	ASSERT(FIELD_QUEUES[ID_QueueFieldsNum]==0);
#endif
	Queues.setFieldsNum(ID_QueueFieldsNum, FIELD_QUEUES);
	const char*fieldlist[]={FIELD_QUEUES[ID_QueueID], 0};
	long fieldsflag[]={0, 0};
	Queues.newIndex(FIELD_QUEUES[ID_QueueID], fieldlist, fieldsflag);

#ifdef _WINDOWS
	ASSERT(FIELD_THREADS[ID_ThreadFieldsNum]==0);
#endif
	Threads.setFieldsNum(ID_ThreadFieldsNum, FIELD_THREADS);
	const char*fieldlist1[]={FIELD_THREADS[ID_ThreadID], 0};
	long fieldsflag1[]={0, 0};
	Threads.newIndex(FIELD_THREADS[ID_ThreadID], fieldlist1, fieldsflag1);

	ToExit=0;
	Parent=0;
	QueuesNum=0;
	ThreadsNum=0;
}

ReferScheduler::~ReferScheduler(){
	reset();
}

void ReferScheduler::reset(){
	//wait for all threads
	waitActiveThreads();

	ReferData* tuple;
	ReferLock* lock=0;

	//delete all threads, may be not neccessary? 
	ThreadsLock.exclusiveLock();
	for (tuple=Threads.moveFirst(); tuple; tuple=Threads.moveNext()){
		if (tuple[ID_ThreadLock].P){
			delete (ReferLock*) tuple[ID_ThreadLock].P;
			tuple[ID_ThreadLock].reset();
		}
	}
	Threads.empty();
	ThreadsLock.exclusiveUnlock();

	//delete all queues
	QueuesLock.exclusiveLock();
	for (tuple=Queues.moveFirst(); tuple; tuple=Queues.moveNext()){
		lock=0;
		if (tuple[ID_QueueLock].P){
			lock=(ReferLock*) tuple[ID_QueueLock].P;
			lock->exclusiveLock();
		}
		if (tuple[ID_Queue].P){
			delete (ReferSet*) tuple[ID_Queue].P;
			tuple[ID_Queue].reset();
		}
		if (tuple[ID_HistoryQueue].P){
			delete (ReferSet*) tuple[ID_HistoryQueue].P;
			tuple[ID_HistoryQueue].reset();
		}
		if (lock){
			lock->exclusiveUnlock();
			delete lock;
			tuple[ID_QueueLock].reset();
		}
	}
	Queues.empty();
	QueuesLock.exclusiveUnlock();

	QueuesNum=0;
	ThreadsNum=0;
}
int ReferScheduler::waitActiveThreads(){
	ToExit=1;
	long thread_count=1;
	while (thread_count){
		ThreadsLock.exclusiveLock();

		thread_count=Threads.TupleCount;

		ThreadsLock.exclusiveUnlock();
		ThreadsLock.wait(1);
	}
	return 0;
}

int ReferScheduler::addQueueTuple(const char* queueid, ReferLinkHeap* name_values, const char** names, const char** values){
	//check unique index
	ReferSet* set=0; 
	ReferLock* lock=0; 
	ReferSet* unique_queue=0; 
	getQueueDataset(queueid, &set, &lock);

	QueuesLock.exclusiveLock();
	//get queue datasets
	ReferData queueid1;  queueid1.Set((char*) queueid, strlen(queueid),false);
	ReferData* tuple=Queues.findFieldString(FIELD_QUEUES[ID_QueueID], &queueid1);
	if (tuple){
		set=(ReferSet*) tuple[ID_Queue].P;
		lock=(ReferLock*) tuple[ID_QueueLock].P;
		unique_queue=(ReferSet*) tuple[ID_HistoryQueue].P;
	}

	//new tuple
	int i;
	ReferData* queue_tuple=set->getQueryTuple(name_values);
	for (i=0; names && values && names[i]; i++){
		int j=set->getFieldIndex(names[i]);
		if (j>=0) queue_tuple[i]=values[i]; 
	}

	//check if exist in unique queue
	int to_skip=false;
	if (unique_queue){
		ReferData* query_tuple=unique_queue->getQueryTuple(); 
		for (i=0; i<unique_queue->FieldsNum; i++){
			ReferData* field=set->getField(queue_tuple, unique_queue->FieldNames[i]);
			query_tuple[i].Set(field->P, field->L, field->Type);
		}

		//find from unique queue
		unique_queue->useIndexName("unique_fields");
		if (unique_queue->moveFirst(query_tuple)){ //existing
			to_skip=true;
		}else{ //not existing
			//add to unique queue
			unique_queue->newTuple(query_tuple, true); 
			unique_queue->QueryTuple=0; 
		}
	}

	if (!to_skip){
		set->newTuple(queue_tuple);
		set->QueryTuple=0;
	}
	
	QueuesLock.exclusiveUnlock();

	return 0;
}
int ReferScheduler::newQueue(const char* queueid, const char** fields, const char** unqiue_fields, long* unique_fields_type, 
							 void* callback_func, int callback_type, int maxthreads, long thread_type){
	//create queue dataset
	ReferData* tuple=newQueue(queueid, 0, 0, callback_func, callback_type, maxthreads); 
	ReferSet* set=0; 
	ReferLock* lock=0; 
	getQueueDataset(queueid, &set, &lock);
	if (lock->exclusiveLock(10)){
		if (callback_type==CALLBACK_TIMER){
			int nfields=0; 
			while (fields[nfields]) nfields++;
			char** newfields=(char**) malloc(sizeof(char*)*(nfields+ID_TimerFieldsNum+1)); 
			newfields[ID_TimerType]="TimerType";
			newfields[ID_TimerInterval]="TimerInterval";
			newfields[ID_TimerLastTime]="TimerLastTime";
			int i=0;
			for (i=0; i<=nfields; i++){
				newfields[ID_TimerFieldsNum+i]=(char*) fields[i];
			}
			set->setFieldsNum(0, (const char**) newfields); 
			free(newfields);
		}else{
			set->setFieldsNum(0, fields); 
		}
		//create index queue
		if (unqiue_fields && unqiue_fields[0]){
			ReferSet* unique_set = new ReferSet; 
			if (unique_set) { 
				unique_set->setFieldsNum(0, unqiue_fields);
				unique_set->newIndex("unique_fields", unqiue_fields, unique_fields_type, true);
			}
			tuple[ID_HistoryQueue].Set((char*) unique_set, 0, false);
		}

		lock->exclusiveUnlock();
	}
	
	//start all threads
	char buf[64];
	ReferData child_id; 
	if (maxthreads){
		for (int i=0; i<maxthreads; i++){
			child_id=queueid; 
			sprintf(buf, "$%d", i); //use queueid$i to access child queue
			child_id+=buf; 
			newThread(child_id.P, thread_type, child_id.P);
		}
	}else{
		newThread(queueid, thread_type, queueid);
	}

	return 0;
}
ReferData* ReferScheduler::newQueue(const char* queueid, const char* parentid, long queuetype, 
									void* callback_func, int callback_type, int children_num){
	QueuesLock.exclusiveLock();
	ReferData* tuple=Queues.newTuple();
	QueuesNum++;
	tuple[ID_QueueID]=queueid;
	tuple[ID_QueueParent]=parentid;
	ReferSet* newset=new ReferSet("ReferScheduler::newQueue::newset");
	tuple[ID_Queue].Set((char*) newset, 0, false);
	ReferLock* newlock=new ReferLock;
	tuple[ID_QueueLock].Set((char*) newlock, 0, false);
	tuple[ID_CallBack].Set((char*) (callback_func), callback_type, false);
	if (parentid){ //is child
		tuple[ID_QueueType].Set(0, queuetype|QUEUE_CHILD, false);
	}else if (children_num) {//is parent
		tuple[ID_QueueType].Set(0, queuetype|QUEUE_PARENT, false);
	}else{ //independent
		tuple[ID_QueueType].Set(0, queuetype, false);
	}
	Queues.commitTuple();
	QueuesLock.exclusiveUnlock();

	//create children queues
	char buf[64];
	ReferData child_id; 
	for (int i=0; i<children_num; i++){
		child_id=queueid; 
		sprintf(buf, "$%d", i); //use queueid$i to access child queue
		child_id+=buf; 
		newQueue(child_id.P, queueid, queuetype, callback_func, callback_type, 0); 
	}

	return tuple;
}

int ReferScheduler::getQueueDataset(const char* queueid, ReferSet** queueset, ReferLock** queuelock){
	if (queueset) *queueset=0;
	if (queuelock) *queuelock=0;

	QueuesLock.exclusiveLock();
	ReferData queueid1;  queueid1.Set((char*) queueid, strlen(queueid),false);
	ReferData* tuple=Queues.findFieldString(FIELD_QUEUES[ID_QueueID], &queueid1);
	if (tuple){
		if (queueset) *queueset=(ReferSet*) tuple[ID_Queue].P;
		if (queuelock) *queuelock=(ReferLock*) tuple[ID_QueueLock].P;
	}
	QueuesLock.exclusiveUnlock();

	return tuple?1:0;
}

int ReferScheduler::newThread(const char* threadid, long threadtype, const char* allowed_queues, ReferLock** threadlock){
	ThreadsLock.exclusiveLock();

	ReferData* tuple=Threads.newTuple();
	ThreadsNum++;
	tuple[ID_ThreadID]=threadid;
	tuple[ID_ThreadType].Set(0, threadtype, false);
	if (allowed_queues) tuple[ID_AllowedQueueIDs]=allowed_queues;

	ReferLock* newlock=new ReferLock;
	tuple[ID_ThreadLock].Set((char*)newlock, 0, false);

	if (threadlock) (*threadlock)=newlock;

	long thread=0;
	void* threadparam[]={this, &thread}; //new thread will set threadparam[1]
	if ((threadtype&THREAD_WINDOWS)){
		#if defined(_WINDOWS) && !defined(NO_WINDOWS)
			#pragma message(__FILE__ " ***** Windows Version *****")
			//start windows thread
			CMiniWin* working_win= new CMiniWin(0);
			working_win->setInitialCall(serviceThread, threadparam);
			working_win->Create(IDD_DIALOG_MINI_WIN, 0);
			//working_win->ShowWindow(IsWindowVisible()?SW_SHOW:SW_HIDE);
			working_win->ShowWindow(SW_SHOW);
			//_beginthread(serviceThread, 0, &threadparam);
		#elif defined _WINDOWS || defined WIN32
			#pragma message(__FILE__ " ***** NO Windows Version *****")
			beginthread(serviceThread, 0, &threadparam);
		#else
			pthread_t thread_id=0;
			pthread_create(&thread_id, 0, serviceThread, &threadparam);
		#endif
	}else{
#if defined _WINDOWS  || defined WIN32
		beginthread(serviceThread, 0, &threadparam);
#else
		pthread_t thread_id=0;
		pthread_create(&thread_id, 0, serviceThread, &threadparam);
#endif
	}
		
	while (!thread){ //wait until the thread has got the parameters
		newlock->wait(1);
	}

	tuple[ID_Thread].Set(0, thread, false);

	Threads.commitTuple();
	ThreadsLock.exclusiveUnlock();
	return 0;
}

ThreadType ReferScheduler::serviceThread(void* p){
	//get intial parameters
	void** param=(void**) p;
	long *myflag=(long*) param[1];
	ReferScheduler* scheduler=(ReferScheduler*) param[0];

	ReferData* thread_tuple=scheduler->Threads.getTuple();
	ReferData thread_id=thread_tuple[ID_ThreadID];
	ReferLock* thread_lock=(ReferLock*) thread_tuple[ID_ThreadLock].P;
	ReferData allowed_queues=thread_tuple[ID_AllowedQueueIDs];
	long thread_type=thread_tuple[ID_ThreadType].L;

	*myflag=1; //signal the master to continue
	//if (thread_type & THREAD_WINDOWS) thread_lock->IsWindowsWait=true; // this does not work!
	thread_lock->wait(1);


	ReferLinkHeap allowed_queuelist;
	tStrOp::splitString(allowed_queues.P, ":",  &allowed_queuelist);

	//service schdeduling
	ReferData* tuple=0;
	ReferData* working_thread_id=0;
	ReferData working_queue_id;
	ReferData working_queue_parent;
	long working_queue_type=0;

	void* callback_func=0;
	int callback_type=ReferScheduler::CALLBACK_REFERSET;

	ReferLock* working_queue_lock=0;
	ReferSet* working_queue=0;
	ReferLock* unique_queue_lock=0;
	ReferSet* unique_queue=0; //in parent queue tuple
	
	ReferData* parent_tuple=0;
	ReferLock* working_parent_lock=0;
	ReferSet* working_parent_queue=0;

	#ifdef TRACING_SERVICE_THREADS
		TRACE("Thread '%s' started.\n", thread_id.P);
	#endif
	while (1){
		//find an availble queue
		//do the first time, or everytime if the threadtype==THREAD_FLOATING;
		if (working_queue_id.isNULL() && scheduler->QueuesLock.exclusiveLock(10)){
			#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
				TRACE("Thread '%s' locks Queues\n", thread_id.P);
			#endif
			working_thread_id=0;
			working_queue_id.reset();
			working_queue_parent.reset();
			working_queue_type=0;

			callback_func=0;
			callback_type=ReferScheduler::CALLBACK_REFERSET;

			working_queue_lock=0;
			working_queue=0;
			unique_queue_lock=0;
			unique_queue=0;

			parent_tuple=0;
			working_parent_lock=0;
			working_parent_queue=0;

			//find and reserve the appropriate queue
			scheduler->Queues.useIndexName(FIELD_QUEUES[ID_QueueID]);
			//continue from last tuple position
			if (tuple) tuple=scheduler->Queues.moveFirst(tuple);
			if (tuple) {
				tuple=scheduler->Queues.moveNext();
			}else{
				tuple=scheduler->Queues.moveFirst();
			}
			//TRACE("allowed_queuelist.Total=%d\n", allowed_queuelist.Total);
			for (; tuple; tuple=scheduler->Queues.moveNext() ){
				if (tuple[ID_WorkingThread].L) continue;
				if (tuple[ID_QueueType].L & QUEUE_PARENT) continue;
				if (allowed_queuelist.Total){
					if (!allowed_queuelist.findName(&tuple[ID_QueueID]) &&
						!allowed_queuelist.findName(&tuple[ID_QueueParent]) ) continue;
				}
				#ifdef TRACING_SERVICE_THREADS
					TRACE("Thread '%s' reserving '%s'\n", thread_id.P, tuple[ID_QueueID].P);
				#endif
				//reserve this queue
				working_thread_id=&tuple[ID_WorkingThread];
				if (working_thread_id->isNULL()){
					working_thread_id->Set(thread_id.P, thread_id.L, true);

					working_queue_id=tuple[ID_QueueID];
					working_queue_parent=tuple[ID_QueueParent];
					callback_func=tuple[ID_CallBack].P; //use parent queue's callback
					callback_type=tuple[ID_CallBack].L;
					working_queue_lock=(ReferLock*) tuple[ID_QueueLock].P;
					working_queue=(ReferSet*) tuple[ID_Queue].P;
					working_queue_type=tuple[ID_QueueType].L;
					unique_queue=(ReferSet*) tuple[ID_HistoryQueue].P; //use parent queue's unique set
					unique_queue_lock=working_queue_lock; 
					break;
				}
			}

			if (working_queue_parent.P){
				parent_tuple=scheduler->Queues.findFieldString(FIELD_QUEUES[ID_QueueID], &working_queue_parent);
				if (parent_tuple){
					working_parent_lock=(ReferLock*) parent_tuple[ID_QueueLock].P;
					working_parent_queue=(ReferSet*) parent_tuple[ID_Queue].P;
					callback_func=parent_tuple[ID_CallBack].P;
					callback_type=parent_tuple[ID_CallBack].L;
					unique_queue=(ReferSet*) parent_tuple[ID_HistoryQueue].P;
					unique_queue_lock=working_parent_lock; 
				}
			}
			#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
				TRACE("Thread '%s' unlocks Queues\n", thread_id.P);
			#endif
			scheduler->QueuesLock.exclusiveUnlock();
		}


		//if it is a children queue, move one data tuple from the parent queue
		if (working_thread_id && working_parent_lock && working_parent_queue){
			//get data from the parent queue first
			if (working_parent_lock->exclusiveLock(10)){
				#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
					TRACE("Thread '%s' locks parent '%s'\n", thread_id.P, parent_tuple[ID_QueueID].P);
				#endif

				ReferData* queue_data=0;
				int to_free_queue_data=true;

				if (working_parent_queue->TupleCount>0){
					if (working_queue_type & QUEUE_FILO){
						queue_data=working_parent_queue->moveLast();
					}else{
						queue_data=working_parent_queue->moveFirst();
					}
					if (queue_data){
						to_free_queue_data=working_parent_queue->DataCursor->Value.L; //whether to free the tuple
					}
					working_parent_queue->dropTuple(0, false); //parent don't delete tuple data
				}
				if (queue_data && working_queue->FieldsNum==0){
					working_queue->copyStruct(working_parent_queue);
				}
				#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
					TRACE("Thread '%s' unlocks parent '%s'\n", thread_id.P, parent_tuple[ID_QueueID].P);
				#endif
				working_parent_lock->exclusiveUnlock();

				if (queue_data) {
					working_queue_lock->exclusiveLock();
					#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
						TRACE("Thread '%s' locks queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					//add the new tuple to the working queue
					#ifdef TRACING_SERVICE_THREADS
						TRACE("Thread '%s' move a tuple from parent queue '%s' to child '%s'\n", thread_id.P, parent_tuple[ID_QueueID].P, tuple[ID_QueueID].P);
					#endif
					working_queue->newTuple(queue_data, to_free_queue_data);
					working_queue->commitTuple();
					#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
						TRACE("Thread '%s' unlocks queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					working_queue_lock->exclusiveUnlock();
				}
			}
		}

		//working on the queue
		long tuple_count=0;
		long done_tuples=0;
		while (working_thread_id && working_queue && working_queue_lock){
			tuple_count=0;

			//check if the tuple has already existed in the unqiue queue
			//don't need this because the addQueueTuple() has already handled it
			/*if (unique_queue){
				if (unique_queue_lock->exclusiveLock(10)){
					if (working_queue_lock!=unique_queue_lock){
						working_queue_lock->exclusiveLock(10);
					}
					for (ReferLink2* queue_link=(ReferLink2*) working_queue->DataHead.Next; queue_link && queue_link!=&working_queue->DataHead; ){
						//will the moveNext be correct after dropping a tuple?? 

						ReferData* query_tuple=unique_queue->getQueryTuple(); 
						for (int i=0; i<unique_queue->FieldsNum; i++){
							ReferData* field=working_queue->getField((ReferData*) queue_link->Value.P, unique_queue->FieldNames[i]);
							query_tuple[i].Set(field->P, field->L, field->Type);
						}

						//find from unique queue
						if (unique_queue->moveFirst(query_tuple)){
							queue_link=(ReferLink2*) queue_link->Next;
							//drop from working queue
							working_queue->dropTuple(queue_link);
						}else{
							queue_link=(ReferLink2*) queue_link->Next;
							//add to unique queue
							unique_queue->newTuple(query_tuple); 
							unique_queue->QueryTuple=0; 
						}
					}

					if (working_queue_lock!=unique_queue_lock){
						working_queue_lock->exclusiveUnlock();
					}
					unique_queue_lock->exclusiveUnlock();
				}else 
					continue;
			}
			*/

			if (callback_type==ReferScheduler::CALLBACK_REFERSET){ //callback func handle the entire referset
				if (working_queue_lock->sharedLock(10)){
					#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
						TRACE("Thread '%s' locks queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					tuple_count=working_queue->TupleCount;
					#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
						TRACE("Thread '%s' unlocks queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					working_queue_lock->sharedUnlock();
				}

				if (tuple_count>0){
					#ifdef TRACING_SERVICE_THREADS
						TRACE("Thread '%s' executes function for queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					(*(RS_CALLBACK_REFERSET)callback_func)(scheduler, &thread_id, &working_queue_id, working_queue, working_queue_lock);
					done_tuples++;
					#ifdef TRACING_SERVICE_THREADS
						TRACE("Thread '%s' completes function for queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
				}else{ 
					break;
				}
			}else if (callback_type==ReferScheduler::CALLBACK_TUPLE){ //callback func handle only tuples
				ReferData* data_tuple=0;
				int to_free_data_tuple=false;
				if (working_queue_lock->exclusiveLock(10)){
					#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
						TRACE("Thread '%s' locks queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					if (working_queue_type & QUEUE_FILO){
						data_tuple=working_queue->moveLast(); 
					}else{
						data_tuple=working_queue->moveFirst(); 
					}
					if (data_tuple){
						to_free_data_tuple=working_queue->DataCursor->Value.L; //whether to free the tuple
						working_queue->dropTuple(0, false); 
					}
					#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
						TRACE("Thread '%s' unlocks queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					working_queue_lock->exclusiveUnlock();
				}
				if (data_tuple){
					#ifdef TRACING_SERVICE_THREADS
						TRACE("Thread '%s' executes function for queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					(*(RS_CALLBACK_TUPLE)callback_func)(scheduler, &thread_id, &working_queue_id, data_tuple);
					done_tuples++;
					if (to_free_data_tuple) delete[] data_tuple; 
				}else{
					break;
				}
			}else if (callback_type==ReferScheduler::CALLBACK_TIMER){ //callback func handle only tuples
				ReferData* data_tuple=0;
				time_t next_time=0;
				if (working_queue_lock->exclusiveLock(10)){
					#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
						TRACE("Thread '%s' locks queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
						time_t current_time=time(0);
						time_t last_time=0, interval=0; 
						for (data_tuple=working_queue->moveFirst(); data_tuple; data_tuple=working_queue->moveNext()){
							last_time=data_tuple[ID_TimerLastTime].getLong(); 
							interval=data_tuple[ID_TimerInterval].getLong(); 
							if (data_tuple[ID_TimerType].P[0]=='0') {//avery n seconds
								if (last_time==0 || current_time-last_time>=interval){
									(*(RS_CALLBACK_TUPLE)callback_func)(scheduler, &thread_id, &working_queue_id, data_tuple);
									done_tuples++;
									
									if (last_time==0){
										last_time=current_time;
									}else{
										last_time+=interval;
									}
									data_tuple[ID_TimerLastTime].setLong(last_time); 

									current_time=time(0);
								}
							}else if (data_tuple[ID_TimerType].P[0]=='1') {//at time x
								if (current_time >= last_time){
									(*(RS_CALLBACK_TUPLE)callback_func)(scheduler, &thread_id, &working_queue_id, data_tuple);
									done_tuples++;

									working_queue->dropTuple(0, true);

									current_time=time(0);
								}
							}
						}
					#if (defined(TRACING_SERVICE_THREADS) && TRACING_SERVICE_THREADS<=1)
						TRACE("Thread '%s' unlocks queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
					#endif
					working_queue_lock->exclusiveUnlock();
				}
				thread_lock->wait(1);
			}else{
#ifdef _WINDOWS
				ASSERT(0);
#endif
			}
			if (scheduler->ToExit) break;
		}

		//leave the queue
		if (working_thread_id && (thread_type&THREAD_FLOATING)){
			#ifdef TRACING_SERVICE_THREADS
				TRACE("Thread '%s' leaving queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
			#endif
			if (scheduler->QueuesLock.exclusiveLock()){
				working_thread_id->reset();
				working_thread_id=0;
				working_queue_id.reset();
				scheduler->QueuesLock.exclusiveUnlock();
				#ifdef TRACING_SERVICE_THREADS
					TRACE("Thread '%s' leaved queue '%s'\n", thread_id.P, tuple[ID_QueueID].P);
				#endif
			}
		}

		if (scheduler->ToExit) break;
		if (!done_tuples) thread_lock->wait(1);
	}

	#ifdef TRACING_SERVICE_THREADS
		TRACE("Thread '%s' exitting\n", thread_id.P);
	#endif
	//leaving the thread, delete from the Threads
	scheduler->ThreadsLock.exclusiveLock();
	if (scheduler->Threads.moveFirst(thread_tuple)){
		delete (ReferLock*) thread_tuple[ID_ThreadLock].P;
		thread_tuple[ID_ThreadLock].reset();

		scheduler->Threads.dropTuple();
		scheduler->ThreadsNum--;
	}

	scheduler->ThreadsLock.exclusiveUnlock();

#if defined _WINDOWS  || defined WIN32
	return;
#else
	return 0;
#endif
}









