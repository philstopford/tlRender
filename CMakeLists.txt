cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(TLRENDER_VERSION_MAJOR "0")
set(TLRENDER_VERSION_MINOR "1")
set(TLRENDER_VERSION_PATCH "0")
set(TLRENDER_VERSION ${TLRENDER_VERSION_MAJOR}.${TLRENDER_VERSION_MINOR}.${TLRENDER_VERSION_PATCH})
add_definitions(-DTLRENDER_VERSION_MAJOR=${TLRENDER_VERSION_MAJOR})
add_definitions(-DTLRENDER_VERSION_MINOR=${TLRENDER_VERSION_MINOR})
add_definitions(-DTLRENDER_VERSION_PATCH=${TLRENDER_VERSION_PATCH})
add_definitions(-DTLRENDER_VERSION="${TLRENDER_VERSION}")

project(
    tlRender
    VERSION ${TLRENDER_VERSION}
    DESCRIPTION "tlRender is an open source library for building playback and review applications for visual effects, film, and animation"
    HOMEPAGE_URL "https://github.com/darbyjohnston/tlRender"
    LANGUAGES CXX C)

#-------------------------------------------------------------------------------
# Build options

set(TLRENDER_MMAP TRUE CACHE BOOL "Enable memory-mapped file I/O")
set(TLRENDER_PYTHON FALSE CACHE BOOL "Enable Python support (for OTIO Python adapters)")
set(TLRENDER_GLFW TRUE CACHE BOOL "Enable support for GLFW")
set(TLRENDER_OCIO TRUE CACHE BOOL "Enable support for OpenColorIO")
set(TLRENDER_AUDIO TRUE CACHE BOOL "Enable support for audio")
set(TLRENDER_JPEG TRUE CACHE BOOL "Enable support for JPEG I/O")
set(TLRENDER_TIFF TRUE CACHE BOOL "Enable support for TIFF I/O")
set(TLRENDER_STB TRUE CACHE BOOL "Enable support for STB I/O (TGA, BMP, PSD)")
set(TLRENDER_PNG TRUE CACHE BOOL "Enable support for PNG I/O")
set(TLRENDER_EXR TRUE CACHE BOOL "Enable support for EXR I/O")
set(TLRENDER_FFMPEG TRUE CACHE BOOL "Enable support for FFmpeg I/O")
set(TLRENDER_USD FALSE CACHE BOOL "Enable support for USD")
set(TLRENDER_BMD FALSE CACHE BOOL "Enable support for Blackmagic Design devices")
set(TLRENDER_BMD_SDK "" CACHE PATH "Full path to the Blackmagic Design SDK")
if(WIN32 OR APPLE)
    set(TLRENDER_NFD_DEFAULT TRUE)
else()
    set(TLRENDER_NFD_DEFAULT FALSE)
endif()
set(TLRENDER_NFD ${TLRENDER_NFD_DEFAULT} CACHE BOOL "Enable support for native file dialogs")
set(TLRENDER_QT6 FALSE CACHE BOOL "Enable support for Qt6")
set(TLRENDER_QT5 FALSE CACHE BOOL "Enable support for Qt5")
set(TLRENDER_PROGRAMS TRUE CACHE BOOL "Build programs")
set(TLRENDER_EXAMPLES TRUE CACHE BOOL "Build examples")
set(TLRENDER_TESTS TRUE CACHE BOOL "Build tests")
set(TLRENDER_GCOV FALSE CACHE BOOL "Enable gcov code coverage")
set(TLRENDER_GPROF FALSE CACHE BOOL "Enable gprof code profiling")
set(TLRENDER_GL_DEBUG FALSE CACHE BOOL "Enable OpenGL debugging")

#-------------------------------------------------------------------------------
# Internal configuration

list(PREPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/cmake/Modules)

if(TLRENDER_MMAP)
    add_definitions(-DTLRENDER_MMAP)
endif()
if(TLRENDER_PYTHON)
    add_definitions(-DTLRENDER_PYTHON)
endif()
if(TLRENDER_USD)
    add_definitions(-DTLRENDER_USD)
endif()
if(TLRENDER_BMD)
    add_definitions(-DTLRENDER_BMD)
