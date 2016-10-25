#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include "thpool.h"      /* here you can also find the interface to each function */
using namespace THPOOL;
using namespace std;

const double S = 0.5;
const double corePercentage = 0.5;

//get the core number of the server
unsigned threadCore()
{
	unsigned count = 1;
	count = sysconf(_SC_NPROCESSORS_CONF);
	return count;
}

CThreadPool::CThreadPool()
{
	threads = NULL;
	threadsN = threadCore()*corePercentage;
	jobqueue = NULL;
	thpool_keepalive = 1;
	statusFlags = new bool[threadsN];
	memset(statusFlags, 1, sizeof(bool)*threadsN);
	pthread_mutex_init(&mutex, NULL);  
}

CThreadPool::CThreadPool(int thNum)
{
	threads = NULL;
	threadsN = thNum;
	jobqueue = NULL;
	thpool_keepalive = 1;
	statusFlags = new bool[threadsN];
	memset(statusFlags, 1, sizeof(bool)*threadsN);
	pthread_mutex_init(&mutex, NULL);  
}

CThreadPool::~CThreadPool()
{
	fn_vDestroy();
	pthread_mutex_destroy(&mutex);
	delete[] statusFlags;
	statusFlags = NULL;
	delete[] paramSet;
	paramSet = NULL;
}

int CThreadPool::thpool_add_work(void*(*function_p)(void *), void* arg_p)
{
	thpool_job_t* newJob = (thpool_job_t*)malloc(sizeof(thpool_job_t));  /* MALLOC job */
	                      
	if (newJob == NULL){
		fprintf(stderr, "thpool_add_work(): Could not allocate memory for new job\n");
		exit(1);
	}
	
	/* add function and argument */
	newJob->function = function_p;
	newJob->arg = arg_p;
	
	/* add job to queue */
	pthread_mutex_lock(&mutex);                  /* LOCK */
	thpool_jobqueue_add(newJob);
	pthread_mutex_unlock(&mutex);                /* UNLOCK */
	
	return 0;
}

void CThreadPool::thpool_wait_work()
{
	bool flag = true;
	while(jobqueue->jobsN != 0)
		sleep(S);
	do
	{
		sleep(S);
		flag = true;
		for(int i = 0; i < threadsN; ++i)
			flag &= statusFlags[i];		
	}while(flag == false);
	return;
}

int CThreadPool::fn_iInitThread()
{		
	if (!threadsN || threadsN<1) 
		threadsN = 1;
	
	threads = (pthread_t*)malloc(threadsN*sizeof(pthread_t));          /* MALLOC thread IDs */
	if (threads==NULL){
		fprintf(stderr, "thpool_init(): Could not allocate memory for thread IDs\n");
		return -1;
	}
	
	/* Initialise the job queue */
	if (thpool_jobqueue_init() == -1){
		fprintf(stderr, "thpool_init(): Could not allocate memory for job queue\n");
		return -1;
	}
	
	/* Initialise semaphore*/
	jobqueue->queueSem = (sem_t*)malloc(sizeof(sem_t));                 /* MALLOC job queue semaphore */
	sem_init(jobqueue->queueSem, 0, 0);									/* no shared, initial value */

	/* Make threads in pool */
	int t;
	paramSet = new thread_param[threadsN];
	for (t=0; t<threadsN; t++){
//		printf("Created thread %d in pool \n", t);
		paramSet[t].handler = this;
		paramSet[t].id = t;
		pthread_create(&(threads[t]), NULL, CThreadPool::thpool_thread_do, (void *)&paramSet[t]); /* MALLOCS INSIDE PTHREAD HERE */
		sleep(S);
	}
	
	return 0;
}

void CThreadPool::fn_vDestroy()
{
	/* Make sure thpool_jobqueue is empty */
	while(thpool_jobqueue_peek() != NULL)
		sleep(S);
	
	int t;
	
	/* End each thread's infinite loop */
	thpool_keepalive = 0; 

	/* Awake idle threads waiting at semaphore */
	for (t=0; t< threadsN; t++){
		if (sem_post(jobqueue->queueSem)){
			fprintf(stderr, "thpool_destroy(): Could not bypass sem_wait()\n");
		}
	}

	/* Kill semaphore */
	if (sem_destroy(jobqueue->queueSem)!=0){
		fprintf(stderr, "thpool_destroy(): Could not destroy semaphore\n");
	}
	
	/* Wait for threads to finish */
	for (t=0; t < threadsN; t++){
		pthread_join(threads[t], NULL);
	}
	
	thpool_jobqueue_empty();
	
	/* Dealloc */
	free(threads);                                                   /* DEALLOC threads             */
	free(jobqueue->queueSem);                                        /* DEALLOC job queue semaphore */
	free(jobqueue);                                                  /* DEALLOC job queue           */
}

