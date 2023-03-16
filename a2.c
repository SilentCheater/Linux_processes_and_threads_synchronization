#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>

#define _GNU_SOURCE
#define P6 "/p6"
#define P6CLOSE "/p6close"

sem_t ex2;
sem_t closeT;
sem_t *p6;
sem_t *p6close;
sem_t max5;
sem_t regulate;
sem_t block;


void* thFuncion(void* arg){
    int* val = (int*) arg;

    p6 = sem_open(P6, O_RDWR);
    p6close = sem_open(P6CLOSE, O_RDWR);

    if(*val==2){
        sem_wait(&ex2);
    }
    if(*val==3){
        sem_wait(p6);
    }

    info(BEGIN, 5, *val);

    if(*val==4){
        sem_post(&ex2);
    }

    
    if(*val==4){
        sem_wait(&closeT);
    }

    info(END, 5, *val);

    if(*val==2){
        sem_post(&closeT);
    }
    if(*val==3){
        sem_post(p6close);
    }

    sem_close(p6);
    sem_close(p6close);
    return NULL;
}

void* otherFunction(void* arg){
    int* val = (int*) arg;
    p6 = sem_open(P6, O_RDWR);
    p6close = sem_open(P6CLOSE, O_RDWR);

    if(*val==4){
        sem_wait(p6close);
    }
    info(BEGIN, 6, *val);
    
    info(END, 6, *val);
    if(*val==3){
        sem_post(p6);
    }
    sem_close(p6);
    sem_close(p6close);

    return NULL;
}

void* lastFunction(void* arg){
    int* val = (int*) arg;

    if(*val != 13){
        sem_wait(&regulate);
    }

    sem_wait(&max5);
    info(BEGIN, 2, *val);

    if(*val != 13){
        sem_wait(&block);
    }

    info(END, 2, *val);

    if(*val == 13){
        for(int i = 0;i<39;i++)
        {
            sem_post(&regulate);
            sem_post(&block);
        }
    }

    sem_post(&max5);
    return NULL;
}

int main(){
    init();

    info(BEGIN, 1, 0);

    if(fork()==0){
        info(BEGIN, 2, 0);

        if(fork()==0){
            info(BEGIN, 3, 0);

            if(fork()==0){
                info(BEGIN, 5, 0);

                pthread_t p5t1, p5t2, p5t3, p5t4;

                int arg1 = 1;
                int arg2 = 2;
                int arg3 = 3;
                int arg4 = 4;

                sem_init(&ex2, 0, 0);
                sem_init(&closeT, 0, 0);
                p6 = sem_open(P6, O_CREAT, 0644, 0);
                p6close = sem_open(P6CLOSE, O_CREAT, 0644, 0);    

                pthread_create(&p5t1, NULL, thFuncion, &arg1);
                pthread_create(&p5t2, NULL, thFuncion, &arg2);
                pthread_create(&p5t3, NULL, thFuncion, &arg3);
                pthread_create(&p5t4, NULL, thFuncion, &arg4);

                if(fork()==0){
                    info(BEGIN, 6, 0);

                    pthread_t p6t1, p6t2, p6t3, p6t4, p6t5;

                    int arg1 = 1;
                    int arg2 = 2;
                    int arg3 = 3;
                    int arg4 = 4;
                    int arg5 = 5;

                    //sem_init(&p6, 0, 0);
                    // sem_init(&closeT, 0, 0);
                    

                    pthread_create(&p6t1, NULL, otherFunction, &arg1);
                    pthread_create(&p6t2, NULL, otherFunction, &arg2);
                    pthread_create(&p6t3, NULL, otherFunction, &arg3);
                    pthread_create(&p6t4, NULL, otherFunction, &arg4);
                    pthread_create(&p6t5, NULL, otherFunction, &arg5);


                    pthread_join(p6t1, NULL);
                    pthread_join(p6t2, NULL);
                    pthread_join(p6t3, NULL);
                    pthread_join(p6t4, NULL);
                    pthread_join(p6t5, NULL);

                    info(END, 6, 0);
                }
                else{
                    wait(NULL);
                    pthread_join(p5t1, NULL);
                    pthread_join(p5t2, NULL);
                    pthread_join(p5t3, NULL);
                    pthread_join(p5t4, NULL);
                    info(END, 5, 0);
                }
                sem_destroy(&ex2);
                sem_destroy(&closeT);
                sem_unlink(P6);
                sem_unlink(P6CLOSE);

            }
            else{
                wait(NULL);
                info(END, 3, 0);
            }
        }
        else{
            if(fork()==0){
                info(BEGIN, 7, 0);

                if(fork()==0){
                    info(BEGIN, 8, 0);
                    info(END, 8, 0);
                }
                else{
                    wait(NULL);
                    info(END, 7, 0);
                }
            }
            else{

            pthread_t threads[39];
            int vals[39];
            sem_init(&max5, 0, 5);
            sem_init(&regulate, 0, 4);
            sem_init(&block, 0, 0);

            for(int i = 1;i<=39;i++){
                vals[i-1]=i;
                pthread_create(&threads[i-1], NULL, lastFunction, &vals[i-1]);
            }

            for(int i = 0; i<39; i++){
                pthread_join(threads[i], NULL);
            }

            sem_destroy(&block);
            sem_destroy(&regulate);
            sem_destroy(&max5);

            wait(NULL);
            wait(NULL);
            info(END, 2, 0);  
            }  
        }
    }
    else{
        if(fork()==0){
            info(BEGIN, 4, 0);
            info(END, 4, 0);
        }
        else{
        wait(NULL);
        wait(NULL);
        info(END, 1, 0);  
        }  
    }
    
    return 0;
}
