# Require a relatively modern CMake version
cmake_minimum_required(VERSION 3.15)

# Define the project and language
project(ConcurrentBankingSystem VERSION 1.0 LANGUAGES CXX)

# Enforce Modern C++ Standard (C++17 is the baseline for std::scoped_lock)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Disable compiler-specific extensions (e.g., GNU extensions) to ensure standard compliance
set(CMAKE_CXX_EXTENSIONS OFF)


include_directories(${CMAKE_SOURCE_DIR}/include)

# Core backend sources
set(CORE_SOURCES
    src/Account.cpp
    src/Bank.cpp
    src/Customer.cpp
    src/TransactionManager.cpp
)

add_library(BankingCore ${CORE_SOURCES})
target_include_directories(BankingCore PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Hardware & System Integration: Threading
# Since this is a concurrent banking system, we MUST link the OS threading library.
# On Linux, this links pthread, which allows std::mutex and std::thread to function.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(BankingCore PRIVATE Threads::Threads)

# Qt GUI target
find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(BankingSystemQt
    src/ui/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/LoginScreen.cpp
    src/ui/LoginScreen.h
    src/ui/AdminDashboard.cpp
    src/ui/AdminDashboard.h
    src/ui/CustomerDashboard.cpp
    src/ui/CustomerDashboard.h
    src/ui/SessionManager.cpp
    src/ui/SessionManager.h
    src/ui/BackendWorker.cpp
    src/ui/BackendWorker.h
    src/ui/AccountTableModel.cpp
    src/ui/AccountTableModel.h
    src/ui/AuditLogModel.cpp
    src/ui/AuditLogModel.h
)
target_link_libraries(BankingSystemQt PRIVATE Qt6::Widgets BankingCore)

# Enforce Strict Compiler Warnings
# This is critical for catching memory bugs and modern C++ anti-patterns early.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(BankingCore PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow         # Warns if a local variable shadows another variable
        -Wnon-virtual-dtor # Warns if a class with virtual functions has a non-virtual destructor (prevents memory leaks)
    )
    target_compile_options(BankingSystemQt PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(BankingCore PRIVATE /W4)
    target_compile_options(BankingSystemQt PRIVATE /W4)
endif()
