#include <errno.h>
#include <sys/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../common.h"

void redo_syscall(pid_t tracee_id, int *status, struct user_regs_struct *regs) {
    (*regs).rax = (*regs).orig_rax;
    (*regs).rip -= 2;
    ptrace(PTRACE_SETREGS, tracee_id, 0, regs);
    ptrace(PTRACE_SYSCALL, tracee_id, 0, 0);
    waitpid(tracee_id, status, 0);
    ptrace(PTRACE_SYSCALL, tracee_id, 0, 0);
    waitpid(tracee_id, status, 0);
}

void trace(pid_t tracee_id) {
    int status;
    waitpid(tracee_id, &status, 0);
    while (1) {
        ptrace(PTRACE_SYSEMU, tracee_id, 0, 0);
        waitpid(tracee_id, &status, 0);
        if (WIFEXITED(status)) {
            break;
        }

        if (WIFSTOPPED(status) && WSTOPSIG(status)) {
            struct user_regs_struct regs;

            ptrace(PTRACE_GETREGS, tracee_id, 0, &regs);
            // printf("syscall number: %llu\n", regs.orig_rax);

            if (regs.orig_rax != SYSCALL_CODE_GETPID) {
                redo_syscall(tracee_id, &status, &regs);
            } else {
                    getpid();
                    regs.rax = tracee_id;
                    ptrace(PTRACE_SETREGS, tracee_id, 0, &regs);
                    ptrace(PTRACE_SYSEMU_SINGLESTEP, tracee_id, 0, 0);
                    waitpid(tracee_id, &status, 0);
            }
        }
    }
}

void highload(int iter_count) {
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    char iter_count_text[20];
    sprintf(iter_count_text, "%d", iter_count);
    execl("./direct_syscall", "./direct_syscall", iter_count_text, NULL);
    perror("execl");
}

int main(int argc, char **argv) {
    int iter_count = DEFAULT_ITER_COUNT;
    if (argc > 1) {
        long conversion_result = strtol(argv[1], NULL, 10);
        if (errno == 0) {
            iter_count = (int)conversion_result;
        }
    }

    pid_t tracee_id = fork();
    if (tracee_id > 0) {
        // printf("tracee id = %d\n", tracee_id);
        trace(tracee_id);
    } else if (tracee_id == 0) {
        highload(iter_count);
    } else {
        perror("fork");
    }
    return 0;
}
