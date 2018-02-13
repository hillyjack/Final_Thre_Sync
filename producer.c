//
// Created by ubuntu on 11/02/18.
//

#include "producer.h"

void createProducer(void *p) {

    int pid = fork();

    if (pid == 0) {

        time_t t;
        srand((unsigned) time(&t) % getpid());

        struct Data *d = (struct Data *) p;
        struct SHM_data *virt_addr = d->current_SHM;
        //printf("%d\n",virt_addr );

        int n = d->sec;
        int lowval = d->lowval;
        int highval = d->highval;
        int num1, num2;

        //printf("lowval - %d, highval - %d\n",lowval ,highval );

        while (1) {


            enum oper op = (enum oper)(rand() % 4 + 1);
            //printf("PRODoper0- %d\n", op);
            num1 = (double)(rand() % (highval - lowval) + lowval);
            num2 = (double)(rand() % (highval - lowval) + lowval);

            ////////reused code
            struct Question Q;
            Q.num1 = num1;
            Q.num2 = num2;
            Q.op = op;

            //printf("PRODnum1 - %f, PRODnumf - %f\n", Q.num1 ,Q.num2 );
            //printf("PRODoper1- %d\n", Q.op);

            sem_wait(&virt_addr->sem1);
            pthread_mutex_lock(&virt_addr->mx1);

            virt_addr->arr[virt_addr->top] = Q;
            if (virt_addr->top == 9) {
                virt_addr->top = 0;
            } else { virt_addr->top++;}

            pthread_mutex_unlock(&virt_addr->mx1);
            sem_post(&virt_addr->sem2);
            ////////////
            sleep(n);
        }
    }
    else{}
}


void createPythonProducer(struct SHM_data *current_SHM) {

    int pid = fork();

    if (pid == 0) {
        struct SHM_data *virt_addr = current_SHM;

        int py_id  = 0;
        char * files_list;

        //create socket
        struct sockaddr_in addr;
        struct sockaddr_in client;
        int addrlen, n = 1;
        int sockfd, pyfd;

        printf("*********starting socket*********\n");
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("socket sockfd - %s\n", strerror(errno));
            close (sockfd);
            exit(1);
        }
        printf("*********memset socket*********\n");
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);// up 1024
        addr.sin_addr.s_addr = INADDR_ANY;//htnl(INADDR_LOOPBACK)
        n = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
        if (n < 0) {
            printf("socket bind - %s\n", strerror(errno));
            close (sockfd);
            exit(1);
        }
        printf("*********I'm listening*********\n");
        listen(sockfd, 1);


        //create PIPE
        char buffer[MAXBUF] = "";
        char buf[MAXBUF] = "";
        printf("*********starting popen*********\n");
        FILE *fp = popen("python /home/ubuntu/CLionProjects/Final_Thre_Sync/py_client.py", "r");


        /* Accept connection and deal with request */
        memset(&client, 0, sizeof(client));
        addrlen = sizeof(client);
        pyfd = accept(sockfd, (struct sockaddr *) &client, &addrlen);
        if (pyfd < 0) {
            printf("sucket python fd - %s\n", strerror(errno));
            close (sockfd);
            exit(1);
        }


        files_list = "question_1.txt,question_2.txt,question_3.txt,question_4.txt,question_5.txt";

        if (send(pyfd,files_list,strlen(files_list),0) <0)
            printf ("error\n");
        else
            printf ("packet send done\n");

        printf("*********list was sent*********\n");
        //sleep(10);
        close (sockfd);
        //read PIPE

        while(1){
            //fread(buf, 1, 1, fp);
            //len = buf[0];
            n = 0;
            int i = 0;

            n = fread(buf, 1,1, fp);

            while (buf[0] != '\0' && buf[0] != EOF && buf[0] !='\n' )
            {
                //n += fread(buf+n, 1,1, fp);
                buffer[i] = buf[0];
                i++;

                n = fread(buf, 1,1, fp);
            }
            buffer[i] = '\0';
            //////////////////////////////////////////
            printf("%s\n", buffer);

            struct Question Q = {0};
            printf("---------------memset-----------------\n");

            /////////Handel with Null
            Q.num1 = atof(strtok(buffer," "));
            Q.num2 = atof(strtok(NULL," "));
            printf("PRODnum1 - %f, PRODnumf - %f\n", Q.num1 ,Q.num2 );
            Q.op = (enum oper)atoi(strtok(NULL," "));
            printf("---------------Question loaded-----------------\n");

            printf("PRODoper1- %d\n", Q.op);

            sem_wait(&virt_addr->sem1);
            pthread_mutex_lock(&virt_addr->mx1);

            virt_addr->arr[virt_addr->top] = Q;
            if (virt_addr->top == 9) {
                virt_addr->top = 0;
            } else { virt_addr->top++;}

            pthread_mutex_unlock(&virt_addr->mx1);
            sem_post(&virt_addr->sem2);
            sleep(5);
        }


    } else {}
}