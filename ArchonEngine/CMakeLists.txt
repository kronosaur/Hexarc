# Auto-generated CMake for ArchonEngine
file(GLOB SRC "*.cpp")
add_library(ArchonEngine STATIC ${SRC})
target_include_directories(ArchonEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(ArchonEngine PUBLIC AEON ArchonClient HTTPUtil )

