#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define MAXBUF 256

#define NUM_OF_Q 10
#define SHARED_MEMORY "/my_memory"

enum oper{PLUS = 0 , MIN, MUL, DIV};

struct Question{
    double num1;
    double num2;
    enum oper op;
};

struct SHM_data{
    sem_t sem1, sem2;
    pthread_mutex_t mx1;
    struct Question arr[10];
    int bottom;
    int top;
};

struct Data{
    struct SHM_data * current_SHM;
    int sec;
    int lowval;
    int highval;
};

void createProducer(void *p) {

    int pid = fork();

    if (pid == 0) {

        time_t t;
        srand((unsigned) time(&t) % getpid());

        struct Data *d = (struct Data *) p;
        struct SHM_data *virt_addr = d->current_SHM;
        printf("%d\n",virt_addr );

        int n = d->sec;
        int lowval = d->lowval;
        int highval = d->highval;
        int num1, num2;

        printf("lowval - %d, highval - %d\n",lowval ,highval );

        while (1) {

            struct Question Q;
            enum oper op = (enum oper)(rand() % 4 + 1);
            printf("PRODoper0- %d\n", op);
            num1 = (double)(rand() % (highval - lowval) + lowval);
            num2 = (double)(rand() % (highval - lowval) + lowval);
            Q.num1 = num1;
            Q.num2 = num2;
            Q.op = op;

            printf("PRODnum1 - %f, PRODnumf - %f\n", Q.num1 ,Q.num2 );
            printf("PRODoper1- %d\n", Q.op);

            sem_wait(&virt_addr->sem1);
            pthread_mutex_lock(&virt_addr->mx1);

            virt_addr->arr[virt_addr->top] = Q;
            if (virt_addr->top == 9) {
                virt_addr->top = 0;
            } else { virt_addr->top++;}

            pthread_mutex_unlock(&virt_addr->mx1);
            sem_post(&virt_addr->sem2);

            sleep(n);
        }
    }
    else{}
}


void createPythonProducer() {

    int pid = fork();

    if (pid == 0) {
        //create PIPE
        char buf[MAXBUF] = "";
        FILE *fp = popen("python ./py_client.py", "r");

        //create sucket
        char buffer[MAXBUF];
        struct sockaddr_in addr;
        struct sockaddr_in client;
        int addrlen, n;
        int sockfd, pyfd;
        //struct sigaction act;

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("sucket sockfd - %s\n", strerror(errno));
            exit(1);
        }
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(2000);// up 1024
        addr.sin_addr.s_addr = INADDR_ANY;//htnl(INADDR_LOOPBACK)
        n = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
        if (n < 0) {
            printf("sucket bind - %s\n", strerror(errno));
            exit(1);
        }
        listen(sockfd, 1);

        /* Accept connection and deal with request */
        memset(&client, 0, sizeof(client));
        addrlen = sizeof(client);
        pyfd = accept(sockfd, (struct sockaddr *) &client, &addrlen);
        if (pyfd < 0) {
            //if (errno == EINTR)         /* Probably a child dying */
            printf("sucket python fd - %s\n", strerror(errno));
            //perror("accept");
            exit(1);                      /* A real problem */
        }


        ///////////////////////////need to change this code
        n = read(pyfd, buffer, MAXBUF);
        buffer[n] = '\0';
        printf("Message; %s\n", buffer);
        /* Could send a reply here if required */


        //read PIPE
        while(1){
            fread(buf, MAXBUF, 1, fp);
            puts(buf);
        }

    } else {}
}


void createSolver(struct SHM_data *current_SHM) {
    int pid = fork();

    if (pid == 0) {
        double res = 0;

        struct SHM_data *virt_addr = current_SHM;
        printf("%d\n",virt_addr );

        while (1) {
            sem_wait(&virt_addr->sem2);
            pthread_mutex_lock(&virt_addr->mx1);

            struct Question corrent_Q = (struct Question)virt_addr->arr[virt_addr->bottom];

            printf("SOLVERnum1 - %f, SOLVERnum2 - %f\n",corrent_Q.num1, virt_addr->arr[virt_addr->bottom].num2 );
            printf("SOLVERoper- %d\n", corrent_Q.op);

            if (corrent_Q.op == PLUS) {
                res = (double) corrent_Q.num1 + corrent_Q.num2;

            } else if (corrent_Q.op == MIN) {
                res = (double) corrent_Q.num1 - corrent_Q.num2;
            } else if (corrent_Q.op == MUL) {
                res = (double) corrent_Q.num1 * corrent_Q.num2;
            } else {
                if (!corrent_Q.num2) { res = (double) corrent_Q.num1 / 1; }

                res = (double) corrent_Q.num1 / corrent_Q.num2;
            }
            printf("---------------------------result= %.2f\n", res);
            memset(&corrent_Q, NULL, sizeof(struct Question));

            if (virt_addr->bottom == 9) {
                virt_addr->bottom = 0;
            } else { virt_addr->bottom++; }

            pthread_mutex_unlock(&virt_addr->mx1);
            sem_post(&virt_addr->sem1);

            sleep(3);
            //break;
        }
    }
}


int main(int argc, char * argv[]);

int main(int argc, char * argv[]) {

    printf("%s\n",argv[2]);
    printf("%s\n",argv[4]);

    int i, md, status, pid;
    long pg_size;
    struct SHM_data *virt_addr;
    time_t t;
    srand((unsigned) time(&t));

    int numOfProducers = 2;
    int numOfConsumers = 1;

    /* Create shared memory object */
    md = shm_open(SHARED_MEMORY, O_TRUNC | O_CREAT | O_RDWR, 0640);
    if (md == -1) {
        printf("smh_open error - %s\n", strerror(errno));
    }
    ftruncate(md, sizeof(struct SHM_data));
    virt_addr = (struct SHM_data *) mmap(0, sizeof(struct SHM_data), PROT_WRITE | PROT_READ, MAP_SHARED, md, 0);
    if ((int) virt_addr == -1) {
        printf("mmap error- %s\n", strerror(errno));
    }
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