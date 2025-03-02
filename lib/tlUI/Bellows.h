// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Bellows widget.
        class Bellows : public IWidget
        {
            TLRENDER_NON_COPYABLE(Bellows);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Bellows();

        public:
            virtual ~Bellows();

            //! Create a new widget.
            static std::shared_ptr<Bellows> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            static std::shared_ptr<Bellows> create(
                const std::string& text,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the text.
            void setText(const std::string&);

            //! Set the widget.
            void setWidget(const std::shared_ptr<IWidget>&);

            //! Get whether the bellows is open.
            bool isOpen() const;

            //! Set whether the bellows is open.
            void setOpen(bool);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
