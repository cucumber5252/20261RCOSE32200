#include <stdio.h>
#include <string.h>

#define BUF_SIZE 1024

int main(void) {
    char buf[BUF_SIZE];

    while (1) {
        printf("$ ");
        fflush(stdout);

        if (fgets(buf, BUF_SIZE, stdin) == NULL)
            break;

        printf("input: %s", buf);
    }

    return 0;
}
