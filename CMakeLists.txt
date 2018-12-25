CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

add_executable(sunshine)

set_target_properties(
        sunshine
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

find_package(OpenSSL 1.1.0 REQUIRED)

TARGET_LINK_LIBRARIES(
        sunshine
        ${OPENSSL_LIBRARIES}
)

TARGET_SOURCES(
        sunshine
        PRIVATE src/main.c
        PRIVATE src/config.c
)

TARGET_INCLUDE_DIRECTORIES(
        sunshine
        PRIVATE include
)