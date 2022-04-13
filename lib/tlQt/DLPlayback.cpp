// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQt/DLPlayback.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>

#include <tlDL/PlaybackDevice.h>

#include <tlCore/Context.h>

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <iostream>

namespace tl
{
    namespace qt
    {
        struct DLPlayback::Private
        {
            std::shared_ptr<dl::PlaybackDevice> device;
            imaging::ColorConfig colorConfig;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            std::vector<qt::TimelinePlayer*> timelinePlayers;
            imaging::Size size = imaging::Size(1920, 1080);
            math::Vector2i viewPos;
            float viewZoom = 1.F;
            bool frameView = true;
            std::vector<timeline::VideoData> videoData;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
            QScopedPointer<QOffscreenSurface> offscreenSurface;
            QScopedPointer<QOpenGLContext> glContext;
        };

        DLPlayback::DLPlayback(
            int deviceIndex,
            const std::shared_ptr<system::Context>& context) :
            _p(new Private)
        {
            TLRENDER_P();

            p.device = dl::PlaybackDevice::create(deviceIndex, context);

            p.glContext.reset(new QOpenGLContext);
            QSurfaceFormat surfaceFormat;
            surfaceFormat.setMajorVersion(4);
            surfaceFormat.setMinorVersion(1);
            surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
            p.glContext->setFormat(surfaceFormat);
            p.glContext->create();

            p.offscreenSurface.reset(new QOffscreenSurface);
            p.offscreenSurface->setFormat(p.glContext->format());
            p.offscreenSurface->create();

            p.glContext->makeCurrent(p.offscreenSurface.get());
            gladLoaderLoadGL();

            p.render = gl::Render::create(context);
        }

        DLPlayback::~DLPlayback()
        {}

        void DLPlayback::setColorConfig(const imaging::ColorConfig& value)
        {
            TLRENDER_P();
            if (value == p.colorConfig)
                return;
            p.colorConfig = value;
            _render();
        }

        void DLPlayback::setImageOptions(const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            _render();
        }

        void DLPlayback::setDisplayOptions(const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            _render();
        }

        void DLPlayback::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            _render();
        }

        void DLPlayback::setTimelinePlayers(const std::vector<qt::TimelinePlayer*>& value)
        {
            TLRENDER_P();
            p.videoData.clear();
            for (const auto& i : p.timelinePlayers)
            {
                disconnect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    this,
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            p.timelinePlayers = value;
            for (const auto& i : p.timelinePlayers)
            {
                _p->videoData.push_back(i->video());
                connect(
                    i,
                    SIGNAL(videoChanged(const tl::timeline::VideoData&)),
                    SLOT(_videoCallback(const tl::timeline::VideoData&)));
            }
            if (p.frameView)
            {
                _frameView();
            }
            _render();
        }

        const math::Vector2i& DLPlayback::viewPos() const
        {
            return _p->viewPos;
        }

        float DLPlayback::viewZoom() const
        {
            return _p->viewZoom;
        }

        void DLPlayback::setViewPosAndZoom(const tl::math::Vector2i&, float)
        {}

        void DLPlayback::setViewZoom(float, const tl::math::Vector2i& focus)
        {}

        void DLPlayback::frameView()
        {}

        void DLPlayback::_videoCallback(const tl::timeline::VideoData& value)
        {
            TLRENDER_P();
            const auto i = std::find(p.timelinePlayers.begin(), p.timelinePlayers.end(), sender());
            if (i != p.timelinePlayers.end())
            {
                const size_t index = i - p.timelinePlayers.begin();
                _p->videoData[index] = value;
            }
            _render();
        }

        void DLPlayback::_frameView()
        {}

        void DLPlayback::_render()
        {
            TLRENDER_P();

            p.glContext->makeCurrent(p.offscreenSurface.get());

            if (!p.buffer ||
                (p.buffer && p.buffer->getSize() != p.size))
            {
                gl::OffscreenBufferOptions options;
                options.colorType = imaging::PixelType::RGBA_U8;
                options.depth = gl::OffscreenDepth::_24;
                options.stencil = gl::OffscreenStencil::_8;
                p.buffer = gl::OffscreenBuffer::create(p.size, options);
            }

            p.render->setColorConfig(p.colorConfig);

            if (p.buffer)
            {
                gl::OffscreenBufferBinding binding(p.buffer);

                p.render->begin(p.size);
                p.render->drawVideo(
                    { p.videoData },
                    { math::BBox2i(0, 0, p.size.w, p.size.h) });
                p.render->end();

                auto image = imaging::Image::create(imaging::Info(p.size, imaging::PixelType::RGBA_U8));

                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glReadPixels(
                    0,
                    0,
                    p.size.w,
                    p.size.h,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    image->getData());

                p.device->display(image);
            }
        }
    }
}
