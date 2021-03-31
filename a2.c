#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>

int sem_id;

// decrement by 1 the semaphore sem_no in the semaphore set sem_id
//    - ask for a permission, i.e. wait until a permission become available
void P(int sem_id, int sem_no)
{
    struct sembuf op = {sem_no, -1, 0};
    semop(sem_id, &op, 1);
}

// increment by 1 the semaphore sem_no in the semaphore set sem_id
//   - release a previously obtained persmission
void V(int sem_id, int sem_no)
{
    struct sembuf op = {sem_no, +1, 0};
    semop(sem_id, &op, 1);
}

void thread_function3(void* arg)
{
    
    //Thread T3.3 must start before T3.5 starts and terminate after T3.5 terminates

    int *tid = (int*) arg;
    int th_id = (int)*tid;

    if(th_id != 3 && th_id != 5 && th_id != 4)
    {
        info(BEGIN,3,th_id);
        info(END,3,th_id);
    }
    else
    {
        if(th_id == 3)
        {
            info(BEGIN,3,th_id);
            V(sem_id,0);//verde
            P(sem_id,1);//rosu
            info(END,3,th_id);

        }
        else
        {   if(th_id == 5)
            {
                P(sem_id,0);//rosu
                info(BEGIN,3,th_id);
                info(END,3,th_id);
                V(sem_id,1);//verde
            }
            else
            {    if(th_id == 4)
                {
                    P(sem_id,2);//rosu
                    info(BEGIN,3,th_id);
                    info(END,3,th_id);
                    V(sem_id,3);//verde
                }
            }
        }   
    }
}

void thread_function8(void* arg)
{
    
    //The thread T8.2 must terminate before T3.4 starts(semaphore 2), but T8.4 must start only after T3.4 terminates(semaphore 3).
    int *tid = (int*) arg;
    int th_id = (int)*tid;

    if(th_id != 2 && th_id != 4)
    {
        info(BEGIN,8,th_id);
        info(END,8,th_id);
    }
    else
    {
        if(th_id == 2)
        {
            info(BEGIN,8,th_id);
            info(END,8,th_id);
            V(sem_id,2);//verde
        }
        else
        {   if(th_id == 4)
            {
                P(sem_id,3);//rosu
                info(BEGIN,8,th_id);
                info(END,8,th_id);
            }
        }
    }
}

void thread_function4(void* arg)
{
    int *tid = (int*) arg;
    int th_id = (int)*tid;

    P(sem_id,4); 
    if(th_id != 12 && th_id != 13 && th_id != 14 && th_id != 15)
    {
        info(BEGIN,4,th_id);
        info(END,4,th_id);
    }  
    if(th_id == 12)
       {
            V(sem_id,5);
            info(BEGIN,4,th_id);
            P(sem_id,6);
            info(END,4,th_id);
        }
        else
        {
            if(th_id == 13)
            {
                V(sem_id,5);
                info(BEGIN,4,th_id);
                P(sem_id,6);
                info(END,4,th_id);
            }
            else
            {
                if(th_id == 14)
                {
                    V(sem_id,5);
                    info(BEGIN,4,th_id);
                    P(sem_id,6);
                    info(END,4,th_id);
                }
            }
        }

    if(th_id == 15)
    {
        info(BEGIN,4,th_id);  
        P(sem_id,5);
        P(sem_id,5);
        P(sem_id,5);                                     
        info(END,4,th_id);                                           
        V(sem_id,6);
        V(sem_id,6);
        V(sem_id,6);
    
    }
    V(sem_id,4);
}

