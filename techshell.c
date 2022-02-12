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
void pwdCheck(char** tokens, char* CWD);
void cdCheck(char** tokens, int tokCount); 
void flowHandler(char** tokens, int tokCount); 

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
		pwdCheck(tokens, CWD);
		cdCheck(tokens, tokCount);

		if (DEBUG) {
			for (int i = 0; i < tokCount; i++) {
				printf("%s\n", tokens[i]);
			}
		}

	}

	if (DEBUG)
		printf("exited\n");

	char* args[5] = {"wc", "techshell.c"};

	execvp("wc", args);


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

void pwdCheck(char** tokens, char* CWD) {

	if (strncmp(tokens[0], "pwd", 3) == 0) {
		printf("%s\n", CWD);
	}

}

void cdCheck(char** tokens, int tokCount) {


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


}

void flowHandler(char** tokens, int tokCount) {

	/*
	priority of I/O symbol checking:
	<, <<, |, >, >>
	<< and >> may not be implemented, depends if i have time 
	note: > and >> will not check for further commands. They signal the end of the sequence.
	< has strange syntax because it comes AFTER the command but signifies input
	*/

	// variables
	/*
	The variable tokens[] is an un-parsed array of strings. 
	It contains the entire line that the user has input, divided into words.
	Each token is a word which is either part of the arguments for a command, 
	the name of a file to be used for IO redirection, 
	or a symbol for IO redirection syntax.

	A command is just a series of strings stored in tokens[],
	so it isn't necessary to make a new array to copy each command again.
	After all, we already have them conventiently stored in tokens[]!
	Instead, a command can just be defined as a certain range of values
	in tokens[]. 

	For example, if I input something like "ls -all | wc -l", 
	then cmd1 will be the range from tokens[0] to tokens[1]. 
	Similarly, cmd2 will be the range from tokens[3] to tokens[4]. 

	So a command can be defined as the range of tokens n to m (inclusive)
	where n is the index of the first token containing part of that command and 
	m is the last index which contains part of the command.

	In this way, the redirection syntax acts sort of as a delimiter. 
	The <, >, and | characters denote the end of a command, 
	and in the case of |, it denotes a command will immediately follow. 
	For <, it denotes the next token will be fileIN, and futher delimiters may 
	also follow. 
	For >, it denotes the next token will be fileOUT, and further delimiters 
	will NOT follow, so the entire sequence may be closed and executed. 

	No matter what the delimiter token looks like, the result is the same:
	The index immediately preceeding the index of this delimiter is the index
	for token m of the previous command!
	Likewise, the index immediately following this index will be the 
	index of n for the command which immediately follows the delimiter!

	So following this logic, a command will be defined as;
	
	int cmd[2] = {n, m};

	and to hold our whole set of commands:

	int cmd[# of delimiters + 1][2]

	*/

	// loop through every token 
	for (int i = 0; i < tokCount, i++) {

		// firstly, count the number of commands, files, and redirects present
		// this way, memory for the necessary number of pipes can be prepared beforehand
		// i think only pipes can vary, because there will only be one file in and one file out, right?
		// *******REMEMBER TO free() THE MEMORY FOR THESE PIPES WHEN ITS DONE*******


	}

}