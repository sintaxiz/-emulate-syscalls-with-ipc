#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include "../common.h"

int main(int argc, char const *argv[]) {
    int iter_count = DEFAULT_ITER_COUNT;

    if (argc > 1) {
        long conversion_result = strtol(argv[1], NULL, 10);
        if (errno == 0) {
            iter_count = (int) conversion_result;
        }
    }
        struct timeval t0, t1, dt;
      
        gettimeofday(&t0, NULL);

    for (int i = 0; i < iter_count; i++) {
        pid_t my_pid = getpid();
        // printf("my pid = %d\n", my_pid);
    }
        gettimeofday(&t1, NULL);
        timersub(&t1, &t0, &dt);
fprintf(stdout, "doSomeThing (thread %ld) took %d.%06d sec\n",
               (long)pthread_self(), dt.tv_sec, dt.tv_usec);

    return 0;
}
