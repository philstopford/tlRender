// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Base class for popup widgets.
        class IPopup : public IWidget
        {
            TLRENDER_NON_COPYABLE(IPopup);

        protected:
            void _init(
                const std::string& name,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IPopup();

        public:
            virtual ~IPopup() = 0;

            //! Close the popup.
            virtual void close() = 0;
        };
    }
}
