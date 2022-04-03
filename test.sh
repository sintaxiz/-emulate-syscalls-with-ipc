#!/bin/sh
set -e

ITER_COUNT=1000000

cmake .
cmake --build .
echo

echo  time measurement:   pipes vs ptrace vs shared memory...
echo  iteration count :    $ITER_COUNT
echo 

echo with pipes:
time ./pipes_tracer $ITER_COUNT
echo

echo with ptrace:
time ./ptrace_tracer $ITER_COUNT
echo

echo with shared memory:
time ./sm_tracer $ITER_COUNT