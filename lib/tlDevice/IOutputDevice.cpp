// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlDevice/IOutputDevice.h>

namespace tl
{
    namespace device
    {
        void IOutputDevice::_init(
            int deviceIndex,
            int displayModeIndex,
            const std::shared_ptr<system::Context>& context)
        {
            _deviceIndex = deviceIndex;
            _displayModeIndex = displayModeIndex;
        }

        IOutputDevice::IOutputDevice()
        {}

        IOutputDevice::~IOutputDevice()
        {}
    }
}
