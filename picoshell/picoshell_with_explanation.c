#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int	picoshell(char **cmds[])
{
	pid_t	pid;
	int		pipefd[2];
	int		prev_fd;
	int		status;
	int		exit_code;
	int		i;

	prev_fd = -1;
	exit_code = 0;
	i = 0;
	// コマンドごとにループ
	while (cmds[i])
	{
		// パイプ（バッファー）作成
		if (cmds[i + 1] && pipe(pipefd))
			return (1);
		// 新しいプロセスを作成
		pid = fork();
		// エラーハンドリング
		if (pid == -1)
		{
			if (cmds[i + 1])
			{
				close(pipefd[0]);
				close(pipefd[1]);
			}
			return (1);
		}
		// ==========
		// 子プロセス
		if (pid == 0)
		{
			if (prev_fd != -1) // 最初のコマンド以外
			{
				if (dup2(prev_fd, STDIN_FILENO) == -1) // 直前のコマンドのpipeのreadをSTDINに繋げる
					exit(1);
				close(prev_fd);
			}
			if (cmds[i + 1]) // 最後のコマンド以外
			{
				close(pipefd[0]); // pipeのreadを閉じる
				if (dup2(pipefd[1], STDOUT_FILENO) == -1) // pipeのwriteをSTDOUTに繋げる
					exit(1);
				close(pipefd[1]);
			}
			// ここまで来たらこのループで作ったpipeのreadはclose, writeはSTDOUT, 直前に作ったpipeのreadはSTDINに繋がっている。
			// プロセスのコンテキストをコマンドで置き換え
			execvp(cmds[i][0], cmds[i]);
			// ここに来たら異常
			exit(1);
		}
		// ==========
		// 以下、親プロセス
		if (prev_fd != -1)
			close(prev_fd);
		if (cmds[i + 1]) // 最後のコマンドじゃない場合
		{
			close(pipefd[1]); // pipeのwriteを閉じる
			prev_fd = pipefd[0];
		}
		i++;
	}
	while (wait(&status) != -1) // 子プロセスを待つ
	{
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			exit_code = 1;
	}
	return (exit_code);
}


/* Test main

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
static int	count_cmds(int argc, char **argv)
{
	int	count = 1;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "|") == 0)
			count++;
	}
	return (count);
}

int	main(int argc, char **argv)
{
	if (argc < 2)
		return (fprintf(stderr, "Usage: %s cmd1 [args] | cmd2 [args] ...\n", argv[0]), 1);

	int	cmd_count = count_cmds(argc, argv);
	char	***cmds = calloc(cmd_count + 1, sizeof(char **));
	if (!cmds)
		return (perror("calloc"), 1);

	int	i = 1, j = 0;
	while (i < argc)
	{
		int	len = 0;
		while (i + len < argc && strcmp(argv[i + len], "|") != 0)
			len++;
		cmds[j] = calloc(len + 1, sizeof(char *));
		if (!cmds[j])
			return (perror("calloc"), 1);
		for (int k = 0; k < len; k++)
			cmds[j][k] = argv[i + k];
		cmds[j][len] = NULL;
		i += len + 1;
		j++;
	}
	cmds[cmd_count] = NULL;

	int	ret = picoshell(cmds);

	// Clean up
	for (int i = 0; cmds[i]; i++)
		free(cmds[i]);
	free(cmds);

	return (ret);
}
*/