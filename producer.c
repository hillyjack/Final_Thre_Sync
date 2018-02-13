//
// Created by ubuntu on 11/02/18.
//

#include "producer.h"

int sockfd, pyfd;

void createProducer(void *p) {

    int pid = fork();

    if (pid == 0) {

        time_t t;
        srand((unsigned) time(&t) % getpid());

        struct Data *d = (struct Data *) p;
        //struct SHM_data *virt_addr = d->current_SHM;
        printf("%d\n",virt_addr );

        int n = d->sec;
        int lowval = d->lowval;
        int highval = d->highval;
        double num1, num2;
        int op;

        //printf("lowval - %d, highval - %d\n",lowval ,highval );

        while (1) {

            op = rand() % 4;
            //printf("PRODoper0- %d\n", op);
            num1 = (double)((rand() % (highval - lowval)) + lowval);
            num2 = (double)((rand() % (highval - lowval)) + lowval);
            //printf("PRODnum1 - %f, PRODnumf - %f\n", num1 ,num2 );
            ////////reused code
            questionToShmArr(num1, num2, op);
            ////////////
            sleep(n);
        }
    }
    else{}
}

void questionToShmArr (double num1, double num2, int op ){
    struct Question Q;
    Q.num1 = num1;
    Q.num2 = num2;
    Q.op = (enum oper)op;

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
}

void createPythonProducer() {

    int pid = fork();

    if (pid == 0) {
        //struct SHM_data *virt_addr = current_SHM;

        int py_id  = 0;
        char * files_list;

        //create socket
        struct sockaddr_in addr;
        struct sockaddr_in client;
        int addrlen, n = 1;
        //int sockfd, pyfd;

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
        char py_id_str[MAXBUF] = "";
        printf("*********starting popen*********\n");

        fp = popen("python ../py_client.py", "r");

        /* Accept connection and deal with request */
        memset(&client, 0, sizeof(client));
        addrlen = sizeof(client);
        pyfd = accept(sockfd, (struct sockaddr *) &client, &addrlen);
        if (pyfd < 0) {
            printf("sucket python fd - %s\n", strerror(errno));
            close (sockfd);
            exit(1);
        }

        //recv pid from python
        n=read(pyfd, py_id_str, MAXBUF); py_id_str[n] = '\0';
        printf ("Message; %s\n", py_id_str);

        files_list = "question_1.txt,question_2.txt,question_3.txt,question_4.txt,question_5.txt";

        if (send(pyfd,files_list,strlen(files_list),0) <0)
            printf ("error\n");
        else
            printf ("packet send done\n");

        printf("*********list was sent*********\n");
        //sleep(10);
        close (sockfd);
        close (pyfd);

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

            printf("---------------memset-----------------\n");

            /////////Handel with Null
            double num1 = 0.0, num2 = 0.0;
            int op = 0;

            num1 = atof(strtok(buffer," "));
            num2 = atof(strtok(NULL," "));
            //printf("PRODnum1 - %f, PRODnumf - %f\n", Q.num1 ,Q.num2 );
            op = atoi(strtok(NULL," "));
            printf("PRODoper1- %d\n", op);

            printf("---------------Var Was Defiened-----------------\n");

            questionToShmArr(num1, num2, op);
            printf("---------------Question loaded-----------------\n");

            sleep(5);
        }


    } else {}
}