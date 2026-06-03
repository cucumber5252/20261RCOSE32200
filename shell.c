#include <stdio.h>
#include <string.h>

#define BUF_SIZE 1024

static void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

int main(void) {
    char buf[BUF_SIZE];

    while (1) {
        printf("$ ");
        fflush(stdout);

        if (fgets(buf, BUF_SIZE, stdin) == NULL)
            break;

        strip_newline(buf);
        if (strlen(buf) == 0)
            continue;

        printf("input: %s\n", buf);
    }

    return 0;
}
