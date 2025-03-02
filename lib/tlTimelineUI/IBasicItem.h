// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <opentimelineio/gap.h>

namespace tl
{
    namespace timelineui
    {
        //! Base class for clips, gaps, and other items.
        class IBasicItem : public IItem
        {
        protected:
            void _init(
                const otime::TimeRange&,
                const std::string& label,
                ui::ColorRole,
                const std::vector<Marker>&,
                const std::string& name,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IBasicItem();

        public:
            virtual ~IBasicItem() = 0;

            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(
                const math::Box2i&,
                bool,
                const ui::ClipEvent&) override;
            void drawEvent(
                const math::Box2i&,
                const ui::DrawEvent&) override;

        protected:
            int _getMargin() const;
            int _getLineHeight() const;
            math::Box2i _getInsideGeometry() const;

            void _timeUnitsUpdate() override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
