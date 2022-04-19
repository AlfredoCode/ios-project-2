#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>

#define ARGCOUNT 5
#define MAXTIME 1000

void param_cnt_msg(){
    fprintf(stderr,"Invalid count of arguments!\n");
    fprintf(stderr,"Correct usage is ./proj2 NO NH TI TB\n");
    exit(1);
}

void param_msg(){
    fprintf(stderr,"Nespravne pouziti parametru! Je nutno zadat cela cisla > 0\n");
    exit(1);
}

void errMaxTime(){
    fprintf(stderr,"Maximální čas muze byt v rozsahu <0;1000>!\n");
    exit(1);
}


int main(int argc, char **argv){
    int NO, NH, TI, TB;

    if (argc != ARGCOUNT){
        param_cnt_msg();  
    }
    for(int i = 1;i < ARGCOUNT; i++){
        if(argv[i][0] < '0' || argv[i][0] > '9'){
            param_msg();
        }
        switch(i){
            case 1:
                NO = strtol(argv[i], NULL, 10);
                break;
            case 2:
                NH = strtol(argv[i], NULL, 10);
                break;
            case3:
                TI = strtol(argv[i], NULL, 10);
                if(TI < 0 || TI > MAXTIME){
                    errMaxTime();
                } 
        }
    }
    printf("%d\n", NO);
    printf("%d\n", NH);
    printf("%d\n", TI);
    printf("%d\n", TB);
    
    return 0;
}