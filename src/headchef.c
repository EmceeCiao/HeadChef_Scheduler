//File for actually doing all the chef work and implementing the cook from start to finish!  
#include "cookbook.h"  
#include "funcs.h"
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
#include <string.h>
#include <errno.h>   
#include <signal.h>
#include <sys/wait.h>  
#include <fcntl.h> 
#include <string.h>  
#include <limits.h>

void signal_child_handler() {
} 

void cook_recipe(RECIPE* cooking_recipe){ 
    //First let's grab the task from the recipe struct 
    TASK* recipe_tasks = cooking_recipe->tasks;  
    int incomplete = 0;   
    while(recipe_tasks){  
        STEP* current_step = recipe_tasks->steps; 
        STEP* next_step = recipe_tasks->steps->next;   
        char* input_file = recipe_tasks->input_file; 
        char* output_file = recipe_tasks->output_file; 
        int inputFD = -1; 
        int outputFD = -1;  
        int current_pipe[2] = {-1, -1}; //READ IS current_pipe[0] and WRITE is current_pipe[1]
        int prev_pipe[2] = {-1, -1};  //Need to see if the previous pipes were used or not, no DF is negative! 
        while(current_step != NULL){  
            //We only need a pipeline if the next step exists!
            if(next_step != NULL){
                if(pipe(current_pipe) == -1){ 
                   fprintf(stderr, "FAILED TO CREATE PIPE!"); 
                   incomplete = 1; 
                   break; 
                } 
            } 
            //We now have our pipe setup let's start getting ready to fork 
            pid_t step_pid;  
            if((step_pid = fork())< 0){
                //We failed to fork so we should incomplete and break!  
                fprintf(stderr, "FAILED TO FORK FOR A STEP!"); 
                incomplete = 1; 
                break; //We didn't fork properly so no need to exit yet 
            } 
            if(step_pid == 0){
                //We are now in the step fork where we use the pipelining that was just created! 
                //1st we check if it's the first or last step and if the file exists since we need to set up redirection in that case 
                if(input_file && current_step == recipe_tasks->steps){
                    //We need to now open up our inputFD and read from it to stdin  
                    if((inputFD = open(input_file, O_RDONLY, 0777)) == -1){
                        fprintf(stderr, "INPUT FILE FAILED TO OPEN"); 
                        exit(EXIT_FAILURE); //Now we can exit because of this error 
                    } 
                    //Else we can actually redirect this to our STDIN 
                    dup2(inputFD, STDIN_FILENO); 
                    close(inputFD); 
                }  
                //The Last Step Code should be pretty similar 
                if(output_file && next_step == NULL){
                    if((outputFD = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0777)) == -1){
                        //ERROR CREATING OUTPUT FILE
                        fprintf(stderr, "OUTPUT FILE FAILED TO BE CREATED"); 
                        exit(EXIT_FAILURE); 
                    } 
                    dup2(outputFD, STDOUT_FILENO); 
                    close(outputFD);
                }   

                //IF we have neither the the first or last part then we are in an inbetween step 
                //We have to redirect our read to stdin and our write to stdout 
                if(prev_pipe[0] != -1){
                    dup2(prev_pipe[0], STDIN_FILENO); 
                    close(prev_pipe[0]); 
                    close(prev_pipe[1]); 
                } 

                if(next_step != NULL){
                    dup2(current_pipe[1], STDOUT_FILENO); 
                    close(current_pipe[0]); 
                    close(current_pipe[1]); 
                } 

                //Now that we've set up the entire pipeline we can actually try running execvp twice!  
                char path_buf[PATH_MAX] = "util/"; 
                //We can now use strncat and safely append to this path! 
                strcat(path_buf, current_step->words[0]); 
                if(execvp(path_buf, current_step->words) == -1){
                    //We try our next execvp call 
                    if(execvp(current_step->words[0], current_step->words) == -1){
                        //Now we know we ran into an error and we can exit with exit failure  
                        fprintf(stderr, "FAILING TO EXECVP"); 
                        exit(EXIT_FAILURE); 
                    }
                }   
            } 
            if(step_pid > 0){
                //PARENT PROCESS 
                //Close Unused Pipes so 
                if(prev_pipe[0] != -1){
                    close(prev_pipe[0]); 
                    close(prev_pipe[1]); 
                } 
                //Forward the pipes to current and then move to the next steps 

                prev_pipe[0] = current_pipe[0];  
                prev_pipe[1] = current_pipe[1]; 
                //We don't wait here as we want steps to be concurrent, we wait outside of the steps
            }  
            current_step = current_step->next; 
            next_step = next_step ? next_step->next : NULL; //Safe way of incrementing, without this it didn't increment properly
        }   
        int status; 
        pid_t finished_step; 
        while((finished_step = wait(&status)) > 0){
            if(WIFEXITED(status)){
                int exit_status = WEXITSTATUS(status); 
                if(exit_status == EXIT_SUCCESS){ 
                    //Don't do anything we are fine
                } 
                if(exit_status == EXIT_FAILURE){ 
                    fprintf(stderr, "ONE OF OUR STEPS FAILED"); 
                    incomplete = 1; 
                }
            }
        } 
        //We let the whole process run and then if we have incomplete we break 
        if(incomplete){
            break; 
        }
        //wait(); //Wait until program finishes, if incomplete break if complete we go to next task  
        //SO we wait for all the steps and 
        recipe_tasks = recipe_tasks->next;  
    }  
    if(incomplete){
        exit(EXIT_FAILURE); 
    }else{
        exit(EXIT_SUCCESS); 
    }
} 