endif()
if(TLRENDER_QT6)
    add_definitions(-DTLRENDER_QT6)
elseif(TLRENDER_QT5)
    add_definitions(-DTLRENDER_QT5)
endif()

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    add_definitions(-DTLRENDER_ASSERT)
endif()

if(TLRENDER_TESTS)
    set(CTEST_OUTPUT_ON_FAILURE ON)
    enable_testing()
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/deps
    ${PROJECT_SOURCE_DIR}/deps/FSeq
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/etc
    ${PROJECT_SOURCE_DIR}/tests)

if(TLRENDER_GPROF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()
if(TLRENDER_GCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")# -fkeep-inline-functions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")# -fkeep-inline-functions")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

set(CMAKE_FIND_FRAMEWORK LAST)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#-------------------------------------------------------------------------------
# Dependencies

# Required dependencies
find_package(Threads REQUIRED)
find_package(GLM REQUIRED)
find_package(Imath REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(minizip REQUIRED)
find_package(OTIO REQUIRED)
find_package(FreeType)
find_package(OpenGL REQUIRED)

# GLFW dependency
if(TLRENDER_GLFW)
    find_package(GLFW REQUIRED)
    add_definitions(-DTLRENDER_GLFW)
endif()

# OpenColorIO dependencies
if(TLRENDER_OCIO)
    find_package(OpenColorIO REQUIRED)
    add_definitions(-DTLRENDER_OCIO)
endif()

# Audio dependencies
if(TLRENDER_AUDIO)
    find_package(libsamplerate REQUIRED)
    find_package(RtAudio REQUIRED)
    add_definitions(-DTLRENDER_AUDIO)
endif()

# I/O dependencies
if(TLRENDER_JPEG)
    find_package(JPEG)
    if(JPEG_FOUND)
        add_definitions(-DTLRENDER_JPEG)
    endif()
endif()
if(TLRENDER_TIFF)
    find_package(TIFF)
    if(TIFF_FOUND)
        add_definitions(-DTLRENDER_TIFF)
    endif()
endif()
if(TLRENDER_STB)
    add_definitions(-DTLRENDER_STB)
endif()
if(TLRENDER_PNG)
    find_package(PNG)
    if(PNG_FOUND)
        add_definitions(-DTLRENDER_PNG)
    endif()
endif()
if(TLRENDER_EXR)
    find_package(OpenEXR)
    if(OpenEXR_FOUND)
        add_definitions(-DTLRENDER_EXR)
    endif()
endif()
if(TLRENDER_FFMPEG)
    find_package(FFmpeg)
    if(FFmpeg_FOUND)
        add_definitions(-DTLRENDER_FFMPEG)
    endif()
endif()
if(TLRENDER_USD)
    find_package(pxr)
    if(pxr_FOUND)
        add_definitions(-DTLRENDER_USD)
    endif()
endif()

# Python dependencies
if(TLRENDER_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development)
endif()

# Native file dialog dependencies
if(TLRENDER_NFD)
    find_package(NFD)
    if(NFD_FOUND)
        add_definitions(-DTLRENDER_NFD)
    endif()
endif()

# Qt dependencies
if(TLRENDER_QT6)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt6 COMPONENTS Quick Widgets OpenGLWidgets Svg HINTS "$ENV{QTDIR}")
elseif(TLRENDER_QT5)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt5 REQUIRED COMPONENTS Quick Widgets Svg HINTS "$ENV{QTDIR}")
endif()

#-------------------------------------------------------------------------------
# Sub-directories

add_subdirectory(deps/FSeq)
set_target_properties(fseq PROPERTIES FOLDER deps)

if(TLRENDER_GL_DEBUG)
    add_definitions(-DTLRENDER_GL_DEBUG)
    add_subdirectory(deps/tlGladDebug)
else()
    add_subdirectory(deps/tlGlad)
endif()

add_subdirectory(lib)
if(TLRENDER_PROGRAMS)
    add_subdirectory(bin)
endif()
if(TLRENDER_TESTS)
    add_subdirectory(tests)
endif()
if(TLRENDER_EXAMPLES)
    add_subdirectory(examples)
endif()