void* CThreadPool::thpool_thread_do(void* tp_p)
{
	thread_param* m_param = (thread_param*)tp_p;
	int id = m_param->id;
	CThreadPool* handler = m_param->handler;

	while(handler->thpool_keepalive){				
		if (sem_wait(handler->jobqueue->queueSem)) {/* WAITING until there is work in the queue */
			perror("thpool_thread_do(): Waiting for semaphore");
			exit(1);
		}

		if (handler->thpool_keepalive){			
			/* Read job from queue and execute it */
			void*(*func_buff)(void* arg);
			void*  arg_buff;
			thpool_job_t* job_p;
			
			handler->statusFlags[id] = false;
			pthread_mutex_lock(&handler->mutex);				/* LOCK */			
			job_p = handler->thpool_jobqueue_peek();
			func_buff = job_p->function;
			arg_buff = job_p->arg;
			handler->thpool_jobqueue_removelast();			
			pthread_mutex_unlock(&handler->mutex);						/* UNLOCK */
			
			func_buff(arg_buff);               					/* run function */
			free(job_p);										/* DEALLOC job */
			handler->statusFlags[id] = true;
		}
		else
		{
			return NULL; /* EXIT thread*/
		}
	}
	return NULL;
}

int CThreadPool::thpool_jobqueue_init()
{
	jobqueue = (thpool_jobqueue*)malloc(sizeof(thpool_jobqueue));      /* MALLOC job queue */
	if (jobqueue==NULL) 
		return -1;
	jobqueue->tail=NULL;
	jobqueue->head=NULL;
	jobqueue->jobsN=0;
	return 0;
}

void CThreadPool::thpool_jobqueue_add(thpool_job_t* newjob_p)
{
	newjob_p->next=NULL;
	newjob_p->prev=NULL;
	
	thpool_job_t *oldFirstJob;
	oldFirstJob = jobqueue->head;
	
	/* fix jobs' pointers */
	switch(jobqueue->jobsN){
		
		case 0:     /* if there are no jobs in queue */
					jobqueue->tail=newjob_p;
					jobqueue->head=newjob_p;
					break;
		
		default: 	/* if there are already jobs in queue */
					oldFirstJob->prev=newjob_p;
					newjob_p->next=oldFirstJob;
					jobqueue->head=newjob_p;

	}

	(jobqueue->jobsN)++;     /* increment amount of jobs in queue */
	sem_post(jobqueue->queueSem);
	return;
}

int CThreadPool::thpool_jobqueue_removelast()
{
	thpool_job_t *oldLastJob;
	oldLastJob = jobqueue->tail;
	
	/* fix jobs' pointers */
	switch(jobqueue->jobsN)
	{
		
		case 0:     /* if there are no jobs in queue */
					return -1;
					break;
		
		case 1:     /* if there is only one job in queue */
					jobqueue->tail=NULL;
					jobqueue->head=NULL;
					break;
					
		default: 	/* if there are more than one jobs in queue */
					oldLastJob->prev->next = NULL;               /* the almost last item */
					jobqueue->tail = oldLastJob->prev;
					
	}
	
	(jobqueue->jobsN)--;

	return 0;
}

thpool_job_t* CThreadPool::thpool_jobqueue_peek()
{
	return jobqueue->tail;
}

void CThreadPool::thpool_jobqueue_empty()
{
	thpool_job_t* curjob;
	curjob = jobqueue->tail;
	
	while(jobqueue->jobsN)
	{
		jobqueue->tail = curjob->prev;
		free(curjob);
		curjob = jobqueue->tail;
		jobqueue->jobsN--;
	}
	
	jobqueue->tail = NULL;
	jobqueue->head = NULL;
}
