#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include "timer.h" 

	pid_t gettid(void) 		
	{
		return syscall(SYS_gettid); 		
	}
	struct thread_args
	{
		int cpus;
		long int limit;
		long int arr[32];
	};
	void *DoWork(void *args)
	{
		pid_t pid = gettid(); 
		int affinity = sched_getcpu();
		//printf("Lightthread0%d starts\n", affinity);
		//printf("PID: %d, CPU: %d\n", pid, affinity);

		struct thread_args *newargs = (struct thread_args *) args;
		int cpus = newargs->cpus;
		long int limit = newargs->limit;
		int i;
		long int num = affinity *(limit / cpus) + 1;
		long int thread_limit = (affinity + 1) *limit / cpus;
		if ((limit - thread_limit) < (limit / cpus))
		{
			thread_limit += (limit - thread_limit);
		}

		
		int primes = 0;
		while (num <= thread_limit)
		{
			i = 2;
			while (i <= num)
			{
				if (num % i == 0)
					break;
				i++;
			}

			if (i == num)
				primes++;

			num++;
		}
		newargs->arr[affinity] = primes;
	}

//-------------------------------------------------------
	void *welcome(void *args)
	{
		pid_t pid = gettid();

		//printf("welcome starts\n");
		//printf("PID: %d, CPU: %d\n", pid, sched_getcpu());

		printf("Program just started... ");
		sleep (0.5);
		printf("Please be patient... ");
		sleep (0.5);
		printf("Results will be printed soon...\n");
	}
//-------------------------------------------------------



	void *DoSum(void *args)
	{
		pid_t pid = gettid();

		//printf("DoSum starts\n");
		//printf("PID: %d, CPU: %d\n", pid, sched_getcpu());

		struct thread_args *newargs = (struct thread_args *) args;
		long int sum = 0;
		for (int i = 0; i < newargs->cpus; i++)
		{
			sum += newargs->arr[i];
		}

		printf("%ld total prime numbers\n", sum);
	}



//int main(int argc , int **argv){


	int main(int argc, char *argv[])
	{
	     stopwatch sw; 
		stopwatch_start(&sw);
	
		if (argc == 2)
		{
			printf("The argument supplied is %s\n", argv[1]);
		}
		else
		{
			printf("Command is ./prime limit.\n");
			exit(0);
		}

		pid_t pid = getpid();
		//printf("Main pid: %d\n", pid);
		int numberOfProcessors = sysconf(_SC_NPROCESSORS_ONLN);
		//printf("Number of cores: %d\n\n", numberOfProcessors);
		
		
		pthread_t threads[numberOfProcessors];
		pthread_attr_t attr; 		
		cpu_set_t cpus; 			
		pthread_attr_init(&attr); 		
		
		
		//---------------------------------------------------------
		pthread_t userEnterface; 		
		CPU_ZERO(&cpus); 		
		CPU_SET(0, &cpus); 		
		pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus); 		
		pthread_create(&userEnterface, NULL, welcome, NULL);		
		pthread_join(userEnterface, NULL); 
		//---------------------------------------------------------	
		
		
		int core[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
		struct thread_args *args = malloc(sizeof(struct thread_args));
		args->cpus = numberOfProcessors;
		args->limit = atoi(argv[1]);
		for (int i = 0; i < numberOfProcessors; i++)
		{
			
			CPU_ZERO(&cpus); 		
			CPU_SET(core[i], &cpus); 		
			pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);			
			pthread_create(&threads[i], NULL, DoWork, args); 	
		}

		for (int i = 0; i < numberOfProcessors; i++)
		{
			pthread_join(threads[i], NULL);
		}
		pthread_t total; 		
		CPU_ZERO(&cpus); 		
		CPU_SET(0, &cpus); 		
		pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus); 		
		pthread_create(&total, NULL, DoSum, args);		
		pthread_join(total, NULL); 		

		//********************************************
		
	stopwatch_stop(&sw); 
 
    long tus = get_interval_by_usec(&sw); 
    double ts = get_interval_by_sec(&sw); 
    printf("your code took %ld us to finish \n", tus);
    printf("your code  took %lf seconds to finish\n", ts); 

    return 0; 
	}