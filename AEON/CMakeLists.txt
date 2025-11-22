# Auto-generated CMake for AEON
file(GLOB SRC "*.cpp")
add_library(AEON STATIC ${SRC})
target_include_directories(AEON PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(AEON PUBLIC Foundation )

