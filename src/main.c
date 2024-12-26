#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "cookbook.h"
#include "funcs.h"

int main(int argc, char *argv[]) {   
    if(validargs(argc, argv) == -1){  
        //printf("ERROR\n");  
        fprintf(stderr, "EXITING WITH FAILURE FROM VALID ARGS");
        exit(EXIT_FAILURE); 
    }   
    if(cook_program() == -1){ 
        fprintf(stderr, "EXITING WITH FAILURE FROM COOK LOGIC"); 
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS); 
    //exit(0);
}
