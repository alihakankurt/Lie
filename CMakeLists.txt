cmake_minimum_required(VERSION 3.27)
project(Lie VERSION 0.1.0 LANGUAGES C)

## -------------------------- ##
##   Global CMake Variables   ##
## -------------------------- ##
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Bin)

## -------------------------- ##
##      Compiler Options      ##
## -------------------------- ##
add_library(CompileOptions INTERFACE)

if (CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(CompileOptions INTERFACE
        ## Warnings
        -Wall -Wextra # Enable all warnings

        -Wshadow # Warn whenever a local variable shadows another local variable
        -Wcast-align # Warn whenever a cast changes the alignment of the pointer
        -Wconversion # Warn for implicit conversions that may alter a value
        -Wsign-conversion # Warn for implicit conversions that may change the sign of an integer value
        -Wunused # Warn whenever a local variable is unused

        -Wmissing-noreturn # Warn about functions which might be candidates for attribute noreturn
        -Wunreachable-code # Warn about code that will never be executed

        -Wformat=2 # Warn about format string problems

        -Wno-unused-function # Disable warnings for unused functions
        -Wno-unused-parameter # Disable warnings for unused parameters

        ## Errors
        -Werror=nonnull # Error when a null pointer constant is passed to [[nonnull]] parameter

        -Werror=address # Error when a suspicious pointer usage is detected
        -Werror=init-self # Error when a variable is initialized with itself
        -Werror=uninitialized # Error when an uninitialized variable is used
        -Werror=pointer-arith # Error on pointer arithmetic for void and function pointers
        -Werror=write-strings # Error on writing to read-only memory

        -Werror=return-type # Error when a return type is mismatched

        -Werror=switch # Error on missing cases in switch statements
        -Werror=implicit-fallthrough # Error on implicit fallthrough in switch statements

        -Werror=missing-include-dirs # Error on missing include directories

        ## Optimization
        $<$<CONFIG:Debug>:-O0 -g3 -ggdb3>
        $<$<CONFIG:Release>:-O3 -march=native>
    )
elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
    target_compile_options(CompileOptions INTERFACE
        /W4 # Enable all warnings
        /wd4201 # Disable warnings for nonstandard extension used: nameless struct/union
        /utf-8 # Set source and execution character set to UTF-8

        ## Optimization
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/O2>
    )
endif()

if (WIN32)
    target_compile_definitions(CompileOptions INTERFACE
        _CRT_SECURE_NO_WARNINGS # Disable warnings for unsafe functions
        _CRT_SECURE_NO_WARNINGS_GLOBALS # Disable warnings for unsafe functions
        _CRT_NONSTDC_NO_WARNINGS # Disable warnings for non-ANSI functions
    )
endif()

## -------------------------- ##
##    Include Directories     ##
## -------------------------- ##
add_library(Includes INTERFACE)
target_include_directories(Includes INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Include>
    $<INSTALL_INTERFACE:Include>
)
target_precompile_headers(Includes INTERFACE Include/Core.h)

## -------------------------- ##
##         Executable         ##
## -------------------------- ##
set(Sources
    Source/Lie.c
    Source/Command.c
    Source/Terminal/Unix.c
)

add_executable(${PROJECT_NAME} ${Sources})
target_link_libraries(${PROJECT_NAME} PRIVATE CompileOptions Includes)
