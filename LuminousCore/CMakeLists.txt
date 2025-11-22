# Auto-generated CMake for LuminousCore
file(GLOB SRC "*.cpp")
add_library(LuminousCore STATIC ${SRC})
target_include_directories(LuminousCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(LuminousCore PUBLIC AEON Foundation Hexe zlibstat )

