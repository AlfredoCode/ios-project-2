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
int *idO_cnt;
int *idH_cnt;
int idO_shared;
int idH_shared;
// int molecule = 0;

sem_t
    *oxy,
    *hydro,
    *mutex,
    *log_write;
    
void sem_initialization(){
    idO_shared = shm_open("idO", O_RDWR|O_CREAT, 0666);
    if(idO_shared < 1){
        perror("error");
    }
    ftruncate(idO_shared, sizeof(int));
    idO_cnt = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, idO_shared, 0);
    if (idO_cnt==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(idO_shared);
        shm_unlink("idO");
        exit(ERR_FAIL); 
    }
    idH_shared = shm_open("idH", O_RDWR|O_CREAT, 0666);
    if(idH_shared < 1){
        perror("error");
    }
    ftruncate(idH_shared, sizeof(int));
    idH_cnt = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, idH_shared, 0);
    if (idH_cnt==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(idH_shared);
        shm_unlink("idH");
        exit(ERR_FAIL); 
    }
    
    // ftruncate(mole_shared, sizeof(int));
    if( (log_write = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(log_write, 1, 1) == -1 || 
    (oxy = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(oxy, 1, 0) == -1 || 
    (mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(mutex, 1, 1) == -1){
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
        (*idO_cnt)++;
        fprintf(fp,"%d: O %d: started\n", *add, idO);
        //fflush(fp);
    sem_post(log_write);
    sem_wait(mutex); 
    sem_wait(log_write);
        (*add)++;
        usleep((rand() % (ti+1))*1000);
        fprintf(fp,"%d: O %d: going to queue\n", *add, idO);

    sem_post(log_write);
   if((*idH_cnt) >= 2){
       sem_post(oxy);
   }
   else{
       sem_post(mutex);
       exit(ERR_SUCC);
   }
    sem_wait(oxy);
    (*add)++;
    fprintf(fp,"%d: O %d: creating molecule\n",*add, idO);
    //usleep((rand() % (tb+1))*1000);   TODO FIX THIS MF TEST.PY SLEEP
    sem_post(mutex);    
    fclose(fp); //Valgrind 
    exit(ERR_SUCC);
}
void oxy_create(int cnt, int* add,int ti, int tb){


    for(int i=1; i <= cnt; i++){
        //printf("%d\n",*add);
  

        pid_t oxygen = fork();
        idO++;
        if(oxygen == 0){  
            printf("forked\n");         
            oxy_proc(add,ti,tb);
            
        }       
    }
    
    //while(wait(NULL)>0);
    
    
    //exit(ERR_SUCC);
    
}
void hydro_proc(int *add, int ti, int tb){
    sem_wait(log_write);
        (*add)++;
        (*idH_cnt)++;
        
        fprintf(fp,"%d: H %d: started\n", *add, idH);
        //fflush(fp);
    sem_post(log_write);
    sem_wait(mutex);
    sem_wait(log_write);
        (*add)++;
        usleep((rand() % (ti+1))*1000);
        fprintf(fp,"%d: H %d: going to queue\n", *add, idH);
    sem_post(log_write);
    //sem_post(oxy);
    //fclose(fp);//Valgrind
    
    sem_post(mutex);
    exit(ERR_SUCC);
}

void hydro_create(int cnt, int* add,int ti, int tb){
    for(int i=1; i <= cnt; i++){
        //printf("%d\n",*add);
        
        pid_t hydro = fork();
        idH++;
        if(hydro == 0){
            printf("forked\n"); 
            hydro_proc(add, ti, tb);
            
        }
     
    }
    //while(wait(NULL)>0);
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
    pid_t oxy_pid, hydro_pid;   
   
        oxy_create(NO, counter, TI, TB);
        //while(wait(NULL)>0);
        hydro_create(NH, counter, TI, TB); 
        while(wait(NULL)>0);

        

    //wait(NULL); // Waits until child is dead
    close(cnt_shared);
    shm_unlink("xhofma11");
    close(idO_shared);
    shm_unlink("idO");
    close(idH_shared);
    shm_unlink("idH");

    

    // shm_unlink("molecule");
    //printf("Finishing\n");
    fclose(fp);


    return 0;
}