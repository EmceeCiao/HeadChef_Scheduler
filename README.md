![github repo badge: Language](https://img.shields.io/badge/Language-C-181717?color=red) ![github repo badge: Testing](https://img.shields.io/badge/Testing-Criterion-181717?color=orange)

# HeadChef_Scheduler

HeadChef_Scheduler is a multi-processing C project created for my System Fundamentals II class meant to be similar to make but with a cooking analogy instead! It's meant to not only schedule but execute tasks concurrently for recipes. We were provided cookbook.h, the cookbook parser, and the cookbook tests, otherwise, the implementation of anything found in src is mine. 

## Design/Features  

Once the passed in cookbook is parsed, we perform a dependency analysis on it, initializing a task queue for the main process with the "recipes" that hold no dependencies. This main process or "head chef" then reads through this task queue, creating sub-processes directing the "chefs" underneath it to tackle these recipes. To properly handle the concurrency, we only let this "head chef" edit the task queue once the "chefs" underneath finish, updating it with new "recipes" that can be done since their dependencies are done. Without having this system of the "head chef" writing and the other "chefs" reading, we would run into many race conditions between the two. 

To update the recipe queue efficiently, I made the design decision of keeping track of the number of dependencies

## Usage 

The following command is used to run the program: 

```bin/cook [-f cookbook] [-c max_cooks] [main_recipe_name]``` 

These are all optional arguments, cookbook representing the cookbook being parsed, max_cooks being the max number of concurrent processes that can be run, and main_recipe_name being the recipe being cooked. 

If no cookbook is passed in it looks for a cookbook.ckb file in the root directory of the program. 
If no max_cooks is passed in it assumes 1. 
If no main_recipe is passed in it assumes the 1st recipe of the cookbook is the main recipe. 

As the recipe starts recipes and finishes it will print out when the recipe started and when it finished. This is not actually tracking the time the step took to finish, it is under a random timer in tenths of a second.  

For creating a cookbook keep in mind the format should be the following: 

```
fake_recipe: sub_recipe1 sub_recipe2
  task 1
  task 2
  task 3

fake_recipe2:
   step 1 | step 2 | step 3
```
Each task in the recipe can have 1 or more steps, with each step being divided by a vertical bar indicating piping the output of one step to the next. 

## Building & Testing

HeadChef_Scheduler can be built using the provided make files and running ```make clean && make all```. 

The testing framework used was [criterion](https://github.com/Snaipe/Criterion), so this must be installed before attempting to run the tests using: 

```bin/cook_tests```

## Acknowledgements 

A lot of my understanding of multi-processing and figuring out such an implementation can be attributed to the excellent explanations that can be found in [*Computer Systems: A Programmer's Perspective*](http://csapp.cs.cmu.edu/3e/home.html) by Randal E. Bryant and David R. O'Hallaron

