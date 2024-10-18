FOLDER=$(pwd)
cd build/port || exit
ctest -T memcheck
cd "$FOLDER" || exit

#compile
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j4
