#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int simple_exec(char *argv[], char *envp[]);
int exec(char *argv[], char *envp[], int end);
void set_w_pipe(int *fd, int has_pipe);
void set_r_pipe(int *fd, int has_pipe);

void print_error(char *message, char *cmd, int fd)
{
	int i = 0;
	while (message[i])
		i++;
	write(fd, message, i);
	write(fd, " ", 1);
	i = 0;
	while (cmd[i])
		i++;
	write(fd, cmd, i);
	write(fd, "\n", 1);
}

int has_pipe(char *argv[])
{
	int i = 0;
	while (argv[i])
	{
		if (argv[i][0] == '|' && !argv[i][1])
			return (1);
		i++;
	}
	return (0);
}

int is_pipe(char *str)
{
	if (str[0] == '|' && !str[1])
		return (1);
	return (0);
}

int main(int argc, char *argv[], char *envp[])
{
	int status = 0;
	int i = 0;

	argv++;
	if (!*argv)
		return (0);
	if (!has_pipe(argv))
		simple_exec(argv, envp);

	while (*argv)
	{
		i = 0;
		while (argv[i] && !is_pipe(argv[i]))
			i++;
		status = exec(argv, envp, i);
		argv = &argv[i] + (argv[i] != NULL);
	}
	return (status);
}

int simple_exec(char *argv[], char *envp[])
{
	if (execve(*argv, argv, envp) != 0)
		print_error("error: cannot execute", *argv, STDERR_FILENO);
	exit (1);
}

int exec(char *argv[], char *envp[], int end)
{
	int fd[2];

	int has_pipe = 0;
	if (argv[end] && argv[end][0] == '|')
	{
		has_pipe = 1;
		pipe(fd);
	}
	if (fork() == 0)
	{
		argv[end] = 0;
		set_w_pipe(fd, has_pipe);
		if (execve(*argv, argv, envp) != 0)
		{
			print_error("error: cannot execute", *argv, STDERR_FILENO);
			exit(1);
		}
	}
	set_r_pipe(fd, has_pipe);
	return (0);
}

void set_w_pipe(int *fd, int has_pipe)
{
	if (has_pipe)
	{
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
	}
}

void set_r_pipe(int *fd, int has_pipe)
{
	if (has_pipe)
	{
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
		close(fd[1]);
	}
}