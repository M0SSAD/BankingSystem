# Require a relatively modern CMake version
cmake_minimum_required(VERSION 3.15)

# Define the project and language
project(ConcurrentBankingSystem VERSION 1.0 LANGUAGES CXX)

# Enforce Modern C++ Standard (C++17 is the baseline for std::scoped_lock)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Disable compiler-specific extensions (e.g., GNU extensions) to ensure standard compliance
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the include directory so files in src/ can use #include "Account.h" cleanly
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find all source files automatically
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Define the final executable
add_executable(BankingSystem ${SOURCES})

# Hardware & System Integration: Threading
# Since this is a concurrent banking system, we MUST link the OS threading library.
# On Linux, this links pthread, which allows std::mutex and std::thread to function.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(BankingSystem PRIVATE Threads::Threads)

# Enforce Strict Compiler Warnings
# This is critical for catching memory bugs and modern C++ anti-patterns early.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(BankingSystem PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic 
        -Wshadow         # Warns if a local variable shadows another variable
        -Wnon-virtual-dtor # Warns if a class with virtual functions has a non-virtual destructor (prevents memory leaks)
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(BankingSystem PRIVATE /W4)
endif()