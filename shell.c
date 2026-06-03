#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_ARGS 64

static void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

static int parse(char *buf, char *argv[]) {
    int argc = 0;
    char *tok = strtok(buf, " \t");
    while (tok != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = tok;
        tok = strtok(NULL, " \t");
    }
    argv[argc] = NULL;
    return argc;
}

static int check_bg(char *argv[], int *argc) {
    if (*argc > 0 && strcmp(argv[*argc - 1], "&") == 0) {
        argv[--(*argc)] = NULL;
        return 1;
    }
    return 0;
}

/* < > 토큰을 argv에서 찾아 dup2 처리 후 제거 */
static void setup_redir(char *argv[], int *argc) {
    for (int i = 0; i < *argc; i++) {
        if (strcmp(argv[i], ">") == 0 && i + 1 < *argc) {
            int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open"); exit(EXIT_FAILURE); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            for (int j = i; j < *argc - 2; j++) argv[j] = argv[j + 2];
            *argc -= 2;
            argv[*argc] = NULL;
            i--;
        } else if (strcmp(argv[i], "<") == 0 && i + 1 < *argc) {
            int fd = open(argv[i + 1], O_RDONLY);
            if (fd < 0) { perror("open"); exit(EXIT_FAILURE); }
            dup2(fd, STDIN_FILENO);
            close(fd);
            for (int j = i; j < *argc - 2; j++) argv[j] = argv[j + 2];
            *argc -= 2;
            argv[*argc] = NULL;
            i--;
        }
    }
}

static void execute(char *argv[], int argc, int bg) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return; }
    if (pid == 0) {
        setup_redir(argv, &argc);
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(EXIT_FAILURE);
    } else {
        if (bg) {
            printf("[%d]\n", pid);
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
                printf("[exit %d]\n", WEXITSTATUS(status));
        }
    }
}

static int builtin(char *argv[]) {
    if (strcmp(argv[0], "exit") == 0)
        exit(0);

    if (strcmp(argv[0], "cd") == 0) {
        const char *path = argv[1] ? argv[1] : getenv("HOME");
        if (chdir(path) < 0) perror("cd");
        return 1;
    }

    if (strcmp(argv[0], "pwd") == 0) {
        char cwd[BUF_SIZE];
        if (getcwd(cwd, sizeof(cwd))) printf("%s\n", cwd);
        else perror("pwd");
        return 1;
    }

    return 0;
}

int main(void) {
    char buf[BUF_SIZE];
    char *argv[MAX_ARGS];

    while (1) {
        printf("$ ");
        fflush(stdout);

        if (fgets(buf, BUF_SIZE, stdin) == NULL)
            break;

        strip_newline(buf);
        if (strlen(buf) == 0)
            continue;

        int argc = parse(buf, argv);
        if (argc == 0)
            continue;

        if (builtin(argv))
            continue;

        int bg = check_bg(argv, &argc);
        if (argc == 0)
            continue;

        execute(argv, argc, bg);
    }

    return 0;
}
