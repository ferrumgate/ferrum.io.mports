FOLDER=$(pwd)
cd build/port || exit
ctest -T memcheck
cd "$FOLDER" || exit
