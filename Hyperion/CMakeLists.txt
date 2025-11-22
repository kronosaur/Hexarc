# Auto-generated CMake for Hyperion
file(GLOB SRC "*.cpp")
add_library(Hyperion STATIC ${SRC})
target_include_directories(Hyperion PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(Hyperion PUBLIC AEON ArchonClient ArchonEngine Foundation Hexe LuminousCore OpenSSL zlibstat )

