#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../common.h"

int pipe_close(int fd, int in_pipe, int out_pipe) {
    write(out_pipe, &fd, 4);
    int close_result;
    read(in_pipe, &close_result, 4);
    // printf("%d\n", close_result);
    return close_result;
}

void tracee(int in_pipe, int out_pipe, int iter_count) {
    for (int i = 0; i < iter_count; i++) {
        pipe_close(1337, in_pipe, out_pipe);
    }
}

void tracer(int in_pipe, int out_pipe) {
    int fd;
    while (read(in_pipe, &fd, 4) > 0) {
        int close_result = close(1337);
        write(out_pipe, &close_result, sizeof(close_result));
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
        close(tracee_out_pipe);
        close(tracee_in_pipe);

        tracer(tracer_in_pipe, tracer_out_pipe);

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
    return 0;
}
