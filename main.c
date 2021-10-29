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


/* struct for user entered commands */
struct commandLine {
    char *command; 
    char **args;
    char *inputFile;
    char *outputFile;
    int background;
    int comment;
};




// parse the user entered 
struct commandLine *parseCommand(char *command){

    // allocate memory for whole struct
    struct commandLine *currCommand = malloc(sizeof(struct commandLine));
    // allocate memory for args array
    currCommand->args = (char**)malloc(513*sizeof(char*));

    char *saveptr;
    int i = 0;
    // have to initiate background to 0
    currCommand->background = 0;

    // get command (putting command as first ele in args array instead of own struct ele currently)
    char *token = strtok_r(command, " ", &saveptr);
    currCommand->args[i] = calloc(strlen(token) + 1, sizeof(char));
    // currCommand->command = calloc(strlen(token) + 1, sizeof(char));
    // if line starts with #, line is comment
    if (strncmp(COMMENT, token, 1) == 0 || token == NULL){
        currCommand->comment = 1;
        return currCommand;
    }
    // strcpy(currCommand->command, token);
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
            // maybe dont use printf here? maybe use write function if issue arises
            printf("cannot open %s for input \n", currCommand->inputFile); 
            exit(1); 
        }
        // // Written to terminal
        // printf("sourceFD == %d\n", sourceFD); 

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

        // // Written to terminal
        // printf("targetFD == %d\n", targetFD); 
    
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

            // // Written to terminal
            // printf("targetFD == %d\n", targetFD); 
        
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
            // // Written to terminal
            // printf("sourceFD == %d\n", sourceFD); 

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





// I couldnt get fflush(stdin) to do anything about stopping an infinite loop, so I found this on stack overflow
// https://stackoverflow.com/questions/53474700/c-scanf-did-not-stop-in-infinite-while-loop
void empty_stdin(void) {
    int c = getchar();

    while (c != '\n' && c != EOF)
        c = getchar();
}


int main(){
	
    char response[2049];
    int foregroundStatus = 0;
    int *statusptr;
    statusptr = &foregroundStatus;
    pid_t spawnPid;
    int childStatus;
    pid_t *backgroundPid;

    // SIGINT handling
    struct sigaction SIGINT_action = {0};

    // Fill out the SIGINT_action struct
    // Register SIG_IGN as the signal handler
    SIGINT_action.sa_handler = SIG_IGN;
    // Block all catchable signals while SIG_IGN is running
    sigfillset(&SIGINT_action.sa_mask);
    // No flags set
    SIGINT_action.sa_flags = 0;
    // Install our signal handler
    sigaction(SIGINT, &SIGINT_action, NULL);

    while(1){
    
        pid_t childCheck = waitpid(-1, &childStatus, WNOHANG);
        if (childCheck != 0 && childCheck != -1){
            while(1){
                printf("Background pid %d is done \n", childCheck);
                fflush(stdout);
                childCheck = waitpid(0, &childStatus, WNOHANG);
                if (childCheck == 0 || childCheck == -1){
                    break;
                }
            }
        }
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

        // comment line command
        if (strlen(response) == 0 || response == NULL){
            continue;
        }

        else {

            struct commandLine *parsedResponse = parseCommand(response);

            // if multiple spaces (i.e. comment line)
            if (parsedResponse->comment == 1){
                continue;
            }

            // cd command
            if (strcmp(parsedResponse->args[0], "cd") == 0){
                // foregroundStatus = 5;
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
                printf("%d \n", *statusptr);
                fflush(stdout);
                // continue;
            }

            // execute other commands
            else {
                pid_t childPid = fork();
                switch(childPid){
                    case -1:
                        perror("fork()\n");
                        exit(1);
                        break;
                    case 0:
                        executeCommand(parsedResponse);
                    default:
                        // if foreground process
                        if (parsedResponse->background == 0){
                           waitpid(childPid, &childStatus, 0); 
                           foregroundStatus = childStatus;
                        //    continue;
                        } 
                        // if background process
                        else {
                            printf("background pid is %d\n", childPid);
                            fflush(stdout);
                            // continue;
                        }
                }

            }

        }
    }


	return 0;
}