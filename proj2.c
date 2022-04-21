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
// SHARED MEMORY
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


#define ARGCOUNT 5
#define MAXTIME 1000

FILE *fp;





int idO = 0;
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
void oxy_proc(int *add){
    (*add)++;
    fprintf(fp,"%d: O %d: started\n", *add, idO);
    fflush(fp);
    exit(0);
}
void oxy_create(int cnt, int* add){
    for(int i=1; i <= cnt; i++){
        //printf("%d\n",*add);
        idO++;
         pid_t oxy = fork();
         
         if(oxy == 0){
             oxy_proc(add);
             
         }
       
    }
    while(wait(NULL)>0);
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

    if(NO < 0 || NH < 0 || TI < 0 || TB < 0){
        param_msg();
    }
    int oxy_shared = shm_open("xhofma11", O_RDWR|O_CREAT, 0666);
        if (oxy_shared<0) { perror("shm_open"); exit(EXIT_FAILURE); }
    ftruncate(oxy_shared, sizeof(int));
    int* counter = mmap(NULL, sizeof(int*), PROT_READ|PROT_WRITE, MAP_SHARED, 
                oxy_shared, 0);

    if (counter==MAP_FAILED) { perror("mmap"); exit(EXIT_FAILURE); };

    fp = fopen("proj2.out", "w");
        if(fp == NULL){
            errFile();
    }
    setbuf(fp, NULL);
    pid_t oxy_pid, hydro_pid, main_pid;

    main_pid = fork();
    proc_valid(main_pid);

    if(main_pid == 0){
        oxy_create(NO, counter);
        while(wait(NULL) > 0); // Parent waits till children kill themself
        exit(0); // Kill parent
        
    }
    wait(NULL); // Waits until child is dead
    close(oxy_shared);
    shm_unlink("xhofma11");
    //printf("Finishing\n");
    fclose(fp);
    return 0;
}