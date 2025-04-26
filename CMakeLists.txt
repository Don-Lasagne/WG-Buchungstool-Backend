cmake_minimum_required(VERSION 3.22.1)
project(wg_buchungstool_backend C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -pedantic-errors -Wall -Wextra -Werror -Wconversion")

add_executable(${PROJECT_NAME}
        src/http_server.c
        src/httplib.c
        src/stringstructlib.c)
add_executable(${PROJECT_NAME}_test
        test/httplib-test.c
        src/httplib.c
        src/stringstructlib.c)
