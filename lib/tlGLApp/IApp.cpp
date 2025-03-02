// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlGLApp/IApp.h>

#include <tlUI/EventLoop.h>
#include <tlUI/IClipboard.h>

#include <tlTimeline/GLRender.h>

#include <tlIO/IOSystem.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Util.h>

#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#if defined(TLRENDER_GL_DEBUG)
#include <tlGladDebug/gl.h>
#else // TLRENDER_GL_DEBUG
#include <tlGlad/gl.h>
#endif // TLRENDER_GL_DEBUG

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <codecvt>
#include <locale>

namespace tl
{
    namespace gl
    {
        namespace
        {
#if defined(TLRENDER_GL_DEBUG)
            void APIENTRY glDebugOutput(
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
                case GL_DEBUG_SEVERITY_HIGH:
                    std::cerr << "HIGH: " << message << std::endl;
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM:
                    std::cerr << "MEDIUM: " << message << std::endl;
                    break;
                case GL_DEBUG_SEVERITY_LOW:
                    std::cerr << "LOW: " << message << std::endl;
                    break;
                //case GL_DEBUG_SEVERITY_NOTIFICATION:
                //    std::cerr << "NOTIFICATION: " << message << std::endl;
                //    break;
                default: break;
                }
            }
#endif // TLRENDER_GL_DEBUG

            class Clipboard : public ui::IClipboard
            {
                TLRENDER_NON_COPYABLE(Clipboard);

            public:
                void _init(
                    GLFWwindow* glfwWindow,
                    const std::shared_ptr<system::Context>& context)
                {
                    IClipboard::_init(context);
                    _glfwWindow = glfwWindow;
                }

                Clipboard()
                {}

            public:
                virtual ~Clipboard()
                {}

                static std::shared_ptr<Clipboard> create(
                    GLFWwindow* glfwWindow,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<Clipboard>(new Clipboard);
                    out->_init(glfwWindow, context);
                    return out;
                }

                std::string getText() const override
                {
                    return glfwGetClipboardString(_glfwWindow);
                }

                void setText(const std::string& value) override
                {
                    glfwSetClipboardString(_glfwWindow, value.c_str());
                }

            private:
                GLFWwindow* _glfwWindow = nullptr;
            };

            class Cursor
            {
            public:
                Cursor(GLFWwindow*, GLFWcursor*);

                ~Cursor();

            private:
                GLFWcursor* _cursor = nullptr;
            };

            Cursor::Cursor(GLFWwindow* window, GLFWcursor* cursor) :
                _cursor(cursor)
            {
                glfwSetCursor(window, cursor);
            }

            Cursor::~Cursor()
            {
                glfwDestroyCursor(_cursor);
            }
        }

        struct IApp::Private
        {
            Options options;

            GLFWwindow* glfwWindow = nullptr;
            image::Size windowSize;
            math::Vector2i windowPos;
            std::shared_ptr<observer::Value<bool> > fullscreen;
            std::shared_ptr<observer::Value<bool> > floatOnTop;
            image::Size frameBufferSize;
            math::Vector2f contentScale = math::Vector2f(1.F, 1.F);
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            bool refresh = false;
            std::unique_ptr<Cursor> cursor;

            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<Clipboard> clipboard;
            int modifiers = 0;
            std::shared_ptr<ui::EventLoop> eventLoop;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<gl::OffscreenBuffer> offscreenBuffer;

            bool running = true;
        };

        void IApp::_init(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>& context,
            const std::string& cmdLineName,
            const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<app::ICmdLineArg> >& args,
            const std::vector<std::shared_ptr<app::ICmdLineOption> >& options)
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<app::ICmdLineOption> > options2 = options;
            options2.push_back(
                app::CmdLineValueOption<image::Size>::create(
                    p.options.windowSize,
                    { "-windowSize", "-ws" },
                    "Window size.",
                    string::Format("{0}x{1}").arg(p.options.windowSize.w).arg(p.options.windowSize.h)));
            options2.push_back(
                app::CmdLineFlagOption::create(
                    p.options.fullscreen,
                    { "-fullscreen", "-fs" },
                    "Enable full screen mode."));
            app::IApp::_init(
                argc,
                argv,
                context,
                cmdLineName,
                cmdLineSummary,
                args,
                options2);
            if (_exit != 0)
            {
                return;
            }

