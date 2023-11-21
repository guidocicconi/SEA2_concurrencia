#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_LENGTH 8
#define TOTAL_DATA 1000
#define CANT_PRODUCERS 2
#define CANT_CONSUMERS 2

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

char producers_active = 1;

void buffer_put(unsigned long value){
    data_buffer.buffer[data_buffer.in_index] = value;
    data_buffer.in_index = (data_buffer.in_index+1) % BUFFER_LENGTH;
    data_buffer.cant_elem++;
}

unsigned long buffer_get(void){
    unsigned long ret;
    ret = data_buffer.buffer[data_buffer.out_index];
    data_buffer.out_index = (data_buffer.out_index+1) % BUFFER_LENGTH;
    data_buffer.cant_elem--;
    return ret;
}

void* producer(void* arg){
    int iloop;
    
    for (iloop = 0; iloop < TOTAL_DATA; iloop++){
        while (data_buffer.cant_elem >= BUFFER_LENGTH){}   
        buffer_put(iloop+1);
    }

    pthread_exit(NULL);
}

void* consumer(void* arg){
    int iloop;
    unsigned long *sum = (unsigned long *)arg;
    *sum = 0;
    
    while (producers_active){
        if(data_buffer.cant_elem > 0){
            *sum += buffer_get();
        }
    }

    pthread_exit(NULL);
}

int main(void){

    int iloop = 0;
    unsigned long sum = 0;
    unsigned long sum_c[CANT_CONSUMERS] = {0};
    unsigned long sum_c_total = 0;
    pthread_t tid_p[CANT_PRODUCERS], tid_c[CANT_CONSUMERS];

    for(iloop = 0; iloop<CANT_PRODUCERS; iloop++){
        pthread_create(tid_p+iloop, NULL, producer, NULL);
    }

    for(iloop = 0; iloop<CANT_CONSUMERS; iloop++){
        pthread_create(tid_c+iloop, NULL, consumer, (void*)(sum_c+iloop));
    }

    for(iloop = 0; iloop<CANT_PRODUCERS; iloop++){
        pthread_join(tid_p[iloop], NULL);
    }

    producers_active = 0;

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