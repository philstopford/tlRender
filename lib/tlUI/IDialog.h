// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        //! Base class for dialog widgets.
        class IDialog : public IPopup
        {
            TLRENDER_NON_COPYABLE(IDialog);

        protected:
            void _init(
                const std::string& name,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IDialog();

        public:
            virtual ~IDialog() = 0;

            //! Open the dialog.
            void open(const std::shared_ptr<EventLoop>&);

            //! Get whether the dialog is open.
            bool isOpen() const;

            //! Close the dialog.
            void close() override;

            //! Set the close callback.
            void setCloseCallback(const std::function<void(void)>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(
                const math::Box2i&,
                const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
