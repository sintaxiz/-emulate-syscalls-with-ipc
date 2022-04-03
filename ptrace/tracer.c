#include <errno.h>
#include <linux/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../common.h"

void trace(pid_t tracee_id) {
    int status;
    waitpid(tracee_id, &status, 0);
    ptrace(PTRACE_SETOPTIONS, tracee_id, 0, PTRACE_O_TRACESYSGOOD);
    struct user_regs_struct state;
    while (!WIFEXITED(status)) {
        ptrace(PTRACE_SYSCALL, tracee_id, 0, 0);
        waitpid(tracee_id, &status, 0);
        if (WIFSTOPPED(status) && WSTOPSIG(status)) {
            struct ptrace_syscall_info syscall_info;
            ptrace(PTRACE_GET_SYSCALL_INFO, tracee_id, sizeof(struct ptrace_syscall_info), &syscall_info);
            if (syscall_info.op == PTRACE_SYSCALL_INFO_ENTRY) {
                if (syscall_info.entry.nr == SYSCALL_CODE_CLOSE) {
                    close(1337);
                }
            }
            ptrace(PTRACE_SYSCALL, tracee_id, 0, 0);
            waitpid(tracee_id, &status, 0);
        }
    }
}

void highload(int iter_count) {
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    char iter_count_text[20];
    sprintf(iter_count_text, "%d", iter_count);
    execl("./ptrace_highload", "./ptrace_highload", iter_count_text, NULL);
    perror("execl");
}

int main(int argc, char **argv) {
    int iter_count;
    if (argc > 1) {
        long conversion_result = strtol(argv[1], NULL, 10);
        if (errno != 0) {
            iter_count = DEFAULT_ITER_COUNT;
        } else {
            iter_count = (int)conversion_result;
        }
    }

    pid_t tracee_id = fork();
    if (tracee_id) {
        trace(tracee_id);
    } else {
        highload(iter_count);
    }
    return 0;
}
