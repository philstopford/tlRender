// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/IRenderTest.h>

#include <tlTimeline/IRender.h>

#include <tlCore/Assert.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        IRenderTest::IRenderTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::IRenderTest", context)
        {}

        std::shared_ptr<IRenderTest> IRenderTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<IRenderTest>(new IRenderTest(context));
        }

        void IRenderTest::run()
        {
            _enums();
            _util();
        }

        void IRenderTest::_enums()
        {
            _enum<InputVideoLevels>("InputVideoLevels", getInputVideoLevelsEnums);
            _enum<Channels>("Channels", getChannelsEnums);
            _enum<AlphaBlend>("AlphaBlend", getAlphaBlendEnums);
            _enum<CompareMode>("CompareMode", getCompareModeEnums);
        }

        void IRenderTest::_util()
        {
            {
                image::Size size = getRenderSize(CompareMode::A, {});
                TLRENDER_ASSERT(0 == size.w);
                TLRENDER_ASSERT(0 == size.h);
            }
        }
    }
}