int cook_program(){     
    int err = 0; 
    //First let's check if the recipe exists in our cookbook! 
    //To do this we first need to open the cookbook, parse it, and then close the file! 
    COOKBOOK *ckb;
    FILE *in;   
    if ((in = fopen(cook_book, "r")) == NULL) {  
        fprintf(stderr, "COOKBOOK COULDN'T BE OPENED"); 
        debug("Cookbook couldn't be opened! \n"); 
        return -1;  
    } 
    ckb = parse_cookbook(in, &err); 
    if(err){ 
        fprintf(stderr, "COULDN'T PARSE COOKBOOK"); 
        debug("Couldn't parse cookbook! \n"); 
        fclose(in); 
        return -1; 
    } 
    fclose(in);  
    RECIPE *main_recipe = find_recipe_in_cookbook(ckb, main_recipe_name); 
    if(main_recipe == NULL){ 
        fprintf(stderr, "MAIN_RECIPE DOESN'T EXIST"); 
        debug("Main Recipe Doesn't Exist! \n");  
        free_cookbook(ckb); 
        return -1; 
    } 
    //Main_Recipe is gotten now we can intialize our queue and pass it in to our dependency analysis  
    /*START REWRITING FROM BELOW */
    WORK_QUEUE* queue = init_queue();   
    WORK_QUEUE* used_recipes = init_queue(); 
    int dep_analysis = dependency_analysis(main_recipe, queue);   
    //QUEUE IS NOW LOADED WITH INFORMATION!  
    if(dep_analysis == -1){ 
        fprintf(stderr, "DEPENDENCY ANALYSIS FAILED");
        free_queue(queue); 
        free_queue(used_recipes); 
        free_cookbook(ckb); 
        return -1; 
    }    
   

//One more time rewrite! 
    sigset_t mask_child, unmask_child; 
    sigemptyset(&mask_child); 
  //  sigemptyset(&unmask_child); 

    sigaddset(&mask_child, SIGCHLD);  
    struct sigaction handle_sigchild; 
    handle_sigchild.sa_handler = signal_child_handler; 
    sigemptyset(&handle_sigchild.sa_mask);  
    handle_sigchild.sa_flags = 0; 

    if (sigaction(SIGCHLD, &handle_sigchild, NULL) < 0) { 
    fprintf(stderr, "SIGACTION FAILED"); 
    free_queue(queue);
    free_queue(used_recipes);
    free_cookbook(ckb);
    return -1;
    }
    if((sigprocmask(SIG_BLOCK, &mask_child, &unmask_child)) < 0){ 
        //ERROR OCCURED;  
        fprintf(stderr, "SIGPROCMASK FAILED"); 
        free_queue(queue); 
        free_queue(used_recipes); 
        free_cookbook(ckb); 
        return -1; 
    }   
    sigdelset(&unmask_child, SIGCHLD);
    //Setting globals  
    int active_cooks = 0; 
    int complete = 0; 
    int incomplete = 0; 
    while(1){ 
        if(active_cooks == 0 && queue->size == 0){ 
            //printf("COMPLETED"); 
            complete = 1; 
            break; 
        } 
        if(active_cooks < max_cooks && queue->size > 0){
            //Get ready to fork and use a recipe 
            RECIPE* chef_recipe = dequeue(queue);   
            enqueue(chef_recipe, used_recipes);  
            pid_t chef_pid; 
            if((chef_pid = fork()) < 0){ 
                //printf("ENTERED"); 
                incomplete = 1; 
                break; 
            } 
            if(chef_pid == 0){
                //Child Process so  
                //execure recipe  
                //EXECUTE RECIPE FUNCTION!  
                //cook_recipe(chef_recipe); 
                cook_recipe(chef_recipe); //Should be a function that cooks recipe and exits
                //exit(0); 
            }else if(chef_pid > 0){
                //Main Process so 
                active_cooks++;     
                //printf("ACTIVE COOKS JUST UPDATED: %d", active_cooks);
                //update the state of the recipe with the PID we now have 
                RECIPE_STATE* state = (RECIPE_STATE*)chef_recipe->state; 
                state->chef_pid = chef_pid; 
            }
        }  
        if(active_cooks == max_cooks && queue->size > 0){  
            //printf("ABOUT TO SIGSUSPEND\n");
            sigsuspend(&unmask_child);  
        }
        int status; 
        pid_t finished_recipe_pid;  
        while((finished_recipe_pid = waitpid(-1, &status, WNOHANG)) > 0){
            if(WIFEXITED(status)){ 
                int exit_status = WEXITSTATUS(status); 
                if(exit_status == EXIT_SUCCESS){ 
                    active_cooks--; 
                    update_queue(finished_recipe_pid, used_recipes, queue); 
                }else{  
                    //printf("INCOMPLETE 1"); 
                    active_cooks--; 
                    incomplete = 1; 
                }
            }
        } 
        if(incomplete){ 
            //printf("INCOMPLETE 2"); 
            break;} 
    }  
    if(complete){ 
        free_cookbook(ckb); 
        free_queue(queue); 
        free_queue(used_recipes);  
        return 0;
    }else if(incomplete){  
        if((used_recipes->size) == 0){
            //DO nothing 
        }else{
            NODE* current = queue->sentinel->next;   
            while(current != queue->sentinel){
                RECIPE_STATE* state = (RECIPE_STATE* )current->recipe->state; 
                kill(state->chef_pid, SIGKILL);  
                current = current->next; 
            }   
            //After sigkilling all of them now let's reap all of it
            int status; 
            pid_t finished_step; 
            while((finished_step = wait(&status)) > 0){
                //DO NOTHING
            } 
        } 
        free_cookbook(ckb); 
        free_queue(queue); 
        free_queue(used_recipes); 
        return -1;
    } 
    return 0; 
}

