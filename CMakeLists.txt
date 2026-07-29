cmake_minimum_required(VERSION 3.20)
project(jzip VERSION 2.1 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

option(ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

find_package(ZLIB REQUIRED)
find_library(TERMCAP_LIBRARY NAMES termcap REQUIRED)

add_executable(jzip
    jzip.c
    control.c
    extern.c
    fileio.c
    input.c
    interpre.c
    license.c
    math.c
    memory.c
    object.c
    operand.c
    osdepend.c
    property.c
    quetzal.c
    screen.c
    text.c
    variable.c
    unixio.c
)

target_compile_definitions(jzip PRIVATE
    POSIX
    HAVE_GETOPT
    HARD_COLORS
    USE_ZLIB
)

target_link_libraries(jzip PRIVATE
    ZLIB::ZLIB
    ${TERMCAP_LIBRARY}
)

add_executable(ckifzs ckifzs.c)

if(ENABLE_SANITIZERS)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
        message(FATAL_ERROR "ENABLE_SANITIZERS requires Clang, AppleClang, or GCC")
    endif()

    set(SANITIZER_FLAGS
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -g
    )

    foreach(target jzip ckifzs)
        target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${target} PRIVATE ${SANITIZER_FLAGS})
    endforeach()
endif()

include(CTest)

set(DFROTZ_EXECUTABLE "/opt/homebrew/bin/dfrotz" CACHE FILEPATH
    "Path to the dfrotz executable used by the interoperability test"
)

if(BUILD_TESTING)
    add_test(
        NAME jzip_frotz_quetzal_interoperability
        COMMAND ${CMAKE_COMMAND} -E env
            "JZIP=$<TARGET_FILE:jzip>"
            "CKIFZS=$<TARGET_FILE:ckifzs>"
            "DFROTZ=${DFROTZ_EXECUTABLE}"
            "ASAN_OPTIONS=halt_on_error=1"
            "UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1"
            /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/testdata/test-interop.sh
    )
endif()
