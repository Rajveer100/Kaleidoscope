# Add LLVM component libraries.
add_llvm_library(LLVMKaleidoscope
  ASTExpr.cpp
  Parser.cpp

  ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include
)
