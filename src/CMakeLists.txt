file(GLOB HEADERS CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/include/Lexer.h"
        "${CMAKE_SOURCE_DIR}/include/Parser.h"
        "${CMAKE_SOURCE_DIR}/include/ASTExpr.h"
        "${CMAKE_SOURCE_DIR}/include/Logger.h"
        "${CMAKE_SOURCE_DIR}/include/CodeGen.h")

file(GLOB SOURCES
        "Lexer.cpp"
        "ASTExpr.cpp"
        "Logger.cpp"
        "Parser.cpp"
        "MainDriver.cpp")

add_library(Kaleidoscope ${SOURCES} ${HEADERS})

target_include_directories(Kaleidoscope PUBLIC ../include)

target_link_libraries(Kaleidoscope PRIVATE ${llvm_libs})

add_executable(MainDriver MainDriver.cpp)

target_link_libraries(MainDriver PRIVATE Kaleidoscope)
