// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

namespace tl
{
    namespace examples
    {
        namespace filmstrip_qtwidget
        {
            App::App(
                int& argc,
                char** argv,
                const std::shared_ptr<system::Context>& context) :
                QApplication(argc, argv)
            {
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "filmstrip-qwidget",
                    "Example using the filmstrip widget.",
                    {
                        app::CmdLineValueArg<std::string>::create(
                            _input,
                            "input",
                            "The input timeline.",
                            true)
                    });
                const int exitCode = getExit();
                if (exitCode != 0)
                {
                    exit(exitCode);
                    return;
                }

                // Initialize Qt.
                setOrganizationName("tlRender");
                setApplicationName("filmstrip-qwidget");
                setStyle("Fusion");

                // Create the context object.
                _contextObject = new qt::ContextObject(context, this);

                // Create the main window.
                _mainWindow = new MainWindow(_input, _context);
                _mainWindow->show();
            }

            App::~App()
            {
                delete _mainWindow;
                _mainWindow = nullptr;
                delete _contextObject;
                _contextObject = nullptr;
            }
        }
    }
}
