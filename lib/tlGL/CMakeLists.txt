set(HEADERS
    Init.h
    Mesh.h
    OffscreenBuffer.h
    Shader.h
    Texture.h
    TextureAtlas.h
    Util.h)
if(TLRENDER_GLFW)
    list(APPEND HEADERS GLFWSystem.h)
endif()
set(PRIVATE_HEADERS)

set(SOURCE
    Init.cpp
    Mesh.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Shader.cpp
    Texture.cpp
    TextureAtlas.cpp
    Util.cpp)
if(TLRENDER_GLFW)
    list(APPEND SOURCE GLFWSystem.cpp)
endif()

set(LIBRARIES tlCore)
if(TLRENDER_GL_DEBUG)
    list(APPEND LIBRARIES tlGladDebug)
else()
    list(APPEND LIBRARIES tlGlad)
endif()
if(TLRENDER_GLFW)
    list(APPEND LIBRARIES GLFW)
endif()
set(LIBRARIES_PRIVATE)

add_library(tlGL ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlGL ${LIBRARIES} ${LIBRARIES_PRIVATE})
set_target_properties(tlGL PROPERTIES FOLDER lib)
set_target_properties(tlGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlGL)
