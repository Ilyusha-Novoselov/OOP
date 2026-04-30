project(Laba_2 VERSION 1.0 LANGUAGES CXX)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cxx"
    "src/*.cpp"
    "src/*.c"
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "inc/*.hxx"
    "inc/*.hpp"
    "inc/*.h"
)

add_executable(Laba_2
    ${SOURCES} 
    ${HEADERS}
)

source_group("CMake Rules" REGULAR_EXPRESSION ".*\\.rule$")

target_include_directories(Laba_2 PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)