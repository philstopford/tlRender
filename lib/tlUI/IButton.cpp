// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct IButton::Private
        {
            bool checkable = false;
            float iconScale = 1.F;
            bool iconInit = false;
            std::future<std::shared_ptr<image::Image> > iconFuture;
            bool checkedIconInit = false;
            std::future<std::shared_ptr<image::Image> > checkedIconFuture;
            bool repeatClick = false;
            bool repeatClickInit = false;
            std::chrono::steady_clock::time_point repeatClickTimer;
        };

        void IButton::_init(
            const std::string& name,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(name, context, parent);
            setMouseHover(true);
        }

        IButton::IButton() :
            _p(new Private)
        {}

        IButton::~IButton()
        {}

        bool IButton::isCheckable() const
        {
            return _p->checkable;
        }

        void IButton::setCheckable(bool value)
        {
            TLRENDER_P();
            if (value == p.checkable)
                return;
            p.checkable = value;
            if (!p.checkable && _checked)
            {
                _checked = false;
                _updates |= Update::Draw;
            }
        }

        bool IButton::isChecked() const
        {
            return _checked;
        }

        void IButton::setChecked(bool value)
        {
            TLRENDER_P();
            if (value == _checked)
                return;
            _checked = value;
            _updates |= Update::Draw;
        }

        void IButton::setText(const std::string& value)
        {
            if (value == _text)
                return;
            _text = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IButton::setFontRole(FontRole value)
        {
            if (value == _fontRole)
                return;
            _fontRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void IButton::setIcon(const std::string& icon)
        {
            TLRENDER_P();
            _icon = icon;
            p.iconInit = true;
            _iconImage.reset();
        }

        void IButton::setCheckedIcon(const std::string& icon)
        {
            TLRENDER_P();
            _checkedIcon = icon;
            p.checkedIconInit = true;
            _checkedIconImage.reset();
        }

        void IButton::setButtonRole(ColorRole value)
        {
            if (value == _buttonRole)
                return;
            _buttonRole = value;
            _updates |= Update::Draw;
        }

        void IButton::setCheckedRole(ColorRole value)
        {
            if (value == _checkedRole)
                return;
            _checkedRole = value;
            _updates |= Update::Draw;
        }

        void IButton::setRepeatClick(bool value)
        {
            TLRENDER_P();
            p.repeatClick = value;
        }

        void IButton::setHoveredCallback(const std::function<void(bool)>& value)
        {
            _hoveredCallback = value;
        }

        void IButton::setPressedCallback(const std::function<void(void)>& value)
        {
            _pressedCallback = value;
        }

        void IButton::setClickedCallback(const std::function<void(void)>& value)
        {
            _clickedCallback = value;
        }

        void IButton::setCheckedCallback(const std::function<void(bool)>& value)
        {
            _checkedCallback = value;
        }

        void IButton::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                _resetMouse();
            }
        }

        void IButton::setEnabled(bool value)
        {
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                _resetMouse();
            }
        }

        void IButton::tickEvent(
            bool parentsVisible,
            bool parentsEnabled,
            const TickEvent& event)
        {
            IWidget::tickEvent(parentsVisible, parentsEnabled, event);
            TLRENDER_P();
            if (event.displayScale != p.iconScale)
            {
                p.iconScale = event.displayScale;
                p.iconInit = true;
                p.iconFuture = std::future<std::shared_ptr<image::Image> >();
                _iconImage.reset();
                p.checkedIconInit = true;
                p.checkedIconFuture = std::future<std::shared_ptr<image::Image> >();
                _checkedIconImage.reset();
            }
            if (!_icon.empty() && p.iconInit)
            {
                p.iconInit = false;
                p.iconFuture = event.iconLibrary->request(_icon, event.displayScale);
            }
            if (p.iconFuture.valid() &&
                p.iconFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                _iconImage = p.iconFuture.get();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
            if (!_checkedIcon.empty() && p.checkedIconInit)
            {
                p.checkedIconInit = false;
                p.checkedIconFuture = event.iconLibrary->request(_checkedIcon, event.displayScale);
            }
            if (p.checkedIconFuture.valid() &&
                p.checkedIconFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                _checkedIconImage = p.checkedIconFuture.get();
                _updates |= Update::Size;
                _updates |= Update::Draw;
            }
            if (_pressed && p.repeatClick)
            {
                const float duration = p.repeatClickInit ? .4F : .02F;
                const auto now = std::chrono::steady_clock::now();
                const std::chrono::duration<float> diff = now - p.repeatClickTimer;
                if (diff.count() > duration)
                {
                    _click();
                    p.repeatClickInit = false;
                    p.repeatClickTimer = now;
                }
            }
        }

        void IButton::clipEvent(
            const math::Box2i& clipRect,
            bool clipped,
            const ClipEvent& event)
        {
            const bool changed = clipped != _clipped;
            IWidget::clipEvent(clipRect, clipped, event);
            if (changed && clipped)
            {
                _resetMouse();
            }
        }

        void IButton::mouseEnterEvent()
        {
            _inside = true;
            _updates |= Update::Draw;
            if (_hoveredCallback)
            {
                _hoveredCallback(_inside);
            }
        }

        void IButton::mouseLeaveEvent()
        {
            _inside = false;
            _updates |= Update::Draw;
            if (_hoveredCallback)
            {
                _hoveredCallback(_inside);
            }
        }

        void IButton::mouseMoveEvent(MouseMoveEvent& event)
        {
            event.accept = true;
            _cursorPos = event.pos;
        }

        void IButton::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            if (acceptsKeyFocus())
            {
                takeKeyFocus();
            }
            _pressed = true;
            _updates |= Update::Draw;
            if (_pressedCallback)
            {
                _pressedCallback();
            }
            if (p.repeatClick)
            {
                p.repeatClickInit = true;
                p.repeatClickTimer = std::chrono::steady_clock::now();
            }
        }

        void IButton::mouseReleaseEvent(MouseClickEvent& event)
        {
            event.accept = true;
            _pressed = false;
            _updates |= Update::Draw;
            if (_geometry.contains(_cursorPos))
            {
                _click();
            }
        }

        void IButton::_click()
        {
            TLRENDER_P();
            if (_clickedCallback)
            {
                _clickedCallback();
            }
            if (p.checkable)
            {
                _checked = !_checked;
                _updates |= Update::Draw;
                if (_checkedCallback)
                {
                    _checkedCallback(_checked);
                }
            }
        }

        void IButton::_resetMouse()
        {
            if (_pressed || _inside)
            {
                _pressed = false;
                _inside = false;
                _updates |= Update::Draw;
                if (_hoveredCallback)
                {
                    _hoveredCallback(_inside);
                }
            }
        }
    }
}
