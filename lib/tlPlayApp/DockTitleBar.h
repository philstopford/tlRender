// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace play
    {
        //! Dock widget title bar.
        class DockTitleBar : public QWidget
        {
            Q_OBJECT

        public:
            DockTitleBar(QWidget* parent = nullptr);

            ~DockTitleBar() override;

        public Q_SLOTS:
            //! Set the title bar text.
            void setText(const QString&);

            //! Set the title bar icon.
            void setIcon(const QIcon&);

        protected:
            void paintEvent(QPaintEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
