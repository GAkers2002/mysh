#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "smsh.h"

#define DFL_PROMPT      "test> "

//Call methods before main
int exeBuiltIn(char **args);
int isInBG(char **args);
int qOut = 0;
int lastCode = 0;

int main()
{
  char  *cmdline, *prompt, **arglist;
  int   result;
  void  setup();

  prompt = DFL_PROMPT ;
  setup();

  while (!qOut && (cmdline = next_cmd(prompt, stdin)) != NULL ){
    if ((arglist = splitline(cmdline)) != NULL  ){
      int handled = !exeBuiltIn(arglist);
      if(!handled){
          int bg = isInBG(arglist);
          pid_t pid;
          //Check if there is a background process
          if(bg){
            pid = fork();
            if(pid == 0){
              execvp(arglist[0], arglist);
              printf("Failed to create background process");
              exit(1);
            }
            else if(pid > 0){
              printf("Background process started. PID: %d\n", pid);
            }
            else{
              printf("Failed to create fork");
            }
          }
          else{
            lastCode = execute(arglist);
          }
      }
      freelist(arglist);
    }
    free(cmdline);

    //Check for completed background tasks
    int status;
    pid_t donePid;
    while((donePid = waitpid(-1, &status, WNOHANG)) > 0){
      printf("Background task completed. PID: %d\n", donePid);
    }
  }
  return 0;
}

int isInBG(char **args){
        //strips the & from the argument
        for(int i = 0; args[i] != NULL; i++){
                if(strcmp(args[i], "&") == 0){
                        args[i] = NULL;
                        return 1;
                }
        }
        return 0;
}

void setup()
/*
 * purpose: initialize shell
 * returns: nothing. calls fatal() if trouble
 */
{
  signal(SIGINT,  SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
}

void fatal(char *s1, char *s2, int n)
{
  fprintf(stderr,"Error: %s,%s\n", s1, s2);
  exit(n);
}

//Runs all built in commands
int exeBuiltIn(char **args){
        //return if value is null
        if(args[0] == NULL){
                return 1;
        }

        //exit
        if(strcmp(args[0], "exit") == 0){
                lastCode = args[1] ? atoi(args[1]) : 0;
                qOut = 1;
                return 0;
        }
        //cd command
        if(strcmp(args[0], "cd") == 0){
                char *dir = args[1] ? args[1] : getenv("HOME");
                if(chdir(dir) != 0){
                        printf("Error changing directories\n");
                        lastCode = 1;
                }
                else{
                        lastCode = 0;
                }
                return 0;
        }
        if(strcmp(args[0], "echo") == 0 && args[1] && strcmp(args[1], "$?") == 0$
                printf("%d\n", lastCode);
                return 0;
        }
        return 1;
}



