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



int NO, NH, TI, TB;
int idO = 0;
int idH = 0;
int expectedMolecules;

int *hydro_tmp;
int *oxy_tmp;

int *idO_cnt;
int *idH_cnt;
int *counter;
int *molecule;
int *bar_cnt;

int hydro_tmp_shared;
int oxy_tmp_shared;
int idO_shared;
int idH_shared;
int cnt_shared;
int molecule_shared;
int bar_cnt_shared;

sem_t
    *enough,
    *oxy,
    *hydro,
    *mutex,
    *log_write,
    *bar_mutex,
    *bar_1,
    *bar_2;

void clean(){
    close(cnt_shared);
    shm_unlink("xhofma11_pid");
    munmap(counter, sizeof(sem_t));
    
    close(idO_shared);
    shm_unlink("xhofma11_idO");
    munmap(idO_cnt, sizeof(sem_t));

    close(idH_shared);
    shm_unlink("xhofma11_idH");
    munmap(idH_cnt, sizeof(sem_t));

    close(molecule_shared);
    shm_unlink("xhofma11_moleculeID");
    munmap(molecule, sizeof(sem_t));

    close(bar_cnt_shared);
    shm_unlink("xhofma11_bar_cnt");
    munmap(bar_cnt, sizeof(sem_t));

    close(hydro_tmp_shared);
    shm_unlink("xhofma11_hydro_tmp");
    munmap(bar_cnt, sizeof(sem_t));

    //TODO DESTROY ALL SEMAPHORES
}



