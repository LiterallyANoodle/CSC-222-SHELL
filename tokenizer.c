/*
Name: Matthew Mahan
Date: 1/14/2022
Desc: Program continuously takes user input and returns the space-separated tokens that were typed. 
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	
	// declare variables
	char userInput[256];
	char* tempInput = (char *)malloc(256 * sizeof(char));
	char* token;
	int tokCount;

	// when "exit" is typed, stop the program
	while (userInput != "exit") {

		// step 1: 
		// make the input space pretty 
		printf("$ ");

		// step 2: 
		// wait for input and store it in userInput
		fgets(userInput, 256, stdin);

		strcpy(tempInput, userInput);

		token = strtok(userInput, " ");

		// step 2.5
		// check if the user wants to exit 
		// printf("token is %s and strcmp %d", token, strcmp(token, "exit\n"));
		if (strcmp(token, "exit\n") == 0) {
			printf("\n");
			break; 
		}

		// step 3:
		// read back the line to the user
		printf("Line read: %s\n", tempInput);

		// a little test 
		// it seems that userInput is changed into the current token
		// as a side effect of strtok
		// token = strtok(userInput, " ");
		// while (token != NULL) {
		// 	printf("%s\n", token);	
		// 	token = strtok(NULL, " ");
		// 	printf("userInput is %s\n", userInput);
		// }
		// printf("passed up lol token is %s\n", token);

		// step 4
		// Say what the tokens are and count them
		tokCount = 0;
		printf("Token(s): \n");
		

		

		while (token != NULL) {

			// print the token read
			printf(" %s\n", token);

			// count the token just printed
			tokCount++; 

			// get the next token 
			token = strtok(NULL, " ");

		}

		// step 5 
		// say how many tokens were counted 
		printf("%d token(s) read.\n", tokCount);

	} 


	return 0;
}