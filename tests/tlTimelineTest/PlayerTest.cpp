// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/PlayerTest.h>

#include <tlTimeline/Player.h>
#include <tlTimeline/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/Assert.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/imageSequenceReference.h>

#include <sstream>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        PlayerTest::PlayerTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::PlayerTest", context)
        {}

        std::shared_ptr<PlayerTest> PlayerTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PlayerTest>(new PlayerTest(context));
        }

        void PlayerTest::run()
        {
            _enums();
            _loop();
            _player();
        }

        void PlayerTest::_enums()
        {
            ITest::_enum<Playback>("Playback", getPlaybackEnums);
            ITest::_enum<Loop>("Loop", getLoopEnums);
            ITest::_enum<TimeAction>("TimeAction", getTimeActionEnums);
        }

        void PlayerTest::_loop()
        {
            {
                const otime::TimeRange timeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
                TLRENDER_ASSERT(otime::RationalTime(0.0, 24.0) == loop(otime::RationalTime(0.0, 24.0), timeRange));
                TLRENDER_ASSERT(otime::RationalTime(1.0, 24.0) == loop(otime::RationalTime(1.0, 24.0), timeRange));
                TLRENDER_ASSERT(otime::RationalTime(23.0, 24.0) == loop(otime::RationalTime(23.0, 24.0), timeRange));
                TLRENDER_ASSERT(otime::RationalTime(0.0, 24.0) == loop(otime::RationalTime(24.0, 24.0), timeRange));
                TLRENDER_ASSERT(otime::RationalTime(23.0, 24.0) == loop(otime::RationalTime(-1.0, 24.0), timeRange));
            }
            {
                const otime::TimeRange timeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
                auto ranges = loop(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0)), timeRange);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0)) == ranges[0]);
                ranges = loop(otime::TimeRange(otime::RationalTime(-10.0, 24.0), otime::RationalTime(34.0, 24.0)), timeRange);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0)) == ranges[0]);
                ranges = loop(otime::TimeRange(otime::RationalTime(-10.0, 24.0), otime::RationalTime(20.0, 24.0)), timeRange);
                TLRENDER_ASSERT(2 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(14.0, 24.0), otime::RationalTime(10.0, 24.0)) == ranges[0]);
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(10.0, 24.0)) == ranges[1]);
                ranges = loop(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(20.0, 24.0)), timeRange);
                TLRENDER_ASSERT(2 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(14.0, 24.0)) == ranges[0]);
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(6.0, 24.0)) == ranges[1]);
            }
            {
                const otime::TimeRange timeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0));
                auto ranges = loop(otime::TimeRange(otime::RationalTime(-1.0, 24.0), otime::RationalTime(2.0, 24.0)), timeRange);
                TLRENDER_ASSERT(1 == ranges.size());
                TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(1.0, 24.0)) == ranges[0]);
            }
        }

        void PlayerTest::_player()
        {
            // Write an OTIO timeline.
            auto otioTrack = new otio::Track();
            auto otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "PlayerTest.", ".ppm", 0, 1, 1, 0));
            const otime::TimeRange clipTimeRange(otime::RationalTime(0.0, 24.0), otime::RationalTime(24.0, 24.0));
            otioClip->set_source_range(clipTimeRange);
            otio::ErrorStatus errorStatus = otio::ErrorStatus::OK;
            otioTrack->append_child(otioClip, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                throw std::runtime_error("Cannot append child");
            }
            otioClip = new otio::Clip;
            otioClip->set_media_reference(new otio::ImageSequenceReference("", "PlayerTest.", ".ppm", 0, 1, 1, 0));
            otioClip->set_source_range(clipTimeRange);
            otioTrack->append_child(otioClip, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                throw std::runtime_error("Cannot append child");
            }
            auto otioStack = new otio::Stack;
            otioStack->append_child(otioTrack, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                throw std::runtime_error("Cannot append child");
            }
            auto otioTimeline = new otio::Timeline;
            otioTimeline->set_tracks(otioStack);
            otioTimeline->set_global_start_time(otime::RationalTime(10.0, 24.0));
            const std::string fileName("PlayerTest.otio");
            otioTimeline->to_json_file(fileName, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                throw std::runtime_error("Cannot write file: " + fileName);
            }

            // Write the image sequence files.
            image::Info imageInfo(16, 16, image::PixelType::RGB_U8);
            imageInfo.layout.endian = memory::Endian::MSB;
            const auto image = image::Image::create(imageInfo);
            io::Info ioInfo;
            ioInfo.video.push_back(imageInfo);
            ioInfo.videoTime = clipTimeRange;
            auto write = _context->getSystem<io::System>()->write(file::Path("PlayerTest.0.ppm"), ioInfo);
            for (size_t i = 0; i < static_cast<size_t>(clipTimeRange.duration().value()); ++i)
            {
                write->writeVideo(otime::RationalTime(i, 24.0), image);
            }

            // Create a timeline player from the OTIO timeline.
            auto timeline = Timeline::create(fileName, _context);
            auto player = Player::create(timeline, _context);
            TLRENDER_ASSERT(player->getTimeline());
            TLRENDER_ASSERT(fileName == player->getPath().get());
            TLRENDER_ASSERT(Options() == player->getOptions());
            const otime::TimeRange timeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(48.0, 24.0));
            TLRENDER_ASSERT(time::compareExact(timeRange, player->getTimeRange()));
            TLRENDER_ASSERT(imageInfo.size == player->getIOInfo().video[0].size);
            TLRENDER_ASSERT(imageInfo.pixelType == player->getIOInfo().video[0].pixelType);
            TLRENDER_ASSERT(timeRange.duration().rate() == player->getDefaultSpeed());

            // Test frames.
            struct FrameOptions
            {
                uint16_t layer = 0;
                PlayerCacheOptions cache;
                size_t requestCount = 16;
                size_t requestTimeout = 1;
            };
            FrameOptions frameOptions2;
            frameOptions2.layer = 1;
            frameOptions2.cache.readAhead = otime::RationalTime(1.0, 24.0);
            frameOptions2.cache.readBehind = otime::RationalTime(0.0, 1.0);
            for (const auto options : std::vector<FrameOptions>({ FrameOptions(), frameOptions2 }))
            {
                player->setCacheOptions(options.cache);
                TLRENDER_ASSERT(options.cache == player->observeCacheOptions()->get());
                auto currentVideoObserver = observer::ValueObserver<timeline::VideoData>::create(
                    player->observeCurrentVideo(),
                    [this](const timeline::VideoData& value)
                    {
                        std::stringstream ss;
                        ss << "Video time: " << value.time;
                        _print(ss.str());
                    });
                auto currentAudioObserver = observer::ListObserver<timeline::AudioData>::create(
                    player->observeCurrentAudio(),
                    [this](const std::vector<timeline::AudioData>& value)
                    {
                        for (const auto& i : value)
                        {
                            std::stringstream ss;
                            ss << "Audio time: " << i.seconds;
                            _print(ss.str());
                        }
                    });
                auto cacheInfoObserver = observer::ValueObserver<PlayerCacheInfo>::create(
                    player->observeCacheInfo(),
                    [this](const PlayerCacheInfo& value)
                    {
                        {
                            std::stringstream ss;
                            ss << "Video/audio cached frames: " << value.videoFrames.size() << "/" << value.audioFrames.size();
                            _print(ss.str());
                        }
                    });
                for (const auto& loop : getLoopEnums())
                {
                    player->setLoop(loop);
                    player->setPlayback(Playback::Forward);
                    for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
                    {
                        player->tick();
                        time::sleep(std::chrono::microseconds(1000000 / 24));
                    }
                    player->setPlayback(Playback::Reverse);
                    for (size_t i = 0; i < static_cast<size_t>(timeRange.duration().value()); ++i)
                    {
                        player->tick();
                        time::sleep(std::chrono::microseconds(1000000 / 24));
                    }
                }
                player->setPlayback(Playback::Stop);
            }

            // Test the playback speed.
            double speed = 24.0;
            auto speedObserver = observer::ValueObserver<double>::create(
                player->observeSpeed(),
                [&speed](double value)
                {
                    speed = value;
                });
            const double defaultSpeed = player->getDefaultSpeed();
            const double doubleSpeed = defaultSpeed * 2.0;
            player->setSpeed(doubleSpeed);
            TLRENDER_ASSERT(doubleSpeed == speed);
            player->setSpeed(defaultSpeed);

            // Test the playback mode.
            Playback playback = Playback::Stop;
            auto playbackObserver = observer::ValueObserver<Playback>::create(
                player->observePlayback(),
                [&playback](Playback value)
                {
                    playback = value;
                });
            player->setLoop(Loop::Loop);
            player->setPlayback(Playback::Forward);
            TLRENDER_ASSERT(Playback::Forward == playback);

            // Test the playback loop mode.
            Loop loop = Loop::Loop;
            auto loopObserver = observer::ValueObserver<Loop>::create(
                player->observeLoop(),
                [&loop](Loop value)
                {
                    loop = value;
                });
            player->setLoop(Loop::Once);
            TLRENDER_ASSERT(Loop::Once == loop);

            // Test the current time.
            player->setPlayback(Playback::Stop);
            otime::RationalTime currentTime = time::invalidTime;
            auto currentTimeObserver = observer::ValueObserver<otime::RationalTime>::create(
                player->observeCurrentTime(),
                [&currentTime](const otime::RationalTime& value)
                {
                    currentTime = value;
                });
            player->seek(otime::RationalTime(10.0, 24.0));
            TLRENDER_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            player->seek(otime::RationalTime(11.0, 24.0));
            TLRENDER_ASSERT(otime::RationalTime(11.0, 24.0) == currentTime);
            player->end();
            TLRENDER_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);
            player->start();
            TLRENDER_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            player->frameNext();
            TLRENDER_ASSERT(otime::RationalTime(11.0, 24.0) == currentTime);
            player->timeAction(TimeAction::FrameNextX10);
            TLRENDER_ASSERT(otime::RationalTime(21.0, 24.0) == currentTime);
            player->timeAction(TimeAction::FrameNextX100);
            TLRENDER_ASSERT(otime::RationalTime(10.0, 24.0) == currentTime);
            player->framePrev();
            TLRENDER_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);
            player->timeAction(TimeAction::FramePrevX10);
            TLRENDER_ASSERT(otime::RationalTime(47.0, 24.0) == currentTime);
            player->timeAction(TimeAction::FramePrevX100);
            TLRENDER_ASSERT(otime::RationalTime(57.0, 24.0) == currentTime);

            // Test the in/out points.
            otime::TimeRange inOutRange = time::invalidTimeRange;
            auto inOutRangeObserver = observer::ValueObserver<otime::TimeRange>::create(
                player->observeInOutRange(),
                [&inOutRange](const otime::TimeRange& value)
                {
                    inOutRange = value;
                });
            player->setInOutRange(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(33.0, 24.0)));
            TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(10.0, 24.0), otime::RationalTime(33.0, 24.0)) == inOutRange);
            player->seek(otime::RationalTime(12.0, 24.0));
            player->setInPoint();
            player->seek(otime::RationalTime(32.0, 24.0));
            player->setOutPoint();
            TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(12.0, 24.0), otime::RationalTime(21.0, 24.0)) == inOutRange);
            player->resetInPoint();
            player->resetOutPoint();
            TLRENDER_ASSERT(otime::TimeRange(otime::RationalTime(10.0, 24.0), timeRange.duration()) == inOutRange);
        }
    }
}
