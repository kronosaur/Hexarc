# Auto-generated CMake for Arcology
file(GLOB SRC "*.cpp")
add_executable(Arcology ${SRC})
target_include_directories(Arcology PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(Arcology PUBLIC AEON ArchonClient ArchonEngine Cryptosaur HexeEngine Hexe Hyperion LuminousCore OpenSSL Win32Service version ws2_32)

