cmake_minimum_required(VERSION 3.16)

project(tiny_webserver
    VERSION 1.0.0
    DESCRIPTION "Tiny HTTP Webserver — MISRA C Demo Project"
    LANGUAGES C
)

# ─── C Standard ─────────────────────────────────────────────────────────────
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ─── Compiler Warnings ───────────────────────────────────────────────────────
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -fstack-protector-strong
    )
endif()

# ─── Source Files ────────────────────────────────────────────────────────────
set(SOURCES
    src/main.c
    src/http_server.c
    src/http_parser.c
    src/router.c
    src/logger.c
)

# ─── Include Directories ─────────────────────────────────────────────────────
include_directories(include)

# ─── Executable ──────────────────────────────────────────────────────────────
add_executable(tiny_webserver ${SOURCES})

# ─── Install ─────────────────────────────────────────────────────────────────
install(TARGETS tiny_webserver DESTINATION bin)

# ─── Export compile commands (for clang-tidy / cppcheck) ────────────────────
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ─── Custom Target: MISRA Check via cppcheck ─────────────────────────────────
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
            ${CMAKE_SOURCE_DIR}/src/
        COMMENT "Running MISRA C:2012 check via cppcheck..."
        VERBATIM
    )
    message(STATUS "cppcheck found: MISRA check target enabled ('make misra_check')")
else()
    message(WARNING "cppcheck NOT found — install it to enable 'make misra_check'")
endif()
