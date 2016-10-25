#ifndef _THPOOL_
#define _THPOOL_

#include <pthread.h>
#include <semaphore.h>

namespace THPOOL
{
	class CThreadPool;

	/* Individual job */
	typedef struct thpool_job_t{
		void*  (*function)(void* arg);                     /**< function pointer         */
		void*                     arg;                     /**< function's argument      */
		struct thpool_job_t*     next;                     /**< pointer to next job      */
		struct thpool_job_t*     prev;                     /**< pointer to previous job  */
	}thpool_job_t;


	/* Job queue as doubly linked list */
	typedef struct thpool_jobqueue{
		thpool_job_t *head;                                /**< pointer to head of queue */
		thpool_job_t *tail;                                /**< pointer to tail of queue */
		int           jobsN;                               /**< amount of jobs in queue  */
		sem_t        *queueSem;                            /**< semaphore(this is probably just holding the same as jobsN) */
	}thpool_jobqueue;
	
	typedef struct thread_param{
		int id;
		CThreadPool *handler;
	}thread_param;

	/* The threadpool */
	class CThreadPool
	{
	public:
		CThreadPool();
		CThreadPool(int thNum);
		~CThreadPool();
	public:
		int fn_iInitThread();
		int thpool_add_work(void*(*function_p)(void *), void* arg_p);
		void thpool_wait_work();
	private:
		void fn_vDestroy();
		
		/*	Intern Operations	*/		
		int thpool_jobqueue_init();

		void thpool_jobqueue_add(thpool_job_t* newjob_p);

		int thpool_jobqueue_removelast();

		thpool_job_t* thpool_jobqueue_peek();

		void thpool_jobqueue_empty();
	private:
		static void* thpool_thread_do(void* tp_p);
	private:
		pthread_t*       threads;                          	/**< pointer to threads' ID   */
		int              threadsN;                         	/**< amount of threads        */
		thpool_jobqueue* jobqueue;                         	/**< pointer to the job queue */
		int thpool_keepalive;							
		bool *statusFlags;	
		thread_param *paramSet;
		pthread_mutex_t mutex;								/* used to serialize queue access */
		
	};
}

#endif
