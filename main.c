#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

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

    // get command (putting command as first ele in args array instead of own struct ele currently)
    char *token = strtok_r(command, " ", &saveptr);
    currCommand->args[i] = calloc(strlen(token) + 1, sizeof(char));
    // currCommand->command = calloc(strlen(token) + 1, sizeof(char));
    // if line starts with #, line is comment
    if (strncmp(COMMENT, token, 1) == 0){
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
    pid_t spawnpid;


    while(1){
        waitpid()
        printf(": ");
        fflush(stdout);
        scanf("%[^\n]s", &response);
        empty_stdin();

        // exit command
        if (strcmp(response, "exit")==0){
            break;
        }

        // comment line
        if (strlen(response) == 0 || response == NULL){
            printf("comment worked\n");
            fflush(stdout);
            break;
        }

        else {
            struct commandLine *parsedResponse = parseCommand(response);

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
            }


            // printf("%d \n", parsedResponse->comment);
            // fflush(stdout);

            // printf("%s \n", parsedResponse->command);
            // fflush(stdout);
            // for (int i = 0; parsedResponse->args[i] != '\0'; i++){
            //     printf("%s \n", parsedResponse->args[i]);
            //     fflush(stdout);
            // }

            // printf("%s \n", parsedResponse->inputFile); 
            // fflush(stdout);

            // printf("%s \n", parsedResponse->outputFile);
            // fflush(stdout);

            // printf("%d \n", parsedResponse->background);
            // fflush(stdout);
            // continue;

        }
    }


	return 0;
}