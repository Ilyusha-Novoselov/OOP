project(GUI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOGEN_SOURCE_GROUP "Generated Files")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets PrintSupport)

file(GLOB_RECURSE SOURCES
	"src/*.cxx"
	"src/*.cpp"
	"src/*.c"
)

file(GLOB_RECURSE HEADERS
	"inc/*.hxx"
	"inc/*.hpp"
	"inc/*.h"
)

file(GLOB_RECURSE QRC_FILES "resources/*.qrc")
file(GLOB_RECURSE QML_FILES "resources/qml/*.qml")

add_executable(GUI WIN32
    ${SOURCES} 
	${HEADERS}
    ${QRC_FILES}
    ${QML_FILES}
)

source_group("Generated Files" REGULAR_EXPRESSION ".*_autogen.*|.*mocs_compilation.*\\.cpp|.*qrc_.*\\.cpp")
source_group("QML Files" REGULAR_EXPRESSION ".*\\.qml$")
source_group("" REGULAR_EXPRESSION ".*\\.qrc$")
source_group("CMake Rules" REGULAR_EXPRESSION ".*\\.rule$")

target_include_directories(GUI PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(GUI
    PRIVATE Qt6::Quick Qt6::Widgets Qt6::PrintSupport
)