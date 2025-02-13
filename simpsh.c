#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "defined_functions/util_str.h"
#include "defined_functions/util_which.h"
#include "defined_functions/util_env.h"
#include "defined_functions/_util_str.h"
#include "defined_functions/util_cd.h"

int process_cmd(char **input_tokens)
{
	int wstatus;
	char *fullpath;

	fullpath = _which(input_tokens[0]);

	if (!fullpath)
	{
		free(fullpath);
		fullpath = NULL;
		return(-1);
	}
	else
	{
		input_tokens[0] = fullpath;
	}

	switch (fork())
	{
		case -1:
			return (-1);
		case 0:
			if (execve(input_tokens[0], input_tokens, environ) == -1)
				return (-1);
			break;
		default:
			wait(&wstatus);
	}

	free(fullpath);
	return (wstatus);
}

int main(void)
{
	char *inputline = NULL;
	char **input_tokens = NULL;
	size_t input_len = 0;
	ssize_t n_read;

	init_env();

	while (1)
	{
		if (isatty(STDIN_FILENO)) /* if interactive mode */
			printf("$ ");

		n_read = getline(&inputline, &input_len, stdin);
		if (n_read == -1) /* EOF or error */
			break;

		if (n_read > 0 && inputline[n_read - 1] == '\n') /* remove \n */
			inputline[n_read - 1] = '\0';

		if (_strcmp(inputline, "exit") == 0) /* exit command */
			break;

		input_tokens = tokenize(inputline, " \t", 1024); /* tokenize */
		if (!input_tokens || !input_tokens[0]) /* if empty input */
		{
			free(input_tokens);
			continue;
		}

		if (_strcmp(input_tokens[0], "cd") == 0)
		{
			change_dir(input_tokens);
		}
		else
		{
			int status = process_cmd(input_tokens); /* process command */
			if (status == -1)
				printf("Command not found: %s\n", input_tokens[0]);
		}

		free(input_tokens);
	}

	reset_env();
	free(inputline);

	return (0);
}
