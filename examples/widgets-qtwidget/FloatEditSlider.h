// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/FloatEditSlider.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            class FloatEditSlider : public QWidget
            {
                Q_OBJECT

            public:
                FloatEditSlider(QWidget* parent = nullptr);

            private:
                std::vector<qtwidget::FloatEditSlider*> _sliders;
            };
        }
    }
}
