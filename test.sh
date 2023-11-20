cmake -B build -DCMAKE_BUILD_TYPE=Release
cd build && make -j4
cd test

echo "========================================================================"
echo "Running bench test 1..."
./test_bench1 1 100000000
./test_bench1 2 100000000
./test_bench1 3 100000000
./test_bench1 4 100000000
./test_bench1 5 100000000
./test_bench1 6 100000000
echo "========================================================================"
echo "Running bench test 2..."
./test_bench2 1 100000000
./test_bench2 2 100000000
./test_bench2 3 100000000
./test_bench2 4 100000000
./test_bench2 5 100000000
./test_bench2 6 100000000
echo "========================================================================"
echo "Running bench test 3..."
./test_bench3 2 100000000
./test_bench3 4 100000000
./test_bench3 6 100000000
./test_bench3 8 100000000
./test_bench3 10 100000000
echo "========================================================================"

echo "Test end"