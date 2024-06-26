//Proyecto #1 Mini-Shell
//Juan Fernandes V-29966562
//Freddy López V-21534219

#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<ctype.h>
#include<sys/types.h> 
#include<sys/wait.h> 

#define MAXCOM 1000 // numero maximo de letras que se pueden secibir
#define MAXLIST 100 // max number of commands to be supported 

// Limpiar el shell 
#define clear() printf("\033[H\033[J") 

// Saludo de inicio 
void init_shell() 
{ 
	clear(); 
	printf("\n\n\n\n******************"
		"************************"); 
	printf("\n\n\n\t****PROYECTO 1: MINI-SHELL****"); 
	printf("\n\n\t-SISTEMAS OPERATIVOS-"); 
	printf("\n\n\n\n*******************"
		"***********************"); 
	char* username = getenv("USER"); 
	printf("\n\n\nUSER is: @%s", username); 
	printf("\n"); 
	sleep(1); 
	clear(); 
} 

char *leer_linea(void)
{
	char *linea = NULL;
	size_t bufsize = 0;	

	if (getline(&linea, &bufsize, stdin) == -1) /* si getline falla */
	{
		if (feof(stdin))
		{
			free(linea); 
			exit(EXIT_SUCCESS); 
		}
		else
		{
			free(linea); 
			perror("error leyendo desde stdin");
			exit(EXIT_FAILURE);
		}
	}
	return (linea);
}

//Funcion para tomar la entrada
int tomar_input(char* str) 
{ 
	char* buf; 

	buf = leer_linea();
	
	buf[strcspn(buf, "\n")] = '\0'; 

	if (strlen(buf) != 0) { 
		//add_history(buf); 
		strcpy(str, buf); 
		return 0; 
	} else { 
		return 1; 
	} 
} 

// Funcion para impimir el directorio actual
void imprimir_dir() 
{ 
	char cwd[1024]; 
	getcwd(cwd, sizeof(cwd)); 
	printf("\nDir: %s \n>>> ", cwd); 
} 

void abrir_ayuda() 
{ 
	puts("\n***MINISHELL***"
		"\nJuan Fernandes"
		"\nFreddy Lopez"
		"\nSistemas Operativos"
        "\ncon"		
		"\nManejo de Operador |"
		"\nManejo de Operador &&"
		"\nManejo de operador ||"); 
	return; 
} 

// Funcion para ejecutar comandos propios
int manejador_comandos_propios(char** parsed) 
{ 
	int NoComandoPropìo = 3, i, switchComandoPropio = 0; 
	char* ListaComandosPropios[NoComandoPropìo]; 
	char* username; 

	ListaComandosPropios[0] = "salir"; 
	ListaComandosPropios[1] = "cd"; 
	ListaComandosPropios[2] = "help"; 	 

	for (i = 0; i < NoComandoPropìo; i++) { 
		if (strcmp(parsed[0], ListaComandosPropios[i]) == 0) { 
			switchComandoPropio = i + 1; 
			break; 
		} 
	} 

	switch (switchComandoPropio) { 
	case 1: 
		printf("\n¡Hasta Luego!\n"); 
		exit(0); 
	case 2: 
		chdir(parsed[1]); 
		return 1; 
	case 3: 
		abrir_ayuda(); 
		return 1; 	
	default: 
		break; 
	} 

	return 0; 
} 

// Funcion donde se ejcuta un comando
void ejecutar_cmd(char** parsed) 
{ 
	pid_t pid = fork(); 

	if (pid < 0) { 
		perror("fork"); 
		return; 
	} else if (pid == 0) { 
		if (execvp(parsed[0], parsed) < 0) { 
			perror("execvp");; 
		} 
		exit(0); 
	} else { 		
		wait(NULL); 
		return; 
	} 
} 

// Fucnion donde se ejecutan comandos conectados por un pipe
void ejecutar_pipe(char** parsed, char** parsedpipe)
{
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
        
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        if (execvp(parsed[0], parsed) < 0) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        
        p2 = fork();

        if (p2 < 0) {
            perror("fork");
            return;
        }
        
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            
            close(pipefd[0]);
            close(pipefd[1]);

            waitpid(p1, NULL, 0);
            waitpid(p2, NULL, 0);
        }
    }
}

