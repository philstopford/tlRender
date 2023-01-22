// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FilesBModel.h>

#include <QApplication>
#include <QPalette>

namespace tl
{
    namespace play
    {
        struct FilesBModel::Private
        {
            std::vector<std::shared_ptr<FilesModelItem> > b;
            std::vector<std::shared_ptr<FilesModelItem> > active;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > bObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
        };

        FilesBModel::FilesBModel(
            const std::shared_ptr<FilesModel>& filesModel,
            qt::TimelineThumbnailProvider* thumbnailProvider,
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            FilesTableModel(filesModel, thumbnailProvider, context, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.bObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                filesModel->observeB(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    const auto prevIndexes = _bIndexes();
                    _p->b = value;
                    for (const auto& i : _bIndexes())
                    {
                        Q_EMIT dataChanged(
                            this->index(i, 0),
                            this->index(i, 1),
                            { Qt::BackgroundRole, Qt::ForegroundRole });
                    }
                    for (const auto& i : prevIndexes)
                    {
                        Q_EMIT dataChanged(
                            this->index(i, 0),
                            this->index(i, 1),
                            { Qt::BackgroundRole, Qt::ForegroundRole });
                    }
                });

            p.activeObserver = observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    _p->active = value;
                });

            p.layersObserver = observer::ListObserver<int>::create(
                filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    for (size_t i = 0; i < value.size() && i < _p->active.size(); ++i)
                    {
                        const auto j = std::find(_files.begin(), _files.end(), _p->active[i]);
                        if (j != _files.end())
                        {
                            const int index = j - _files.begin();
                            Q_EMIT dataChanged(
                                this->index(index, 1),
                                this->index(index, 1),
                                { Qt::DisplayRole, Qt::EditRole });
                        }
                    }
                });
        }

        FilesBModel::~FilesBModel()
        {}

        int FilesBModel::columnCount(const QModelIndex & parent) const
        {
            return 2;
        }

        Qt::ItemFlags FilesBModel::flags(const QModelIndex& index) const
        {
            TLRENDER_P();
            Qt::ItemFlags out = FilesTableModel::flags(index);
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                switch (index.column())
                {
                case 1: out |= Qt::ItemIsEditable; break;
                }
            }
            return out;
        }

        QVariant FilesBModel::data(const QModelIndex& index, int role) const
        {
            QVariant out = FilesTableModel::data(index, role);
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::DisplayRole:
                {
                    switch (index.column())
                    {
                    case 1:
                        if (!item->ioInfo.video.empty() &&
                            item->videoLayer < item->ioInfo.video.size())
                        {
                            const std::string s = item->ioInfo.video[item->videoLayer].name;
                            out.setValue(QString::fromUtf8(s.c_str()));
                        }
                        break;
                    }
                    break;
                }
                case Qt::EditRole:
                    switch (index.column())
                    {
                    case 1: out.setValue(item->videoLayer); break;
                    }
                    break;
                case Qt::BackgroundRole:
                {
                    const auto bIndexes = _bIndexes();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(), index.row());
                    if (i != bIndexes.end())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                    }
                    break;
                }
                case Qt::ForegroundRole:
                {
                    const auto bIndexes = _bIndexes();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(), index.row());
                    if (i != bIndexes.end())
                    {
                        out.setValue(QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                    }
                    break;
                }
                default: break;
                }
            }
            return out;
        }

        bool FilesBModel::setData(const QModelIndex& index, const QVariant& value, int role)
        {
            TLRENDER_P();
            bool out = false;
            if (index.isValid() &&
                index.row() >= 0 &&
                index.row() < _files.size() &&
                index.column() >= 0 &&
                index.column() < 2)
            {
                const auto& item = _files[index.row()];
                switch (role)
                {
                case Qt::EditRole:
                    switch (index.column())
                    {
                    case 1:
                        _filesModel->setLayer(item, value.toInt());
                        out = true;
                        break;
                    }
                    break;
                default: break;
                }
            }
            return out;
        }

        QVariant FilesBModel::headerData(int section, Qt::Orientation orientation, int role) const
        {
            QVariant out;
            if (Qt::Horizontal == orientation)
            {
                switch (role)
                {
                case Qt::DisplayRole:
                    switch (section)
                    {
                    case 0: out = tr("Name"); break;
                    case 1: out = tr("Layer"); break;
                    }
                    break;
                default: break;
                }
            }
            return out;
        }

        std::vector<int> FilesBModel::_bIndexes() const
        {
            TLRENDER_P();
            std::vector<int> out;
            for (const auto& b : p.b)
            {
                out.push_back(_index(b));
            }
            return out;
        }
    }
}
