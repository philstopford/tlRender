// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IntModel.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace ui
    {
        struct IntModel::Private
        {
            std::shared_ptr<observer::Value<int> > value;
            std::shared_ptr<observer::Value<math::IntRange> > range;
            int step = 1;
            int largeStep = 10;
            std::shared_ptr<observer::Value<bool> > hasDefaultValue;
            int defaultValue = 0;
        };

        void IntModel::_init(const std::shared_ptr<system::Context>&)
        {
            TLRENDER_P();
            p.value = observer::Value<int>::create(0);
            p.range = observer::Value<math::IntRange>::create(math::IntRange(0, 100));
            p.hasDefaultValue = observer::Value<bool>::create(false);
        }

        IntModel::IntModel() :
            _p(new Private)
        {}

        IntModel::~IntModel()
        {}

        std::shared_ptr<IntModel> IntModel::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<IntModel>(new IntModel);
            out->_init(context);
            return out;
        }

        int IntModel::getValue() const
        {
            return _p->value->get();
        }

        void IntModel::setValue(int value)
        {
            TLRENDER_P();
            const math::IntRange& range = p.range->get();
            const int tmp = math::clamp(value, range.getMin(), range.getMax());
            _p->value->setIfChanged(tmp);
        }

        std::shared_ptr<observer::IValue<int> > IntModel::observeValue() const
        {
            return _p->value;
        }

        const math::IntRange& IntModel::getRange() const
        {
            return _p->range->get();
        }

        void IntModel::setRange(const math::IntRange& range)
        {
            TLRENDER_P();
            if (p.range->setIfChanged(range))
            {
                setValue(p.value->get());
            }
        }

        std::shared_ptr<observer::IValue<math::IntRange> > IntModel::observeRange() const
        {
            return _p->range;
        }

        int IntModel::getStep() const
        {
            return _p->step;
        }

        void IntModel::setStep(int value)
        {
            _p->step = value;
        }

        void IntModel::incrementStep()
        {
            TLRENDER_P();
            setValue(p.value->get() + p.step);
        }

        void IntModel::decrementStep()
        {
            TLRENDER_P();
            setValue(p.value->get() - p.step);
        }

        int IntModel::getLargeStep() const
        {
            return _p->largeStep;
        }

        void IntModel::setLargeStep(int value)
        {
            _p->largeStep = value;
        }

        void IntModel::incrementLargeStep()
        {
            TLRENDER_P();
            setValue(p.value->get() + p.largeStep);
        }

        void IntModel::decrementLargeStep()
        {
            TLRENDER_P();
            setValue(p.value->get() - p.largeStep);
        }

        bool IntModel::hasDefaultValue() const
        {
            return _p->hasDefaultValue->get();
        }

        std::shared_ptr<observer::IValue<bool> > IntModel::observeHasDefaultValue() const
        {
            return _p->hasDefaultValue;
        }

        int IntModel::getDefaultValue() const
        {
            return _p->defaultValue;
        }

        void IntModel::setDefaultValue(int value)
        {
            _p->hasDefaultValue->setIfChanged(true);
            _p->defaultValue = value;
        }

        void IntModel::setDefaultValue()
        {
            setValue(_p->defaultValue);
        }

        void IntModel::clearDefaultValue()
        {
            _p->hasDefaultValue->setIfChanged(false);
        }
    }
}
