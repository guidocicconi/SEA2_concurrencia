#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_LENGTH 8
#define TOTAL_DATA 1000
#define CANT_PRODUCERS 2
#define CANT_CONSUMERS 5

typedef struct
{
    int buffer[BUFFER_LENGTH];
    int in_index;
    int out_index;
    int cant_elem;
} data_buffer_t;

data_buffer_t data_buffer = {
    .buffer = {0},
    .in_index = 0,
    .out_index = 0,
    .cant_elem = 0
};

int data_consumed = CANT_PRODUCERS*TOTAL_DATA;
pthread_mutex_t buffer_mutex, consumed_mutex;
sem_t buffer_put_sem, buffer_get_sem;
pthread_t tid_p[CANT_PRODUCERS], tid_c[CANT_CONSUMERS];

void buffer_put(unsigned long value){
    pthread_mutex_lock(&buffer_mutex);
    data_buffer.buffer[data_buffer.in_index] = value;
    data_buffer.in_index = (data_buffer.in_index+1) % BUFFER_LENGTH;
    data_buffer.cant_elem++;
    pthread_mutex_unlock(&buffer_mutex);
}

unsigned long buffer_get(void){
    unsigned long ret;
    pthread_mutex_lock(&buffer_mutex);
    ret = data_buffer.buffer[data_buffer.out_index];
    data_buffer.out_index = (data_buffer.out_index+1) % BUFFER_LENGTH;
    data_buffer.cant_elem--;
    pthread_mutex_unlock(&buffer_mutex);
    return ret;
}

void* producer(void* arg){
    int iloop;
    
    for (iloop = 0; iloop < TOTAL_DATA; iloop++){
        sem_wait(&buffer_put_sem);
        buffer_put(iloop+1);
        sem_post(&buffer_get_sem);
    }

    pthread_exit(NULL);
}

void* consumer(void* arg){
    int iloop=0;
    unsigned long *sum = (unsigned long *)arg;
    *sum = 0;
    
    while (data_consumed){
        sem_wait(&buffer_get_sem);
        if(data_consumed){
            *sum += buffer_get();
            sem_post(&buffer_put_sem);

            pthread_mutex_lock(&consumed_mutex);
            data_consumed--;
            pthread_mutex_unlock(&consumed_mutex);
        }
    }  

    sem_post(&buffer_get_sem);

    pthread_exit(NULL);
}

int main(void){

    int iloop = 0;
    unsigned long sum = 0;
    unsigned long sum_c[CANT_CONSUMERS] = {0};
    unsigned long sum_c_total = 0;

    pthread_mutex_init(&buffer_mutex, NULL);
    pthread_mutex_init(&consumed_mutex, NULL);
    sem_init(&buffer_get_sem, 0, 0);
    sem_init(&buffer_put_sem, 0, BUFFER_LENGTH);

    for(iloop = 0; iloop<CANT_PRODUCERS; iloop++){
        pthread_create(tid_p+iloop, NULL, producer, NULL);
    }

    for(iloop = 0; iloop<CANT_CONSUMERS; iloop++){
        pthread_create(tid_c+iloop, NULL, consumer, (void*)(sum_c+iloop));
    }

    for(iloop = 0; iloop<CANT_PRODUCERS; iloop++){
        pthread_join(tid_p[iloop], NULL);
    }

    for(iloop = 0; iloop<CANT_CONSUMERS; iloop++){
        pthread_join(tid_c[iloop], NULL);
    }

    for (iloop = 0; iloop<TOTAL_DATA; iloop++){
        sum += iloop+1;
    }
    sum *= CANT_PRODUCERS;

    for (iloop = 0; iloop<CANT_CONSUMERS; iloop++){
        sum_c_total += sum_c[iloop];
    }

    printf("La suma deber%ca dar: %d\n", 161, sum);
    printf("La suma da: %d\n", sum_c_total);

    return 0;
}