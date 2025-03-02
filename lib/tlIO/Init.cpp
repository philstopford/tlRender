// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlIO/Init.h>

#include <tlIO/IOSystem.h>

#include <tlGL/Init.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace io
    {
        void init(const std::shared_ptr<system::Context>& context)
        {
            gl::init(context);
            if (!context->getSystem<System>())
            {
                context->addSystem(System::create(context));
            }
        }
    }
}
