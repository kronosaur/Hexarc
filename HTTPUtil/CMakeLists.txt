# Auto-generated CMake for HTTPUtil
file(GLOB SRC "*.cpp")
add_library(HTTPUtil STATIC ${SRC})
target_include_directories(HTTPUtil PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(HTTPUtil PUBLIC AEON Foundation OpenSSL )

