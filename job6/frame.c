/*
 shell frame
 while(1)
 {
 	print > ;
	read line;
	split line into args;
	// buildin command
	if (args[0] == "exit")	// exit can not implement by child proc
		exit(0);
	if (args[0] == "cd")
	// extern command
	// implement through child proc
	pid = fork();
	if pid == 0 then 
		execvp(args[0],args);
	wait(null);
 }
*/

// split line
// cause need to split a line by '|'--pipe\ ' '--args \ '\n'--end of line 
int split(char* line,char *separator, char *word_array[])
{
	int word_count = 0;
}
