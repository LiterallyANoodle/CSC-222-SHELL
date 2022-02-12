/*
Name: Matthew Mahan
Date: 2/20/2022
Desc: Program is a custom shell with file redirection and other basic features. 
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

#define STR_BUFFER 256
#define COMMAND_MAX 32 // maximum allowed tokens in any single-line command

#define DEBUG true

int tokenizer(char* userInput, char** result);


int main(int argc, char *argv[])
{
	
	// Features covered by driver code:
	// 1. repeated input 
	// 4. informative shell prompt 

	// variables 
	char userInput[STR_BUFFER] = "";
	char* tokens[COMMAND_MAX];
	char* exit = "exit";
	char CWD[STR_BUFFER];
	
	int tokCount = 0;
	int pipefd[2]; // pipe file descriptor 
	pid_t pid;
	
	char* testText = "testing dup2 and pipe1\n";

	// loop to repeatedly take input
	tokens[0] = "";
	while (strncmp(tokens[0], exit, 4) != 0) {

		if (DEBUG) {
			printf("%s\n", tokens[0]);
			printf("%d\n", strncmp(tokens[0], exit, 4));
		}

		// step 1: 
		// make the input space informative
		printf("%s$ ", getcwd(CWD, STR_BUFFER));

		// step 2: 
		// wait for input and store it in userInput
		fgets(userInput, 256, stdin);

		// step 3: 
		// tokenize the input
		tokCount = tokenizer(userInput, tokens);

		// step 4:
		// check for special non-exit commands: pwd, cd
		if (strncmp(tokens[0], "pwd", 3) == 0) {
			printf("%s\n", CWD);
		}

		if (strncmp(tokens[0], "cd", 2) == 0) {

			if (tokCount < 2) {
				printf("Please specify a directory to change to.\nUsage: cd <path> \n");
			} 
			else if (chdir(tokens[1]) == -1) {
				printf("Error: Invalid path.\nUsage: cd <path> \n");
				if (DEBUG) {
					perror("Invalid path");
				}
			}

		}

		if (DEBUG) {
			for (int i = 0; i < tokCount; i++) {
				printf("%s\n", tokens[i]);
			}
		}

	}

	if (DEBUG)
		printf("exited\n");


	return 0;
}

// this is copied from the old assignment and modified to fit 
int tokenizer(char* userInput, char** result) {

	// declare variables
	// char* result[COMMAND_MAX];
	char* token;
	int tokCount;



	token = strtok(userInput, " ");

	// step 4
	// Say what the tokens are and count them
	tokCount = 0;

	int i = 0;
	while (token != NULL) {

		// add the token read to an array
		result[i] = token;

		// count the token just printed
		tokCount++; 
		i++;

		// get the next token 
		token = strtok(NULL, " ");

	}

	// remove a trailing space token if there is one 
	if (isspace(result[tokCount - 1][0]) != 0) {
		result[tokCount - 1] = NULL; 
		tokCount--; 
		if (DEBUG)
			printf("removed trailing space\n");
	}

	// it seems these tokens take the form "word\n\0" when they are the last token,
	// so I have to manually remove that \n to clean these up 
	// since strlen() returns the len without the null byte,
	// the position of \n is going to be len - 1
	if (result[tokCount - 1][strlen(result[tokCount-1]) - 1] == '\n') {
		// make it a null byte (hoping this doesn't cause errors)
		result[tokCount - 1][strlen(result[tokCount-1]) - 1] = '\0';
	}

	// step 5 
	// say how many tokens were counted 
	if (DEBUG) {
		printf("%d token(s) read.\n", tokCount);	
	}
	

	return tokCount;

}