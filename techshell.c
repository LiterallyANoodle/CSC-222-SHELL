/*
Name: Matthew Mahan
Date: 2/20/2022
Desc: Program is a custom shell with file redirection and other basic features. 
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	
	// Features covered by driver code:
	// 1. repeated input 
	// 4. informative shell prompt 

	// variables 
	char userInput[256] = "";
	int tokPipe[2]; // pipe for tokenizer 
	pid_t tpid;

	// steps to receive from tokenizer:
	// 1. create pipe
	// 2. fork

	// 3(child). execvp()
	// 4(child). write()
	// 5(child). close()
	// 6(child). exit()

	// 3(parent). read()
	// 4(parent). close()
	// 5(parent). wait()


	// input loop
	while (strcmp(userInput, "exit") != 0) {



	}



	return 0;
}