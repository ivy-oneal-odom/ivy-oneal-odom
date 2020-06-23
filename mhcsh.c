#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define PROMPT_TEXT "mhcsh> "
#define INPUT_LEN 128

char WHITESPACE[4*5] = {" \n \t \r"};
char BUILT_IN_COMMANDS[5][10] = {"exit", "cd", "pwd", "setpath", "printpath"};
int NUM_BUILT_IN_COMMANDS = 5;



char path[INPUT_LEN];			//The string which contains the path variable. I found it easier to store this as a string and tokenize it as needed, rather than store the tokens here
int pathFilled = 0;				//Whether or not the path is set to something

//Exits the shell with an exit code of 0
int my_exit(int argc, char argv[INPUT_LEN][INPUT_LEN]){
	exit(0);
}

//Change Directory to whatever is in argv[1] or getenv("HOME") if argv[1] == NULL
int cd(int argc, char argv[INPUT_LEN][INPUT_LEN]){
	if(argc == 1){
		if(chdir(getenv("HOME")) != 0){
			perror("Error in cd\n");
			return 1;
		}
	}else{
		if(chdir(argv[1]) != 0){
			perror("Error in cd\n");
			return 1;
		}
	}

	return 0;
}

//Prints the working directory
int pwd(int argc, char argv[INPUT_LEN][INPUT_LEN]){
	char wd[512];
	if(getcwd(wd, sizeof(wd)) == NULL){
		perror("Error Getting CWD\n");
		return 1;
	}
	printf("%s\n", wd);

	return 0;
}

//Sets the path variable
int setpath(int argc, char argv[INPUT_LEN][INPUT_LEN]){
	if(argc == 1){
		perror("Please specify a PATH to set\n");
		return 1;
	}

	// if(strcmp(argv[1], "PATH") == 0){		//This just makes our lives easier. If you run "setpath PATH" it will set path to $PATH, which is helpful
	// 	strcpy(path, getenv("PATH"));
	// }else{
	// 	strcpy(path, argv[1]);
	// }

	strcpy(path, argv[1]);

	pathFilled = strlen(path);		//If a non-0 new path was set, make pathFilled true

	return 0;
}

//Prints the path variable
int printpath(int argc, char argv[INPUT_LEN][INPUT_LEN]){
	if(!pathFilled){
		perror("No PATH\n");
		return 1;
	}

	printf("%s\n", path);

	return 0;
}

//Handle non-built-in commands
int nonBuiltIn(int argc, char argv[INPUT_LEN][INPUT_LEN], int foreground){
	if(!pathFilled){
		perror("Please specify a PATH\n");
		return 1;
	}else{
		pid_t pid;

		pid = fork();

		if(pid < 0){	//Error forking
			perror("Cannot Fork\n");
			return 1;
		}else if(pid > 0){			//I am the parent and should wait
			if(foreground){			//I should only wait if we're running in the foreground
				int status;
				waitpid(pid, &status, 0);
			}
		}else{				//I am the child and should execute
			//Get PATH tokens
			char pathTokens[INPUT_LEN][INPUT_LEN];		//The List of Tokens
			int numPathTokens = 0;						//The Number of Tokens

			char *pathToken;
			pathToken = strtok(path, ":");

			int pathTokenIndex = 0;

			while(pathToken != NULL){
				strcpy(pathTokens[pathTokenIndex], pathToken);
				pathTokenIndex++;
				pathToken = strtok(NULL, ":");
			}

			numPathTokens = pathTokenIndex;

			//Check for command in $PATH for the file and fork + exec if it exists
			for(int i = 0; i < numPathTokens; i++){
				char commandPath[INPUT_LEN];
				strcpy(commandPath, pathTokens[i]);
				strcat(commandPath, "/");
				strcat(commandPath, argv[0]);

				if(access(commandPath, X_OK) != -1){
					char *args[argc + 1];
					for(int j = 0; j < argc; j++){
						args[j] = argv[j];
					}

					args[argc - 1 + foreground] = '\0';		//We want to overwrite the '&' if it exists, and we want to terminate the args array with NULL

					execve(commandPath, args, 0);
				}
			}

			//If it doesn't exist in $PATH, cannot be executed, report an error but keep the shell running
			perror("Error Executing\n");
			return 1;
		}
	}

	return 0;
}

int main(int argc, char *argv[]){

	//Exit Gracefully if there are any arguments
	if(argc != 1){
		printf("Please do not specify any arguments.\n");
		return 1;
	}


	//The core loop of the shell
	while(1){
		//Print Prompt
		printf("%s", PROMPT_TEXT);

		//Get Input (max len 128 bytes)
		char input_string[INPUT_LEN];
		fgets(input_string, INPUT_LEN, stdin);

		if(strlen(input_string) != 0){		//Handle blank lines
			//Parse Input into Tokens (handling blank lines and consecutive whitespace without breaking)
			char tokens[INPUT_LEN][INPUT_LEN];		//The List of Tokens
			int numTokens = 0;						//The Number of Tokens

			char *token;
			token = strtok(input_string, WHITESPACE);

			int tokenIndex = 0;

			while(token != NULL){
				strcpy(tokens[tokenIndex], token);
				tokenIndex++;
				token = strtok(NULL, WHITESPACE);
			}

			numTokens = tokenIndex;

			//Flag Foreground/Background with a terminating '&' (foreground is true if it should be run in foreground, false otherwise)
			int foreground = 1;
			if(strcmp(tokens[numTokens - 1], "&") == 0){
				foreground = 0;
			}

			//Check if Input is Built-In Command and Execute it if it is a valid command (if it returns with an error, keep running the shell)
			int commandIndex = -1;			//Initialized to -1. If it's a built in command, it will be changed to the index of that command
			for(int i = 0; i < NUM_BUILT_IN_COMMANDS; i++){
				if(strcmp(BUILT_IN_COMMANDS[i], tokens[0]) == 0){
					commandIndex = i;
				}
			}

			if(commandIndex != -1){
				switch(commandIndex){
					case 0:
						my_exit(numTokens, tokens);
						break;
					case 1:
						cd(numTokens, tokens);
						break;
					case 2:
						pwd(numTokens, tokens);
						break;
					case 3:
						setpath(numTokens, tokens);
						break;
					case 4:
						printpath(numTokens, tokens);
						break;
				}
			}else{
				nonBuiltIn(numTokens, tokens, foreground);
			}

		}
	}

	return 0;
}