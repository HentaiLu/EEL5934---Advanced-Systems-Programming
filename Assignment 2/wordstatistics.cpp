#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <stdexcept>
#include <algorithm>

#define MAX 10

//Defining three pools of vector strings
std::vector<std::string> pool_mapper;
std::vector<std::string> pool_reducer;
std::vector<std::string> pool_summarizer;


//Defining Mutex
pthread_mutex_t mutex,mutex_m_r,mutex_r_s;


//Defining condition variables
pthread_cond_t cond_upd,cond_map,cond_map_red,cond_red,cond_sum,cond_wc;


//Variables

std::vector<std::string> temp_buff;
int freq[100];
int flag =0;
int off =0;
char key[100][100];
int limit;

/**************************************************************
	UPDATER FUNCTION                                      *
**************************************************************/



void* updater_func(void* file)
{
	std::string x;
	std::ifstream inFile; 
	inFile.open((char*)file);
	if (!inFile) {
	std::cout << "Unable to open file";
	pthread_exit(0);	
		     }

	while (inFile >> x) {
				
				pthread_mutex_lock(&mutex);
				if(pool_mapper.size() != 0)
				pthread_cond_wait(&cond_upd, &mutex);
				pool_mapper.push_back(x);
				pthread_cond_signal(&cond_map);
				pthread_mutex_unlock(&mutex);
	
	
		            }

	inFile.close();
	sleep(1);
	pthread_exit(0);




}

/***************************************************************
	MAPPER FUNCTION                                        *
***************************************************************/


void* mapper_func(void * param)
{
	while(1){
			int i;
						
			pthread_mutex_lock(&mutex);
			if(pool_mapper.size() == 0)
			pthread_cond_wait(&cond_map, &mutex);
			pthread_mutex_lock(&mutex_m_r);
			if(pool_reducer.size()!= 0)
			pthread_cond_wait(&cond_map_red,&mutex_m_r);
			for(i=0;i < pool_mapper.size();i++)
			{	
				char s[100];
				strcpy(s,"(");
				strcat(s,pool_mapper[i].c_str());
				strcat(s,",1)");
				pool_reducer.push_back(s);
							
			}
			pthread_cond_signal(&cond_red);
			pthread_mutex_unlock(&mutex_m_r);
			pool_mapper.clear();
			pthread_cond_signal(&cond_upd);
			pthread_mutex_unlock(&mutex);


		}

}


/***************************************************************
          REDUCER FUNCTION                                     *
***************************************************************/


void* reducer_func(void * param)

{
	while(1){

		
		int i,j;
		int size;
		char key[100];
		int pos;
		std::vector<std::string>::iterator it;
		pthread_mutex_lock(&mutex_m_r);
		if(pool_reducer.size() == 0)
		pthread_cond_wait(&cond_red, &mutex_m_r);
		pthread_mutex_lock(&mutex_r_s);
		if(flag !=0)
		pthread_cond_wait(&cond_sum, &mutex_r_s);
		for(i=0;i < pool_reducer.size();i++)
			{	
				char s[100];
				strcpy(s,pool_reducer[i].c_str());
				it=find(pool_summarizer.begin(),pool_summarizer.end(),s);
    	 			pos = distance(pool_summarizer.begin(), it);
    				if(it!=pool_summarizer.end()){	

        							freq[pos]++;
    							     }
    				else{

        			pool_summarizer.push_back(s);				

			        }

			}

		flag=1;
		pthread_cond_signal(&cond_wc);			
		pthread_mutex_unlock(&mutex_r_s);	
		pool_reducer.clear();
		pthread_cond_signal(&cond_map_red);
		pthread_mutex_unlock(&mutex_m_r);		
		
	}
}

/****************************************************************
	SUMMARIZER FUNCTION (WORD COUNT WRITER)			*
								*
****************************************************************/


void* summary_func(void* param)
{
	while(1){
	
	std::string c;
	char d[50];
	int count=0;
	char* key_temp;
	//char key[100][100];
	int j;
	int signal;
	
	
	pthread_mutex_lock(&mutex_r_s);
	if(flag !=1)
	pthread_cond_wait(&cond_wc, &mutex_r_s);
	for (std::vector<std::string>::const_iterator i = pool_summarizer.begin(); i != pool_summarizer.end(); ++i)
    	{
			c=*i;	
			strcpy(d,c.c_str());	
			key_temp = strtok(d," (,)");
			//if(key_temp[0] > key[count-1][0])
			//{ std::cout<<key_temp;}
			strcpy(key[count],key_temp);
			
			count++;
	}
	
	flag =0;
	
	
	pthread_cond_signal(&cond_sum);
	pthread_mutex_unlock(&mutex_r_s);
	limit=count;

	      }

}

/*********************************************************************
       WORD COUNT						     *
******************************** !************************************/
void word_count()

{
	int i=0;
	char value[100];
	


}

using namespace std;


//mainline of the program
int main(int argc, char *argv[])
{
	int i;
	int thread_count_mapper;
	int thread_count_reducer;
	char* file;
	char * value;
	std::ofstream file_out;
	/*Threads for performing different functions*/
	pthread_t updater;   
	pthread_t mapper[MAX];
	pthread_t reducer[MAX];
	pthread_t summary;
	
	



	file=argv[1];                      //filename
	thread_count_mapper=atoi(argv[2]); //no of mapper threads
	thread_count_reducer=atoi(argv[3]);//no of reducer threads

	
	/*Initializing Condition variables*/
	pthread_cond_init(&cond_upd, NULL);
	pthread_cond_init(&cond_map, NULL);
	pthread_cond_init(&cond_map_red,NULL);
	pthread_cond_init(&cond_red, NULL);
	pthread_cond_init(&cond_sum, NULL);
	pthread_cond_init(&cond_wc, NULL);


	/*Initializing Mutex variable*/
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&mutex_m_r,NULL);
	pthread_mutex_init(&mutex_r_s,NULL);


	for(i=0;i<100;i++)
	{
		freq[i]=1;
	
	}



	pthread_create(&updater, NULL,updater_func,(void*)file);  						//Single thread for updating the mapper pool
	for (i = 0; i < thread_count_mapper; i++) {  		  						// create Mappers
             						pthread_create(&mapper[i], NULL,mapper_func,NULL);
					       	  }
	
	for (i = 0; i < thread_count_reducer; i++) { 								//create Reducers
							pthread_create(&reducer[i], NULL,reducer_func,NULL);
						   }

	pthread_create(&summary, NULL,summary_func,NULL);                                                        //Thread for writting the Output


	pthread_join(updater,NULL);
	//word_count();
	//sleep(1);
	file_out.open("Wordcount.txt", std::ios_base::app);
	for(i=0;i < limit;i++){
	file_out<<"("<<key[i]<<","<<freq[i]<<")"<<std::endl;	;
	}
	file_out.close();
	return 0;

}
