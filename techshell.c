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
// void cmdPositioner(int cmdList, char** tokens, int tokCount, int* delimiterPositions, int numDelimiters, int numFilenames);

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

		flowHandler(tokens, tokCount);

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
			printf("Please specify a directory to change to.\nUsage: cd [path] \n");
		} 
		else if (chdir(tokens[1]) == -1) {
			printf("Error: Invalid path.\nUsage: cd [path] \n");
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

	int cmdList[# of delimiters + 1 - # of filenames][2];

	*/

	// variables (many of these represent indeces as explained above)
	int INfilename;
	int OUTfilename;
	int numFilenames = 0;
	int numPipes = 0;
	int numDelimiters = 0;
	int delimiterPositions[COMMAND_MAX];

	// loop through every token to look for delimiting tokens
	// essentially, this loop identifies and separates everything
	for (int i = 0; i < tokCount; i++) {

		// firstly, count the number of commands, files, and redirects present
		// this way, memory for the necessary number of pipes can be prepared beforehand
		// i think only pipes can vary, because there will only be one file in and one file out, right?
		// *******REMEMBER TO free() THE MEMORY FOR THESE PIPES WHEN ITS DONE*******
		switch (tokens[i][0]) {
			case '<' :
				if (i - 1 >= 0) {
					delimiterPositions[numDelimiters] = i;
					numDelimiters++;
					numFilenames++;
				} else {
					printf("Error: Invalid syntax.\nUsage: [command] < [path]\n");
					return; 
				}
				if (i + 1 <= tokCount - 1) {
					INfilename = i + 1;
				} else {
					printf("Error: Invalid syntax.\nUsage: [command] < [path]\n");
					return; 
				} 
				if (delimiterPositions[i] + 2 <= tokCount - 1) {
					if (delimiterPositions[i] + 2 != delimiterPositions[i+1]) {
						printf("Error: Invalid syntax.\nUsage: [command] < [path]\n");
						return;
					}
				}
				break; 

			case '>' :
				if (i - 1 >= 0) {
					delimiterPositions[numDelimiters] = i;
					numDelimiters++;
					numFilenames++;
				} else {
					printf("Error: Invalid syntax.\nUsage: [command] > [path]\n");
					return;
				}
				if (i + 1 <= tokCount - 1) {
					OUTfilename = i + 1;
				} else {
					printf("Error: Invalid syntax.\nUsage: [command] > [path]\n");
					return; 
				} 
				if (delimiterPositions[i] + 2 <= tokCount - 1) {
					printf("Error: Invalid syntax.\nUsage: [command] > [path]\n");
					return;
				}
				break; 

			case '|' : 
				if (i - 1 >= 0) {
					delimiterPositions[numDelimiters] = i;
					numDelimiters++;
					numPipes++;
				} else {
					printf("Error: Invalid syntax.\nUsage: [command] | [command]\n");
					return;
				}
				if (i + 1 <= tokCount - 1) {
					// after modifying this switch case, idk what to put here lol
					if (DEBUG)
						printf("Valid syntax. :)\n");
				} else {
					printf("Error: Invalid syntax.\nUsage: [command] | [command]\n");
					return;
				}
				break; 

			// token is just another arg for a command or filename
			default :
				break; 

		}


	}

	// I just wanna say here at this debug that 
	// I wrote this entire switch case in one go and it worked first try*
	// *i had one small syntax error but it was an easy fix
	// Finally I am success :')
	if (DEBUG) {

		printf("numDelimiters: %d\n", numDelimiters);

		for (int i = 0; i < numDelimiters; i++) {

			printf("delimiterPositions[%d]: %d\n", i, delimiterPositions[i]);
			printf("Delimiter at postion %d: %s\n", delimiterPositions[i], tokens[delimiterPositions[i]]);

		}

	}

	// Now that the delimiter positions are known AND the # of filenames,
	// we can calculate both the number of commands present AND where they 
	// start and end.
	int rowWidth = (numDelimiters - numFilenames + 1);
	int cmdList[(rowWidth * 2)];
	// cmdPositioner(cmdList, tokens, tokCount, delimiterPositions, numDelimiters, numFilenames);

	// this was originally a 2D array like cmdList[cmd index][tokens index]
	// however 2D arrays cannot be passed to functions so instead 
	// cmdList[cmd index][tokens index] = cmdList[cmd index + tokens index * width of one row]
	// this is functionally the same, just a little syntax
	cmdList[0 + 0*rowWidth] = 0;
	int numInReads = 0; // need this to help with some indexing issues

	for (int i = 0; i < numDelimiters; i++) {

		// commands exist on either side of | 
		if (tokens[delimiterPositions[i]][0] == '|') {
			if (i >= 1) {
				// this is that weird syntax popping up again
				// since < can come after a command it inputs to
				if (tokens[delimiterPositions[i-1]][0] == '<') {
					// m has already been stored for the last command
					// store n for the next command
					cmdList[i+1-numInReads + 0*rowWidth] = delimiterPositions[i] + 1;
				} else {
					// store m for the last command
					cmdList[i-numInReads + 1*rowWidth] = delimiterPositions[i] - 1;
					// store n for the next command
					cmdList[i+1-numInReads + 0*rowWidth] = delimiterPositions[i] + 1;
				}
			} else {
				// store m for the last command
				cmdList[i-numInReads + 1*rowWidth] = delimiterPositions[i] - 1;
				// store n for the next command
				cmdList[i+1-numInReads + 0*rowWidth] = delimiterPositions[i] + 1;
			}
		} 
		// commands exist on the left side of <
		// a filename exists on the right side of < (already accounted for)
		else if (tokens[delimiterPositions[i]][0] == '<') {
			// store m for the last command 
			cmdList[i + 1*rowWidth] = delimiterPositions[i] - 1;
			// how annoying >:(
			numInReads++;
		} 
		// commands MAY exist on the left side of >
		// a filename exists on the right side of > (already accounted for)
		else if (tokens[delimiterPositions[i]][0] == '>') {
			if (i >= 1) {
				// this is that weird syntax popping up again
				// since < can come after a command it inputs to
				if (tokens[delimiterPositions[i]][0] == '<') {
					// m has already been stored for the last command
					// there is no need to store n for the next command
					if (DEBUG) {
						printf("Found >\n");
					}
				} else {
					// store m for the last command 
					cmdList[i-numInReads + 1*rowWidth] = delimiterPositions[i] - 1;
					// there is no need to store n for the next command
				}
			} else {
				// store m for the last command 
				cmdList[i-numInReads + 1*rowWidth] = delimiterPositions[i] - 1;
				// there is no need to store n for the next command
			}
		}

	}

	// if the last delimiter is < or >, 
	// then the m for the last cmd has been stored 
	// if there are no delimiters or the last delimiter is a |, 
	// then the m for the last cmd is the last token 
	if (numDelimiters == 0) {
		cmdList[0 + 1*rowWidth] = tokCount - 1; 
	} else {
		if (tokens[delimiterPositions[numDelimiters-1]][0] == '|') {
			cmdList[numDelimiters - numFilenames + 1*rowWidth] = tokCount - 1;
		}
	}

	if (DEBUG) {

		for (int i = 0; i < numDelimiters - numFilenames + 1; i++)
		{
			printf("Command #%d\n", i);
			printf("Start index: %d\n", cmdList[i + 0*rowWidth]);
			printf("End index: %d\n", cmdList[i + 1*rowWidth]);
		}

	}

}

