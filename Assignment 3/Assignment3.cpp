#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <cstdlib>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

struct shared_data {
  sem_t mutex;
  sem_t barrier;
  sem_t phil[10];
  int situ_state[10];
};

struct shared_data *shared;

void take_fork(int phil_indx,int N);
void test(int phil_indx,int N);
void put_fork(int phil_indx,int N);

void philosopher(int i,int M,int N)
{
	int j=0;
	while(j < M)
	{

	//sem_wait(&shared->mutex);
        sleep(1);
	//std::cout<<"hello"<<"by"<<i<<std::endl;
	std::cout<<"Philosopher "<<i<<" is Thinking"<<std::endl;
        take_fork(i,N);
        sleep(0);
        put_fork(i,N);
	j++;
	
	sem_wait(&shared->barrier);
        sem_post(&shared->barrier);
	}

	//std::cout<<j<<std::endl;
}

void take_fork(int phil_indx,int N)
{
    sem_wait(&shared->mutex);
    shared->situ_state[phil_indx] = 1;
    //printf("Philosopher %d is Hungry\n",phil_indx+1);
     std::cout<<"Philosopher " <<phil_indx<<" is Hungry"<<std::endl;
    test(phil_indx,N);
    sem_post(&shared->mutex);
    sem_wait(&shared->phil[phil_indx]);
    sleep(1);
}
  
void test(int phil_indx,int N)
{
    if (shared->situ_state[phil_indx] == 1 && shared->situ_state[(phil_indx-1+N)%N] != 2 && shared->situ_state[(phil_indx+1)%N] != 2)
    {
        shared->situ_state[phil_indx] = 2;
        sleep(2);
        //printf("Philosopher %d takes fork %d and %d\n",phil_indx+1,LEFT+1,phil_indx+1);
	std::cout<<"Philosopher "<<phil_indx<<" is Eating"<<std::endl;
        //printf("Philosopher %d is Eating\n",phil_indx+1);
        sem_post(&shared->phil[phil_indx]);
    }
}
  
void put_fork(int phil_indx,int N)
{
    sem_wait(&shared->mutex);
    shared->situ_state[phil_indx] = 0;
    //printf("Philosopher %d putting fork %d and %d down\n",phil_indx+1,LEFT+1,phil_indx+1);
    //printf("Philosopher %d is thinking\n",phil_indx+1);
    std::cout<<"Philosopher "<<phil_indx<<" is Thinking"<<std::endl;
    test((phil_indx-1+N)%N,N);
    test((phil_indx+1)%N,N);
    sem_post(&shared->mutex);
}


void initialize_shared(int N)
{
  int i;
  int prot=(PROT_READ|PROT_WRITE);
  int flags=(MAP_SHARED|MAP_ANONYMOUS);
  shared= static_cast<shared_data*>(mmap(0,sizeof(*shared),prot,flags,-1,0));
  memset(shared,'\0',sizeof(*shared));
  sem_init(&shared->mutex,1,1);
  sem_init(&shared->barrier,1,N);
  for(i=0;i<N;++i) sem_init(&shared->phil[i],1,0);
}


void finalize_shared(int N)
{
  int i;
  for(i=0;i<N;++i) sem_destroy(&shared->phil[i]);
  munmap(shared, sizeof(*shared));
}

using namespace std;

int  main(int argc, char *argv[])

{
	int i;
	int N= atoi(argv[1]);
	int M= atoi(argv[2]);
	pid_t pid, pids[20];
	struct shared_data data;
	shared=&data;
	for(i=0;i < N;i++)
	{
		shared->situ_state[i]=0;
        }	
	initialize_shared(N);

	
	for(i=0;i<N;++i)
  	{
   		 pid = fork();
    		 if(pid==0)
    		{
      			
      			philosopher(i,M,N);
			
      			_exit(0);
   	        }
    		else if(pid>0)
  	        {
      		
     		    pids[i] = pid;
                    cout<<i<<" "<<pids[i]<<endl;
                }
               else
                {
     		    perror("fork");
                    _exit(0);
   	        }
       }
  
  	for(i=0;i<N;++i) waitpid(pids[i],NULL,0);
	finalize_shared(N);
	return 0;

}

