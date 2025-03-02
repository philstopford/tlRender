// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

namespace tl
{
    namespace timeline
    {
        //! OpenGL renderer.
        class GLRender : public IRender
        {
            TLRENDER_NON_COPYABLE(GLRender);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            GLRender();

        public:
            virtual ~GLRender();

            //! Create a new renderer.
            static std::shared_ptr<GLRender> create(const std::shared_ptr<system::Context>&);

            void begin(
                const image::Size&,
                const ColorConfigOptions& = ColorConfigOptions(),
                const LUTOptions& = LUTOptions(),
                const RenderOptions& = RenderOptions()) override;
            void end() override;

            image::Size getRenderSize() const override;
            void setRenderSize(const image::Size&) override;
            math::Box2i getViewport() const override;
            void setViewport(const math::Box2i&) override;
            void clearViewport(const image::Color4f&) override;
            bool getClipRectEnabled() const override;
            void setClipRectEnabled(bool) override;
            math::Box2i getClipRect() const override;
            void setClipRect(const math::Box2i&) override;
            math::Matrix4x4f getTransform() const override;
            void setTransform(const math::Matrix4x4f&) override;

            void drawRect(
                const math::Box2i&,
                const image::Color4f&) override;
            void drawMesh(
                const geom::TriangleMesh2&,
                const math::Vector2i& position,
                const image::Color4f&) override;
            void drawColorMesh(
                const geom::TriangleMesh2&,
                const math::Vector2i& position,
                const image::Color4f&) override;
            void drawText(
                const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
                const math::Vector2i& position,
                const image::Color4f&) override;
            void drawTexture(
                unsigned int,
                const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F)) override;
            void drawImage(
                const std::shared_ptr<image::Image>&,
                const math::Box2i&,
                const image::Color4f& = image::Color4f(1.F, 1.F, 1.F),
                const ImageOptions& = ImageOptions()) override;
            void drawVideo(
                const std::vector<VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<ImageOptions>& = {},
                const std::vector<DisplayOptions>& = {},
                const CompareOptions& = CompareOptions()) override;

        private:
            void _setColorConfig(const ColorConfigOptions&);
            void _setLUT(const LUTOptions&);
            void _drawVideoA(
                const std::vector<VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<ImageOptions>&,
                const std::vector<DisplayOptions>&,
                const CompareOptions&);
            void _drawVideoB(
                const std::vector<VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<ImageOptions>&,
                const std::vector<DisplayOptions>&,
                const CompareOptions&);
            void _drawVideoWipe(
                const std::vector<VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<ImageOptions>&,
                const std::vector<DisplayOptions>&,
                const CompareOptions&);
            void _drawVideoOverlay(
                const std::vector<VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<ImageOptions>&,
                const std::vector<DisplayOptions>&,
                const CompareOptions&);
            void _drawVideoDifference(
                const std::vector<VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<ImageOptions>&,
                const std::vector<DisplayOptions>&,
                const CompareOptions&);
            void _drawVideoTile(
                const std::vector<VideoData>&,
                const std::vector<math::Box2i>&,
                const std::vector<ImageOptions>&,
                const std::vector<DisplayOptions>&,
                const CompareOptions&);
            void _drawVideo(
                const VideoData&,
                const math::Box2i&,
                const std::shared_ptr<ImageOptions>&,
                const DisplayOptions&);

            TLRENDER_PRIVATE();
        };
    }
}
