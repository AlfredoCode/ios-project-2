#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
//THESE TWO BELOW ARE USED FOR FORK()
#include <sys/types.h>
#include <unistd.h>
//THIS BELOW IS USED FOR WAIT()
#include <sys/wait.h>

#include <fcntl.h>
#define ARGCOUNT 5
#define MAXTIME 1000

FILE *fp;

int idO = 0,
    counter = 1;
sem_t
    *oxy,
    *hydro,
    *mutex,
    *bar_mutex,
    *bar_mutex2,
    *writing,
    *barrier1,
    *barrier2,
    *barrier3;
    

void proc_valid(int fileDesc){
    if (fileDesc == -1){
        perror("Process creation failed");
        //clean();
        exit(1);
    }
}

void param_cnt_msg(){
    fprintf(stderr,"Invalid count of arguments!\n");
    fprintf(stderr,"Correct usage is ./proj2 NO NH TI TB\n");
    exit(1);
}

void param_msg(){
    fprintf(stderr,"Invalid usage of arguments! Arguments have to be Integer > 0\n");
    exit(1);
}

void errMaxTime(){
    fprintf(stderr,"Max time can be in range <0;1000>!\n");
    exit(1);
}
void errFile(){
    fprintf(stderr,"File could not be opened!\n");
    exit(1);   
}
void oxy_proc(){
    
    printf("%d: O %d: started\n", counter, idO);
    exit(0);
}
void oxy_create(int cnt){
    for(int i=1; i <= cnt; i++){
         pid_t oxy = fork();
         idO++;
         if(oxy == 0){
             oxy_proc();
         }
       
    }
        exit(0);
}

int main(int argc, char **argv){
    int NO, NH, TI, TB;
    
    if (argc != ARGCOUNT){
        param_cnt_msg();  
    }
    for(int i = 1;i < ARGCOUNT; i++){
        char *err = NULL;
        switch(i){
            case 1:
                NO = strtol(argv[i], &err, 10);
                break;
            case 2:
                NH = strtol(argv[i], &err, 10);
                break;
            case 3:
                TI = strtol(argv[i], &err, 10);
                if(TI < 0 || TI > MAXTIME){
                    errMaxTime();
                }
                break; 
            case 4:
                TB = strtol(argv[i], &err, 10);
                if(TB < 0 || TB > MAXTIME){
                    errMaxTime();
                } 
                break;
        }
        if(*err != 0){
            param_msg();
        }
    }
    printf("%d\n",NO);
    fp = fopen("proj2.out", "w");
        if(fp == NULL){
            errFile();
    }
    pid_t oxy_pid, hydro_pid, main_pid;

    main_pid = fork();
    proc_valid(main_pid);

    if(main_pid != 0){
        oxy_create(NO);
        
    }

    
    fclose(fp);
    return 0;
}