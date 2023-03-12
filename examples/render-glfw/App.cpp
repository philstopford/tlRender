// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlGL/Render.h>

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <tlGlad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <array>

namespace tl
{
    namespace examples
    {
        namespace render_glfw
        {
            namespace
            {
                void glfwErrorCallback(int, const char* description)
                {
                    std::cerr << "GLFW ERROR: " << description << std::endl;
                }

                /*void APIENTRY glDebugOutput(
                    GLenum         source,
                    GLenum         type,
                    GLuint         id,
                    GLenum         severity,
                    GLsizei        length,
                    const GLchar * message,
                    const void *   userParam)
                {
                    switch (severity)
                    {
                    case GL_DEBUG_SEVERITY_HIGH_KHR:
                    case GL_DEBUG_SEVERITY_MEDIUM_KHR:
                        std::cerr << "DEBUG: " << message << std::endl;
                        break;
                    default: break;
                    }
                }*/
            }

            void App::_init(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "render-glfw",
                    "Example GLFW rendering application.",
                    {
                        app::CmdLineValueArg<std::string>::create(
                            _input,
                            "input",
                            "The input timeline.")
                    },
                {
                    app::CmdLineValueOption<std::string>::create(
                        _options.compareFileName,
                        { "-compare", "-b" },
                        "A/B comparison \"B\" file name."),
                    app::CmdLineValueOption<imaging::Size>::create(
                        _options.windowSize,
                        { "-windowSize", "-ws" },
                        "Window size.",
                        string::Format("{0}x{1}").arg(_options.windowSize.w).arg(_options.windowSize.h)),
                    app::CmdLineFlagOption::create(
                        _options.fullScreen,
                        { "-fullScreen", "-fs" },
                        "Enable full screen mode."),
                    app::CmdLineValueOption<bool>::create(
                        _options.hud,
                        { "-hud" },
                        "Enable the HUD (heads up display).",
                        string::Format("{0}").arg(_options.hud),
                        "0, 1"),
                    app::CmdLineValueOption<timeline::Playback>::create(
                        _options.playback,
                        { "-playback", "-p" },
                        "Playback mode.",
                        string::Format("{0}").arg(_options.playback),
                        string::join(timeline::getPlaybackLabels(), ", ")),
                    app::CmdLineValueOption<otime::RationalTime>::create(
                        _options.seek,
                        { "-seek" },
                        "Seek to the given time."),
                    app::CmdLineValueOption<otime::TimeRange>::create(
                        _options.inOutRange,
                        { "-inOutRange" },
                        "Set the in/out points range."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.fileName,
                        { "-colorConfig", "-cc" },
                        "Color configuration file name (e.g., config.ocio)."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.input,
                        { "-colorInput", "-ci" },
                        "Input color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.display,
                        { "-colorDisplay", "-cd" },
                        "Display color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.colorConfigOptions.view,
                        { "-colorView", "-cv" },
                        "View color space."),
                    app::CmdLineValueOption<std::string>::create(
                        _options.lutOptions.fileName,
                        { "-lut" },
                        "LUT file name."),
                    app::CmdLineValueOption<timeline::LUTOrder>::create(
                        _options.lutOptions.order,
                        { "-lutOrder" },
                        "LUT operation order.",
                        string::Format("{0}").arg(_options.lutOptions.order),
                        string::join(timeline::getLUTOrderLabels(), ", "))
                });
            }

            App::App()
            {}

            App::~App()
            {
                _render.reset();
                if (_glfwWindow)
                {
                    glfwDestroyWindow(_glfwWindow);
                }
                glfwTerminate();
            }

            std::shared_ptr<App> App::create(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(argc, argv, context);
                return out;
            }

