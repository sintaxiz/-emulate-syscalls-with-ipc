ITER_COUNT=1000000

cmake .
cmake --build .
echo

echo time measurement: pipes vs ptrace...
echo iteration count: $ITER_COUNT
echo 

echo with pipes:
time ./pipes_tracer $ITER_COUNT
echo

echo with ptrace:
time ./ptrace_tracer $ITER_COUNT