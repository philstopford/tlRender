// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/IItem.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/marker.h>

namespace tl
{
    namespace timelineui
    {
        bool ItemOptions::operator == (const ItemOptions& other) const
        {
            return
                inOutDisplay == other.inOutDisplay &&
                cacheDisplay == other.cacheDisplay &&
                clipRectScale == other.clipRectScale &&
                thumbnails == other.thumbnails &&
                thumbnailHeight == other.thumbnailHeight &&
                waveformHeight == other.waveformHeight &&
                waveformPrim == other.waveformPrim &&
                thumbnailFade == other.thumbnailFade &&
                showTransitions == other.showTransitions &&
                showMarkers == other.showMarkers;
        }

        bool ItemOptions::operator != (const ItemOptions& other) const
        {
            return !(*this == other);
        }

        std::vector<Marker> getMarkers(const otio::Item* item)
        {
            std::vector<Marker> out;
            for (const auto& marker : item->markers())
            {
                out.push_back({
                    marker->name(),
                    getMarkerColor(marker->color()),
                    marker->marked_range() });
            }
            return out;
        }

        image::Color4f getMarkerColor(const std::string& value)
        {
            const std::map<std::string, image::Color4f> colors =
            {
                //! \bug The OTIO marker variables are causing undefined
                //! symbol errors on Linux and macOS.
                /*{otio::Marker::Color::pink, image::Color4f(1.F, .752F, .796F)},
                { otio::Marker::Color::red, image::Color4f(1.F, 0.F, 0.F) },
                { otio::Marker::Color::orange, image::Color4f(1.F, .75F, 0.F) },
                { otio::Marker::Color::yellow, image::Color4f(1.F, 1.F, 0.F) },
                { otio::Marker::Color::green, image::Color4f(0.F, 1.F, 0.F) },
                { otio::Marker::Color::cyan, image::Color4f(0.F, 1.F, 1.F) },
                { otio::Marker::Color::blue, image::Color4f(0.F, 0.F, 1.F) },
                { otio::Marker::Color::purple, image::Color4f(0.5F, 0.F, .5F) },
                { otio::Marker::Color::magenta, image::Color4f(1.F, 0.F, 1.F) },
                { otio::Marker::Color::black, image::Color4f(0.F, 0.F, 0.F) },
                { otio::Marker::Color::white, image::Color4f(1.F, 1.F, 1.F) }*/
                { "PINK", image::Color4f(1.F, .752F, .796F)},
                { "RED", image::Color4f(1.F, 0.F, 0.F)},
                { "ORANGE", image::Color4f(1.F, .75F, 0.F) },
                { "YELLOW", image::Color4f(1.F, 1.F, 0.F)},
                { "GREEN", image::Color4f(0.F, 1.F, 0.F) },
                { "CYAN", image::Color4f(0.F, 1.F, 1.F)},
                { "BLUE", image::Color4f(0.F, 0.F, 1.F)},
                { "PURPLE", image::Color4f(0.5F, 0.F, .5F)},
                { "MAGENTA", image::Color4f(1.F, 0.F, 1.F)},
                { "BLACK", image::Color4f(0.F, 0.F, 0.F)},
                { "WHITE", image::Color4f(1.F, 1.F, 1.F)}
            };
            const auto i = colors.find(value);
            return i != colors.end() ? i->second : image::Color4f();
        }

        struct IItem::Private
        {
            std::shared_ptr<observer::ValueObserver<bool> > timeUnitsObserver;
        };

        void IItem::_init(
            const std::string& name,
            const ItemData& data,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
            TLRENDER_P();

            _data = data;

            p.timeUnitsObserver = observer::ValueObserver<bool>::create(
                data.timeUnitsModel->observeTimeUnitsChanged(),
                [this](bool)
                {
                    _timeUnitsUpdate();
                });
        }

        IItem::IItem() :
            _p(new Private)
        {}

        IItem::~IItem()
        {}

        void IItem::setScale(double value)
        {
            if (value == _scale)
                return;
            _scale = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void IItem::setOptions(const ItemOptions& value)
        {
            if (value == _options)
                return;
            _options = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        math::Box2i IItem::_getClipRect(
            const math::Box2i& value,
            double scale)
        {
            math::Box2i out;
            const math::Vector2i c = value.getCenter();
            out.min.x = (value.min.x - c.x) * scale + c.x;
            out.min.y = (value.min.y - c.y) * scale + c.y;
            out.max.x = (value.max.x - c.x) * scale + c.x;
            out.max.y = (value.max.y - c.y) * scale + c.y;
            return out;
        }

        std::string IItem::_getDurationLabel(const otime::RationalTime& value)
        {
            const otime::RationalTime rescaled = value.rescaled_to(_data.speed);
            return string::Format("{0}").
                arg(_data.timeUnitsModel->getLabel(rescaled));
        }

        void IItem::_timeUnitsUpdate()
        {}
    }
}
