if(TLRENDER_GLFW)
    add_subdirectory(test-patterns)
    add_subdirectory(render-gl)
    add_subdirectory(widgets-gl)
    if(TLRENDER_USD)
        add_subdirectory(usdrecord-gl)
    endif()
endif()
if(TLRENDER_QT6 OR TLRENDER_QT5)
    add_subdirectory(filmstrip-qtwidget)
    add_subdirectory(panorama-qtwidget)
    add_subdirectory(simple-qtquick)
    add_subdirectory(simple-qtwidget)
    add_subdirectory(widgets-qtwidget)
endif()
