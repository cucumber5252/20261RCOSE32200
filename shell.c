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

static void execute(char *argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
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

        execute(argv);
    }

    return 0;
}
