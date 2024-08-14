#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int simple_exec(char *argv[], char *envp[]);
int exec(char *argv[], char *envp[], int end);
void set_w_pipe(int *fd, int has_pipe);
void set_r_pipe(int *fd, int has_pipe);
int cd(char *argv[]);

void print_error(char *wrong_cmd, int fd)
{
	int i = 0;

	write(fd, "error: cannot execute ", 22);
	i = 0;
	while (wrong_cmd[i])
		i++;
	write(fd, wrong_cmd, i);
	write(fd, "\n", 1);
}

int has_op(char *argv[])
{
	int i = 0;
	while (argv[i])
	{
		if ((argv[i][0] == '|' || argv[i][0] == ';') && !argv[i][1])
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

int is_semi(char *str)
{
	if (str[0] == ';' && !str[1])
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
	if (!has_op(argv))
		simple_exec(argv, envp);

	while (*argv)
	{
		i = 0;
		while (argv[i] && !is_pipe(argv[i]) && !is_semi(argv[i]))
			i++;
		status = exec(argv, envp, i);
		argv = &argv[i] + (argv[i] != NULL);
	}
	return (status);
}

int simple_exec(char *argv[], char *envp[])
{
	if (!strcmp(*argv, "cd"))
		exit (cd(argv));
	if (execve(*argv, argv, envp) != 0)
		print_error(*argv, STDERR_FILENO);
	exit (1);
}

int exec(char *argv[], char *envp[], int end)
{
	int fd[2];
	int status;

	int has_pipe = 0;
	if (argv[end] && argv[end][0] == '|')
	{
		has_pipe = 1;
		pipe(fd);
	}
	int pid = fork();
	if (pid == 0)
	{
		argv[end] = 0;
		set_w_pipe(fd, has_pipe);
		if (execve(*argv, argv, envp) != 0)
		{
			print_error(*argv, STDERR_FILENO);
			exit(1);
		}
	}
	waitpid(pid, &status, 0);
	set_r_pipe(fd, has_pipe);
	return WIFEXITED(status) && WEXITSTATUS(status);
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

void cd_error(char *wrong_path)
{
	write(STDERR_FILENO, "error: cd: cannot change directory to ", 38);
	int i = 0;
	while (wrong_path[i])
		i++;
	write(STDERR_FILENO, wrong_path, i);
	write(STDERR_FILENO, "\n", 1);
}

int cd(char *argv[])
{
	if (argv[0] && argv[1] && !argv[2])
	{
		if (chdir(argv[1]) == -1)
		{
			cd_error(argv[1]);
			return (1);
		}
		return (0);
	}
	write(STDERR_FILENO, "error: cd: bad arguments\n", 25);
	return (1);
}
