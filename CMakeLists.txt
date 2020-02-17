cmake_minimum_required(VERSION 3.5)
project(ccgit C)

set(CMAKE_C_STANDARD 11)

add_executable(ccgit driver.c lexer.c hash.c hash.h lexer.h lexerDef.h parser.c parser.h parserDef.h)