// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>
#include <tlQt/TimelinePlayer.h>

#include <tlTimeline/IRender.h>

#include <QWidget>

namespace tl
{
    namespace qt
    {
        class TimelineThumbnailObject;
    };

    namespace qtwidget
    {
        //! Timeline slider.
        class TimelineSlider : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                tl::timeline::TimeUnits timeUnits
                READ timeUnits
                WRITE setTimeUnits)
            Q_PROPERTY(
                bool thumbnails
                READ hasThumbnails
                WRITE setThumbnails)
            Q_PROPERTY(
                bool stopOnScrub
                READ hasStopOnScrub
                WRITE setStopOnScrub)

        public:
            TimelineSlider(
                qt::TimelineThumbnailObject*,
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            virtual ~TimelineSlider();

            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Set the color configuration options.
            void setColorConfigOptions(const timeline::ColorConfigOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the timeline player.
            void setTimelinePlayer(qt::TimelinePlayer*);

            //! Get the time units.
            timeline::TimeUnits timeUnits() const;

            //! Get whether thumbnails are displayed.
            bool hasThumbnails() const;

            //! Get whether playback is stopped when scrubbing.
            bool hasStopOnScrub() const;

        public Q_SLOTS:
            //! Set the time units.
            void setTimeUnits(tl::timeline::TimeUnits);

            //! Set whether thumbnails are displayed.
            void setThumbnails(bool);

            //! Set whether playback is stopped when scrubbing.
            void setStopOnScrub(bool);

        protected:
            void resizeEvent(QResizeEvent*) override;
            void paintEvent(QPaintEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;
            void wheelEvent(QWheelEvent*) override;

        private Q_SLOTS:
            void _thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&);

        private:
            otime::RationalTime _posToTime(int) const;
            int _timeToPos(const otime::RationalTime&) const;

            void _thumbnailsUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
