#include "lib/ulib.h"

int argparse(char *input, char **argv);
char buf[1024];

int main(void)
{
    char cmd[128];
    char *argv[16] = {0};
    int pid;

    printf("\n\nShell Ready.\n");

    while (1) {
        printf("> ");
        read(stdin, cmd, sizeof(cmd));
        if (*cmd == '\0') {
            continue;
        }
        int is_background = 0;
        for (int i = 0; i < 128; i++) {
            if (cmd[i] == '&') {
                is_background = 1;
            }
        }
        if ((pid = fork()) < 0) { // fork error
            printf("fork error\n");
        }
        else if (pid == 0) { // child
            argparse(cmd, argv);
            if (execv(argv[0], argv) < 0) {
                printf("exec %s fail\n", cmd);
            }
            exit();
        }
        else { // parent
            if (is_background == 0) {
                wait();
            }
        }
        memset(cmd, 0, sizeof(cmd));
    }
    return 0;
}

int argparse(char *input, char **argv)
{
    int argc = 0;
    memset(buf, 0, sizeof(buf));

    argv[argc++] = buf;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ' ') {
            buf[i]       = '\0';
            argv[argc++] = &buf[i + 1];
        }
        else {
            buf[i] = input[i];
        }
    }

    return argc;
}
