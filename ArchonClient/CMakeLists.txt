# Auto-generated CMake for ArchonClient
file(GLOB SRC "*.cpp")
add_library(ArchonClient STATIC ${SRC})
target_include_directories(ArchonClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(ArchonClient PUBLIC HTTPUtil OpenSSL AEON)

