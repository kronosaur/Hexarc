# Auto-generated CMake for Cryptosaur
file(GLOB SRC "*.cpp")
add_library(Cryptosaur STATIC ${SRC})
target_include_directories(Cryptosaur PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(Cryptosaur PUBLIC AEON ArchonClient ArchonEngine Foundation Hexe OpenSSL zlibstat )

