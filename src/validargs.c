#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <getopt.h>
#include "funcs.h" 

//Setting our Globals to their defaults, well besides main_recipe_name as that will be parsed or gotten through the next program call 
//We'll just check if it's null, if it is grab the first recipe in cookbook as the correct recipe to start with! 

int max_cooks = 1;  
char* cook_book = "cookbook.ckb";
char* main_recipe_name = NULL; 

int validargs(int argc, char **argv) { 
    int f_flag = 0; 
    int c_flag = 0;  
    int error_occured = 0; 
    int cli; 

    while ((cli = getopt(argc, argv, ":f:c:")) != -1) {
        switch(cli) {
            case 'f':  
                if (f_flag == 1) {
                    error_occured = 1;
                } 
                cook_book = optarg; 
                f_flag = 1; 
                break;  
            case 'c':  
                if (c_flag == 1) {
                    error_occured = 1;
                } 
                char* end; 
                long val = strtol(optarg, &end, 10); 
                if (*end != '\0' || val <= 0) {
                    error_occured = 1; 
                }
                max_cooks = (int)val;  
                c_flag = 1;
                break;  
            case ':': 
                error_occured = 1;
                break;
            case '?': 
                error_occured = 1;  
                break;   
            default: 
                break; 
        }
    }    

    if (optind < argc) {
        main_recipe_name = argv[optind]; 
    } 
    if (optind + 1 < argc) {
        error_occured = 1; 
    }  

    // printf("Cookbook File: %s\n", cook_book); 
    // printf("Max Cooks: %d\n", max_cooks);  
    // printf("Recipe Name: %s\n", main_recipe_name); 

    //Checking if cook_book exists, if not then it can't be parsed. 
    FILE* in; 
    if ((in = fopen(cook_book, "r")) == NULL) { 
        error_occured = 1; 
    } else {
        fclose(in); 
    } 
    if(error_occured){
        return -1; 
    }else{
        return 0; 
    } 
}