void sem_initialization(){
    idO_shared = shm_open("xhofma11_idO", O_RDWR|O_CREAT, 0666);
    if(idO_shared < 1){
        fprintf(stderr,"error");
    }
    ftruncate(idO_shared, sizeof(int));
    idO_cnt = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, idO_shared, 0);
    if (idO_cnt==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(idO_shared);
        shm_unlink("xhofma11_idO");
        exit(ERR_FAIL); 
    }

    idH_shared = shm_open("xhofma11_idH", O_RDWR|O_CREAT, 0666);
    if(idH_shared < 1){
        fprintf(stderr,"error");
    }
    ftruncate(idH_shared, sizeof(int));
    idH_cnt = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, idH_shared, 0);
    if (idH_cnt==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(idH_shared);
        shm_unlink("xhofma11_idH");
        exit(ERR_FAIL); 
    }

    hydro_tmp_shared = shm_open("xhofma11_hydro_tmp", O_RDWR|O_CREAT, 0666);
    if(hydro_tmp_shared < 1){
        fprintf(stderr,"error");
    }
    ftruncate(hydro_tmp_shared, sizeof(int));
    hydro_tmp = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, hydro_tmp_shared, 0);
    if (hydro_tmp==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(hydro_tmp_shared);
        shm_unlink("xhofma11_hydro_tmp");
        exit(ERR_FAIL); 
    }

    oxy_tmp_shared = shm_open("xhofma11_hydro_tmp", O_RDWR|O_CREAT, 0666);
    if(oxy_tmp_shared < 1){
        fprintf(stderr,"error");
    }
    ftruncate(oxy_tmp_shared, sizeof(int));
    oxy_tmp = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, oxy_tmp_shared, 0);
    if (hydro_tmp==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(oxy_tmp_shared);
        shm_unlink("xhofma11_hydro_tmp");
        exit(ERR_FAIL); 
    }

    cnt_shared = shm_open("xhofma11_pid", O_RDWR|O_CREAT, 0666);
        if (cnt_shared<0) { 
            fprintf(stderr,"shm_open\n"); 
            exit(ERR_FAIL); 
        }
    ftruncate(cnt_shared, sizeof(int));
    counter = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED, cnt_shared, 0);

    molecule_shared = shm_open("xhofma11_moleculeID", O_RDWR|O_CREAT, 0666);
        if (molecule_shared<0) { 
            fprintf(stderr,"shm_open\n"); 
            close(molecule_shared);
            shm_unlink("xhofma11_moleculeID");
            exit(ERR_FAIL); 
        }
    ftruncate(molecule_shared, sizeof(int));
    molecule = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED, molecule_shared, 0);
    *molecule = 1;

    bar_cnt_shared = shm_open("xhofma11_bar_cnt", O_RDWR|O_CREAT, 0666);
        if (bar_cnt_shared<0) { 
            fprintf(stderr,"shm_open\n"); 
            close(bar_cnt_shared);
            shm_unlink("xhofma11_bar_cnt");
            exit(ERR_FAIL); 
        }
    ftruncate(bar_cnt_shared, sizeof(int));
    bar_cnt = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED, bar_cnt_shared, 0);
    
    // ftruncate(mole_shared, sizeof(int));
    if( (log_write = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(log_write, 1, 1) == -1 || 
    (oxy = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(oxy, 1, 0) == -1 ||
    (hydro = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(hydro, 1, 0) == -1 || 
    (mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(mutex, 1, 1) == -1 || 
    (bar_mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(bar_mutex, 1, 1) == -1 || 
    (bar_1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(bar_1, 1, 0) == -1 || 
    (bar_2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(bar_2, 1, 1) == -1 || 
    (enough = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(enough, 1, 1) == -1){
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
void oxy_proc(int ti, int tb){
    sem_wait(log_write);
         
        (*counter)++;
        fprintf(fp,"%d: O %d: started\n", *counter, idO);
        //fflush(fp);
    sem_post(log_write);

     
    // if((*hydro_tmp) < 2){
    //     sem_wait(log_write);
    //     fprintf(fp,"%d: O %d: not Enough H\n", *counter, idO);       //THIS MF DOES NOT WORK FIX IT ASAP
        
    //     sem_post(log_write);
    //     sem_post(mutex);
    //     exit(0);

    // }
    sem_wait(log_write);
        (*counter)++;
              
        usleep((rand() % (ti+1))*1000);
        fprintf(fp,"%d: O %d: going to queue\n", *counter, idO);       
    sem_post(log_write);

    sem_wait(mutex);
    
    sem_wait(log_write); 
        if(expectedMolecules < (*molecule)){
            if((*hydro_tmp) < 2){
                fprintf(fp,"%d: O %d: not enough H\n", *counter, idO);
                //sem_post(mutex);
                exit(0);
            }
                   

        }
    sem_post(log_write);
    

    (*idO_cnt)++; 
    if((*idH_cnt) >= 2){
        //sem_post(oxy);
        
        sem_post(hydro);
        sem_post(hydro);
        (*idH_cnt) -= 2;
        (*hydro_tmp) -= 2;
        sem_post(oxy);
        (*idO_cnt)--;
        
    }
    else{
        
        sem_post(mutex);
        
    }

    sem_wait(oxy);
    
    


    sem_wait(log_write);
    (*counter)++;
    fprintf(fp,"%d: O %d: creating molecule %d\n",*counter, idO,*molecule);
    usleep((rand() % (tb+1))*1000);
    sem_post(log_write);
    

    //BARRIER
    sem_wait(bar_mutex);
        (*bar_cnt)++;
       
        if((*bar_cnt) == 3){
            
            sem_wait(bar_2);
            sem_post(bar_1);
        }
    sem_post(bar_mutex);
    
    sem_wait(bar_1);
    
    sem_post(bar_1);
    
    sem_wait(log_write);
    (*counter)++;
    
    fprintf(fp,"%d: O %d: molecule %d created %d\n", *counter, idO, *molecule, *hydro_tmp);    
    sem_post(log_write);


    
    sem_wait(bar_mutex);
        (*bar_cnt)--;
        if((*bar_cnt) == 0){
            sem_wait(bar_1);
            sem_post(bar_2);
        }

        
    sem_post(bar_mutex);

    sem_wait(bar_2);
    sem_post(bar_2);
    
    (*molecule)++;
    
    sem_post(mutex); 
    
    fclose(fp); //Valgrind 
    exit(ERR_SUCC);
}
void oxy_create(int cnt,int ti, int tb){


    for(int i=1; i <= cnt; i++){
        //printf("%d\n",*counter);
        

        pid_t oxygen = fork();
        idO++;
        if(oxygen == 0){  
            oxy_proc(ti,tb);
            
        }       
    }
    
    //while(wait(NULL)>0);
    
    
    //exit(ERR_SUCC);
    
}
void hydro_proc(int ti, int tb){
    sem_wait(log_write);
        (*counter)++;
        fprintf(fp,"%d: H %d: started\n", *counter, idH);
        //fflush(fp);
    sem_post(log_write);
    usleep((rand() % (ti+1))*1000);
    sem_wait(log_write);
        (*counter)++;
        
        fprintf(fp,"%d: H %d: going to queue\n", *counter, idH);
    sem_post(log_write);

    sem_wait(mutex);
    
   sem_wait(log_write); 
        if(expectedMolecules < (*molecule)){
            if((*hydro_tmp) < 2 || (*oxy_tmp < 1)){
                fprintf(fp,"%d: H %d: not enough O or H\n", *counter, idH);
                sem_post(mutex);
                exit(0);
            }
                   

        }
    sem_post(log_write);

    (*idH_cnt)++;
    if((*idH_cnt) >= 2 && (*idO_cnt) >= 1){
        sem_post(hydro);
        sem_post(hydro);
        (*idH_cnt) -= 2;
        (*hydro_tmp) -= 2;
        sem_post(oxy);
        (*idO_cnt)--;

    }
    else{
        
        sem_post(mutex);
        //exit(ERR_SUCC);
    }


    sem_wait(hydro);
    
    sem_wait(log_write);
    (*counter)++;
    
    fprintf(fp,"%d: H %d: creating molecule %d\n",*counter, idH,*molecule);
    
    sem_post(log_write);

    //BARRIER
    sem_wait(bar_mutex);
        (*bar_cnt)++;
        if((*bar_cnt) == 3){
            sem_wait(bar_2);
            sem_post(bar_1);
        }

    sem_post(bar_mutex);
     
    sem_wait(bar_1);
    
    sem_post(bar_1);

    sem_wait(log_write);
    (*counter)++;
    fprintf(fp,"%d: H %d: molecule %d created\n", *counter, idH, *molecule);
    sem_post(log_write);

    sem_wait(bar_mutex);
        (*bar_cnt)--;
        if((*bar_cnt) == 0){
            sem_wait(bar_1);
            sem_post(bar_2);
        }
    sem_post(bar_mutex);

    sem_wait(bar_2);
    sem_post(bar_2);

    
    fclose(fp);//Valgrind
    exit(ERR_SUCC);
}

void hydro_create(int cnt,int ti, int tb){
    for(int i=1; i <= cnt; i++){
        //printf("%d\n",cnt);
        
        pid_t hydro = fork();
        idH++;
        if(hydro == 0){
            //printf("forked_hydro\n");
            hydro_proc(ti, tb);
            
        }
     
    }
    //while(wait(NULL)>0);
    //exit(ERR_SUCC);
    
}





int main(int argc, char **argv){
    
    
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

    if(NO <= 0 || NH <= 0 || TI < 0 || TB < 0){
        param_msg();
    }

    sem_initialization();
    (*hydro_tmp) = NH; // TODO FOR NOT ENOUGH O H
    (*oxy_tmp) = NO;
    if((2*NO) <= NH){
        expectedMolecules = NO;
    }
    else{
        expectedMolecules = NH/2;
    }
    //printf("Expected: %d", expectedMolecules);

    

    if (counter==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(cnt_shared);
        shm_unlink("xhofma11_pid");
        exit(ERR_FAIL); 
    }
    fp = fopen("proj2.out", "w");
        if(fp == NULL){
            errFile();
    }
    setbuf(fp, NULL);
    pid_t oxy_pid, hydro_pid;   
   
        oxy_create(NO, TI, TB);
        //while(wait(NULL)>0);
        hydro_create(NH, TI, TB); 
        while(wait(NULL)>0){
            //printf("forked\n");
        }

        

    //wait(NULL); // Waits until child is dead
    
    clean();
    

    // shm_unlink("molecule");
    //printf("Finishing\n");
    fclose(fp);


    return 0;
}