            // Create observers.
            p.fullscreen = observer::Value<bool>::create(false);
            p.floatOnTop = observer::Value<bool>::create(false);

            // Create the window.
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
#if defined(TLRENDER_GL_DEBUG)
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // TLRENDER_GL_DEBUG
            p.glfwWindow = glfwCreateWindow(
                p.options.windowSize.w,
                p.options.windowSize.h,
                cmdLineName.c_str(),
                NULL,
                NULL);
            if (!p.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }
            
            glfwSetWindowUserPointer(p.glfwWindow, this);
            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(p.glfwWindow, &width, &height);
            p.frameBufferSize.w = width;
            p.frameBufferSize.h = height;
            glfwGetWindowContentScale(p.glfwWindow, &p.contentScale.x, &p.contentScale.y);
            glfwMakeContextCurrent(p.glfwWindow);
            if (!gladLoaderLoadGL())
            {
                throw std::runtime_error("Cannot initialize GLAD");
            }
#if defined(TLRENDER_GL_DEBUG)
            GLint flags = 0;
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
            }
#endif // TLRENDER_GL_DEBUG
            const int glMajor = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
            const int glMinor = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
            const int glRevision = glfwGetWindowAttrib(p.glfwWindow, GLFW_CONTEXT_REVISION);
            _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
            glfwSetFramebufferSizeCallback(p.glfwWindow, _frameBufferSizeCallback);
            glfwSetWindowContentScaleCallback(p.glfwWindow, _windowContentScaleCallback);
            glfwSetWindowRefreshCallback(p.glfwWindow, _windowRefreshCallback);
            setFullScreen(p.options.fullscreen);
            glfwSetCursorEnterCallback(p.glfwWindow, _cursorEnterCallback);
            glfwSetCursorPosCallback(p.glfwWindow, _cursorPosCallback);
            glfwSetMouseButtonCallback(p.glfwWindow, _mouseButtonCallback);
            glfwSetScrollCallback(p.glfwWindow, _scrollCallback);
            glfwSetKeyCallback(p.glfwWindow, _keyCallback);
            glfwSetCharCallback(p.glfwWindow, _charCallback);
            glfwSetDropCallback(p.glfwWindow, _dropCallback);
            glfwShowWindow(p.glfwWindow);

            // Initialize the user interface.
            p.style = ui::Style::create(_context);
            p.iconLibrary = ui::IconLibrary::create(_context);
            p.clipboard = Clipboard::create(p.glfwWindow, _context);
            p.eventLoop = ui::EventLoop::create(
                p.style,
                p.iconLibrary,
                p.clipboard,
                _context);
            p.eventLoop->setCursor(
                [this](ui::StandardCursor value)
                {
                    _setCursor(value);
                });
            p.eventLoop->setCursor(
                [this](
                    const std::shared_ptr<image::Image>& image,
                    const math::Vector2i& hotspot)
                {
                    _setCursor(image, hotspot);
                });
            p.eventLoop->setCapture(
                [this](const math::Box2i& value)
                {
                    return _capture(value);
                });

            // Initialize the renderer.
            p.render = timeline::GLRender::create(_context);
        }

        IApp::IApp() :
            _p(new Private)
        {}

        IApp::~IApp()
        {
            TLRENDER_P();
            p.eventLoop.reset();
            p.render.reset();
            p.offscreenBuffer.reset();
            if (p.glfwWindow)
            {
                glfwDestroyWindow(p.glfwWindow);
            }
        }

        void IApp::run()
        {
            TLRENDER_P();
            if (_exit != 0)
            {
                return;
            }

            // Start the main loop.
            while (p.running && !glfwWindowShouldClose(p.glfwWindow))
            {
                glfwPollEvents();

                _context->tick();

                _tick();

                p.eventLoop->setDisplaySize(p.frameBufferSize);
                p.eventLoop->setDisplayScale(p.contentScale.x);
                p.eventLoop->tick();

                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                if (gl::doCreate(p.offscreenBuffer, p.frameBufferSize, offscreenBufferOptions))
                {
                    p.offscreenBuffer = gl::OffscreenBuffer::create(
                        p.frameBufferSize,
                        offscreenBufferOptions);
                }
                if ((p.eventLoop->hasDrawUpdate() || p.refresh) &&
                    p.offscreenBuffer)
                {
                    p.refresh = false;
                    {
                        gl::OffscreenBufferBinding binding(p.offscreenBuffer);
                        p.render->begin(
                            p.frameBufferSize,
                            p.colorConfigOptions,
                            p.lutOptions);
                        p.eventLoop->draw(p.render);
                        p.render->end();
                    }
                    glViewport(
                        0,
                        0,
                        GLsizei(p.frameBufferSize.w),
                        GLsizei(p.frameBufferSize.h));
                    glClearColor(0.F, 0.F, 0.F, 0.F);
                    glClear(GL_COLOR_BUFFER_BIT);
                    glBindFramebuffer(
                        GL_READ_FRAMEBUFFER,
                        p.offscreenBuffer->getID());
                    glBlitFramebuffer(
                        0,
                        0,
                        p.frameBufferSize.w,
                        p.frameBufferSize.h,
                        0,
                        0,
                        p.frameBufferSize.w,
                        p.frameBufferSize.h,
                        GL_COLOR_BUFFER_BIT,
                        GL_LINEAR);
                    glfwSwapBuffers(p.glfwWindow);
                }

                time::sleep(std::chrono::milliseconds(5));
            }
        }

        void IApp::exit(int r)
        {
            _exit = r;
            _p->running = false;
        }

        const std::shared_ptr<ui::EventLoop> IApp::getEventLoop() const
        {
            return _p->eventLoop;
        }

        const std::shared_ptr<ui::Style> IApp::getStyle() const
        {
            return _p->style;
        }

        image::Size IApp::getWindowSize() const
        {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(_p->glfwWindow, &width, &height);
            return image::Size(width, height);
        }

        void IApp::setWindowSize(const image::Size& value)
        {
            glfwSetWindowSize(_p->glfwWindow, value.w, value.h);
        }

        bool IApp::isFullScreen() const
        {
            return _p->fullscreen->get();
        }

        std::shared_ptr<observer::IValue<bool> > IApp::observeFullScreen() const
        {
            return _p->fullscreen;
        }

        void IApp::setFullScreen(bool value)
        {
            TLRENDER_P();
            if (p.fullscreen->setIfChanged(value))
            {
                if (value)
                {
                    int width = 0;
                    int height = 0;
                    glfwGetWindowSize(p.glfwWindow, &width, &height);
                    p.windowSize.w = width;
                    p.windowSize.h = height;

                    GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode* glfwVidmode = glfwGetVideoMode(glfwMonitor);
                    glfwGetWindowPos(p.glfwWindow, &p.windowPos.x, &p.windowPos.y);
                    glfwSetWindowMonitor(
                        p.glfwWindow,
                        glfwMonitor,
                        0,
                        0,
                        glfwVidmode->width,
                        glfwVidmode->height,
                        glfwVidmode->refreshRate);
                }
                else
                {
                    GLFWmonitor* glfwMonitor = glfwGetPrimaryMonitor();
                    glfwSetWindowMonitor(
                        p.glfwWindow,
                        NULL,
                        p.windowPos.x,
                        p.windowPos.y,
                        p.windowSize.w,
                        p.windowSize.h,
                        0);
                }
            }
        }

        bool IApp::isFloatOnTop() const
        {
            return _p->floatOnTop->get();
        }

        std::shared_ptr<observer::IValue<bool> > IApp::observeFloatOnTop() const
        {
            return _p->floatOnTop;
        }

        void IApp::setFloatOnTop(bool value)
        {
            TLRENDER_P();
            if (p.floatOnTop->setIfChanged(value))
            {
                glfwSetWindowAttrib(
                    p.glfwWindow,
                    GLFW_FLOATING,
                    value ? GLFW_TRUE : GLFW_FALSE);
            }
        }
        void IApp::_setColorConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            if (value == p.colorConfigOptions)
                return;
            p.colorConfigOptions = value;
            p.refresh = true;
        }

        void IApp::_setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            p.refresh = true;
        }

        void IApp::_setCursor(ui::StandardCursor value)
        {
            TLRENDER_P();
            GLFWcursor* cursor = nullptr;
            switch (value)
            {
            case ui::StandardCursor::Arrow:
                break;
            case ui::StandardCursor::IBeam:
                cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
                break;
            case ui::StandardCursor::Crosshair:
                cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
                break;
            case ui::StandardCursor::Hand:
                cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
                break;
            case ui::StandardCursor::HResize:
                cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
                break;
            case ui::StandardCursor::VResize:
                cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
                break;
            }
            glfwSetCursor(p.glfwWindow, cursor);
        }

        void IApp::_setCursor(
            const std::shared_ptr<image::Image>& image,
            const math::Vector2i& hotspot)
        {
            TLRENDER_P();
            GLFWimage glfwImage;
            glfwImage.width = image->getWidth();
            glfwImage.height = image->getHeight();
            glfwImage.pixels = image->getData();
            GLFWcursor* glfwCursor = glfwCreateCursor(&glfwImage, hotspot.x, hotspot.y);
            p.cursor.reset(new Cursor(p.glfwWindow, glfwCursor));
        }

        std::shared_ptr<image::Image> IApp::_capture(const math::Box2i& value)
        {
            TLRENDER_P();
            const image::Size size(value.w(), value.h());
            const image::Info info(size, image::PixelType::RGBA_U8);
            auto out = image::Image::create(info);

            gl::OffscreenBufferBinding binding(p.offscreenBuffer);

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_SWAP_BYTES, 0);
            glReadPixels(
                value.min.x,
                p.frameBufferSize.h - value.min.y - size.h,
                size.w,
                size.h,
                gl::getReadPixelsFormat(info.pixelType),
                gl::getReadPixelsType(info.pixelType),
                out->getData());

            auto flipped = image::Image::create(info);
            for (size_t y = 0; y < size.h; ++y)
            {
                uint8_t* p = flipped->getData() + y * size.w * 4;
                memcpy(
                    p,
                    out->getData() + (size.h - 1 - y) * size.w * 4,
                    size.w * 4);
                //for (size_t x = 0; x < size.w; ++x, p += 4)
                //{
                //    p[3] /= 2;
                //}
            }
            return flipped;
        }

        void IApp::_drop(const std::vector<std::string>&)
        {}

        void IApp::_tick()
        {}

        void IApp::_frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->frameBufferSize.w = width;
            app->_p->frameBufferSize.h = height;
        }
            
        void IApp::_windowContentScaleCallback(GLFWwindow* glfwWindow, float x, float y)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->contentScale.x = x;
            app->_p->contentScale.y = y;
        }

        void IApp::_windowRefreshCallback(GLFWwindow* glfwWindow)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->refresh = true;
        }

        void IApp::_cursorEnterCallback(GLFWwindow* glfwWindow, int value)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->eventLoop->cursorEnter(GLFW_TRUE == value);
        }

        void IApp::_cursorPosCallback(GLFWwindow* glfwWindow, double x, double y)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            math::Vector2i pos;
