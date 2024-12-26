//Externing our global variables which will be so far cook_book for the cookbook name, max_cooks for max_cookcount and then main_recipe_name  
#ifndef FUNCS_H 
#define FUNCS_H 
#include "cookbook.h" 
#include <unistd.h> 
#include <sys/types.h>  
#include "debug.h"

extern char* cook_book; 
extern int   max_cooks; 
extern char* main_recipe_name;  
int validargs(int argc, char **argv);   

//Some Free cookbook function to close everything in cookbook
void free_cookbook(COOKBOOK *ckb);  

//Some find recipe function since we'll need to look for the main_recipe in the cookbook! 
RECIPE* find_recipe_in_cookbook(COOKBOOK *ckb, char* recipe_name); 

//Some function to print recipe state! 
void print_recipe_info(RECIPE* recipe);
//Structs 

/* WORKQUEUE, NODE and RECIPE_STATE Definitions to be used to decide what recipes to cook and what recipes were cooked */  
typedef struct Node { 
    RECIPE *recipe;  
    struct Node* next; 
    struct Node* prev; 
} NODE;  

typedef struct work_queue {  
    NODE* sentinel; 
    int size; 
} WORK_QUEUE; 

/* Recipe_Meta Data Struct to be used for the state of recipes 
   Will let me track 
    1) Dependencies remaining -> Helps with enqueueing and gets updated everytime a child returns 
    2) Visit_State -> Will help with defining circular dependencies 
    3) Completed -> Will be used again for updating and enqueueing 
    4) Chef_PID -> Which Recipe even finished! */  
typedef struct recipe_state{ 
    int deps; 
    int visit_state; 
    int completed;  
    pid_t chef_pid; 
} RECIPE_STATE; 
//Functions Related to Queue and Dependency Analysis
int is_in_queue(RECIPE* recipe, WORK_QUEUE* queue);  

RECIPE* search_by_PID(pid_t chef_pid, WORK_QUEUE* queue); 

int enqueue(RECIPE* adding, WORK_QUEUE* queue); 

RECIPE* dequeue(WORK_QUEUE* queue); 

void free_queue(WORK_QUEUE* queue);  

//int dependency_analysis(RECIPE *recipe, WORK_QUEUE* queue, WORK_QUEUE* used_recipes);  

int dependency_analysis(RECIPE *recipe, WORK_QUEUE* queue);  

int update_queue(pid_t chef_pid, WORK_QUEUE* pid_queue, WORK_QUEUE* working_queue);  

WORK_QUEUE* init_queue();

void print_queue(WORK_QUEUE* queue); 
//Functions related to main_chef program(); 
int cook_program(); 

#endif