include(ExternalProject)

set(JPEG_DEPS ZLIB)
if(NOT WIN32)
    set(JPEG_DEPS ${JPEG_DEPS} NASM)
endif()

set(JPEG_ENABLE_SHARED ON)
set(JPEG_ENABLE_STATIC OFF)
if(NOT BUILD_SHARED_LIBS)
    set(JPEG_ENABLE_SHARED OFF)
    set(JPEG_ENABLE_STATIC ON)
endif()

set(JPEG_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib
    -DENABLE_SHARED=${JPEG_ENABLE_SHARED}
    -DENABLE_STATIC=${JPEG_ENABLE_STATIC})
if(NOT WIN32)
    set(JPEG_ARGS ${JPEG_ARGS} -DCMAKE_ASM_NASM_COMPILER=${CMAKE_INSTALL_PREFIX}/bin/nasm)
endif()

ExternalProject_Add(
    JPEG
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/JPEG
    DEPENDS ${JPEG_DEPS}
    URL "http://sourceforge.net/projects/libjpeg-turbo/files/2.0.6/libjpeg-turbo-2.0.6.tar.gz?download"
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/JPEG-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/JPEG/src/JPEG/CMakeLists.txt
    CMAKE_ARGS ${JPEG_ARGS})
