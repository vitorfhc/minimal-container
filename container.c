#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

const int UNSHARE_FLAGS = CLONE_NEWUSER | CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | CLONE_FILES;

void exit_with_error(const char msg[]) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void unshare_namespaces() {
    int ret;
    ret = unshare(UNSHARE_FLAGS);
    if(ret == -1) {
        exit_with_error("unshare");
    }
}

void exec_program(char *args[]) {
    int ret;
    pid_t pid;
    pid = fork();
    switch(pid) {
        case -1: // error
        exit_with_error("fork");
        break;
        case 0: // child
        ret = execvp("/bin/bash", args);
        if(ret == -1) exit_with_error("execvp");
        break;
        default: // parent
        if(wait(NULL) == -1) exit_with_error("wait");
        break;
    }
}

int main(int argc, char *argv[]) {
    /*
        Flow:
        1. Unshare the namespaces
        2. Fork the process
        3. Exec the program
    */

    unshare_namespaces();
    exec_program(argv);

    return 0;
}