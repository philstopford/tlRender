// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/DoubleModel.h>
#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Double precision floating point number editor.
        class DoubleEdit : public IWidget
        {
            TLRENDER_NON_COPYABLE(DoubleEdit);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<DoubleModel>&,
                const std::shared_ptr<IWidget>& parent);

            DoubleEdit();

        public:
            virtual ~DoubleEdit();

            //! Create a new widget.
            static std::shared_ptr<DoubleEdit> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<DoubleModel>& = nullptr,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the value.
            double getValue() const;

            //! Set the value.
            void setValue(double);

            //! Set the callback.
            void setCallback(const std::function<void(double)>&);

            //! Get the range.
            const math::DoubleRange& getRange() const;

            //! Set the range.
            void setRange(const math::DoubleRange&);

            //! Set the step.
            void setStep(double);

            //! Set the large step.
            void setLargeStep(double);

            //! Get the model.
            const std::shared_ptr<DoubleModel>& getModel() const;

            //! Set the number of digits to display.
            void setDigits(int);

            //! Set the display precision.
            void setPrecision(int);

            //! Set the font role.
            void setFontRole(FontRole);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
