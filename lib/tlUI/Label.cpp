// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Label.h>

#include <tlUI/LayoutUtil.h>

#include <tlCore/String.h>

namespace tl
{
    namespace ui
    {
        struct Label::Private
        {
            std::string text;
            size_t textWidth = 0;
            std::string textTmp;
            std::vector<std::string> lines;
            ColorRole textRole = ColorRole::Text;
            SizeRole marginRole = SizeRole::None;
            FontRole fontRole = FontRole::Label;

            struct SizeData
            {
                int margin = 0;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                bool textInit = true;
                math::Vector2i textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::vector<std::shared_ptr<image::Glyph> > > glyphs;
            };
            DrawData draw;
        };

        void Label::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Label", context, parent);
            _hAlign = HAlign::Left;
        }

        Label::Label() :
            _p(new Private)
        {}

        Label::~Label()
        {}

        std::shared_ptr<Label> Label::create(
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<Label>(new Label);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<Label> Label::create(
            const std::string& text,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Label>(new Label);
            out->_init(context, parent);
            out->setText(text);
            return out;
        }

        void Label::setText(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.text)
                return;
            p.text = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _textUpdate();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setTextWidth(size_t value)
        {
            TLRENDER_P();
            if (value == p.textWidth)
                return;
            p.textWidth = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _textUpdate();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setTextRole(ColorRole value)
        {
            TLRENDER_P();
            if (value == p.textRole)
                return;
            p.textRole = value;
            _updates |= Update::Draw;
        }

        void Label::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void Label::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.margin = event.style->getSizeRole(p.marginRole, event.displayScale);

            p.size.fontMetrics = event.getFontMetrics(p.fontRole);
            const auto fontInfo = event.style->getFontRole(p.fontRole, event.displayScale);
            if (fontInfo != p.size.fontInfo || p.size.textInit)
            {
                p.size.fontInfo = fontInfo;
                p.size.textInit = false;
                p.size.textSize = event.fontSystem->getSize(p.textTmp, fontInfo);
                p.draw.glyphs.clear();
            }

            _sizeHint.x =
                p.size.textSize.x +
                p.size.margin * 2;
            _sizeHint.y =
                p.size.textSize.y +
                p.size.margin * 2;
        }

        void Label::clipEvent(
            const math::Box2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            IWidget::clipEvent(clipRect, clipped, event);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void Label::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            //event.render->drawRect(_geometry, image::Color4f(.5F, .3F, .3F));

            const math::Box2i g = align(
                _geometry,
                _sizeHint,
                Stretch::Fixed,
                Stretch::Fixed,
                _hAlign,
                _vAlign).margin(-p.size.margin);

            if (!p.textTmp.empty() && p.draw.glyphs.empty())
            {
                for (const auto& line : p.lines)
                {
                    p.draw.glyphs.push_back(event.fontSystem->getGlyphs(line, p.size.fontInfo));
                }
            }
            math::Vector2i pos = g.min;
            for (const auto& glyphs : p.draw.glyphs)
            {
                event.render->drawText(
                    glyphs,
                    math::Vector2i(pos.x, pos.y + p.size.fontMetrics.ascender),
                    event.style->getColorRole(p.textRole));
                pos.y += p.size.fontMetrics.lineHeight;
            }
        }
        
        void Label::_textUpdate()
        {
            TLRENDER_P();
            if (!p.text.empty() && p.textWidth > 0)
            {
                p.textTmp = p.text.substr(0, std::min(p.textWidth, p.text.size()));
            }
            else
            {
                p.textTmp = p.text;
            }
            const auto lines = string::split(
                p.textTmp,
                { '\n', '\r' },
                string::SplitOptions::KeepEmpty);
            p.lines.clear();
            for (const auto& line : lines)
            {
                p.lines.push_back(line);
            }
        }
    }
}
