// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/DoubleSlider.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct DoubleSlider::Private
        {
            std::shared_ptr<DoubleModel> model;

            struct SizeData
            {
                int border = 0;
                int handle = 0;
                image::FontMetrics fontMetrics;
            };
            SizeData size;

            struct MouseData
            {
                bool inside = false;
                math::Vector2i pos;
                bool pressed = false;
            };
            MouseData mouse;

            std::function<void(double)> callback;

            std::shared_ptr<observer::ValueObserver<double> > valueObserver;
            std::shared_ptr<observer::ValueObserver<math::DoubleRange> > rangeObserver;
        };

        void DoubleSlider::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::DoubleSlider", context, parent);
            TLRENDER_P();

            setMouseHover(true);
            setAcceptsKeyFocus(true);
            setHStretch(Stretch::Expanding);

            p.model = model;
            if (!p.model)
            {
                p.model = DoubleModel::create(context);
            }

            p.valueObserver = observer::ValueObserver<double>::create(
                p.model->observeValue(),
                [this](double value)
                {
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                    if (_p->callback)
                    {
                        _p->callback(value);
                    }
                });

            p.rangeObserver = observer::ValueObserver<math::DoubleRange>::create(
                p.model->observeRange(),
                [this](const math::DoubleRange&)
                {
                    _updates |= Update::Size;
                    _updates |= Update::Draw;
                });
        }

        DoubleSlider::DoubleSlider() :
            _p(new Private)
        {}

        DoubleSlider::~DoubleSlider()
        {}

        std::shared_ptr<DoubleSlider> DoubleSlider::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<DoubleModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DoubleSlider>(new DoubleSlider);
            out->_init(context, model, parent);
            return out;
        }

        double DoubleSlider::getValue() const
        {
            return _p->model->getValue();
        }

        void DoubleSlider::setValue(double value)
        {
            _p->model->setValue(value);
        }

        void DoubleSlider::setCallback(const std::function<void(double)>& value)
        {
            _p->callback = value;
        }

        const math::DoubleRange& DoubleSlider::getRange() const
        {
            return _p->model->getRange();
        }

        void DoubleSlider::setRange(const math::DoubleRange& value)
        {
            _p->model->setRange(value);
        }

        void DoubleSlider::setStep(double value)
        {
            _p->model->setStep(value);
        }

        void DoubleSlider::setLargeStep(double value)
        {
            _p->model->setLargeStep(value);
        }

        void DoubleSlider::setDefaultValue(double value)
        {
            _p->model->setDefaultValue(value);
        }

        const std::shared_ptr<DoubleModel>& DoubleSlider::getModel() const
        {
            return _p->model;
        }

        void DoubleSlider::setVisible(bool value)
        {
            const bool changed = value != _visible;
            IWidget::setVisible(value);
            if (changed && !_visible)
            {
                _resetMouse();
            }
        }

        void DoubleSlider::setEnabled(bool value)
        {
            const bool changed = value != _enabled;
            IWidget::setEnabled(value);
            if (changed && !_enabled)
            {
                _resetMouse();
            }
        }

        void DoubleSlider::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.border = event.style->getSizeRole(SizeRole::Border, event.displayScale);
            p.size.handle = event.style->getSizeRole(SizeRole::Handle, event.displayScale);

            auto fontInfo = event.style->getFontRole(FontRole::Label, event.displayScale);
            p.size.fontMetrics = event.fontSystem->getMetrics(fontInfo);

            _sizeHint.x =
                event.style->getSizeRole(SizeRole::Slider, event.displayScale) +
                p.size.border * 6;
            _sizeHint.y =
                p.size.fontMetrics.lineHeight +
                p.size.border * 6;
        }

        void DoubleSlider::clipEvent(
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

        void DoubleSlider::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            if (_keyFocus)
            {
                event.render->drawMesh(
                    border(g, p.size.border * 2),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::KeyFocus));
            }
            else
            {
                event.render->drawMesh(
                    border(g.margin(-p.size.border), p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
            }

            event.render->drawRect(
                g.margin(-p.size.border * 2),
                event.style->getColorRole(ColorRole::Base));

            const math::Box2i g2 = _getSliderGeometry();
            //event.render->drawRect(
            //    g2,
            //    image::Color4f(1.F, 0.F, 0.F, .5F));
            int pos = 0;
            if (p.model)
            {
                pos = _valueToPos(p.model->getValue());
            }
            const math::Box2i g3(
                pos - p.size.handle / 2,
                g2.y(),
                p.size.handle,
                g2.h());
            event.render->drawRect(
                g3,
                event.style->getColorRole(ColorRole::Button));
            if (p.mouse.pressed)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (p.mouse.inside)
            {
                event.render->drawRect(
                    g3,
                    event.style->getColorRole(ColorRole::Hover));
            }
        }

        void DoubleSlider::mouseEnterEvent()
        {
            TLRENDER_P();
            p.mouse.inside = true;
            _updates |= Update::Draw;
        }

        void DoubleSlider::mouseLeaveEvent()
        {
            TLRENDER_P();
            p.mouse.inside = false;
            _updates |= Update::Draw;
        }

        void DoubleSlider::mouseMoveEvent(MouseMoveEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pos = event.pos;
            if (p.mouse.pressed && p.model)
            {
                p.model->setValue(_posToValue(p.mouse.pos.x));
            }
        }

        void DoubleSlider::mousePressEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = true;
            if (p.model)
            {
                p.model->setValue(_posToValue(p.mouse.pos.x));
            }
            takeKeyFocus();
            _updates |= Update::Draw;
        }

        void DoubleSlider::mouseReleaseEvent(MouseClickEvent& event)
        {
            TLRENDER_P();
            event.accept = true;
            p.mouse.pressed = false;
            _updates |= Update::Draw;
        }

        void DoubleSlider::keyPressEvent(KeyEvent& event)
        {
            TLRENDER_P();
            if (isEnabled() && p.model && 0 == event.modifiers)
            {
                switch (event.key)
                {
                case Key::Left:
                case Key::Down:
                    event.accept = true;
                    p.model->decrementStep();
                    break;
                case Key::Right:
                case Key::Up:
                    event.accept = true;
                    p.model->incrementStep();
                    break;
                case Key::PageUp:
                    event.accept = true;
                    p.model->incrementLargeStep();
                    break;
                case Key::PageDown:
                    event.accept = true;
                    p.model->decrementLargeStep();
                    break;
                case Key::End:
                    event.accept = true;
                    p.model->setValue(p.model->getRange().getMin());
                    break;
                case Key::Home:
                    event.accept = true;
                    p.model->setValue(p.model->getRange().getMax());
                    break;
                case Key::Escape:
                    if (hasKeyFocus())
                    {
                        event.accept = true;
                        releaseKeyFocus();
                    }
                    break;
                default: break;
                }
            }
        }

        void DoubleSlider::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }

        math::Box2i DoubleSlider::_getSliderGeometry() const
        {
            TLRENDER_P();
            return _geometry.margin(
                -(p.size.border * 3 + p.size.handle / 2),
                -(p.size.border * 3),
                -(p.size.border * 3 + p.size.handle / 2),
                -(p.size.border * 3));
        }

        double DoubleSlider::_posToValue(int pos) const
        {
            TLRENDER_P();
            const math::Box2i g = _getSliderGeometry();
            const double v = (pos - g.x()) / static_cast<double>(g.w());
            double out = 0.0;
            if (p.model)
            {
                const math::DoubleRange& range = p.model->getRange();
                out = range.getMin() + (range.getMax() - range.getMin()) * v;
            }
            return out;
        }

        int DoubleSlider::_valueToPos(double value) const
        {
            TLRENDER_P();
            const math::Box2i g = _getSliderGeometry();
            double v = 0.0;
            if (p.model)
            {
                const math::DoubleRange& range = p.model->getRange();
                if (range.getMin() != range.getMax())
                {
                    v = (value - range.getMin()) /
                        static_cast<double>(range.getMax() - range.getMin());
                }
            }
            return g.x() + g.w() * v;
        }

        void DoubleSlider::_resetMouse()
        {
            TLRENDER_P();
            if (p.mouse.pressed || p.mouse.inside)
            {
                p.mouse.pressed = false;
                p.mouse.inside = false;
                _updates |= Update::Draw;
            }
        }
    }
}
