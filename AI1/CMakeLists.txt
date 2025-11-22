# Auto-generated CMake for AI1
file(GLOB SRC "*.cpp")
add_executable(AI1 ${SRC})
target_include_directories(AI1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(AI1 PUBLIC ArchonClient Hexe OpenSSL AEON Foundation)

