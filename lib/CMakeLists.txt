# Add LLVM component libraries.
add_llvm_library(LLVMKaleidoscope
  ASTExpr.cpp
  Lexer.cpp
  Logger.cpp
  Parser.cpp

  ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include
)
