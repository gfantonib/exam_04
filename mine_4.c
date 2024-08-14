#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void simple_exec(char **argv, char **envp);
int exec(char **argv, char **envp, int i);
void cd(char **argv);
void set_w_pipe(int *fd, int has_pipe);
void set_r_pipe(int *fd, int has_pipe);

void print_fd(char *str, int fd)
{
	int i = 0;
	while (str[i])
		i++;
	write(fd, str, i);
}

int has_op(char **argv)
{
	int i = 0;
	while (argv[i])
	{
		if (!strcmp(argv[i], "|") || !strcmp(argv[i], ";"))
			return 1;
		i++;
	}
	return 0;
}

int main(int argc, char **argv, char **envp)
{
	if (argc == 1)
		return 0;

	argv++;

	if(!has_op(argv))
		simple_exec(argv, envp);

	int status = 0;
	int i = 0;
	while (*argv)
	{
		i = 0;
		while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
			i++;
		status = exec(argv, envp, i);
		argv = &argv[i] + (argv[i] != 0);
	}
	return status;
}

void simple_exec(char **argv, char **envp)
{
	if (!strcmp(*argv, "cd"))
		cd(argv);
	execve(*argv, argv, envp);
	print_fd("error: cannot execute ", 2);
	print_fd(*argv, 2);
	print_fd("\n", 2);
	exit(1);
}

int exec(char **argv, char **envp, int i)
{
	int fd[2], has_pipe, pid, status;

	has_pipe = (argv[i] && !strcmp(argv[i], "|"));
	if (has_pipe)
	{
		if (pipe(fd) != 0)
		{
			print_fd("error: fatal\n", 2);
			exit(1);
		}
	}

	pid = fork();
	if (pid == 0)
	{
		argv[i] = 0;
		set_w_pipe(fd, has_pipe);
		if (!strcmp(*argv, "cd"))
			cd(argv);
		execve(*argv, argv, envp);
		print_fd("error: cannot execute ", 2);
		print_fd(*argv, 2);
		print_fd("\n", 2);
		exit(1);
	}
	waitpid(pid, &status, 0);
	set_r_pipe(fd, has_pipe);
	return WIFEXITED(status) && WEXITSTATUS(status);
}

void set_w_pipe(int *fd, int has_pipe)
{
	if (has_pipe)
	{
		if (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1)
		{
			print_fd("error: fatal\n", 2);
			exit(1);
		}
	}
}

void set_r_pipe(int *fd, int has_pipe)
{
	if (has_pipe)
	{
		if (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1)
		{
			print_fd("error: fatal\n", 2);
			exit(1);
		}
	}
}

void cd(char **argv)
{
	if (argv[0] && argv[1] && !argv[2])
	{
		if (chdir(argv[1]) != 0)
		{
			print_fd("error: cd: cannot change directory to ", 2);
			print_fd(argv[1], 2);
			print_fd("\n", 2);
			exit(1);
		}
		exit(0);
	}
	print_fd("error: cd: bad arguments\n", 2);
	exit(1);
}