            void App::run()
            {
                if (_exit != 0)
                {
                    return;
                }

                // Read the timelines.
                auto timeline = timeline::Timeline::create(_input, _context);
                auto timelinePlayer = timeline::TimelinePlayer::create(timeline, _context);
                _timelinePlayers.push_back(timelinePlayer);
                auto ioInfo = timelinePlayer->getIOInfo();
                if (!ioInfo.video.empty())
                {
                    _videoSizes.push_back(ioInfo.video[0].size);
                }
                _videoData.push_back(timeline::VideoData());
                if (!_options.compareFileName.empty())
                {
                    timeline = timeline::Timeline::create(_options.compareFileName, _context);
                    timelinePlayer = timeline::TimelinePlayer::create(timeline, _context);
                    timelinePlayer->setExternalTime(_timelinePlayers[0]);
                    _timelinePlayers.push_back(timelinePlayer);
                    ioInfo = timelinePlayer->getIOInfo();
                    if (!ioInfo.video.empty())
                    {
                        _videoSizes.push_back(ioInfo.video[0].size);
                    }
                    _videoData.push_back(timeline::VideoData());
                }

                // Initialize GLFW.
                glfwSetErrorCallback(glfwErrorCallback);
                int glfwMajor = 0;
                int glfwMinor = 0;
                int glfwRevision = 0;
                glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
                _log(string::Format("GLFW version: {0}.{1}.{2}").arg(glfwMajor).arg(glfwMinor).arg(glfwRevision));
                if (!glfwInit())
                {
                    throw std::runtime_error("Cannot initialize GLFW");
                }

                // Create the window.
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
                //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
                _glfwWindow = glfwCreateWindow(
                    _options.windowSize.w,
                    _options.windowSize.h,
                    "render-glfw",
                    NULL,
                    NULL);
                if (!_glfwWindow)
                {
                    throw std::runtime_error("Cannot create window");
                }
                glfwSetWindowUserPointer(_glfwWindow, this);
                int width = 0;
                int height = 0;
                glfwGetFramebufferSize(_glfwWindow, &width, &height);
                _frameBufferSize.w = width;
                _frameBufferSize.h = height;
                glfwGetWindowContentScale(_glfwWindow, &_contentScale.x, &_contentScale.y);
                glfwMakeContextCurrent(_glfwWindow);
                if (!gladLoaderLoadGL())
                {
                    throw std::runtime_error("Cannot initialize GLAD");
                }
                /*GLint flags = 0;
                glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
                if (flags & static_cast<GLint>(GL_CONTEXT_FLAG_DEBUG_BIT))
                {
                    glEnable(GL_DEBUG_OUTPUT);
                    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                    glDebugMessageCallback(glDebugOutput, context.get());
                    glDebugMessageControl(
                        static_cast<GLenum>(GL_DONT_CARE),
                        static_cast<GLenum>(GL_DONT_CARE),
                        static_cast<GLenum>(GL_DONT_CARE),
                        0,
                        nullptr,
                        GLFW_TRUE);
                }*/
                const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
                const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
                const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
                _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
                glfwSetFramebufferSizeCallback(_glfwWindow, _frameBufferSizeCallback);
                glfwSetWindowContentScaleCallback(_glfwWindow, _windowContentScaleCallback);
                if (_options.fullScreen)
                {
                    _fullscreenWindow();
                }
                glfwSetKeyCallback(_glfwWindow, _keyCallback);
                glfwShowWindow(_glfwWindow);

                // Create the renderer.
                _fontSystem = imaging::FontSystem::create(_context);
                _render = gl::Render::create(_context);

                // Print the shortcuts help.
                _printShortcutsHelp();

                // Start the main loop.
                if (time::isValid(_options.inOutRange))
                {
                    _timelinePlayers[0]->setInOutRange(_options.inOutRange);
                    _timelinePlayers[0]->seek(_options.inOutRange.start_time());
                }
                if (time::isValid(_options.seek))
                {
                    _timelinePlayers[0]->seek(_options.seek);
                }
                _timelinePlayers[0]->setPlayback(_options.playback);
                _startTime = std::chrono::steady_clock::now();
                while (_running && !glfwWindowShouldClose(_glfwWindow))
                {
                    glfwPollEvents();
                    _tick();
                }
            }

            void App::exit()
            {
                _running = false;
            }

            void App::_fullscreenWindow()
            {
                _options.fullScreen = true;

                int width = 0;
                int height = 0;
                glfwGetWindowSize(_glfwWindow, &width, &height);
                _options.windowSize.w = width;
                _options.windowSize.h = height;

                GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
                glfwGetWindowPos(_glfwWindow, &_windowPos.x, &_windowPos.y);
                glfwSetWindowMonitor(_glfwWindow, glfwMonitor, 0, 0, glfwVidmode->width, glfwVidmode->height, glfwVidmode->refreshRate);
            }

