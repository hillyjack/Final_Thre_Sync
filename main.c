#include "includes.h"
#include "producer.h"
#include "consumer.h"

int main(int argc, char * argv[]) {

    //printf("%s\n",argv[2]);
    //printf("%s\n",argv[4]);
    int numOfProducers = 2;
    int numOfConsumers = 1;

    int i, md, status, pid;
    long pg_size;
    struct SHM_data *virt_addr;
    time_t t;
    srand((unsigned) time(&t));


    /* Create shared memory object */
    md = shm_open(SHARED_MEMORY, O_TRUNC | O_CREAT | O_RDWR, 0640);
    if (md == -1) {printf("smh_open error - %s\n", strerror(errno));}
    ftruncate(md, sizeof(struct SHM_data));
    virt_addr = (struct SHM_data *) mmap(0, sizeof(struct SHM_data), PROT_WRITE | PROT_READ, MAP_SHARED, md, 0);
    if ((int) virt_addr == -1) {printf("mmap error- %s\n", strerror(errno));}
    printf("%d\n", virt_addr);

    memset(&virt_addr->arr, 0, NUM_OF_Q * sizeof(struct Question));
    virt_addr->top = 0;
    virt_addr->bottom = 0;

    //initialize mutex
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&virt_addr->mx1, &mutexattr);

    //initialize semaphore
    sem_init(&virt_addr->sem1, 1, 10);
    sem_init(&virt_addr->sem2, 1, 0);

    //create regular producers
    for (i = 0; i < numOfProducers-1; ++i) {
        struct Data data1;
        data1.current_SHM = virt_addr;
        data1.sec = 2;
        data1.lowval = 5;
        data1.highval = 100;
        createProducer(&data1);
    }
    //create python producer
    createPythonProducer();

    //create Consumers
    for (i = 0; i < numOfConsumers; ++i) {
        createSolver(virt_addr);
    }

    wait(&status);
    //pid = waitpid(-1, &status, 0);
    status = munmap(virt_addr, pg_size);  /* Unmap the page */
    status = close(md);                   /*   Close file   */
    status = shm_unlink("my_memory");     /* Unlink shared-memory object */
    return 0;
}