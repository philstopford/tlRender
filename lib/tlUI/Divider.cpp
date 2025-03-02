// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/Divider.h>

namespace tl
{
    namespace ui
    {
        void Divider::_init(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::Divider", context, parent);
            setBackgroundRole(ColorRole::Border);
            switch (orientation)
            {
            case Orientation::Horizontal:
                setVStretch(Stretch::Expanding);
                break;
            case Orientation::Vertical:
                setHStretch(Stretch::Expanding);
                break;
            }
        }

        Divider::Divider()
        {}

        Divider::~Divider()
        {}

        std::shared_ptr<Divider> Divider::create(
            Orientation orientation,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<Divider>(new Divider);
            out->_init(orientation, context, parent);
            return out;
        }

        void Divider::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint.x = _sizeHint.y = event.style->getSizeRole(SizeRole::Border, event.displayScale);
        }
    }
}