            void App::_normalWindow()
            {
                _options.fullScreen = false;

                GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                glfwSetWindowMonitor(_glfwWindow, NULL, _windowPos.x, _windowPos.y, _options.windowSize.w, _options.windowSize.h, 0);
            }

            void App::_fullscreenCallback(bool value)
            {
                if (value)
                {
                    _fullscreenWindow();
                }
                else
                {
                    _normalWindow();
                }
                _log(string::Format("Fullscreen: {0}").arg(_options.fullScreen));
            }

            void App::_frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
            {
                App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
                app->_frameBufferSize.w = width;
                app->_frameBufferSize.h = height;
                app->_renderDirty = true;
            }

            void App::_windowContentScaleCallback(GLFWwindow* glfwWindow, float x, float y)
            {
                App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
                app->_contentScale.x = x;
                app->_contentScale.y = y;
                app->_renderDirty = true;
            }

            void App::_keyCallback(GLFWwindow* glfwWindow, int key, int scanCode, int action, int mods)
            {
                if (GLFW_RELEASE == action || GLFW_REPEAT == action)
                {
                    App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
                    switch (key)
                    {
                    case GLFW_KEY_ESCAPE:
                        app->exit();
                        break;
                    case GLFW_KEY_U:
                        app->_fullscreenCallback(!app->_options.fullScreen);
                        break;
                    case GLFW_KEY_H:
                        app->_hudCallback(!app->_options.hud);
                        break;
                    case GLFW_KEY_SPACE:
                        app->_playbackCallback(
                            timeline::Playback::Stop == app->_timelinePlayers[0]->observePlayback()->get() ?
                            timeline::Playback::Forward :
                            timeline::Playback::Stop);
                        break;
                    case GLFW_KEY_HOME:
                        app->_timelinePlayers[0]->start();
                        break;
                    case GLFW_KEY_END:
                        app->_timelinePlayers[0]->end();
                        break;
                    case GLFW_KEY_LEFT:
                        app->_timelinePlayers[0]->framePrev();
                        break;
                    case GLFW_KEY_RIGHT:
                        app->_timelinePlayers[0]->frameNext();
                        break;
                    }
                }
            }

            void App::_printShortcutsHelp()
            {
                _print(
                    "\n"
                    "Keyboard shortcuts:\n"
                    "\n"
                    "    Escape - Exit\n"
                    "    U      - Fullscreen mode\n"
                    "    H      - HUD enabled\n"
                    "    Space  - Start/stop playback\n"
                    "    Home   - Go to the start time\n"
                    "    End    - Go to the end time\n"
                    "    Left   - Go to the previous frame\n"
                    "    Right  - Go to the next frame\n");
            }

            void App::_tick()
            {
                // Update.
                _context->tick();
                for (const auto& timelinePlayer : _timelinePlayers)
                {
                    timelinePlayer->tick();
                }
                for (size_t i = 0; i < _timelinePlayers.size(); ++i)
                {
                    const auto& videoData = _timelinePlayers[i]->observeCurrentVideo()->get();
                    if (!timeline::isTimeEqual(videoData, _videoData[i]))
                    {
                        _videoData[i] = videoData;
                        _renderDirty = true;
                    }
                }
                _hudUpdate();

                // Render the video.
                if (_renderDirty)
                {
                    _render->begin(
                        _frameBufferSize,
                        _options.colorConfigOptions,
                        _options.lutOptions);
                    _drawVideo();
                    if (_options.hud)
                    {
                        _drawHUD();
                    }
                    _render->end();
                    glfwSwapBuffers(_glfwWindow);
                    _renderDirty = false;
                }
                else
                {
                    time::sleep(std::chrono::milliseconds(5));
                }

                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - _startTime;
                const float v = (std::sinf(diff.count()) + 1.F) / 2.F;
                _compareOptions.wipeCenter.x = v;
                _compareOptions.overlay = v;
                _rotation = diff.count() * 2.F;
            }

