#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define INPUT "<"
#define OUTPUT ">"
#define BACKGROUND "&"
#define COMMENT "#"

// global volatile variable for handling SIGTSTP
volatile sig_atomic_t got_signal = 0;


/* struct for user entered commands */
struct commandLine { 
    char **args;
    char *inputFile;
    char *outputFile;
    int background;
    int comment;
};


// parse the user entered command
struct commandLine *parseCommand(char *command){

    // allocate memory for whole struct
    struct commandLine *currCommand = malloc(sizeof(struct commandLine));
    // allocate memory for args array
    currCommand->args = (char**)malloc(513*sizeof(char*));

    char *saveptr;
    int i = 0;
    // have to initiate background to 0
    currCommand->background = 0;

    // put command as first element in args array
    char *token = strtok_r(command, " ", &saveptr);
    currCommand->args[i] = calloc(strlen(token) + 1, sizeof(char));
    // if line starts with #, line is comment
    if (strncmp(COMMENT, token, 1) == 0 || token == NULL){
        currCommand->comment = 1;
        return currCommand;
    }
    strcpy(currCommand->args[i], token);
    i++;

    int test = 0;
    int inputTest;
    int outputTest;
    int backgroundTest;

    // parse the rest of the users command
    while (token != NULL){
        if (test == 0){
            token = strtok_r(NULL, " ", &saveptr);
            }
        if (token != NULL){
            // this is for checking the '&' being the last argument in the command.    
            test = 0;
            // if the next token is an input file
            inputTest = strcmp(INPUT, token);
            if (inputTest == 0){
                token = strtok_r(NULL, " ", &saveptr);
                currCommand->inputFile = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currCommand->inputFile, token);
                continue;
            }
            // if the next token is an output file
            outputTest = strcmp(OUTPUT, token);
            if (outputTest == 0){
                token = strtok_r(NULL, " ", &saveptr);
                currCommand->outputFile = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currCommand->outputFile, token);
                continue;
            }
            // if the next token is potentially a background command
            backgroundTest = strcmp(BACKGROUND, token);
            if (backgroundTest == 0){
                token = strtok_r(NULL, " ", &saveptr);
                if (token == NULL){
                    currCommand->background = 1;
                    continue;
                } else {
                    currCommand->args[i] = calloc(2, sizeof(char));
                    currCommand->args[i] = "&";
                    i++;
                    test = 1;
                    continue;
                }
            }
            // add token to args 
            if (token != NULL){
                currCommand->args[i] = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currCommand->args[i], token);
                i++;
            }
        }
    }
    return currCommand;
}




