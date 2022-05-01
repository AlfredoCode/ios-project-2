#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
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
    *enough2,
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
    munmap(hydro_tmp, sizeof(sem_t));

    close(oxy_tmp_shared);
    shm_unlink("xhofma11_oxy_tmp");
    munmap(oxy_tmp, sizeof(sem_t));

    sem_destroy(enough2);
    sem_destroy(enough);
    sem_destroy(oxy);
    sem_destroy(hydro);
    sem_destroy(mutex);
    sem_destroy(log_write);
    sem_destroy(bar_mutex);
    sem_destroy(bar_1);
    sem_destroy(bar_2);
}


void sem_initialization(){
    //clean();
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

    oxy_tmp_shared = shm_open("xhofma11_oxy_tmp", O_RDWR|O_CREAT, 0666);
    if(oxy_tmp_shared < 1){
        fprintf(stderr,"error");
    }
    ftruncate(oxy_tmp_shared, sizeof(int));
    oxy_tmp = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, oxy_tmp_shared, 0);
    if (oxy_tmp==MAP_FAILED) { 
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
    if (counter==MAP_FAILED) { 
        fprintf(stderr,"mmap\n"); 
        close(cnt_shared);
        shm_unlink("xhofma11_pid");
        exit(ERR_FAIL); 
    }
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
    
    if( (log_write = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(log_write, 1, 1) == -1 || 
    (oxy = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(oxy, 1, 0) == -1 ||
    (hydro = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(hydro, 1, 0) == -1 || 
    (mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(mutex, 1, 1) == -1 || 
    (bar_mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(bar_mutex, 1, 1) == -1 || 
    (bar_1 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(bar_1, 1, 0) == -1 || 
    (bar_2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(bar_2, 1, 1) == -1 || 
    (enough = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(enough, 1, 1) == -1 || 
    (enough2 = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED || sem_init(enough2, 1, 1) == -1){
        fprintf(stderr,"Error initializing semaphores!\n");
        exit(ERR_FAIL);
    }    
}
void proc_valid(int fileDesc){
    if (fileDesc == -1){
        fprintf(stderr,"Process creation failed");
        clean();
        exit(ERR_FAIL);
    }
}

void param_cnt_msg(){
    fprintf(stderr,"Invalid count of arguments! Correct usage is ./proj2 NO NH TI TB\n");
    exit(ERR_FAIL);
}

void param_msg(){
    fprintf(stderr,"Invalid usage of arguments!\n");
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
    sem_post(log_write);

    sem_wait(log_write);
    (*counter)++;
    usleep((rand() % (ti+1))*1000);
    fprintf(fp,"%d: O %d: going to queue\n", *counter, idO);       
    sem_post(log_write);

    sem_wait(mutex);
    (*idO_cnt)++; 
    if((*idH_cnt) >= 2){
        sem_post(hydro);
        sem_post(hydro);
        (*idH_cnt) -= 2;
        (*hydro_tmp) -= 2;
        sem_post(oxy);
        (*idO_cnt)--;
        (*oxy_tmp)--;
    }
    else{
        sem_post(mutex); 
    }

    sem_wait(oxy);
    if(expectedMolecules < (*molecule)){
        sem_wait(enough); 
        sem_wait(log_write);
        (*counter)++;
        (*oxy_tmp)--;
        fprintf(fp,"%d: O %d: not enough H\n", *counter, idO);
        sem_post(log_write);
        sem_post(enough);
        sem_post(oxy);

        fclose(fp);
        exit(ERR_SUCC);         
    }
    sem_wait(log_write);
    (*counter)++;
    fprintf(fp,"%d: O %d: creating molecule %d\n",*counter, idO,*molecule);
    sem_post(log_write);
    usleep((rand() % (tb+1))*1000);

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
    fprintf(fp,"%d: O %d: molecule %d created\n", *counter, idO, *molecule);    
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
    if((*hydro_tmp) == 0 && (*oxy_tmp) != 0){
        sem_post(oxy);
    }
    else if((*hydro_tmp) != 0 && (*oxy_tmp) == 0){
        sem_post(hydro);
    }
    fclose(fp); //Valgrind 
    exit(ERR_SUCC);
}
void oxy_create(int cnt,int ti, int tb){
    for(int i=1; i <= cnt; i++){
        pid_t oxygen = fork();
        idO++;
        if(oxygen == 0){  
            oxy_proc(ti,tb);
        }       
    }
}
void hydro_proc(int ti){
    sem_wait(log_write);
    (*counter)++;
    fprintf(fp,"%d: H %d: started\n", *counter, idH);
    sem_post(log_write);

    usleep((rand() % (ti+1))*1000);

    sem_wait(log_write);
    (*counter)++;
    fprintf(fp,"%d: H %d: going to queue\n", *counter, idH);
    sem_post(log_write);

    sem_wait(mutex);
    if(expectedMolecules < (*molecule)){
        sem_wait(enough); 
        sem_wait(log_write);
        (*counter)++;
        fprintf(fp,"%d: H %d: not enough O or H\n", *counter, idH);
        sem_post(log_write);
        sem_post(enough);
        
        sem_post(mutex);
        if((*oxy_tmp) >= 1){
            sem_post(oxy);  
        }
        if((*hydro_tmp) >= 1){
            sem_post(hydro);
        }
        fclose(fp);
        exit(ERR_SUCC);      
    }

    (*idH_cnt)++;
    if((*idH_cnt) >= 2 && (*idO_cnt) >= 1){
        sem_post(hydro);
        sem_post(hydro);
        (*idH_cnt) -= 2;
        (*hydro_tmp) -= 2;
        sem_post(oxy);
        (*idO_cnt)--;
        (*oxy_tmp)--;
    }
    else{    
        sem_post(mutex);
    }

    sem_wait(hydro);
    if(expectedMolecules < (*molecule)){
        
        sem_wait(enough); 
        sem_wait(log_write);
        (*counter)++;
        (*hydro_tmp)--;
        fprintf(fp,"%d: H %d: not enough O or H\n", *counter, idH);
        
        sem_post(log_write);
        sem_post(enough);
        sem_post(hydro);
        fclose(fp);
        exit(ERR_SUCC);         
    }

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

void hydro_create(int cnt,int ti){
    for(int i=1; i <= cnt; i++){
        pid_t hydro = fork();
        idH++;
        if(hydro == 0){
            hydro_proc(ti);   
        }
    }   
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
                if(strcmp(argv[i], "") == 0){
                    errMaxTime();
                }
                TI = strtol(argv[i], &err, 10);
                if(TI < 0 || TI > MAXTIME){
                    errMaxTime();
                }
                break; 
            case 4:
                if(strcmp(argv[i], "") == 0){
                    errMaxTime();
                }
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

    (*hydro_tmp) = NH;
    (*oxy_tmp) = NO;

    if((2*NO) <= NH){
        expectedMolecules = NO;
    }
    else{
        expectedMolecules = NH/2;
    }

    fp = fopen("proj2.out", "w");
    if(fp == NULL){
        errFile();
    }

    setbuf(fp, NULL);
    oxy_create(NO, TI, TB);
    hydro_create(NH, TI); 
    while(wait(NULL)>0);
    clean();
    fclose(fp);
    return 0;
}