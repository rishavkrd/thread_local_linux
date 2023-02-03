// A special flag for mmap() to allocate thread-local pages

#define MAP_THREADLOCAL 0x10000000
#define SYS_copy_pte 326
#define SYS_change_pte_prot 327
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/mman.h>
#include <pthread.h>

void * thread_local_page = NULL;


#define COUNTING 1000

void * access_memory(void * param)

{
    printf("thread %lu started\n", (unsigned long) param);
    for(int i=0;i<COUNTING;i++){
        (*((int *)thread_local_page))++;
        //printf("thread %lu counted to %d\n", (unsigned long) param, *(int *)thread_local_page);
    } 
    // for(int n = 0; n < 1000; n++) {
    //     *(int *)thread_local_page++;
    // }
    // printf("thread %lu counted to %d\n", (unsigned long) param, *(int *)thread_local_page);
    printf("thread %lu counted to %d\n", (unsigned long) param, *(int *)thread_local_page);
    return NULL;

}


int main(int argc, const char ** argv)

{

    pthread_t thread1, thread2;
    thread_local_page = (int *) mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_THREADLOCAL, -1, 0);
    //thread_local_page=(int *)malloc(sizeof(int *));
    //*(int *)thread_local_page=0;
    if (!thread_local_page) {
        printf("mmap() failed");
        return -1;
    }
    int res = 268435456;
    //printf("MAP_THREADLOCAL 0x10000000: %x, flag: %x\n",MAP_THREADLOCAL,MAP_ANON|MAP_PRIVATE|MAP_THREADLOCAL);
    //if (res & MAP_THREADLOCAL) printf("MAP_THREADLOCAL & res is true \n");
    pthread_create(&thread1, NULL, &access_memory, (void*) 1);
    pthread_create(&thread2, NULL, &access_memory, (void*) 2);


    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    printf("final count: %d\n",*(int *)thread_local_page);

    return 0;

}