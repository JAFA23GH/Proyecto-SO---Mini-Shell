all: Minishell

Minishell:	minishell.o
			gcc -o Minishell minishell.o

minishell.o: 	minishell.c
				gcc -c minishell.c

clean:
	rm *.o Minishell