int main()
{   init();
    info(BEGIN, 1, 0);
    sem_id = semget(IPC_PRIVATE, 7, IPC_CREAT | 0600);
    if (sem_id < 0) {
        perror("Error creating the semaphore set");
        exit(2);
    }
    // Initialize the first 2 semaphores
    semctl(sem_id, 0, SETVAL, 0);
    semctl(sem_id, 1, SETVAL, 0);
    //The thread T8.2 must terminate before T3.4 starts
    semctl(sem_id, 2, SETVAL, 0);
    //T8.4 must start only after T3.4 terminates
    semctl(sem_id, 3, SETVAL, 0);
    //check if at most 4 threads are running concurrently
    semctl(sem_id, 4, SETVAL, 4);
    //nr of threads starting
    semctl(sem_id, 5, SETVAL, 0);
    //nr of threads ending
    semctl(sem_id, 6, SETVAL, 0);

    int p2 = fork();
    switch (p2)
    {
        case -1:
            perror("Cannot create a new child");
            exit(1);
        case 0:
        {
            info(BEGIN, 2, 0);
            int p3 = fork();
            switch(p3)
            {
                case -1:
                    perror("Cannot create a new child\n");
                    exit(1);
                case 0:
                {
                    info(BEGIN, 3, 0);
                    
                    //==ex3  
                    pthread_t threads_3[5];
                    int thread_id[5];
                    for(int i = 0; i < 5; i++)
                    {   
                        thread_id[i] = i + 1;
                        if (pthread_create(&threads_3[i], NULL, (void* (*) (void*)) thread_function3, &thread_id[i])!=0) {
                            perror("Error creating a new thread");
                            exit(1);
                        }
                    }

                    int p4 = fork();
                    switch (p4)
                    {
                        case -1:
                            perror("Cannot create a new child\n");
                            exit(1);
                        case 0: //p6
                        {   
                            info(BEGIN, 4, 0);

                            //==ex4  
                            pthread_t threads_4[45];
                            int thread_id[45];
                            for(int i = 0; i < 45; i++)
                            {   
                                thread_id[i] = i + 1;
                                if (pthread_create(&threads_4[i], NULL, (void* (*) (void*)) thread_function4, &thread_id[i])!=0) {
                                    perror("Error creating a new thread");
                                    exit(1);
                                }
                            }

                            for (int i = 0; i < 45; i++)
                                pthread_join(threads_4[i], NULL);

                            int p6 = fork();
                            switch (p6)
                            {
                                case -1:
                                    perror("Cannot create a new child\n");
                                    exit(1);
                                case 0: 
                                    info(BEGIN, 6, 0);
                                    info(END, 6, 0);
                                    exit(0);
                                default :
                                {   waitpid(p6,NULL,0);
                                    int p8 = fork();
                                    switch (p8)
                                    {
                                        case -1:
                                            perror("Cannot create a new child\n");
                                            exit(1);
                                        case 0: 
                                        {
                                            info(BEGIN, 8, 0);

                                            //==ex5
                                            pthread_t threads_8[4];
                                            int thread_id[4];
                                            for(int i = 0; i < 4; i++)
                                            {   
                                                thread_id[i] = i + 1;
                                                if (pthread_create(&threads_8[i], NULL, (void* (*) (void*)) thread_function8, &thread_id[i])!=0) {
                                                    perror("Error creating a new thread");
                                                    exit(1);
                                                }
                                            }

                                            for (int i = 0; i < 4; i++)
                                                pthread_join(threads_8[i], NULL);

                                            int p9 = fork();
                                            switch (p9)
                                            {
                                                case -1:
                                                    perror("Cannot create a new child\n");
                                                    exit(1);
                                                case 0: 
                                                    info(BEGIN, 9, 0);
                                                    info(END, 9, 0);
                                                    exit(0);
                                                default :
                                                    waitpid(p9,NULL,0);
                                                    info(END, 8, 0);
                                                    exit(0);
                                            }

                                            for (int i = 0; i < 5; i++)
                                                pthread_join(threads_3[i], NULL);
                                        }
                                        default :
                                            waitpid(p8,NULL,0);
                                    }
                                    info(END, 4, 0);
                                    exit(0);
                                }
                            }
                        }
                        default :
                            waitpid(p4,NULL,0);
                            info(END, 3, 0);
                            exit(0);
                            
                    }
                }
                default :
                    waitpid(p3,NULL,0);
                    info(END, 2, 0);
                    exit(0);
                            
            }
        }
        default:
        {
            waitpid(p2,NULL,0);
            int p5 = fork();
            switch (p5)
            {   case -1:
                    // error case
                    perror("Cannot create a new child");
                    exit(1);
                case 0:
                    info(BEGIN, 5, 0);
                    info(END, 5, 0);
                    exit(0);
                default:
                {
                    waitpid(p5,NULL,0);
                    int p7 = fork();
                    switch (p7)
                    {
                        case -1:
                            perror("Cannot create a new child\n");
                            exit(1);
                        case 0: //p7
                            info(BEGIN, 7, 0);
                            info(END, 7, 0);
                            exit(0);
                        default :
                            waitpid(p7,NULL,0);
                            
                    }
                    
                }
            }
            info(END, 1, 0);
            exit(0);
        }
    }
    semctl(sem_id, 0, IPC_RMID, 0);
    semctl(sem_id, 1, IPC_RMID, 0);
    semctl(sem_id, 2, IPC_RMID, 0);
    semctl(sem_id, 3, IPC_RMID, 0);
    semctl(sem_id, 4, IPC_RMID, 0);
    semctl(sem_id, 5, IPC_RMID, 0);
    semctl(sem_id, 6, IPC_RMID, 0);
    return 0;
}
