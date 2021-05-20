// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlrCore/Math.h>
#include <tlrCore/StringFormat.h>
#include <tlrCore/Time.h>

#include <QMessageBox>

namespace tlr
{
    App::App(int& argc, char** argv) :
        QApplication(argc, argv)
    {
        IApp::_init(
            argc,
            argv,
            "tlrplay-qwidget",
            "Play an editorial timeline.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "Input",
                    "The input timeline.",
                    true)
            });
        const int exitCode = getExit();
        if (exitCode != 0)
        {
            exit(exitCode);
            return;
        }

        Q_INIT_RESOURCE(tlrQt);

        qRegisterMetaType<qt::TimeObject::Units>("tlr::qt::TimeObject::Units");
        qRegisterMetaTypeStreamOperators<qt::TimeObject::Units>("tlr::qt::TimeObject::Units");

        QCoreApplication::setOrganizationName("tlRender");
        QCoreApplication::setApplicationName("tlrplay-qwidget");

        setStyle("Fusion");

        _timeObject = new qt::TimeObject(this);
        _settingsObject = new SettingsObject(_timeObject, this);

        _mainWindow = new MainWindow(_settingsObject, _timeObject);

        if (!_input.empty())
        {
            open(_input.c_str());
        }

        _mainWindow->show();
    }

    void App::open(const QString& fileName)
    {
        try
        {
            auto timeline = timeline::Timeline::create(fileName.toLatin1().data());
            auto timelinePlayer = new qt::TimelinePlayer(timeline, this);
            timelinePlayer->setFrameCacheReadAhead(_settingsObject->frameCacheReadAhead());
            timelinePlayer->setFrameCacheReadBehind(_settingsObject->frameCacheReadBehind());
            timelinePlayer->connect(
                _settingsObject,
                SIGNAL(frameCacheReadAheadChanged(int)),
                SLOT(setFrameCacheReadAhead(int)));
            timelinePlayer->connect(
                _settingsObject,
                SIGNAL(frameCacheReadBehindChanged(int)),
                SLOT(setFrameCacheReadBehind(int)));
            _timelinePlayers.append(timelinePlayer);

            Q_EMIT opened(timelinePlayer);

            _settingsObject->addRecentFile(fileName);
        }
        catch (const std::exception& e)
        {
            QMessageBox dialog;
            dialog.setText(e.what());
            dialog.exec();
        }
    }

    void App::close(qt::TimelinePlayer* timelinePlayer)
    {
        const int i = _timelinePlayers.indexOf(timelinePlayer);
        if (i != -1)
        {
            _timelinePlayers.removeAt(i);
            Q_EMIT closed(timelinePlayer);
            timelinePlayer->setParent(nullptr);
            delete timelinePlayer;
        }
    }

    void App::closeAll()
    {
        while (!_timelinePlayers.empty())
        {
            close(_timelinePlayers.back());
        }
    }
}
