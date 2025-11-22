# Auto-generated CMake for Hexe
file(GLOB SRC "*.cpp")
add_library(Hexe STATIC ${SRC})
target_include_directories(Hexe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(Hexe PUBLIC AEON Foundation )

