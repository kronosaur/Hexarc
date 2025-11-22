# Auto-generated CMake for HexeEngine
file(GLOB SRC "*.cpp")
add_library(HexeEngine STATIC ${SRC})
target_include_directories(HexeEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(HexeEngine PUBLIC AEON ArchonClient ArchonEngine Foundation Hexe OpenSSL zlibstat )

