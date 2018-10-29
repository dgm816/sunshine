CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

add_executable(sunshine)

find_package(OpenSSL 1.1.0 REQUIRED)

TARGET_LINK_LIBRARIES(
        sunshine
        ${OPENSSL_LIBRARIES}
)

TARGET_SOURCES(
        sunshine
        PRIVATE main.c
)