            void App::_drawVideo()
            {
                const auto& size = _frameBufferSize;

                _compareOptions.mode = timeline::CompareMode::A;
                _drawVideo(
                    math::BBox2i(0, 0, size.w / 3, size.h / 3),
                    _compareOptions,
                    0.F);
                _compareOptions.mode = timeline::CompareMode::A;
                _drawVideo(
                    math::BBox2i(size.w / 3, 0, size.w / 3, size.h / 3),
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::B;
                _drawVideo(
                    math::BBox2i(size.w / 3 * 2, 0, size.w / 3, size.h / 3),
                    _compareOptions,
                    _rotation);

                _compareOptions.mode = timeline::CompareMode::Wipe;
                _drawVideo(
                    math::BBox2i(0, size.h / 3, size.w / 3, size.h / 3),
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::Overlay;
                _drawVideo(
                    math::BBox2i(size.w / 3, size.h / 3, size.w / 3, size.h / 3),
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::Difference;
                _drawVideo(
                    math::BBox2i(size.w / 3 * 2, size.h / 3, size.w / 3, size.h / 3),
                    _compareOptions,
                    _rotation);

                _compareOptions.mode = timeline::CompareMode::Horizontal;
                _drawVideo(
                    math::BBox2i(0, size.h / 3 * 2, size.w / 3, size.h / 3),
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::Vertical;
                _drawVideo(
                    math::BBox2i(size.w / 3, size.h / 3 * 2, size.w / 3, size.h / 3),
                    _compareOptions,
                    _rotation);
                _compareOptions.mode = timeline::CompareMode::Tile;
                _drawVideo(
                    math::BBox2i(size.w / 3 * 2, size.h / 3 * 2, size.w / 3, size.h / 3),
                    _compareOptions,
                    _rotation);
            }

            void App::_drawVideo(
                const math::BBox2i& bbox,
                const timeline::CompareOptions& compareOptions,
                float rotation)
            {
                const auto viewportSize = bbox.getSize();
                const float viewportAspect = viewportSize.y > 0 ?
                    (viewportSize.x / static_cast<float>(viewportSize.y)) :
                    1.F;
                const auto renderSize = timeline::getRenderSize(
                    compareOptions.mode,
                    _videoSizes);
                const float renderSizeAspect = renderSize.getAspect();
                imaging::Size transformSize;
                math::Vector2f transformOffset;
                if (renderSizeAspect > 1.F)
                {
                    transformSize.w = renderSize.w;
                    transformSize.h = renderSize.w / viewportAspect;
                    transformOffset.x = renderSize.w / 2.F;
                    transformOffset.y = renderSize.w / viewportAspect / 2.F;
                }
                else
                {
                    transformSize.w = renderSize.h * viewportAspect;
                    transformSize.h = renderSize.h;
                    transformOffset.x = renderSize.h * viewportAspect / 2.F;
                    transformOffset.y = renderSize.h / 2.F;
                }

                _render->setClipRectEnabled(true);
                _render->setViewport(bbox);
                _render->setClipRect(bbox);
                //_render->clearViewport(imaging::Color4f(1.F, 0.F, 0.F));
                _render->setTransform(math::ortho(
                    0.F,
                    static_cast<float>(transformSize.w),
                    static_cast<float>(transformSize.h),
                    0.F,
                    -1.F,
                    1.F) *
                    math::translate(math::Vector3f(transformOffset.x, transformOffset.y, 0.F)) *
                    math::rotateZ(rotation) *
                    math::translate(math::Vector3f(-renderSize.w / 2, -renderSize.h / 2, 0.F)));
                _render->drawVideo(
                    _videoData,
                    timeline::tiles(compareOptions.mode, _videoSizes),
                    {},
                    {},
                    compareOptions);
                _render->setClipRectEnabled(false);
            }

            void App::_hudUpdate()
            {
            }

            void App::_hudCallback(bool value)
            {
                _options.hud = value;
                _renderDirty = true;
                _log(string::Format("HUD: {0}").arg(_options.hud));
            }

            void App::_drawHUD()
            {
                const uint16_t fontSize =
                    math::clamp(
                        ceilf(14 * _contentScale.y),
                        0.F,
                        static_cast<float>(std::numeric_limits<uint16_t>::max()));
            }

            void App::_playbackCallback(timeline::Playback value)
            {
                _timelinePlayers[0]->setPlayback(value);
                _log(string::Format("Playback: {0}").arg(_timelinePlayers[0]->observePlayback()->get()));
            }
        }
    }
}
