#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "../common.h"

int main(int argc, char const *argv[]) {
    int iter_count = DEFAULT_ITER_COUNT;

    if (argc > 1) {
        long conversion_result = strtol(argv[1], NULL, 10);
        if (errno == 0) {
            iter_count = (int) conversion_result;
        }
    }
    for (int i = 0; i < iter_count; i++) {
        pid_t my_pid = getpid();
        // printf("my pid = %d\n", my_pid);
    }
    return 0;
}
