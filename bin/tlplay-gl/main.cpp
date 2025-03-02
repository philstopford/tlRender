// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/App.h>

#include <tlTimelineUI/Init.h>

#include <iostream>

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto context = tl::system::Context::create();
        tl::timelineui::init(context);
        auto app = tl::play_gl::App::create(argc, argv, context);
        if (0 == app->getExit())
        {
            app->run();
            r = app->getExit();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        r = 1;
    }
    return r;
}
