// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/FilesModel.h>

#include <tlCore/Time.h>

#include <QAbstractTableModel>

namespace tl
{
    namespace qt
    {
        class TimelineThumbnailObject;
    }

    namespace play_qt
    {
        //! Base class for table models.
        class IFilesTableModel : public QAbstractTableModel
        {
            Q_OBJECT

        public:
            IFilesTableModel(
                const std::shared_ptr<play::FilesModel>&,
                qt::TimelineThumbnailObject*,
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            virtual ~IFilesTableModel() = 0;

            //! Get the files.
            const std::vector<std::shared_ptr<play::FilesModelItem> >& files() const;

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            Qt::ItemFlags flags(const QModelIndex&) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        private Q_SLOTS:
            void _thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&);

        protected:
            int _index(const std::shared_ptr<play::FilesModelItem>&) const;

            std::shared_ptr<play::FilesModel> _filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > _files;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
