#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>

#define NUM_THREADS     3
#define HIGH_PRIO_SERVICE 1
#define MID_PRIO_SERVICE 2
#define LOW_PRIO_SERVICE 3
#define NUM_MSGS 3

pthread_t threads[NUM_THREADS];
pthread_attr_t rt_sched_attr;
int rt_max_prio, rt_min_prio, min, old_ceiling, new_ceiling;
struct sched_param rt_param;
struct sched_param nrt_param;

pthread_mutex_t msgSem;
pthread_mutexattr_t rt_safe;
int rt_protocol;

volatile int runInterference=0, CScount=0;
volatile int idleCount=0;
int intfTime=0;


#ifdef DARWIN
void compElapsedTime(struct timeval *startT, struct timeval *endT, struct timeval *deltaT)
{
  int secs=0, usecs=0;

  secs = endT->tv_sec - startT->tv_sec;
  usecs = endT->tv_usec - startT->tv_usec;

  if(usecs < 0)
  {
    usecs = endT->tv_usec + (1000000000 - startT->tv_usec);
    secs--;
  }

  deltaT->tv_sec = secs;
  deltaT->tv_usec = usecs;

}
#else
void compElapsedTime(struct timespec *startT, struct timespec *endT, struct timespec *deltaT)
{
  int secs=0, nsecs=0;

  secs = endT->tv_sec - startT->tv_sec;
  nsecs = endT->tv_nsec - startT->tv_nsec;

  if(nsecs < 0)
  {
    nsecs = endT->tv_nsec + (1000000000 - startT->tv_nsec);
    secs--;
  }

  deltaT->tv_sec = secs;
  deltaT->tv_nsec = nsecs;

}
#endif

void *semPrinter(void *threadid)
{
   int i;
   struct timespec sleepTime, dTime;
#ifdef DARWIN
   struct timeval lowStart, lowEnd, highStart, highEnd, elapsedTime;
#else
   struct timespec lowStart, lowEnd, highStart, highEnd, elapsedTime;
#endif

   if((int)threadid == LOW_PRIO_SERVICE)
   {
#ifdef DARWIN
     gettimeofday(&lowStart, (struct timezone *)NULL);
     printf("Low prio thread requesting shared resource at %d sec, %d usec\n", lowStart.tv_sec, lowStart.tv_usec);
#else
     gettimeofday(&lowStart);
     printf("Low prio thread requesting shared resource at %d sec, %d nsec\n", lowStart.tv_sec, lowStart.tv_nsec);
#endif
   }
   else
   {
#ifdef DARWIN
     gettimeofday(&highStart, (struct timezone *)NULL);
     printf("High prio thread requesting shared resource at %d sec, %d usec\n", highStart.tv_sec, highStart.tv_usec);
#else
     gettimeofday(&highStart);
     printf("High prio thread requesting shared resource at %d sec, %d nsec\n", highStart.tv_sec, highStart.tv_nsec);
#endif
   }

   pthread_mutex_lock(&msgSem);

   CScount++;

   if((int)threadid == LOW_PRIO_SERVICE)
   {
	   printf("Low prio task in CS\n");
   }
   else
   {
	   printf("High prio task in CS\n");
   }

   sleepTime.tv_sec = 1;
   sleepTime.tv_nsec = 0;

   // Camp out long enough to get stuck here, but not too long because
   // POSIX semaphores a picky about being held too long
   nanosleep((const struct timespec *)&sleepTime, &dTime);

   for(i=0; i<NUM_MSGS; i++)
   {
     printf("\nPrint msg # %d from task %d", i, threadid);

     if((int)threadid == LOW_PRIO_SERVICE)
     {
       printf(" low prio task\n");
     }
     else
     {
       printf(" high prio task\n");
     }
   }

   pthread_mutex_unlock(&msgSem);

   if((int)threadid == LOW_PRIO_SERVICE)
   {
#ifdef DARWIN
     gettimeofday(&lowEnd, (struct timezone *)NULL);
     compElapsedTime(&lowStart, &lowEnd, &elapsedTime);
     printf("Low prio thread done in %d sec, %d usec\n", elapsedTime.tv_sec, elapsedTime.tv_usec);
#else
     gettimeofday(&lowEnd);
     compElapsedTime(&lowStart, &lowEnd, &elapsedTime);
     printf("Low prio thread done in %d sec, %d nsec\n", elapsedTime.tv_sec, elapsedTime.tv_nsec);
#endif
   }
   else
   {
#ifdef DARWIN
     gettimeofday(&highEnd, (struct timezone *)NULL);
     compElapsedTime(&highStart, &highEnd, &elapsedTime);
     printf("High prio thread done in %d sec, %d usec\n", elapsedTime.tv_sec, elapsedTime.tv_usec);
#else
     gettimeofday(&highEnd);
     compElapsedTime(&highStart, &highEnd, &elapsedTime);
     printf("High prio thread done in %d sec, %d nsec\n", elapsedTime.tv_sec, elapsedTime.tv_nsec);
#endif
   }

   pthread_exit(NULL);
}

