cmake_minimum_required (VERSION 2.8.11)
project (perfomance_test)
add_executable (pipes_tracer ./pipes/tracer.c common.h)
add_executable (ptrace_highload direct-syscall/highload.c)
add_executable (ptrace_tracer ./ptrace/tracer.c)
add_executable (direct_syscall ./direct-syscall/highload.c)

add_executable (sm_tracer shared-memory/tracer.c)