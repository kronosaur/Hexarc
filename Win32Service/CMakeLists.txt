# Auto-generated CMake for Win32Service
file(GLOB SRC "*.cpp")
add_library(Win32Service STATIC ${SRC})
target_include_directories(Win32Service PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)

