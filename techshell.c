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

#define STR_BUFFER 256
#define COMMAND_MAX 32 // maximum allowed tokens in any single-line command

void tokenizer(char* userInput, char** result);


int main(int argc, char *argv[])
{
	
	// Features covered by driver code:
	// 1. repeated input 
	// 4. informative shell prompt 

	// variables 
	char userInput[STR_BUFFER] = "";
	char* tokens[COMMAND_MAX];
	char CWD[STR_BUFFER];
	int pipefd[2]; // pipe file descriptor 
	pid_t pid;
	char* testText = "testing dup2 and pipe1\n";


	// step 1: 
	// make the input space informative
	printf("%s$ ", getcwd(CWD, STR_BUFFER));

	// step 2: 
	// wait for input and store it in userInput
	fgets(userInput, 256, stdin);

	// step 3: 
	// tokenize the input
	tokenizer(userInput, tokens);

	for (int i = 0; i < COMMAND_MAX; i++) {
		printf("%s\n", tokens[i]);
	}


	// steps to receive from tokenizer:
	// 1. create pipe
	// 2. fork

	if (pipe(pipefd) == -1) {
		printf("pipe failure\n");
	}

	pid = fork();

	// 3(child). dup2()
	// 4(child). execvp()
	// 5(child). close() 
	// 6(child). write()
	// 7(child). exit()

	if (pid == 0) {

		// char* testText = "testing dup2 and pipe1\n";

		// int out = open(stdout, O_WRONLY);
		// 1 is the file descriptor for stdout 
		dup2(pipefd[1], STDOUT_FILENO); 
		close(pipefd[0]);
		
		// write(pipefd[1], testText, STR_BUFFER);
		// printf("testing dup2 and pipe2 also sizeof:\n");
		// write(pipefd[1], testIntp, sizeof(int));

		char* args[] = {"it do not matter", NULL};
		execvp("hello", args);

		close(pipefd[1]);
		exit(0);

	}

	// 3(parent). read()
	// 4(parent). close()
	// 5(parent). wait()

	else {

		wait(0);
		printf("test\n");
		close(pipefd[1]);

		int readBytes = read(pipefd[0], userInput, STR_BUFFER);
		

		close(pipefd[0]);
		printf("%d\n", readBytes);

		if (readBytes >= 0) {
			fflush(stdout);
			printf("read success\n");
			// fflush(stdout);
			printf("%s\n", userInput);
		} else {
			printf("read failure\n");
		}

		

	}



	// input loop
	// while (strcmp(userInput, "exit") != 0) {



	// }



	return 0;
}


void tokenizer(char* userInput, char** result) {

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

	// step 5 
	// say how many tokens were counted 
	printf("%d token(s) read.\n", tokCount);

	return;

}