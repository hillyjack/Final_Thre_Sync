//
// Created by ubuntu on 12/02/18.
//

#include "consumer.h"

void createSolver(struct SHM_data *current_SHM) {
    int pid = fork();

    if (pid == 0) {

        //set signal block
        sigset_t sig_block;
        sigemptyset(&sig_block);
        sigaddset(&sig_block, SIGINT);
        sigaddset(&sig_block, SIGTSTP);
        pthread_sigmask(SIG_BLOCK,&sig_block,NULL);

        //stop running when parent proc ends
        prctl(PR_SET_PDEATHSIG, SIGHUP);

        double res = 0;
        struct SHM_data *virt_addr = current_SHM;
        //printf("%d\n",virt_addr );

        while (1) {
            sem_wait(&virt_addr->sem2);
            pthread_mutex_lock(&virt_addr->mx1);

            struct Question corrent_Q = (struct Question)virt_addr->arr[virt_addr->bottom];

            //printf("SOLVERnum1 - %f, SOLVERnum2 - %f\n",corrent_Q.num1, virt_addr->arr[virt_addr->bottom].num2 );
            //printf("SOLVERoper- %d\n", corrent_Q.op);

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
            printf("result= %.2f\n", res);
            memset(&corrent_Q, 0, sizeof(struct Question));

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