// Funcion para ejecutar una linea de comando que tenga el operador &&
void ejecutar_AND(char** parsed, char** parsedAND) 
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
    }

    if (pid == 0) {
        
		if(manejador_comandos_propios(parsed) == 0){
			if (execvp(parsed[0], parsed) < 0) {
            perror("execvp failed for the first command");
            exit(EXIT_FAILURE);
        	}
		}        
    } else {
        
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Primer comando ejecutado de forma exitosa
            pid = fork();
            if (pid < 0) {
                perror("fork failed");
            }
            if (pid == 0) {
                
				if (manejador_comandos_propios(parsedAND) == 0)
				{
					if (execvp(parsedAND[0], parsedAND) < 0) {
                    perror("execvp failed for the second command");
                    exit(EXIT_FAILURE);
                	}
				}                
            } else {
                
                waitpid(pid, &status, 0);
            }
        } else {
            // Fallo del primer comando           
        }
    }  
} 

// Funcion para ejecutar una linea de comando que tenga el operador ||
void ejecutar_OR(char** parsed, char** parsedOR) 
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        perror("fork failed");        
    }

    if (pid == 0) {
        
        if(manejador_comandos_propios(parsed) == 0){
			if (execvp(parsed[0], parsed) < 0) {
            perror("execvp failed for the first command");
            exit(EXIT_FAILURE);
        	}
		} 
    } else {
        
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Primer comando ejecutado de forma exitosa         
        } else {			
			if (manejador_comandos_propios(parsedOR) == 0)
			{
				if (execvp(parsedOR[0], parsedOR) < 0) {
					perror("execvp failed for the second command"); 
					exit(EXIT_FAILURE);              
				}                      
        	}
		}		
    }    
} 

// Función para separar los comandos de una líne que utilice |
void parse_pipe(char* str, char** strpiped) 
{ 
	char delim[] = "|";
	char *status = NULL;
			
	strpiped[0] = strtok_r(str, delim, &status);

	strpiped[1] = strtok_r(NULL, delim, &status); 		
}

// Función para separar los comandos de una líne que utilice &&
void parse_AND(char* str, char** strAND) 
{ 
	char delim[] = "&&";
	char *status = NULL;
			
	strAND[0] = strtok_r(str, delim, &status);

	strAND[1] = strtok_r(NULL, delim, &status);
}

// Función para separar los comandos de una líne que utilice ||
void parse_OR(char* str, char** strOR) 
{ 
	char delim[] = "||";
	char *status = NULL;
			
	strOR[0] = strtok_r(str, delim, &status);

	strOR[1] = strtok_r(NULL, delim, &status);
	
}

// ffuncion para "parsear" las palabras de una linea de comandos
void parse_espacio(char* str, char** parsed) 
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

// Funcion que identifica el operador que se está utilizando en la linea de comando
int identificar_operacion(char *str) {
    if (strstr(str, "||") != NULL) {        
		return 2;
    } else if (strstr(str, "|") != NULL) {        
		return 1;
    } else if (strstr(str, "&&") != NULL) {        
		return 3;
    } else {       
		return 0;
    }

	return 0;
}

// Funcion que procesa el string dado como entrada
int procesar_string(char* str, char** parsed, char** parsedOP) 
{ 

    char **comillas[MAXLIST];
	
	char* strOP[2];
	
	int piped = 0, and = 0, or = 0; 

	int op = 0; 

	op = identificar_operacion(str);	

	switch (op)
	{
	case 1:
		piped = 1;
		parse_pipe(str, strOP);
		parse_espacio(strOP[0], parsed);
		parse_espacio(strOP[1], parsedOP);		
		break;
	case 2:
		or = 2;
		parse_OR(str, strOP);
		parse_espacio(strOP[0], parsed);
		parse_espacio(strOP[1], parsedOP);	
		break;
	case 3:
		and = 3;
		parse_AND(str, strOP);	
		parse_espacio(strOP[0], parsed);
		parse_espacio(strOP[1], parsedOP);	
		break;
	default:
		parse_espacio(str, parsed);		
		break;
	}
	
	if (manejador_comandos_propios(parsed)) 
		return 0; 
	else
		return 1 + piped + and + or; 
} 

int main() 
{ 
	char inputString[MAXCOM], *parsedArgs[MAXLIST]; 
	char* parsedArgsOP[MAXLIST]; 	
	
	int execFlag = 0; 
	init_shell(); 

	while (1) { 
		
		imprimir_dir(); 
		
		if (tomar_input(inputString)) 
			continue; 
		
		execFlag = procesar_string(inputString, 
		parsedArgs, parsedArgsOP); 	
		
		if (execFlag == 1)// No utiliza operador especial 
			ejecutar_cmd(parsedArgs); 
		if (execFlag == 2) //Pipe
			ejecutar_pipe(parsedArgs, parsedArgsOP);			
		if (execFlag == 3) //OR			
			ejecutar_OR(parsedArgs, parsedArgsOP); 		
		if (execFlag == 4) //AND		
			ejecutar_AND(parsedArgs, parsedArgsOP);
	} 

	return 0; 
} 
