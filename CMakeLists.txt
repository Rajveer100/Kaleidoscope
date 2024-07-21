cmake_minimum_required(VERSION 3.28)
project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 23)

set(LLVM_DIR "/opt/homebrew/Cellar/llvm/18.1.8/lib/cmake/llvm" CACHE PATH "LLVM installation directory")

list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew/Cellar/llvm/18.1.8/")

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs support core irreader)

add_subdirectory(src)
