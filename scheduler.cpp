/*
	Lukas Taylor
	CS470
	lab 5

*/
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>	//vector
#include <algorithm>	//sort

using namespace std;

struct PROCESS
{
	int processID, burstTime, priority, state;
	bool operator() (PROCESS i, PROCESS j) {return (i.priority < j.priority);}
}process; 

vector<PROCESS> CPUoneVector;
vector<PROCESS> CPUtwoVector;
pthread_t threadID[4];
pthread_mutex_t lock;
int statusFlag, numProcesCPUone, numProcessCPUtwo;
void setProcessMembers(PROCESS *process);
int randNumGenerator(int max);
void threadExecution();
void *FCFS (void* arg);
void *priorityScheduling (void* arg);
void *aging (void* arg);
void *status (void* arg);

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		cout << "Error: Number of processes per CPU not specified." << endl;
		exit(0);
	}
	else if(argc == 3)
	{
		numProcesCPUone = atoi(argv[1]);	//number of processes 
		numProcessCPUtwo = atoi(argv[2]);	
	}
	srand(time(NULL));
	threadExecution();
	return 0;
}
void setProcessMembers(PROCESS *process)
{
	process->processID = randNumGenerator(1000);	//generate random PID between 0 and 10000
	process->burstTime = randNumGenerator(5);	//generate CPU burst between 0 and 5 randomly
	process->priority = randNumGenerator(127);	//generate rand num between 0 and 127
	process->state = 1;	//initial state set to 1 signifying ready state
}
int randNumGenerator(int max)
{
	int randNum;
	randNum = rand() % max + 1;	//generates random number between 0 and max
	return randNum;
}
/*
	Function that creates threads
*/
void threadExecution()
{
	int i = 0;	//while loop counter
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        cout << "mutex init failed." << endl;;
        exit(1);
    }
    while(i < 4)	//need while loop to iterate through array of threads and create each one.
    {
    	if(i == 0)
    	{
    		pthread_create(&(threadID[i]), NULL, &FCFS, NULL);	//First Come First Serve CPU function
    	}
    	else if(i == 1)
    	{
    		pthread_create(&(threadID[i]), NULL, &priorityScheduling, NULL);	//Priority Scheduling CPU function
    	}
        else if (i == 2)
        {
        	pthread_create(&(threadID[i]), NULL, &aging, NULL);	//Aging function
        }
        else if (i == 3)
        {
        	pthread_create(&(threadID[i]), NULL, &status, NULL);	//Status mimicking function
        }
        else
        {
        	cout << "Error in creating thread: " << endl;
        	exit(1);
        }
        i++;
    }   
    pthread_join(threadID[0], NULL);	//join FCFS threads (CPU one)
    pthread_join(threadID[1], NULL);	//join priority scheduling thread (CPU two)
    pthread_mutex_destroy(&lock);	//destory mutex lock
}
/*
	Implements mock FCFS CPU scheduling
*/
void *FCFS (void* arg)
{
	if(CPUoneVector.size() == 0)
	{
		for(int i = 0; i < numProcesCPUone; i++)
		{
			CPUoneVector.push_back(PROCESS());	//create a vector of PROCESS's 
			setProcessMembers(&CPUoneVector[i]);	//sets each PROCESS member (PCB)
		}
	}
	while(CPUoneVector.size() != 0)	//while vector size is not 0 go through
	{
		for(int i = 0; i < CPUoneVector.size(); i++)
		{
			if(CPUoneVector[i].state == 1)	//ready state
			{
				for(int k = 0; k < CPUoneVector.size(); k++)	//loop through vector printing to track processes 
				{
					pthread_mutex_lock(&lock);
					cout << "\t\tProcess ID: " 
						 << CPUoneVector[k].processID 
						 << "\tProcess state: " 
						 << CPUoneVector[k].state 
						 << endl;
					pthread_mutex_unlock(&lock);
				}
				cout << "Handling process: " << CPUoneVector[i].processID << endl;
				sleep(CPUoneVector[i].burstTime);
				cout << "Process: " 
					 << CPUoneVector[i].processID 
					 << " terminated\n" 
					 << endl;
				CPUoneVector.erase(CPUoneVector.begin() + i );
				break;	//break if process is not in ready state
			}
		}
		pthread_create(&(threadID[3]), NULL, &status, NULL);
	}

}

/*
	Implements mock priority CPU scheduling
*/
void *priorityScheduling (void* arg)
{
 	if(CPUtwoVector.size() == 0)
 	{
		for(int i = 0; i < numProcessCPUtwo; i++)
		{
			CPUtwoVector.push_back(PROCESS());	//puts process into vector
			setProcessMembers(&CPUtwoVector[i]);	//sets each member of the PROCESS struct
		}
 	}	
	sort(CPUtwoVector.begin(), CPUtwoVector.end(), process);	//sort vector according to priority of process
	while(CPUtwoVector.size() != 0)
	{
		for(int i = 0; i < CPUtwoVector.size(); i++)
		{
			if(CPUtwoVector[i].state == 1)	//ready state
			{
				for(int k = 0; k < CPUtwoVector.size(); k++)	//loop through sorted vector printing to track processes
				{
					pthread_mutex_lock(&lock);
					cout << "\t\tProcess ID: " << CPUtwoVector[k].processID 
						 << "\tProcess state: " << CPUtwoVector[k].state 
						 << " Priority: " << CPUtwoVector[k].priority << endl;
					pthread_mutex_unlock(&lock);
				}

				cout << "Handling process: " << CPUtwoVector[i].processID << endl;
				sleep(CPUtwoVector[i].burstTime);	//sleep for the burst time of process
				cout << "Process " 
					 << CPUtwoVector[i].processID 
					 << " terminated.\n" 
					 << endl;

				CPUtwoVector.erase(CPUtwoVector.begin() + i );	//delete whichever process has been handled at current iteration
				break;	//break if the process is not in the ready state
			}
		}
		pthread_create(&(threadID[3]), NULL, &status, NULL);
		pthread_create(&(threadID[2]), NULL, &aging, NULL);	//Aging function
	}

}
/*
	Used for aging thread
*/
void *aging (void* arg)
{
 	while(CPUtwoVector.size() != 0)
 	{
 		for(int i = 0; i < CPUtwoVector.size(); i++)
 		{
 			if(CPUtwoVector[i].priority == 0)
 			{
 				CPUtwoVector[i].priority++;
 			}
 			CPUtwoVector[i].priority--;
 		}
 		sleep(2);
 	}
}
/*
	Used for status mimicking thread
*/
void *status (void* arg)
{
	while(CPUoneVector.size() != 0)
	{	
		for(int i = 0; i < CPUoneVector.size(); i++)
		{
			statusFlag = (rand() % 2);	//ready state is 1 and wait state is 0	
			CPUoneVector[i].state = statusFlag;
		}
		sleep(5);
	}
	while(CPUtwoVector.size() != 0)
	{
		for(int i = 0; i < CPUtwoVector.size(); i++)
		{
			statusFlag = (rand() % 2);	//ready state is 1 and wait state is 0	
			CPUtwoVector[i].state = statusFlag;
		}
		sleep(5);
	}
}