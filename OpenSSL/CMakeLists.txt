# Auto-generated CMake for OpenSSL
file(GLOB SRC "*.cpp")
add_library(OpenSSL STATIC ${SRC})

# OpenSSLUtil/CMakeLists.txt (or wherever you define OpenSSLUtil)
set(OPENSSL_ROOT ${PROJECT_SOURCE_DIR}/Dependencies/openssl)

target_include_directories(OpenSSL PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Include
	${OPENSSL_ROOT}/include
)

target_link_libraries(OpenSSL PUBLIC 

	Foundation

    # Debug
    $<$<CONFIG:Debug>:${OPENSSL_ROOT}/lib/Debug/libssl.lib>
    $<$<CONFIG:Debug>:${OPENSSL_ROOT}/lib/Debug/libcrypto.lib>

    # Release
    $<$<CONFIG:Release>:${OPENSSL_ROOT}/lib/Release/libssl.lib>
    $<$<CONFIG:Release>:${OPENSSL_ROOT}/lib/Release/libcrypto.lib>

    ws2_32 crypt32 user32 gdi32 advapi32  # Win system libs OpenSSL needs
)

