project(GUI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOGEN_SOURCE_GROUP "Generated Files")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QCP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/qcustomplot")

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets PrintSupport)

file(GLOB_RECURSE QCP_SOURCES "${QCP_DIR}/src/*.cpp")
file(GLOB_RECURSE QCP_HEADERS "${QCP_DIR}/inc/*.h")

source_group("3rdparty\\QCustomPlot" FILES ${QCP_SOURCES} ${QCP_HEADERS})

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

file(GLOB_RECURSE QRC_FILES CONFIGURE_DEPENDS "resources/*.qrc")
file(GLOB_RECURSE QML_FILES CONFIGURE_DEPENDS "resources/qml/*.qml")

add_executable(GUI WIN32
    ${SOURCES} 
    ${HEADERS}
    ${QCP_SOURCES}
    ${QCP_HEADERS}
    ${QRC_FILES}
    ${QML_FILES}
)

source_group("Generated Files" REGULAR_EXPRESSION ".*_autogen.*|.*mocs_compilation.*\\.cpp|.*qrc_.*\\.cpp")
source_group("QML Files" REGULAR_EXPRESSION ".*\\.qml$")
source_group("" REGULAR_EXPRESSION ".*\\.qrc$")
source_group("CMake Rules" REGULAR_EXPRESSION ".*\\.rule$")

target_include_directories(GUI PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${QCP_DIR}/inc 
)

target_link_libraries(GUI
    PRIVATE Qt6::Quick Qt6::Widgets Qt6::PrintSupport
)

# --- Создание изолированной Portable версии ---

get_target_property(_qt_core_location Qt6::Core IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qt_core_location}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

if(WIN32 AND WINDEPLOYQT_EXECUTABLE)
    # Задаем имя независимой папки (она появится в директории сборки build)
    set(PORTABLE_DIR "${CMAKE_BINARY_DIR}/PortableApp")

    add_custom_command(TARGET GUI POST_BUILD
        # 1. Создаем чистую папку (если ее еще нет)
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PORTABLE_DIR}"
        
        # 2. Копируем туда только сам скомпилированный исполняемый файл
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GUI>" "${PORTABLE_DIR}"
        
        # 3. Запускаем windeployqt для файла внутри этой новой папки
        COMMAND ${WINDEPLOYQT_EXECUTABLE}
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/resources/qml"
                --compiler-runtime
                # --no-translations # раскомментируйте, если не хотите тащить файлы переводов самого Qt
                "${PORTABLE_DIR}/$<TARGET_FILE_NAME:GUI>"
                
        COMMENT "Создание полностью независимой папки ${PORTABLE_DIR} с EXE и DLL..."
    )
endif()