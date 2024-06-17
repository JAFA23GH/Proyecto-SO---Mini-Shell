// C Program to design a shell in Linux 
#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
/*
#include<readline/readline.h> 
#include<readline/history.h> */

#define MAXCOM 1000 // max number of letters to be supported 
#define MAXLIST 100 // max number of commands to be supported 

// Clearing the shell using escape sequences 
#define clear() printf("\033[H\033[J") 

// Greeting shell during startup 
void init_shell() 
{ 
	clear(); 
	printf("\n\n\n\n******************"
		"************************"); 
	printf("\n\n\n\t****MY SHELL****"); 
	printf("\n\n\t-USE AT YOUR OWN RISK-"); 
	printf("\n\n\n\n*******************"
		"***********************"); 
	char* username = getenv("USER"); 
	printf("\n\n\nUSER is: @%s", username); 
	printf("\n"); 
	sleep(1); 
	clear(); 
} 

char *read_line(void)
{
	char *line = NULL;
	size_t bufsize = 0;	

	if (getline(&line, &bufsize, stdin) == -1) /* if getline fails */
	{
		if (feof(stdin)) /* test for the eof */
		{
			free(line); /* avoid memory leaks when ctrl + d */
			exit(EXIT_SUCCESS); /* we recieved an eof */
		}
		else
		{
			free(line); /* avoid memory leaks when getline fails */
			perror("error while reading the line from stdin");
			exit(EXIT_FAILURE);
		}
	}
	return (line);
}

//Function to take input 
int takeInput(char* str) 
{ 
	char* buf; 

	buf = read_line();
	
	buf[strcspn(buf, "\n")] = '\0'; 

	if (strlen(buf) != 0) { 
		//add_history(buf); 
		strcpy(str, buf); 
		return 0; 
	} else { 
		return 1; 
	} 
} 

// Function to print Current Directory. 
void printDir() 
{ 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	printf("\nDir: %s \n>>> ", cwd); 
} 

// Function where the system command is executed 
void execArgs(char** parsed) 
{ 
	// Forking a child 
	pid_t pid = fork(); 

	if (pid == -1) { 
		printf("\nFailed forking child.."); 
		return; 
	} else if (pid == 0) { 
		if (execvp(parsed[0], parsed) < 0) { 
			printf("\nCould not execute command.."); 
		} 
		exit(0); 
	} else { 
		// waiting for child to terminate 
		wait(NULL); 
		return; 
	} 
} 

// Function where the piped system commands is executed 
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    p1 = fork();
    if (p1 < 0) {
        perror("fork");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        if (execvp(parsed[0], parsed) < 0) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent executing
        p2 = fork();

        if (p2 < 0) {
            perror("fork");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // parent executing, waiting for two children
            close(pipefd[0]);
            close(pipefd[1]);

            waitpid(p1, NULL, 0);
            waitpid(p2, NULL, 0);
        }
    }
}

void execArgsAND(char** parsed, char** parsedAND) 
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
    }

    if (pid == 0) {
        // Child process
        if (execvp(parsed[0], parsed) < 0) {
            perror("execvp failed for the first command");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // The first command executed successfully
            pid = fork();
            if (pid < 0) {
                perror("fork failed");
            }
            if (pid == 0) {
                // Child process
                if (execvp(parsedAND[0], parsedAND) < 0) {
                    perror("execvp failed for the second command");
                    exit(EXIT_FAILURE);
                }
            } else {
                // Parent process
                waitpid(pid, &status, 0);
            }
        } else {
            // The first command failed
            // fprintf(stderr, "The first command failed, so the second command will not be executed\n");
        }
    }  
} 

void execArgsOR(char** parsed, char** parsedOR) 
{
	pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        perror("fork failed");        
    }

    if (pid == 0) {
        // Child process
        if (execvp(parsed[0], parsed) < 0) {
            perror("execvp failed for the first command");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // The first command executed successfully
            // fprintf(stderr, "The first command executed successfully, so the second command will not be executed\n");
        } else {
			// fprintf(stderr, "The first command failed, so the second command will be executed\n"); 
			if (execvp(parsedOR[0], parsedOR) < 0) {
                perror("execvp failed for the second command");                
            }                      
        }
    }    
} 


// Help command builtin 
void openHelp() 
{ 
	puts("\n***WELCOME TO MY SHELL HELP***"
		"\nCopyright @ Suprotik Dey"
		"\n-Use the shell at your own risk..."
		"\nList of Commands supported:"
		"\n>cd"
		"\n>ls"
		"\n>exit"
		"\n>all other general commands available in UNIX shell"
		"\n>pipe handling"
		"\n>improper space handling"); 

	return; 
} 

// Function to execute builtin commands 
int ownCmdHandler(char** parsed) 
{ 
	int NoOfOwnCmds = 4, i, switchOwnArg = 0; 
	char* ListOfOwnCmds[NoOfOwnCmds]; 
	char* username; 

	ListOfOwnCmds[0] = "exit"; 
	ListOfOwnCmds[1] = "cd"; 
	ListOfOwnCmds[2] = "help"; 
	ListOfOwnCmds[3] = "hello"; 

	for (i = 0; i < NoOfOwnCmds; i++) { 
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) { 
			switchOwnArg = i + 1; 
			break; 
		} 
	} 

	switch (switchOwnArg) { 
	case 1: 
		printf("\nGoodbye\n"); 
		exit(0); 
	case 2: 
		chdir(parsed[1]); 
		return 1; 
	case 3: 
		openHelp(); 
		return 1; 
	case 4: 
		username = getenv("USER"); 
		printf("\nHello %s.\nMind that this is "
			"not a place to play around."
			"\nUse help to know more..\n", 
			username); 
		return 1; 
	default: 
		break; 
	} 

	return 0; 
} 