void *idle(void *threadid)
{
  struct timespec sleepTime, dTime;
  int oldstate, oldtype;

  //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
  //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

  printf("idle starting\n");
  while(runInterference)
  {
    idleCount++;
    if(runInterference == 0) break;
  }
  printf("idle stopping\n");

  //sleepTime.tv_sec = 1;
  //sleepTime.tv_nsec = 0;
  //nanosleep((const struct timespec *)&sleepTime, &dTime);
  pthread_exit(NULL);
}


void print_scheduler(void)
{
   int schedType;

#ifdef DARWIN
   pthread_attr_getschedpolicy(&rt_sched_attr, &schedType);
#else
   schedType = sched_getscheduler(getpid());
#endif

   switch(schedType)
   {
     case SCHED_FIFO:
	   printf("Pthread Policy is SCHED_FIFO\n");
	   break;
     case SCHED_OTHER:
	   printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
	   printf("Pthread Policy is SCHED_OTHER\n");
	   break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }
}

int main (int argc, char *argv[])
{
   int rc, invSafe=0, i, scope;
   struct timespec sleepTime, dTime;

   CScount=0;

   if(argc < 2)
   {
     printf("Usage: prioinvert interfere-seconds [safe|unsafe]\n");
     exit(-1);
   }
   else if(argc == 2)
   {
     sscanf(argv[1], "%d", &intfTime);
     printf("interference time = %d secs\n", intfTime);
     printf("unsafe mutex will be created\n");
   }
   else if(argc >= 3)
   {
     sscanf(argv[1], "%d", &intfTime);
     printf("interference time = %d secs\n", intfTime);
     if(strncmp("safe", argv[2], 4) == 0)
     {
       invSafe=1;
       printf("safe mutex will be created\n");
     }
     else
       printf("unsafe mutex will be created\n");
   }

   print_scheduler();

   pthread_attr_init(&rt_sched_attr);
   pthread_attr_setinheritsched(&rt_sched_attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&rt_sched_attr, SCHED_FIFO);

   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);

#ifdef DARWIN
#else
   rc=sched_getparam(getpid(), &nrt_param);
   rt_param.sched_priority = rt_max_prio;
   rc=sched_setscheduler(getpid(), SCHED_FIFO, &rt_param);

   if (rc) {printf("ERROR; sched_setscheduler rc is %d\n", rc); perror(NULL); exit(-1);}
