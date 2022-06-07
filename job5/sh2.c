#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PATH 20

/*
FUNC  : split
param :
	@line : source string 
	@separators : chars to split the line
	@word_array : return words
return : the word count split from the line
other : the line will be modified
*/
int split(char *line, char *separators, char *work_array[])
{
	int word_count = 0;
	char *word;
	
	word = strtok(line, " ");
	while (word != NULL)
	{
		word_array[word_count] = word;
		word_count++;
		word = strtok(NULL, " ");
	}
	
	return word_count;
}

#define MAX_ARGC 10
struct command {
	int argc = 0;
	char *argv[MAX_ARGC] = {NULL} ;
	char *input = NULL;
	char *output = NULL;
	char *append = NULL;
};

/*
FUNC : parse_command
param :
	@line : command line
	@command : command structed
*/
void parse_command(char *line, struct command command)
{
	
}

#define MAX_LINE_WORDS 128
int main()
{
	char line[MAX_LINE_WORDS] = {'\0'};
	int word_count = 0;
	char *word_array[10];
	while(1)
	{
		gets(line);
		word_count = split(line, " ", word_array);
		
		pid = fork();
		if(pid == 0)
		{
			execvp(word_array[0],word_array);
		}
		wait();
	}
	
	return 0;
}
