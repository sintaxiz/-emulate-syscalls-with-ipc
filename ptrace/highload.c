#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int iter_count = 10e5;

int main(int argc, char const *argv[]) {
    if (argc > 1) {
        long conversion_result = strtol(argv[1], NULL, 10);
        if (errno == 0) {
            iter_count = (int) conversion_result;
        }
    }
    for (int i = 0; i < iter_count; i++) {
        close(1337);
    }
    return 0;
}
