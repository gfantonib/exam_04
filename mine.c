#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int has_pipe(char *argv[]);
int simple_exec(char *argv[], char *envp[]);
int is_pipe(char *str);
int exec(char *argv[], int end);

void print(char *message, int fd)
{
	int i = 0;
	while (message[i])
		i++;
	write(fd, message, i);
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
		status = exec(argv, i);
		argv = &argv[i] + (argv[i] != NULL);
	}
	return (status);
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

int simple_exec(char *argv[], char *envp[])
{
	print("simple_exec()\n", STDOUT_FILENO);
	if (execve(*argv, argv, envp) != 0)
	{
		print("error: cannot execute ", STDERR_FILENO);
		print(*argv, STDERR_FILENO);
		print("\n", STDERR_FILENO);
	}
	exit (1);
}

int exec(char *argv[], int end)
{
	print("exec()\n", STDOUT_FILENO);
	return (0);
}