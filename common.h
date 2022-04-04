#ifndef PERFOMANCE_TEST_COMMON_H
#define PERFOMANCE_TEST_COMMON_H

// x86_64 architecture
//      :~$ ausyscall --dump              -- shows syscall + number
#define SYSCALL_CODE_GETPID 39

#define DEFAULT_ITER_COUNT 10e5

#endif //PERFOMANCE_TEST_COMMON_H