// function for finding pipe 
int parsePipe(char* str, char** strpiped) 
{ 
	char delim[] = "|";
	char *state = NULL;
			
	strpiped[0] = strtok_r(str, delim, &state);

	strpiped[1] = strtok_r(NULL, delim, &state); 		
}

// function for finding AND 
int parseAND(char* str, char** strAND) 
{ 
	char delim[] = "&&";
	char *state = NULL;
			
	strAND[0] = strtok_r(str, delim, &state);

	strAND[1] = strtok_r(NULL, delim, &state); 	
}

// function for finding OR 
void parseOR(char* str, char** strOR) 
{ 
	char delim[] = "||";
	char *state = NULL;
			
	strOR[0] = strtok_r(str, delim, &state);

	strOR[1] = strtok_r(NULL, delim, &state);
	
}

// function for parsing command words 
void parseSpace(char* str, char** parsed) 
{ 
	int i; 

	for (i = 0; i < MAXLIST; i++) { 
		parsed[i] = strsep(&str, " "); 

		if (parsed[i] == NULL) 
			break; 
		if (strlen(parsed[i]) == 0) 
			i--; 
	} 
} 

int check_operation(char *str) {
    if (strstr(str, "||") != NULL) {
        // printf("Double Pipe (OR)\n");
		return 2;
    } else if (strstr(str, "|") != NULL) {
        // printf("Single Pipe\n");
		return 1;
    } else if (strstr(str, "&&") != NULL) {
        // printf("AND\n");
		return 3;
    } else {
        // printf("No Pipe\n");
		return 0;
    }

	return 0;
}

int processString(char* str, char** parsed, char** parsedpipe, char** parsedAND, char** parsedOR) 
{ 

	char* strpiped[2];
	char* strAND[2];
	char* strOR[2];
	int piped = 0, and = 0, or = 0; 

	int op = 0; 

	op = check_operation(str);		

	switch (op)
	{
	case 1:
		piped = 1;
		parsePipe(str, strpiped);
		parseSpace(strpiped[0], parsed); 
		parseSpace(strpiped[1], parsedpipe);
		break;
	case 2:
		or = 2;
		parseOR(str, strOR);		
		parseSpace(strOR[0], parsed); 
		parseSpace(strOR[1], parsedOR); 
		break;
	case 3:
		and = 3;
		parseAND(str, strAND);		
		parseSpace(strAND[0], parsed); 
		parseSpace(strAND[1], parsedAND);
		break;
	default:
		parseSpace(str, parsed);
		break;
	}

	// if (op == 1) { 
	// 	piped = 1;
	// 	printf("Es piped\n");
	// 	parsePipe(str, strpiped);	
	// 	printf("strpiped 0: %s\n", strpiped[0]);	
	// 	printf("strpiped 1: %s\n", strpiped[1]);	
	// 	parseSpace(strpiped[0], parsed); 
	// 	parseSpace(strpiped[1], parsedpipe);
	// 	for (int i = 0; i < 2; i++)
	// 	{
	// 		printf("parsed: %s\n", parsed[i]);
	// 	}
	// 	for (int i = 0; i < 2; i++)
	// 	{
	// 		printf("parsedAND: %s\n", parsedpipe[i]);
	// 	} 

	// } else if (op == 3) { 
	// 	and = 3;
	// 	printf("Es AND\n");
	// 	parseAND(str, strAND);		
	// 	// printf("strAND 0: %s\n", strAND[0]);	
	// 	// printf("strAND 1: %s\n", strAND[1]);	
	// 	parseSpace(strAND[0], parsed); 
	// 	parseSpace(strAND[1], parsedAND);
	// 	// for (int i = 0; i < 2; i++)
	// 	// {
	// 	// 	printf("parsed: %s\n", parsed[i]);
	// 	// }
	// 	// for (int i = 0; i < 2; i++)
	// 	// {
	// 	// 	printf("parsedAND: %s\n", parsedAND[i]);
	// 	// }			 
	// } else if (op == 2) {
	// 	or = 2;
	// 	printf("Es OR\n");
	// 	parseOR(str, strOR);
	// 	// printf("Es OR\n");
	// 	parseSpace(strOR[0], parsed); 
	// 	parseSpace(strOR[1], parsedOR); 
	        
	// } else {
	// 	parseSpace(str, parsed);
	// }

	if (ownCmdHandler(parsed)) 
		return 0; 
	else
		return 1 + piped + and + or; 

} 

int main() 
{ 
	char inputString[MAXCOM], *parsedArgs[MAXLIST]; 
	char* parsedArgsPiped[MAXLIST]; 
	char* parsedArgsAND[MAXLIST];
	char* parsedArgsOR[MAXLIST];
	
	int execFlag = 0; 
	init_shell(); 

	while (1) { 
		// print shell line 
		printDir(); 
		// take input 
		if (takeInput(inputString)) 
			continue; 
		// process 
		execFlag = processString(inputString, 
		parsedArgs, parsedArgsPiped, parsedArgsAND, parsedArgsOR); 
		// execflag returns zero if there is no command 
		// or it is a builtin command, 
		// 1 if it is a simple command. 
		// 2 if it is including a pipe. 

		// execute 
		if (execFlag == 1) 
			execArgs(parsedArgs); 
		if (execFlag == 2) //Pipe
			execArgsPiped(parsedArgs, parsedArgsPiped);			
		if (execFlag == 3) //OR			
			execArgsOR(parsedArgs, parsedArgsOR); 		
		if (execFlag == 4) //AND		
			execArgsAND(parsedArgs, parsedArgsAND);
	} 

	return 0; 
} 