#endif

   print_scheduler();

   if(invSafe)
   {
#if defined(_POSIX_THREAD_PRIO_PROTECT)
#warning "PRIO_PROTECT"
     pthread_mutexattr_init(&rt_safe);

     if(pthread_mutexattr_getprotocol(&rt_safe, &rt_protocol) != 0)
       perror("mutex getprotocol");
     else
       if(rt_protocol == PTHREAD_PRIO_PROTECT) printf("PTHREAD_PRIO_PROTECT\n");
       else if(rt_protocol == PTHREAD_PRIO_INHERIT) printf("PTHREAD_PRIO_INHERIT\n");
       else if(rt_protocol == PTHREAD_PRIO_NONE) printf("PTHREAD_PRIO_NONE\n");
       else printf("UNKNOWN PTHREAD_PRIO_POLICY\n");  

     if(pthread_mutexattr_setprotocol(&rt_safe, PTHREAD_PRIO_PROTECT) != 0)
       perror("mutex setprotocol failed");

     if(pthread_mutexattr_getprotocol(&rt_safe, &rt_protocol) != 0)
       perror("mutex getprotocol");
     else
       if(rt_protocol == PTHREAD_PRIO_PROTECT) printf("PTHREAD_PRIO_PROTECT\n");
       else if(rt_protocol == PTHREAD_PRIO_INHERIT) printf("PTHREAD_PRIO_INHERIT\n");
       else if(rt_protocol == PTHREAD_PRIO_NONE) printf("PTHREAD_PRIO_NONE\n");
       else printf("UNKNOWN PTHREAD_PRIO_POLICY\n");  

     //if(pthread_mutex_setprioceiling(&msgSem, 0, &old_ceiling) != 0)
     //  perror("mutex setprioceiling failed");

     //if(pthread_mutex_getprioceiling(&msgSem, &new_ceiling) != 0)
     //  perror("mutex getprioceiling failed");

     //printf("setting ceiling to %d, old ceiling was %d\n", new_ceiling, old_ceiling);
     pthread_mutex_init(&msgSem, &rt_safe);
#elif defined(_POSIX_THREAD_PRIO_INHERIT)
#warning "PRIO_INHERIT"
     pthread_mutexattr_init(&rt_safe);
     pthread_mutexattr_getprotocol(&rt_safe, &rt_protocol);
     pthread_mutexattr_setprotocol(&rt_safe, PTHREAD_PRIO_INHERIT);
     pthread_mutex_init(&msgSem, &rt_safe);
#else
     pthread_mutex_init(&msgSem, NULL);
     printf("unsafe mutex created, no safe option available on this platform\n");
#warning "no safe semaphore option on this platform"
#endif
   }
   else
   {
     // Set default protocol for mutex
     pthread_mutex_init(&msgSem, NULL);
   }

   printf("min prio = %d, max prio = %d\n", rt_min_prio, rt_max_prio);
   pthread_attr_getscope(&rt_sched_attr, &scope);

   if(scope == PTHREAD_SCOPE_SYSTEM)
     printf("PTHREAD SCOPE SYSTEM\n");
   else if (scope == PTHREAD_SCOPE_PROCESS)
     printf("PTHREAD SCOPE PROCESS\n");
   else
     printf("PTHREAD SCOPE UNKNOWN\n");

   rt_param.sched_priority = rt_min_prio;
   pthread_attr_setschedparam(&rt_sched_attr, &rt_param);

   printf("Creating thread %d\n", LOW_PRIO_SERVICE);
   rc = pthread_create(&threads[0], &rt_sched_attr, semPrinter, (void *)LOW_PRIO_SERVICE);

   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}

   printf("Low prio print to screen thread spawned\n");


   while(CScount < 1)
   {
     sleepTime.tv_sec = 0;
     sleepTime.tv_nsec = 100000000;
     nanosleep((const struct timespec *)&sleepTime, &dTime);
   }

   printf("CScount = %d\n", CScount);

   rt_param.sched_priority = rt_max_prio;
   pthread_attr_setschedparam(&rt_sched_attr, &rt_param);

   printf("Creating thread %d\n", HIGH_PRIO_SERVICE);
   rc = pthread_create(&threads[1], &rt_sched_attr, semPrinter, (void *)HIGH_PRIO_SERVICE);

   if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}
   printf("High prio print to screen thread spawned\n");

   idleCount=0;
   runInterference=1;
   rt_param.sched_priority = (rt_max_prio + rt_min_prio)/2;
   pthread_attr_setschedparam(&rt_sched_attr, &rt_param);

   if(intfTime > 0)
   {
     printf("Creating thread %d\n", MID_PRIO_SERVICE);
     rc = pthread_create(&threads[2], &rt_sched_attr, idle, (void *)MID_PRIO_SERVICE);

     if (rc) {printf("ERROR; pthread_create() rc is %d\n", rc); perror(NULL); exit(-1);}
     pthread_detach(threads[2]);
     printf("Middle prio interference thread spawned\n");

     for(i=0; i<intfTime; i++)
     {
       sleepTime.tv_sec = 1;
       sleepTime.tv_nsec = 0;
       nanosleep((const struct timespec *)&sleepTime, &dTime);
     }

     printf("idleCount=%d\n", idleCount); 
     runInterference=0; 

     //if(pthread_cancel(threads[2]) != 0)
     //  perror("cancel thread");

     printf("Stopped interference\n");
  
     //if(pthread_join(threads[2], NULL) == 0)
     //  printf("middle %d done\n", threads[2]);
     //else
     //  perror("middle");
   }

   printf("will join CS threads\n");

   if(pthread_join(threads[0], NULL) == 0)
     printf("low %d done\n", threads[0]);
   else
     perror("low");

   if(pthread_join(threads[1], NULL) == 0)
     printf("high %d done\n", threads[1]);
   else
     perror("high");

   printf("Final idleCount=%d\n", idleCount); 

   if(pthread_mutex_destroy(&msgSem) != 0)
     perror("mutex destroy");

   if(pthread_attr_destroy(&rt_sched_attr) != 0)
     perror("attr destroy");

#ifdef DARWIN
#else
   rc=sched_setscheduler(getpid(), SCHED_OTHER, &nrt_param);
#endif

   printf("All done\n");

   exit(0);
}

