#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define READ_END 0
#define WRITE_END 1

int main()
{

	pid_t mpr_chd, red_chd;
	int fd[2]; 
	int status;
	pipe(fd); 
	mpr_chd = fork();
	if (mpr_chd == 0)
	{

		close(fd[READ_END]); 
		dup2(fd[WRITE_END], STDOUT_FILENO); 
		execl("./mapper", "mapper", (char *) 0);
	}

	red_chd = fork();
	if (red_chd == 0)
	{
		close (fd[WRITE_END]); 
		dup2(fd[READ_END], STDIN_FILENO); 
		execl("./reducer", "reducer", (char *) 0);
	}
	close (fd[READ_END]); 
	close (fd[WRITE_END]);
	wait( &status); 

	return 0;
}