#if defined(__APPLE__)
            //! \bug The mouse position needs to be scaled on macOS?
            pos.x = x * app->_p->contentScale.x;
            pos.y = y * app->_p->contentScale.y;
#else // __APPLE__
            pos.x = x;
            pos.y = y;
#endif // __APPLE__
            app->_p->eventLoop->cursorPos(pos);
        }

        namespace
        {
            int fromGLFWModifiers(int value)
            {
                int out = 0;
                if (value & GLFW_MOD_SHIFT)
                {
                    out |= static_cast<int>(ui::KeyModifier::Shift);
                }
                if (value & GLFW_MOD_CONTROL)
                {
                    out |= static_cast<int>(ui::KeyModifier::Control);
                }
                if (value & GLFW_MOD_ALT)
                {
                    out |= static_cast<int>(ui::KeyModifier::Alt);
                }
                if (value & GLFW_MOD_SUPER)
                {
                    out |= static_cast<int>(ui::KeyModifier::Super);
                }
                return out;
            }
        }

        void IApp::_mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int modifiers)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->modifiers = modifiers;
            app->_p->eventLoop->mouseButton(button, GLFW_PRESS == action, fromGLFWModifiers(modifiers));
        }

        void IApp::_scrollCallback(GLFWwindow* glfwWindow, double dx, double dy)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->eventLoop->scroll(dx, dy, fromGLFWModifiers(app->_p->modifiers));
        }

        namespace
        {
            ui::Key fromGLFWKey(int key)
            {
                ui::Key out = ui::Key::Unknown;
                switch (key)
                {
                case GLFW_KEY_SPACE: out = ui::Key::Space; break;
                case GLFW_KEY_APOSTROPHE: out = ui::Key::Apostrophe; break;
                case GLFW_KEY_COMMA: out = ui::Key::Comma; break;
                case GLFW_KEY_MINUS: out = ui::Key::Minus; break;
                case GLFW_KEY_PERIOD: out = ui::Key::Period; break;
                case GLFW_KEY_SLASH: out = ui::Key::Slash; break;
                case GLFW_KEY_0: out = ui::Key::_0; break;
                case GLFW_KEY_1: out = ui::Key::_1; break;
                case GLFW_KEY_2: out = ui::Key::_2; break;
                case GLFW_KEY_3: out = ui::Key::_3; break;
                case GLFW_KEY_4: out = ui::Key::_4; break;
                case GLFW_KEY_5: out = ui::Key::_5; break;
                case GLFW_KEY_6: out = ui::Key::_6; break;
                case GLFW_KEY_7: out = ui::Key::_7; break;
                case GLFW_KEY_8: out = ui::Key::_8; break;
                case GLFW_KEY_9: out = ui::Key::_9; break;
                case GLFW_KEY_SEMICOLON: out = ui::Key::Semicolon; break;
                case GLFW_KEY_EQUAL: out = ui::Key::Equal; break;
                case GLFW_KEY_A: out = ui::Key::A; break;
                case GLFW_KEY_B: out = ui::Key::B; break;
                case GLFW_KEY_C: out = ui::Key::C; break;
                case GLFW_KEY_D: out = ui::Key::D; break;
                case GLFW_KEY_E: out = ui::Key::E; break;
                case GLFW_KEY_F: out = ui::Key::F; break;
                case GLFW_KEY_G: out = ui::Key::G; break;
                case GLFW_KEY_H: out = ui::Key::H; break;
                case GLFW_KEY_I: out = ui::Key::I; break;
                case GLFW_KEY_J: out = ui::Key::J; break;
                case GLFW_KEY_K: out = ui::Key::K; break;
                case GLFW_KEY_L: out = ui::Key::L; break;
                case GLFW_KEY_M: out = ui::Key::M; break;
                case GLFW_KEY_N: out = ui::Key::N; break;
                case GLFW_KEY_O: out = ui::Key::O; break;
                case GLFW_KEY_P: out = ui::Key::P; break;
                case GLFW_KEY_Q: out = ui::Key::Q; break;
                case GLFW_KEY_R: out = ui::Key::R; break;
                case GLFW_KEY_S: out = ui::Key::S; break;
                case GLFW_KEY_T: out = ui::Key::T; break;
                case GLFW_KEY_U: out = ui::Key::U; break;
                case GLFW_KEY_V: out = ui::Key::V; break;
                case GLFW_KEY_W: out = ui::Key::W; break;
                case GLFW_KEY_X: out = ui::Key::X; break;
                case GLFW_KEY_Y: out = ui::Key::Y; break;
                case GLFW_KEY_Z: out = ui::Key::Z; break;
                case GLFW_KEY_LEFT_BRACKET: out = ui::Key::LeftBracket; break;
                case GLFW_KEY_BACKSLASH: out = ui::Key::Backslash; break;
                case GLFW_KEY_RIGHT_BRACKET: out = ui::Key::RightBracket; break;
                case GLFW_KEY_GRAVE_ACCENT: out = ui::Key::GraveAccent; break;
                case GLFW_KEY_ESCAPE: out = ui::Key::Escape; break;
                case GLFW_KEY_ENTER: out = ui::Key::Enter; break;
                case GLFW_KEY_TAB: out = ui::Key::Tab; break;
                case GLFW_KEY_BACKSPACE: out = ui::Key::Backspace; break;
                case GLFW_KEY_INSERT: out = ui::Key::Insert; break;
                case GLFW_KEY_DELETE: out = ui::Key::Delete; break;
                case GLFW_KEY_RIGHT: out = ui::Key::Right; break;
                case GLFW_KEY_LEFT: out = ui::Key::Left; break;
                case GLFW_KEY_DOWN: out = ui::Key::Down; break;
                case GLFW_KEY_UP: out = ui::Key::Up; break;
                case GLFW_KEY_PAGE_UP: out = ui::Key::PageUp; break;
                case GLFW_KEY_PAGE_DOWN: out = ui::Key::PageDown; break;
                case GLFW_KEY_HOME: out = ui::Key::Home; break;
                case GLFW_KEY_END: out = ui::Key::End; break;
                case GLFW_KEY_CAPS_LOCK: out = ui::Key::CapsLock; break;
                case GLFW_KEY_SCROLL_LOCK: out = ui::Key::ScrollLock; break;
                case GLFW_KEY_NUM_LOCK: out = ui::Key::NumLock; break;
                case GLFW_KEY_PRINT_SCREEN: out = ui::Key::PrintScreen; break;
                case GLFW_KEY_PAUSE: out = ui::Key::Pause; break;
                case GLFW_KEY_F1: out = ui::Key::F1; break;
                case GLFW_KEY_F2: out = ui::Key::F2; break;
                case GLFW_KEY_F3: out = ui::Key::F3; break;
                case GLFW_KEY_F4: out = ui::Key::F4; break;
                case GLFW_KEY_F5: out = ui::Key::F5; break;
                case GLFW_KEY_F6: out = ui::Key::F6; break;
                case GLFW_KEY_F7: out = ui::Key::F7; break;
                case GLFW_KEY_F8: out = ui::Key::F8; break;
                case GLFW_KEY_F9: out = ui::Key::F9; break;
                case GLFW_KEY_F10: out = ui::Key::F10; break;
                case GLFW_KEY_F11: out = ui::Key::F11; break;
                case GLFW_KEY_F12: out = ui::Key::F12; break;
                case GLFW_KEY_LEFT_SHIFT: out = ui::Key::LeftShift; break;
                case GLFW_KEY_LEFT_CONTROL: out = ui::Key::LeftControl; break;
                case GLFW_KEY_LEFT_ALT: out = ui::Key::LeftAlt; break;
                case GLFW_KEY_LEFT_SUPER: out = ui::Key::LeftSuper; break;
                case GLFW_KEY_RIGHT_SHIFT: out = ui::Key::RightShift; break;
                case GLFW_KEY_RIGHT_CONTROL: out = ui::Key::RightControl; break;
                case GLFW_KEY_RIGHT_ALT: out = ui::Key::RightAlt; break;
                case GLFW_KEY_RIGHT_SUPER: out = ui::Key::RightSuper; break;
                }
                return out;
            }
        }

        void IApp::_keyCallback(GLFWwindow* glfwWindow, int key, int scanCode, int action, int modifiers)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            app->_p->modifiers = modifiers;
            switch (action)
            {
            case GLFW_PRESS:
            case GLFW_REPEAT:
                app->_p->eventLoop->key(
                    fromGLFWKey(key),
                    true,
                    fromGLFWModifiers(modifiers));
                break;
            case GLFW_RELEASE:
                app->_p->eventLoop->key(
                    fromGLFWKey(key),
                    false,
                    fromGLFWModifiers(modifiers));
                break;
            }
        }

        namespace
        {
#if defined(_WINDOWS)
            //! \bug https://social.msdn.microsoft.com/Forums/vstudio/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
            typedef unsigned int tl_char_t;
#else // _WINDOWS
            typedef char32_t tl_char_t;
#endif // _WINDOWS
        }

        void IApp::_charCallback(GLFWwindow* glfwWindow, unsigned int c)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            std::wstring_convert<std::codecvt_utf8<tl_char_t>, tl_char_t> utf32Convert;
            app->_p->eventLoop->text(utf32Convert.to_bytes(c));
        }

        void IApp::_dropCallback(GLFWwindow* glfwWindow, int count, const char** fileNames)
        {
            IApp* app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(glfwWindow));
            std::vector<std::string> tmp;
            for (int i = 0; i < count; ++i)
            {
                tmp.push_back(fileNames[i]);
            }
            app->_drop(tmp);
        }
    }
}
