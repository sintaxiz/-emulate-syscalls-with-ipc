#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include "../common.h"

const int SHM_SIZE_BYTES = 1024;
const int SHM_MODE = 0400 + 0200; // read + write for user

enum cmd_t {
    EXIT, SYSCALL
};

#define SHM_TRACER_WAITS 0X00
#define SHM_TRACER_WRITES 0X01

typedef struct cmd {
    enum cmd_t cmd_type;
} cmd;

void check_error(int code, char *err_desc) {
    if (code == -1) {
        perror(err_desc);
        exit(-1);
    }
}

char *write_cmd(char *shm_addr, cmd command) {
    *(cmd *) shm_addr = command;
    return shm_addr + sizeof(cmd);
}

void write_close_syscall(char *shm_addr, int fd) {
    char *shm_pos = shm_addr;
    *shm_pos = SYSCALL_CODE_CLOSE;
    ++shm_pos;
    *(int *) shm_pos = fd;
}

int shm_close(int fd, char *shm_addr) {
    cmd close_cmd = {SYSCALL};
    char *curr_shm_addr = write_cmd(shm_addr + 1, close_cmd);
    write_close_syscall(curr_shm_addr, fd);

    *shm_addr = SHM_TRACER_WRITES;
    while (*shm_addr != SHM_TRACER_WAITS);
    int return_code = *(int *) (shm_addr + 1);
    // printf("answer is: %d\n", return_code);
    return return_code;
}


void shm_exit(char *shm_addr) {
    cmd command = {EXIT};
    write_cmd(shm_addr + 1, command);
    *shm_addr = SHM_TRACER_WRITES;
}

void tracee(char *shm_addr, int iter_count) {
    for (int i = 0; i < iter_count; ++i) {
        shm_close(1337, shm_addr);
    }
    shm_exit(shm_addr);
}

cmd wait_for_cmd(const char *shm_addr) {
    while (*shm_addr == SHM_TRACER_WAITS);
    return *(cmd *) (shm_addr + 1);
}

void tracer(char *shm_addr) {
    bool tracing = true;
    while (tracing) {
        cmd command = wait_for_cmd(shm_addr);
        char *shm_pos = shm_addr + sizeof(cmd) + 1;
        switch (command.cmd_type) {
            case EXIT:
                tracing = false;
                // printf("Get exit command. Exiting...\n");
                break;
            case SYSCALL:
                // printf("Get syscall command\n");
                char syscall = *(shm_pos);
                if (syscall == SYSCALL_CODE_CLOSE) {
                    int fd = *(int *) (shm_pos + 1);
                    // printf("get close request. %d\n", fd);
                    *(int *) (shm_addr + 1) = close(fd);
                }
                break;
            default:
                // printf("error command type! ignoring\n");
                break;
        }
        *shm_addr = 0x00;
    }
}

int main(int argc, char **argv) {
    int iter_count = DEFAULT_ITER_COUNT;
    if (argc > 1) {
        long conversion_result = strtol(argv[1], NULL, 10);
        if (errno != 0) {
            iter_count = DEFAULT_ITER_COUNT;
        } else {
            iter_count = (int)conversion_result;
        }
    }

    int shm_id = shmget(IPC_PRIVATE, SHM_SIZE_BYTES, SHM_MODE);          // create shared memory segment
    check_error(shm_id, "shmget");

    char *shm_addr = shmat(shm_id, 0, 0);               // attach shared memory to process
    if (shm_addr == (void *) -1) {
        perror("shmat");
        exit(-1);
    }

    *shm_addr = SHM_TRACER_WAITS; // init state -- no communication btw processes

    int pid = fork();
    if (pid > 0) {
        tracer(shm_addr);
    } else if (pid == 0) {
        tracee(shm_addr, iter_count);
        exit(0);
    } else {
        perror("fork");
        exit(-1);
    }
    shmdt(shm_addr);                                                    // detach
    shmctl(shm_id, IPC_RMID, NULL);                                // delete shared memory segment

    return 0;
}
