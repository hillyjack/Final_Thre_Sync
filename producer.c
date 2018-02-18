//
// Created by ubuntu on 11/02/18.
//

#include "producer.h"
char * files_list = NULL;

//char * dirPath = "../questions-files";     //IDE
char * dirPath = "./questions-files";      //CLI

int sockfd, pyfd;

void createProducer(void *p) {

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

        time_t t;
        srand((unsigned) time(&t) % getpid());

        struct Data *d = (struct Data *) p;
        int n = d->sec;
        int lowval = d->lowval;
        int highval = d->highval;
        double num1, num2;
        int op;

        while (1) {

            op = rand() % 4;
            num1 = (double)((rand() % (highval - lowval)) + lowval);
            num2 = (double)((rand() % (highval - lowval)) + lowval);
            questionToShmArr(num1, num2, op);
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

    sem_wait(&virt_addr->sem1);
    pthread_mutex_lock(&virt_addr->mx1);

    virt_addr->arr[virt_addr->top] = Q;
    if (virt_addr->top == 9) {
    virt_addr->top = 0;
    } else { virt_addr->top++;}

    pthread_mutex_unlock(&virt_addr->mx1);
    sem_post(&virt_addr->sem2);
}

void createFilesList(){
    DIR *dirObj;
    struct dirent *dirQF;
    if((dirObj = opendir(dirPath)) == NULL) {
        fprintf(stderr,"cannot open directory 'questions-files'");
        return;
    }
    while((dirQF = readdir(dirObj)) != NULL) {
        if(strcmp(dirQF->d_name + strlen(dirQF->d_name) - 4,".txt") == 0) {
            if (files_list == NULL) {
                files_list = malloc(strlen(dirQF->d_name));
                strcat(files_list, dirQF->d_name);
            }

            else{
                files_list = realloc(files_list, (strlen(files_list) + strlen(dirQF->d_name) + 1));
                strcat(files_list, ",");
                strcat(files_list, dirQF->d_name);
            }
        }
    }
    files_list = realloc(files_list,strlen(files_list) + 1);
    strcat(files_list, "\0");

    closedir(dirObj);
}

void createPythonProducer() {

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

        int d = 0;
        int py_id  = 0;

        //create socket
        struct sockaddr_in addr;
        struct sockaddr_in client;
        int addrlen, n = 1;

        //creating socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("socket sockfd - %s\n", strerror(errno));
            close (sockfd);
            exit(1);
        }
        //memset socket
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);// up 1024
        addr.sin_addr.s_addr = INADDR_ANY;//htnl(INADDR_LOOPBACK)

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) == -1)
        {
            printf("setsockopt(SO_REUSEADDR) failed. error: %s\n", strerror(errno));
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        n = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));

        if (n < 0) {
            printf("socket bind - %s\n", strerror(errno));
            close (sockfd);
            exit(1);
        }
        listen(sockfd, 1);

        //create PIPE
        char buffer[MAXBUF] = "";
        char buf[MAXBUF] = "";
        char py_id_str[MAXBUF] = "";

        //fp = popen("python ../py_client.py", "r");      //IDE
        fp = popen("python ./py_client.py", "r");      //CLI

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
        py_id = atoi(py_id_str);
        createFilesList();

        if (send(pyfd,files_list,strlen(files_list),0) <0)
            printf ("error\n");
        close (sockfd);
        close (pyfd);

        //read pipe

        while(1){
            n = 0;
            int i = 0;

            n = fread(buf, 1,1, fp);

            while (buf[0] != '\0' && buf[0] != EOF && buf[0] !='\n' )
            {
                buffer[i] = buf[0];
                i++;
                n = fread(buf, 1,1, fp);
            }
            buffer[i] = '\0';

            double num1 = 0.0, num2 = 0.0;
            int op = 0;

            num1 = atof(strtok(buffer," "));
            num2 = atof(strtok(NULL," "));
            op = atoi(strtok(NULL," "));

            questionToShmArr(num1, num2, op);

            //random signal sending
            d = rand()%5;
            if (d > 2)
            {
                d = 0;
                kill(py_id, SIGUSR1);
            }
            sleep(2);
        }
    }else {}
}
