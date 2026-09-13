cmake_minimum_required(VERSION 3.14.0)
project(EmuControl VERSION 1.5.0)

include(cmake/verstring.cmake)
get_verstring(VERSTRING)

if(NOT TARGET devicetree)
    add_subdirectory(devicetree.resource EXCLUDE_FROM_ALL)
endif()

add_executable(EmuControl
    src/startup.c
    src/main.c
    src/mbox.c
    src/presets.c
)

target_link_options(EmuControl PRIVATE -ffreestanding -m68040 -nostdlib -nostartfiles -Wl,-e,__start)
target_compile_options(EmuControl PRIVATE -Os -m68040 -m68881 -fomit-frame-pointer)
target_compile_definitions(EmuControl PRIVATE VERSION_STRING="${VERSTRING}")
target_include_directories(EmuControl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(EmuControl mui amiga devicetree)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/EmuControl DESTINATION EmuControl/)
install(FILES EmuControl.info DESTINATION EmuControl/)
