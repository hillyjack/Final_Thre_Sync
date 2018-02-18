//
// Created by ubuntu on 12/02/18.
//

#ifndef FINAL_THRE_SYNC_INCLUDES_H
#define FINAL_THRE_SYNC_INCLUDES_H


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
#include <memory.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/prctl.h>


#define MAXBUF 50
#define PORT 2000
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

extern struct SHM_data *virt_addr;
extern int md;
extern long pg_size;
extern int sockfd, pyfd;
extern FILE *fp;

struct Data{
    //struct SHM_data * current_SHM;
    int sec;
    int lowval;
    int highval;
};

#endif //FINAL_THRE_SYNC_INCLUDES_H
