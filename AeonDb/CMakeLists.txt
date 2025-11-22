# Auto-generated CMake for AeonDb
file(GLOB SRC "*.cpp")
add_executable(AeonDb ${SRC})
target_include_directories(AeonDb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(AeonDb PUBLIC AEON ArchonClient ArchonEngine Foundation Hexe LuminousCore version ws2_32)

