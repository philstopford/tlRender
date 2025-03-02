// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/GLRenderPrivate.h>

#if defined(TLRENDER_GL_DEBUG)
#include <tlGladDebug/gl.h>
#else // TLRENDER_GL_DEBUG
#include <tlGlad/gl.h>
#endif // TLRENDER_GL_DEBUG

#include <glm/gtc/matrix_transform.hpp>

namespace tl
{
    namespace timeline
    {
        void GLRender::drawRect(
            const math::Box2i& box,
            const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.rects);

            p.shaders["rect"]->bind();
            p.shaders["rect"]->setUniform("color", color);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (p.vbos["rect"])
            {
                p.vbos["rect"]->copy(convert(geom::box(box), p.vbos["rect"]->getType()));
            }
            if (p.vaos["rect"])
            {
                p.vaos["rect"]->bind();
                p.vaos["rect"]->draw(GL_TRIANGLES, 0, p.vbos["rect"]->getSize());
            }
        }

        void GLRender::drawMesh(
            const geom::TriangleMesh2& mesh,
            const math::Vector2i& position,
            const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.meshes);
            const size_t size = mesh.triangles.size();
            p.currentStats.meshTriangles += mesh.triangles.size();
            if (size > 0)
            {
                p.shaders["mesh"]->bind();
                const auto transform =
                    p.transform *
                    math::translate(math::Vector3f(
                        position.x,
                        position.y,
                        0.F));
                p.shaders["mesh"]->setUniform("transform.mvp", transform);
                p.shaders["mesh"]->setUniform("color", color);

                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                if (!p.vbos["mesh"] || (p.vbos["mesh"] && p.vbos["mesh"]->getSize() < size * 3))
                {
                    p.vbos["mesh"] = gl::VBO::create(size * 3, gl::VBOType::Pos2_F32);
                    p.vaos["mesh"].reset();
                }
                if (p.vbos["mesh"])
                {
                    p.vbos["mesh"]->copy(convert(mesh, gl::VBOType::Pos2_F32));
                }

                if (!p.vaos["mesh"] && p.vbos["mesh"])
                {
                    p.vaos["mesh"] = gl::VAO::create(p.vbos["mesh"]->getType(), p.vbos["mesh"]->getID());
                }
                if (p.vaos["mesh"] && p.vbos["mesh"])
                {
                    p.vaos["mesh"]->bind();
                    p.vaos["mesh"]->draw(GL_TRIANGLES, 0, size * 3);
                }
            }
        }

        void GLRender::drawColorMesh(
            const geom::TriangleMesh2& mesh,
            const math::Vector2i& position,
            const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.meshes);
            const size_t size = mesh.triangles.size();
            p.currentStats.meshTriangles += mesh.triangles.size();
            if (size > 0)
            {
                p.shaders["colorMesh"]->bind();
                const auto transform =
                    p.transform *
                    math::translate(math::Vector3f(
                        position.x,
                        position.y,
                        0.F));
                p.shaders["colorMesh"]->setUniform("transform.mvp", transform);
                p.shaders["colorMesh"]->setUniform("color", color);

                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                if (!p.vbos["colorMesh"] || (p.vbos["colorMesh"] && p.vbos["colorMesh"]->getSize() < size * 3))
                {
                    p.vbos["colorMesh"] = gl::VBO::create(size * 3, gl::VBOType::Pos2_F32_Color_F32);
                    p.vaos["colorMesh"].reset();
                }
                if (p.vbos["colorMesh"])
                {
                    p.vbos["colorMesh"]->copy(convert(mesh, gl::VBOType::Pos2_F32_Color_F32));
                }

                if (!p.vaos["colorMesh"] && p.vbos["colorMesh"])
                {
                    p.vaos["colorMesh"] = gl::VAO::create(p.vbos["colorMesh"]->getType(), p.vbos["colorMesh"]->getID());
                }
                if (p.vaos["colorMesh"] && p.vbos["colorMesh"])
                {
                    p.vaos["colorMesh"]->bind();
                    p.vaos["colorMesh"]->draw(GL_TRIANGLES, 0, size * 3);
                }
            }
        }

        void GLRender::Private::drawTextMesh(const geom::TriangleMesh2& mesh)
        {
            const size_t size = mesh.triangles.size();
            currentStats.textTriangles += size;
            if (size > 0)
            {
                if (!vbos["text"] || (vbos["text"] && vbos["text"]->getSize() < size * 3))
                {
                    vbos["text"] = gl::VBO::create(size * 3, gl::VBOType::Pos2_F32_UV_U16);
                    vaos["text"].reset();
                }
                if (vbos["text"])
                {
                    vbos["text"]->copy(convert(mesh, vbos["text"]->getType()));
                }
                if (!vaos["text"] && vbos["text"])
                {
                    vaos["text"] = gl::VAO::create(vbos["text"]->getType(), vbos["text"]->getID());
                }
                if (vaos["text"] && vbos["text"])
                {
                    vaos["text"]->bind();
                    vaos["text"]->draw(GL_TRIANGLES, 0, size * 3);
                }
            }
        }

        void GLRender::drawText(
            const std::vector<std::shared_ptr<image::Glyph> >& glyphs,
            const math::Vector2i& pos,
            const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.text);

            p.shaders["text"]->bind();
            p.shaders["text"]->setUniform("color", color);
            p.shaders["text"]->setUniform("textureSampler", 0);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
            uint8_t textureIndex = 0;
            const auto textures = p.glyphTextureAtlas->getTextures();
            glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);

            int x = 0;
            int32_t rsbDeltaPrev = 0;
            geom::TriangleMesh2 mesh;
            size_t meshIndex = 0;
            for (const auto& glyph : glyphs)
            {
                if (glyph)
                {
                    if (rsbDeltaPrev - glyph->lsbDelta > 32)
                    {
                        x -= 1;
                    }
                    else if (rsbDeltaPrev - glyph->lsbDelta < -31)
                    {
                        x += 1;
                    }
                    rsbDeltaPrev = glyph->rsbDelta;

                    if (glyph->image && glyph->image->isValid())
                    {
                        gl::TextureAtlasID id = 0;
                        const auto i = p.glyphIDs.find(glyph->info);
                        if (i != p.glyphIDs.end())
                        {
                            id = i->second;
                        }
                        gl::TextureAtlasItem item;
                        if (!p.glyphTextureAtlas->getItem(id, item))
                        {
                            id = p.glyphTextureAtlas->addItem(glyph->image, item);
                            p.glyphIDs[glyph->info] = id;
                        }
                        if (item.textureIndex != textureIndex)
                        {
                            textureIndex = item.textureIndex;
                            glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);

                            p.drawTextMesh(mesh);
                            mesh = geom::TriangleMesh2();
                            meshIndex = 0;
                        }

                        const math::Vector2i& offset = glyph->offset;
                        const math::Box2i box(
                            pos.x + x + offset.x,
                            pos.y - offset.y,
                            glyph->image->getWidth(),
                            glyph->image->getHeight());
                        const auto& min = box.min;
                        const auto& max = box.max;

                        mesh.v.push_back(math::Vector2f(min.x, min.y));
                        mesh.v.push_back(math::Vector2f(max.x + 1, min.y));
                        mesh.v.push_back(math::Vector2f(max.x + 1, max.y + 1));
                        mesh.v.push_back(math::Vector2f(min.x, max.y + 1));
                        mesh.t.push_back(math::Vector2f(item.textureU.getMin(), item.textureV.getMin()));
                        mesh.t.push_back(math::Vector2f(item.textureU.getMax(), item.textureV.getMin()));
                        mesh.t.push_back(math::Vector2f(item.textureU.getMax(), item.textureV.getMax()));
                        mesh.t.push_back(math::Vector2f(item.textureU.getMin(), item.textureV.getMax()));

                        geom::Triangle2 triangle;
                        triangle.v[0].v = meshIndex + 1;
                        triangle.v[1].v = meshIndex + 2;
                        triangle.v[2].v = meshIndex + 3;
                        triangle.v[0].t = meshIndex + 1;
                        triangle.v[1].t = meshIndex + 2;
                        triangle.v[2].t = meshIndex + 3;
                        mesh.triangles.push_back(triangle);
                        triangle.v[0].v = meshIndex + 3;
                        triangle.v[1].v = meshIndex + 4;
                        triangle.v[2].v = meshIndex + 1;
                        triangle.v[0].t = meshIndex + 3;
                        triangle.v[1].t = meshIndex + 4;
                        triangle.v[2].t = meshIndex + 1;
                        mesh.triangles.push_back(triangle);

                        meshIndex += 4;
                    }

                    x += glyph->advance;
                }
            }
            p.drawTextMesh(mesh);
        }

        void GLRender::drawTexture(
            unsigned int id,
            const math::Box2i& box,
            const image::Color4f& color)
        {
            TLRENDER_P();
            ++(p.currentStats.textures);

            p.shaders["texture"]->bind();
            p.shaders["texture"]->setUniform("color", color);
            p.shaders["texture"]->setUniform("textureSampler", 0);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0));
            glBindTexture(GL_TEXTURE_2D, id);

            if (p.vbos["texture"])
            {
                p.vbos["texture"]->copy(convert(geom::box(box), p.vbos["texture"]->getType()));
            }
            if (p.vaos["texture"])
            {
                p.vaos["texture"]->bind();
                p.vaos["texture"]->draw(GL_TRIANGLES, 0, p.vbos["texture"]->getSize());
            }
        }

        void GLRender::drawImage(
            const std::shared_ptr<image::Image>& image,
            const math::Box2i& box,
            const image::Color4f& color,
            const ImageOptions& imageOptions)
        {
            TLRENDER_P();
            ++(p.currentStats.images);

            const auto& info = image->getInfo();
            auto textures = p.textureCache.get(info, imageOptions.imageFilters);
            copyTextures(image, textures);

            p.shaders["image"]->bind();
            p.shaders["image"]->setUniform("color", color);
            p.shaders["image"]->setUniform("pixelType", static_cast<int>(info.pixelType));
            image::VideoLevels videoLevels = info.videoLevels;
            switch (imageOptions.videoLevels)
            {
            case InputVideoLevels::FullRange:  videoLevels = image::VideoLevels::FullRange;  break;
            case InputVideoLevels::LegalRange: videoLevels = image::VideoLevels::LegalRange; break;
            default: break;
            }
            p.shaders["image"]->setUniform("videoLevels", static_cast<int>(videoLevels));
            p.shaders["image"]->setUniform("yuvCoefficients", image::getYUVCoefficients(info.yuvCoefficients));
            p.shaders["image"]->setUniform("imageChannels", image::getChannelCount(info.pixelType));
            p.shaders["image"]->setUniform("mirrorX", info.layout.mirror.x);
            p.shaders["image"]->setUniform("mirrorY", info.layout.mirror.y);
            switch (info.pixelType)
            {
            case image::PixelType::YUV_420P_U8:
            case image::PixelType::YUV_422P_U8:
            case image::PixelType::YUV_444P_U8:
            case image::PixelType::YUV_420P_U16:
            case image::PixelType::YUV_422P_U16:
            case image::PixelType::YUV_444P_U16:
                p.shaders["image"]->setUniform("textureSampler1", 1);
                p.shaders["image"]->setUniform("textureSampler2", 2);
            default:
                p.shaders["image"]->setUniform("textureSampler0", 0);
                break;
            }

            switch (imageOptions.alphaBlend)
            {
            case AlphaBlend::None:
                glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ONE);
                break;
            case AlphaBlend::Straight:
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
                break;
            case AlphaBlend::Premultiplied:
                glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
                break;
            default: break;
            }

            if (p.vbos["image"])
            {
                p.vbos["image"]->copy(convert(geom::box(box), p.vbos["image"]->getType()));
            }
            if (p.vaos["image"])
            {
                p.vaos["image"]->bind();
                p.vaos["image"]->draw(GL_TRIANGLES, 0, p.vbos["image"]->getSize());
            }

            p.textureCache.add(info, imageOptions.imageFilters, textures);
        }
    }
}
