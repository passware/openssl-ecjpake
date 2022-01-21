PROJECT(openssl-ecjpake)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(SOURCES
  ecjpake.c
  ecjpake_err.c

  openssl/ecjpake.h
)

ADD_LIBRARY(openssl-ecjpake ${SOURCES})
SET_TARGET_PROPERTIES(openssl-ecjpake PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/openssl/ecjpake.h)

TARGET_LINK_LIBRARIES(openssl-ecjpake PRIVATE OpenSSL::Crypto)
TARGET_INCLUDE_DIRECTORIES(openssl-ecjpake PRIVATE openssl)

INSTALL(
    TARGETS openssl-ecjpake
    PUBLIC_HEADER DESTINATION include/openssl
)

IF (${BUILD_TESTS})
    ADD_EXECUTABLE(openssl-ecjpake-test ecjpaketest.c)
    TARGET_LINK_LIBRARIES(openssl-ecjpake-test PRIVATE openssl-ecjpake OpenSSL::Crypto ZLIB::ZLIB)
    TARGET_INCLUDE_DIRECTORIES(openssl-ecjpake-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    IF (WIN32)
        TARGET_LINK_LIBRARIES(openssl-ecjpake-test PRIVATE Ws2_32.lib Crypt32.lib)
    ENDIF()

    INSTALL(TARGETS openssl-ecjpake-test)
ENDIF()
