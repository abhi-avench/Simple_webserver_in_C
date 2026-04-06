cmake_minimum_required(VERSION 3.16)

project(tiny_webserver
    VERSION 1.0.0
    DESCRIPTION "Tiny HTTP Webserver - MISRA C Project"
    LANGUAGES C
)

# C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/http_server.c
    ${CMAKE_SOURCE_DIR}/src/http_parser.c
    ${CMAKE_SOURCE_DIR}/src/router.c
    ${CMAKE_SOURCE_DIR}/src/logger.c
)

# Executable
add_executable(tiny_webserver ${SOURCES})

# Include directories
target_include_directories(tiny_webserver
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

# Compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(tiny_webserver PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -fstack-protector-strong
    )
endif()

# Put executable directly in build/
set_target_properties(tiny_webserver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Install
install(TARGETS tiny_webserver DESTINATION bin)

# MISRA check via cppcheck
find_program(CPPCHECK cppcheck)

if(CPPCHECK)
    add_custom_target(misra_check
        COMMAND ${CPPCHECK}
            --enable=all
            --addon=misra
            --std=c99
            --suppress=missingIncludeSystem
            --output-file=${CMAKE_BINARY_DIR}/misra_report.txt
            -I ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
        COMMENT "Running MISRA C:2012 check via cppcheck..."
        VERBATIM
    )
    message(STATUS "cppcheck found: MISRA check target enabled ('cmake --build build --target misra_check')")
else()
    message(WARNING "cppcheck NOT found - install it to enable 'misra_check'")
endif()