// This function executes the user given command and handles input, output, and background info
int executeCommand(struct commandLine* currCommand){


    // if inputFile is specified
    if(currCommand->inputFile != NULL){
        // Open source file
        int sourceFD = open(currCommand->inputFile, O_RDONLY);
        if (sourceFD == -1) { 
            printf("cannot open %s for input \n", currCommand->inputFile); 
            exit(1); 
        }

        // Redirect stdin to source file
        int result = dup2(sourceFD, 0);
        if (result == -1) { 
            perror("source dup2()"); 
            exit(2); 
        }
    }

    // if outputFile is specified
    if(currCommand->outputFile != NULL){
        // Open target file
        int targetFD = open(currCommand->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if (targetFD == -1) { 
            perror("target open()"); 
            exit(1); 
        }
    
        // Redirect stdout to target file
        int result = dup2(targetFD, 1);
        if (result == -1) { 
            perror("target dup2()"); 
            exit(2); 
        }
    }

    // if background is true and no specified outputfile is given, output is /dev/null (output is muted bc its background process)
    if(currCommand->background == 1){
        // if output not specified
        if(currCommand->outputFile == NULL){
            // Open target file
            int targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (targetFD == -1) { 
                perror("target open()"); 
                exit(1); 
            }
        
            // Redirect stdout to target file
            int result = dup2(targetFD, 1);
            if (result == -1) { 
                perror("target dup2()"); 
                exit(2); 
            }
        }

        // if input not specified
        if(currCommand->inputFile == NULL){
            // Open source file
            int sourceFD = open("/dev/null", O_RDONLY);
            if (sourceFD == -1) { 
                perror("source open()"); 
                exit(1); 
            }

            // Redirect stdin to source file
            int result = dup2(sourceFD, 0);
            if (result == -1) { 
                perror("source dup2()"); 
                exit(2); 
            }
        }

    }

    // execute the command
    execvp(currCommand->args[0], currCommand->args);
    perror(currCommand->args[0]);
    exit(2);
    
    return 2;
}


void handle_SIGTSTP(int sig){
    if(got_signal == 0){
        char* message = "\nEntering foreground-only mode (& is now ignored)\n:";
        write(1, message, 52);
        got_signal = 1;
    } else {
        char* message = "\nExiting foreground-only mode\n:";
        write(1, message, 32);
        got_signal = 0;
    }
}


// I couldnt get fflush(stdin) to do anything about stopping an infinite loop, so I found this on stack overflow
// https://stackoverflow.com/questions/53474700/c-scanf-did-not-stop-in-infinite-while-loop
void empty_stdin(void) {
    int c = getchar();

    while (c != '\n' && c != EOF)
        c = getchar();
}


int main(){
	
    char response[2049];
    int backgroundPids[1000] = { 0 };
    int foregroundStatus = 0;
    int childStatus;
    int statusMessage;

    // SIGINT handling
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // SIGTSTP handling
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    while(1){
        
        // check for terminated background processes
        pid_t childCheck = waitpid(-1, &childStatus, WNOHANG);
        if (childCheck != 0 && childCheck != -1){
            while(2){
                // check backgroundPid array to verify that ChildCheck is a background process
                for(int i = 0; i < 1000; i++){
                    if(backgroundPids[i] == childCheck){
                        if(WIFEXITED(childStatus)){
                            printf("Child %d exited normally with status %d\n", childCheck, WEXITSTATUS(childStatus));
                        } else{
                            printf("Child %d exited abnormally due to signal %d\n", childCheck, WTERMSIG(childStatus));
                        }
                        fflush(stdout);
                        backgroundPids[i] = 0;
                        break;
                    }
                }
                childCheck = waitpid(-1, &childStatus, WNOHANG);
                if (childCheck == 0 || childCheck == -1){
                    break;
                }
            }
        }

        // New line for user input
        printf(": ");
        fflush(stdout);
        scanf("%[^\n]s", &response);
        // without this empty_stdin function, the loop would be infinite
        empty_stdin();


        // exit command
        if (strcmp(response, "exit")==0){
            killpg(getpid(), SIGTERM);
            return 0;
        }

        // comment line 
        if (strlen(response) == 0 || response == NULL){
            continue;
        }

        else {

            struct commandLine *parsedResponse = parseCommand(response);

            // if SIGTSTP is recieved, no background processes will be handled
            if (got_signal == 1){
                parsedResponse->background = 0;
            }

            // if comment line, just ignore and go to next line
            // if (strlen(response) == 0 || response == NULL){
            //     parsedResponse->comment = 1;
            // }
            if (parsedResponse->comment == 1){
                continue;
            }

            // cd command
            if (strcmp(parsedResponse->args[0], "cd") == 0){
                if (parsedResponse->args[1] == NULL){
                    char *home = getenv("HOME");
                    chdir(home);
                    continue;
                } else {
                    chdir(parsedResponse->args[1]);
                    continue;
                }
            }

            // status command
            if (strcmp(parsedResponse->args[0], "status")==0){
                // if statusMessage == 0, its a regular exit, else it was terminated by a signal
                if (statusMessage == 0){
                    printf("exit value %d \n", foregroundStatus);
                } else {
                    printf("terminated by signal %d \n", foregroundStatus);
                }
                fflush(stdout);
            }

            // execute non built-in commands
            else {
                pid_t spawnPid = fork();
                switch(spawnPid){
                    case -1:
                        perror("fork()\n");
                        exit(1);
                        break;
                    case 0:

                        // if command is a foreground process, we want SIG_INT to use its default behavior
                        if (parsedResponse->background == 0){
                            // SIGINT handling
                            struct sigaction SIGINT_action = {0};
                            SIGINT_action.sa_handler = SIG_DFL;
                            sigfillset(&SIGINT_action.sa_mask);
                            SIGINT_action.sa_flags = 0;
                            sigaction(SIGINT, &SIGINT_action, NULL);
                        }

                        // SIGSTP handling
                        struct sigaction SIGTSTP_action = {0};
                        SIGTSTP_action.sa_handler = SIG_IGN;
                        sigfillset(&SIGTSTP_action.sa_mask);
                        SIGTSTP_action.sa_flags = 0;
                        sigaction(SIGTSTP, &SIGTSTP_action, NULL);


                        executeCommand(parsedResponse);
                    default:
                        // if foreground process
                        if (parsedResponse->background == 0){
                           pid_t childPid = waitpid(spawnPid, &childStatus, 0);
                           // set status after every foregound process completes 
                           if(WIFEXITED(childStatus) && childPid != -1){
                                // statusMessage = 0 means exited normally
                                statusMessage = 0;
                                foregroundStatus = WEXITSTATUS(childStatus);
                            } else {
                                // statusMessge = 1 means terminted by signal
                                printf("Terminated by signal %d\n", WTERMSIG(childStatus));
                                fflush(stdout);
                                statusMessage = 1;
                                foregroundStatus = WTERMSIG(childStatus);
                            }
                        } 
                        // if background process
                        else {
                            printf("background pid is %d\n", spawnPid);
                            // add background Pid to backgroundPid array
                            for(int i = 0; i < 1000; i++){
                                if(backgroundPids[i] == 0){
                                    backgroundPids[i] = spawnPid;
                                    break;
                                }
                            }
                            fflush(stdout);
                        }
                }

            }

        }
    }


	return 0;
}