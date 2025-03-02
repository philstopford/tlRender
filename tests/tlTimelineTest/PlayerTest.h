// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class PlayerTest : public tests::ITest
        {
        protected:
            PlayerTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<PlayerTest> create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _enums();
            void _loop();
            void _player();
        };
    }
}
