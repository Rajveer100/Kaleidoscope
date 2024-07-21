# Set source file for main driver
set(SOURCES MainDriver.cpp)

# Create main driver executable
add_executable(KaleidoscopeMain ${SOURCES})

# Include necessary directories
target_include_directories(KaleidoscopeMain PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Link against library and LLVM components
target_link_libraries(KaleidoscopeMain PRIVATE LLVMKaleidoscope ${llvm_libs})
