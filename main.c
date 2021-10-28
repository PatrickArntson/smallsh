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
    currCommand->args = (char**)malloc(512*sizeof(char*));

    char *saveptr;

    // get command 
    char *token = strtok_r(command, " ", &saveptr);
    currCommand->command = calloc(strlen(token) + 1, sizeof(char));
    // if line starts with #, line is comment
    if (strncmp(COMMENT, token, 1) == 0){
        currCommand->comment = 1;
        return currCommand;
    }
    strcpy(currCommand->command, token);

    int test = 0;
    int i = 0;
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




int main(){
	



    printf(": ");
    char response[2048];
    scanf("%[^\n]s", response);

    struct commandLine *parsedResponse = parseCommand(response);

    printf("%d \n", parsedResponse->comment);


    printf("%s \n", parsedResponse->command);


    for (int i = 0; parsedResponse->args[i] != '\0'; i++){
        printf("%s \n", parsedResponse->args[i]);
    }

    printf("%s \n", parsedResponse->inputFile);

    printf("%s \n", parsedResponse->outputFile);

    printf("%d \n", parsedResponse->background);




	return 0;
}