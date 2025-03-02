include(ExternalProject)

set(NFD_GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git")
set(NFD_GIT_TAG "v1.0.3")

set(NFD_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DNFD_BUILD_TESTS=OFF)

ExternalProject_Add(
    NFD
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/NFD
    GIT_REPOSITORY ${NFD_GIT_REPOSITORY}
    GIT_TAG ${NFD_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${NFD_ARGS})
