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
#define ERR_FAIL 1
#define ERR_SUCC 0

FILE *fp;





int idO = 0;
int idH = 0;

sem_t
    *oxy,
    *hydro,
    *mutex,
    *log_write;
    
void sem_initialization(){
    
    if( (log_write = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(log_write, 1, 1) == -1){
        fprintf(stderr,"Error initializing semaphores!\n");
        exit(ERR_FAIL);
    }    
}
void proc_valid(int fileDesc){
    if (fileDesc == -1){
        fprintf(stderr,"Process creation failed");
        //clean();
        exit(ERR_FAIL);
    }
}

void param_cnt_msg(){
    fprintf(stderr,"Invalid count of arguments!\n");
    fprintf(stderr,"Correct usage is ./proj2 NO NH TI TB\n");
    exit(ERR_FAIL);
}

void param_msg(){
    fprintf(stderr,"Invalid usage of arguments! Arguments have to be Integer > 0\n");
    exit(ERR_FAIL);
}

void errMaxTime(){
    fprintf(stderr,"Max time can be in range <0;1000>!\n");
    exit(ERR_FAIL);
}
void errFile(){
    fprintf(stderr,"File could not be opened!\n");
    exit(ERR_FAIL);   
}

//TODO file_write func
void file_write(char* process, char* status){

}


void oxy_proc(int *add,int ti, int tb){
    sem_wait(log_write);
        (*add)++;
        fprintf(fp,"%d: O %d: started\n", *add, idO);
        //fflush(fp);
    sem_post(log_write);
    sem_wait(log_write);
        (*add)++;
        usleep(rand() % ti);
        fprintf(fp,"%d: O %d: going to queue\n", *add, idO);
    sem_post(log_write);
    fclose(fp); //Valgrind 
    exit(ERR_SUCC);
}
void oxy_create(int cnt, int* add,int ti, int tb){
    for(int i=1; i <= cnt; i++){
        //printf("%d\n",*add);
        idO++;
         pid_t oxygen = fork();
         
         if(oxygen == 0){
             oxy_proc(add,ti,tb);
             
             
         }
       
    }
    while(wait(NULL)>0);
    //exit(ERR_SUCC);
    
}
void hydro_proc(int *add){
    sem_wait(log_write);
        (*add)++;
        fprintf(fp,"%d: H %d: started\n", *add, idH);
        //fflush(fp);
    sem_post(log_write);
    fclose(fp);//Valgrind
    exit(ERR_SUCC);
}

void hydro_create(int cnt, int* add){
    for(int i=1; i <= cnt; i++){
        //printf("%d\n",*add);
        idH++;
         pid_t hydro = fork();
         
         if(hydro == 0){
             hydro_proc(add);
             
         }
       
    }
    while(wait(NULL)>0);
    //exit(ERR_SUCC);
    
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

    sem_initialization();


    int cnt_shared = shm_open("xhofma11", O_RDWR|O_CREAT, 0666);
        if (cnt_shared<0) { 
            fprintf(stderr,"shm_open\n"); 
            exit(ERR_FAIL); 
        }
    ftruncate(cnt_shared, sizeof(int));
    int* counter = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED, cnt_shared, 0);

    if (counter==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(cnt_shared);
        shm_unlink("xhofma11");
        exit(ERR_FAIL); 
        }
    fp = fopen("proj2.out", "w");
        if(fp == NULL){
            errFile();
    }
    setbuf(fp, NULL);
    pid_t oxy_pid, hydro_pid, main_pid;

    // main_pid = fork();
    // proc_valid(main_pid);

    // if(main_pid == 0){
        oxy_create(NO, counter, TI, TB);
        hydro_create(NH, counter); 
         //while(wait(NULL) > 0); // Parent waits till children kill themself
        //  exit(0);
    // }
   
   
    //wait(NULL); // Waits until child is dead
    close(cnt_shared);
    shm_unlink("xhofma11");
    //printf("Finishing\n");
    fclose(fp);
    return 0;
}