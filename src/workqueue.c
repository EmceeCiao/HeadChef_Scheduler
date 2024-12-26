//Intial File Made not sure what for yet, may contain functions such as add to workqueue and doing other related things. 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <getopt.h>
#include "funcs.h"  

void free_cookbook(COOKBOOK *ckb){
    //To free a cookbook I first have to free each of the words in the steps and then free the steps  
    if(!ckb) return; 
    RECIPE *curr_recipe = ckb->recipes; 
    while(curr_recipe){
        RECIPE * next_recipe = curr_recipe->next; 
        //Need to deal with the tasks which is nested and free the words arrays and the steps so 
        TASK *curr_task = curr_recipe->tasks;  
        while(curr_task){
            TASK *next_task = curr_task->next;  
            STEP *curr_step = curr_task->steps; 
            while(curr_step){
                STEP *next_step = curr_step->next;   
                char** curr_word = curr_step->words;  
                //Free Each Word in the Curr Word Array, we know it's null terminated
                while(*curr_word != NULL){
                    free(*curr_word); 
                    curr_word++; 
                } 
                //free(curr_word); 
                free(curr_step->words);  
                //Now we can free this step and go to the next one
                free(curr_step); 
                curr_step = next_step; 
            }  
            //Now that we've done this we can free the input and output file pointers if they exist 
            if(curr_task->input_file){
                free(curr_task->input_file); 
            } 
            if(curr_task->output_file){
                free(curr_task->output_file);
            }
            free(curr_task); 
            curr_task = next_task; 
        }  
        //So far we've freed all the tasks since we've done tasks steps, input, and output file  
        //Now we need to work on the recipe links  
        RECIPE_LINK* curr_link_this_depends_on = curr_recipe->this_depends_on; 
        while(curr_link_this_depends_on){
            RECIPE_LINK* next_link_this_depends_on = curr_link_this_depends_on->next; 
            free(curr_link_this_depends_on->name); 
            free(curr_link_this_depends_on);  
            curr_link_this_depends_on = next_link_this_depends_on; 
        } 
        RECIPE_LINK * curr_link_depends_on_this = curr_recipe->depend_on_this; 
        while(curr_link_depends_on_this){
            RECIPE_LINK * next_link_depends_on_this = curr_link_depends_on_this->next; 
            free(curr_link_depends_on_this); 
            curr_link_depends_on_this = next_link_depends_on_this; 
        } 
        free(curr_recipe->name); 
        //Free Recipe State if state exists 
        if(curr_recipe->state){
            free(curr_recipe->state); 
        }  
        free(curr_recipe);  
        curr_recipe = next_recipe; 
    } 
    free(ckb); 
} 
RECIPE* find_recipe_in_cookbook(COOKBOOK *ckb, char* recipe_name){
    if(recipe_name == NULL){
        return ckb->recipes; 
    }else{
        for(RECIPE *recipe = ckb->recipes; recipe != NULL; recipe = recipe->next){
            if(strcmp(recipe->name, recipe_name) == 0){
                return recipe; 
            }
        } 
        return NULL; //This means the recipe doesn't exist in the cookbook which should be an erro! 
    }
}
/*Function for adding recipe to end of the list*/
int enqueue(RECIPE* adding, WORK_QUEUE* queue){ 
    //Adding with Sentinel Node is Simple 
    NODE* node = malloc(sizeof(NODE));
    if(node == NULL){ 
        return -1; //MALLOC FAILED 
    } 
    //Otherwise all we need to do is add to the end of our sentinel node so before it
    NODE* sentinel = queue->sentinel;    
    node->recipe = adding; 
    node->next = sentinel; 
    node->prev = sentinel->prev; 
    sentinel->prev->next = node;  
    sentinel->prev = node;  

    queue->size++;  
    return 0; 
} 
/*Function for removing recipe at the start of the list*/
RECIPE* dequeue(WORK_QUEUE* queue){ 
    if(queue->size == 0){ 
        return NULL; 
    } 
    //Now we are removing from the head so we just remove from the sentinel node  
    //Let me just grab the node I'm removing 
    NODE* temp = queue->sentinel->next; 
    RECIPE* returning = temp->recipe;  
    //Set the next arrays previous back to the sentinel and set the sentinel next to be the next thing
    queue->sentinel->next = temp->next;  
    temp->next->prev = queue->sentinel;  
    queue->size--;   

    free(temp); //Free the Node we're removing 
    return returning; 
}  
/*Function for freeing our Work_Queue at the end when we are exiting our main cook process*/
void free_queue(WORK_QUEUE * queue){
    while(queue->size > 0){
        dequeue(queue); 
    } 
    free(queue->sentinel); 
    free(queue); 
} 
/*Function for finding if a recipe is in queue 
  Returns 0 if not found, 1 if found
*/
int is_in_queue(RECIPE* recipe, WORK_QUEUE* queue){ 
    if(queue->size == 0){ 
        //debug("QUEUE SIZE WAS 0!"); 
        return 0; //Not in queue
    } 
    NODE* current = queue->sentinel->next; 

    while(current != queue->sentinel){ 
        //debug("LOOKING!"); 
        if(strcmp(current->recipe->name, recipe->name) == 0){ 
            //debug("RECIPE WAS FOUND!"); 
            return 1; 
        } 
        current = current->next;  
    } 
    return 0; //Not in queue
}  
RECIPE* search_by_PID(pid_t chef_pid, WORK_QUEUE* queue){
    if(queue->size == 0){
        return NULL; 
    } 
    NODE* current = queue->sentinel->next; 
    while(current != queue->sentinel){ 
        //State garunteed to exist since it's in our workqueue 
        RECIPE_STATE* state = (RECIPE_STATE *)current->recipe->state;   
        if(state->chef_pid == chef_pid){
            return current->recipe; 
        }
        current = current->next; 
    } 
    return NULL; //not in queue; 
}

