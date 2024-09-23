cmake_minimum_required(VERSION 3.5.0)
project(port VERSION 0.1.0 LANGUAGES C CXX)

enable_testing()
include(CTest)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DFDEBUG")

include(FetchContent)
FetchContent_Declare(
    uv
    URL https://dist.libuv.org/dist/v1.48.0/libuv-v1.48.0.tar.gz
)

FetchContent_MakeAvailable(uv)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/build/_deps/uv-src/include)

add_executable(port main.cpp)
target_link_libraries(port PUBLIC uv)

add_subdirectory(src)
add_subdirectory(test)