#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../common.h"

int pipe_getpid(int in_pipe, int out_pipe) {
    short syscall_getpid = SYSCALL_CODE_GETPID;
    write(out_pipe, &syscall_getpid, 2);
    pid_t getpid_result;
    read(in_pipe, &getpid_result, sizeof(pid_t));
    return getpid_result;
}

void tracee(int in_pipe, int out_pipe, int iter_count) {
    for (int i = 0; i < iter_count; i++) {
        pipe_getpid(in_pipe, out_pipe);
    }
}

void tracer(int in_pipe, int out_pipe, pid_t tracee_id) {
    short syscall;
    while (read(in_pipe, &syscall, 2) > 0) {
        if (syscall == SYSCALL_CODE_GETPID) {
            getpid();
            pid_t getpid_result = tracee_id;
            write(out_pipe, &getpid_result, sizeof(pid_t));
        } else {
            printf("unknown command.\n");
        }
    }
}

int main(int argc, char const *argv[]) {
    int iter_count = DEFAULT_ITER_COUNT;
    if (argc > 1) {
        long conversion_result = strtol(argv[1], NULL, 10);
        if (errno == 0) {
            iter_count = (int)conversion_result;
        }
    }

    int pipes[2];
    int pipes2[2];

    if (pipe(pipes) == -1 || pipe(pipes2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    int tracer_in_pipe = pipes[0];
    int tracer_out_pipe = pipes2[1];

    int tracee_out_pipe = pipes[1];
    int tracee_in_pipe = pipes2[0];

    pid_t tracee_id = fork();
    if (tracee_id == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    // Tracer
    if (tracee_id != 0) {
        // printf("tracee id = %d", tracee_id);
        close(tracee_out_pipe);
        close(tracee_in_pipe);

        tracer(tracer_in_pipe, tracer_out_pipe, tracee_id);

        close(tracer_in_pipe);
        close(tracer_out_pipe);

        wait(NULL);
        exit(EXIT_SUCCESS);
    }
    // Tracee
    else {
        close(tracer_in_pipe);
        close(tracer_out_pipe);

        tracee(tracee_in_pipe, tracee_out_pipe, iter_count);

        close(tracee_out_pipe);
        close(tracee_in_pipe);
        exit(EXIT_SUCCESS);
    }
}