void print_queue(WORK_QUEUE* queue){
    NODE* current = queue->sentinel->next; 
    while(current != queue->sentinel){   
        printf("%s -> ", current->recipe->name);  

        RECIPE_STATE *state = (RECIPE_STATE*)current->recipe->state;  
        printf("RECIPE NAME:%s\n", current->recipe->name);     
        printf("Dependencies:%d\n", state->deps); 
        printf("Visit_State:%d\n", state->visit_state);  
        printf("CHEF_PID: %d\n", state->chef_pid); 
        printf("\n"); 
        current = current->next;  
    } 
}
void print_recipe_info(RECIPE* recipe){ 
    RECIPE_STATE *state = (RECIPE_STATE*)recipe->state;  
    printf("RECIPE NAME:%s\n", recipe->name);     
    printf("Dependencies:%d\n", state->deps); 
    printf("Visit_State:%d\n", state->visit_state);  
    printf("CHEF_PID: %d\n", state->chef_pid); 
    printf("\n"); 
}

WORK_QUEUE* init_queue(){
    WORK_QUEUE* queue = malloc(sizeof(WORK_QUEUE)); 
    if(queue == NULL){ 
        return NULL;   
    } 
    NODE* sentinel = malloc(sizeof(NODE)); 
    if(sentinel == NULL){
        free(queue); 
        return NULL; 
    } 
    sentinel->recipe = NULL; 
    sentinel->next = sentinel; 
    sentinel->prev = sentinel; 
    queue->sentinel = sentinel; 
    queue->size = 0; 
    return queue; 
}
//My dependency analysis will not intialize the workqueue! 
int dependency_analysis(RECIPE *recipe, WORK_QUEUE* queue){
    if(!recipe){return -1;} 
    if(!recipe->state){
        RECIPE_STATE *state = malloc(sizeof(RECIPE_STATE)); 
        state->deps = 0; 
        state->visit_state = 0; 
        state->completed = 0; 
        state->chef_pid = 0;  
        recipe->state = state;   
        //Number of dependencies
        for(RECIPE_LINK *deps = recipe->this_depends_on; deps != NULL; deps = deps->next){ 
            state->deps++; 
        }
    }   
    //Now we recursively populate!  
    for(RECIPE_LINK * deps = recipe->this_depends_on; deps!= NULL; deps = deps->next){ 
        if(dependency_analysis(deps->recipe, queue) < 0){
            return -1; 
        }
    } 
    //Now that we've recursively populated as needed, we can now add it to the work queue! 
    //debug("RECIPE VISITED: %s\n", recipe->name); 

    RECIPE_STATE *state = (RECIPE_STATE*)recipe->state; 
    state->visit_state = 2;  //Marking as Visited for now, will be more important in circular dependency checking if we implement  
    if(state->deps == 0){  
        if(is_in_queue(recipe, queue) == 1){
            //Ignore and do nothing!
        }else{
        //debug("RECIPE QUEUED: %s\n", recipe->name);
        enqueue(recipe, queue);
        }
    } 
    return 0; 
} 

int update_queue(pid_t chef_pid, WORK_QUEUE* pid_queue, WORK_QUEUE* working_queue){
    //This is because we will be searching PIDs through our used_recipe queue as this keeps the PIDS but we need to update the working queue 
    RECIPE* recipe_found = search_by_PID(chef_pid, pid_queue); 
    if(recipe_found == NULL){
        return -1; 
    } 
    RECIPE_LINK* founded_link = recipe_found->depend_on_this;  
    while(founded_link){ 
        if(founded_link->recipe->state){
            RECIPE_STATE * state = (RECIPE_STATE *)founded_link->recipe->state;  
            state->deps--; 
            if(state->deps == 0){ 
                //print_recipe_info(founded_link->recipe); 
                enqueue(founded_link->recipe, working_queue); 
            }
        } 
        founded_link = founded_link->next; 
    }  
    return 0; //SUCCESSFULL EXIT
}
// int dependency_analysis(RECIPE *recipe, WORK_QUEUE* queue, WORK_QUEUE* used_recipes){
//     if(!recipe){return -1;} 
//     if(!recipe->state){
//         RECIPE_STATE *state = malloc(sizeof(RECIPE_STATE)); 
//         state->deps = 0; 
//         state->visit_state = 0; 
//         state->completed = 0; 
//         state->chef_pid = 0;  
//         recipe->state = state;   
//         //Number of dependencies
//         for(RECIPE_LINK *deps = recipe->this_depends_on; deps != NULL; deps = deps->next){ 
//             state->deps++; 
//         }
//     }   
//     //Now we recursively populate!  
//     for(RECIPE_LINK * deps = recipe->this_depends_on; deps!= NULL; deps = deps->next){ 
//         if(dependency_analysis(deps->recipe, queue, used_recipes) < 0){
//             return -1; 
//         }
//     } 
    
//     if(is_in_queue(recipe, used_recipes) == 1){
//         //ignore
//     }else{enqueue(recipe, used_recipes);} 
//     //Now that we've recursively populated as needed, we can now add it to the work queue! 
//     //debug("RECIPE VISITED: %s\n", recipe->name); 

//     RECIPE_STATE *state = (RECIPE_STATE*)recipe->state; 
//     state->visit_state = 2;  //Marking as Visited for now, will be more important in circular dependency checking if we implement  
//     if(state->deps == 0){  
//         if(is_in_queue(recipe, queue) == 1){
//             //Ignore and do nothing!
//         }else{
//         //debug("RECIPE QUEUED: %s\n", recipe->name);
//         enqueue(recipe, queue);
//         }
//     } 
//     return 0; 
// } 
