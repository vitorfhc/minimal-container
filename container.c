#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/mount.h>
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

int make_fork() {
    int ret;
    pid_t pid;
    pid = fork();
    if(pid == -1) exit_with_error("fork");
    return pid;
}

void exec_program(const char cmd[], char **args) {
    if(execvp(cmd, args) == -1) exit_with_error("exec");
}

void mount_proc() {
    if(mount("none", "/proc", "proc", 0, NULL))
        exit_with_error("mount");
}

void change_root() {
    if(chroot("./rootfs/debian") == -1) exit_with_error("chroot");
    if(chdir("/") == -1) exit_with_error("chdir");
}

int main(int argc, char *argv[]) {
    /*
        Flow:
        1. Unshare the namespaces (unshare)
        2. Fork the process (fork)
        3. Change the root (chroot)
        4. Mount a new procfs (mount)
        5. Exec the program (execvp)
    */

    pid_t pid, wait_pid;

    unshare_namespaces();
    pid = make_fork();
    if(pid != 0) { // parent
        wait_pid = wait(NULL);
        if(wait_pid == -1)
            exit_with_error("wait");
        return 0;
    }

    // from here on only child is running
    change_root();
    mount_proc();
    exec_program("bash", argv);
    // end of child's execution

    return 0;
}
