# Get LLVM libs property.
llvm_map_components_to_libnames(LLVM_LIBS 
  Support
  Core 
  IRReader 
  Analysis
  AsmParser
  AsmPrinter
  ExecutionEngine
  BitReader
  BitWriter
  TransformUtils
  CodeGen
  Target
  Object
  Remarks
  OrcJIT
  RuntimeDyld
  Support
  AArch64
)

# Add main driver executable.
add_llvm_executable(main-driver MainDriver.cpp)

# Update compile flags and link libraries.
llvm_update_compile_flags(main-driver)

target_link_libraries(main-driver PRIVATE LLVMKaleidoscope ${LLVM_LIBS})
