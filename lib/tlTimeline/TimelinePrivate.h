// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/Timeline.h>

#include <opentimelineio/clip.h>

#include <atomic>
#include <list>
#include <mutex>
#include <thread>

namespace tl
{
    namespace timeline
    {
        struct Timeline::Private
        {
            bool getVideoInfo(const otio::Composable*);
            bool getAudioInfo(const otio::Composable*);

            float transitionValue(double frame, double in, double out) const;

            void tick();
            void requests();
            void finishRequests();

            ReadCacheItem getRead(
                const otio::Clip*,
                const io::Options&);
            std::future<io::VideoData> readVideo(
                const otio::Track*,
                const otio::Clip*,
                const otime::RationalTime&,
                uint16_t videoLayer);
            std::future<io::AudioData> readAudio(
                const otio::Track*,
                const otio::Clip*,
                const otime::TimeRange&);

            void trimAudio(
                const std::shared_ptr<audio::Audio>&,
                int64_t seconds,
                const otime::TimeRange&);

            std::weak_ptr<system::Context> context;
            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
            std::shared_ptr<observer::Value<bool> > timelineChanges;
            file::Path path;
            file::Path audioPath;
            Options options;
            std::shared_ptr<ReadCache> readCache;
            otime::TimeRange timeRange = time::invalidTimeRange;
            io::Info ioInfo;

            struct VideoLayerData
            {
                VideoLayerData() {};
                VideoLayerData(VideoLayerData&&) = default;

                std::future<io::VideoData> image;
                std::future<io::VideoData> imageB;
                Transition transition = Transition::None;
                float transitionValue = 0.F;
            };
            struct VideoRequest
            {
                VideoRequest() {};
                VideoRequest(VideoRequest&&) = default;

                otime::RationalTime time = time::invalidTime;
                uint16_t videoLayer = 0;
                std::promise<VideoData> promise;

                std::vector<VideoLayerData> layerData;
            };

            struct AudioLayerData
            {
                AudioLayerData() {};
                AudioLayerData(AudioLayerData&&) = default;

                int64_t seconds = -1;
                otime::TimeRange timeRange;
                std::future<io::AudioData> audio;
            };
            struct AudioRequest
            {
                AudioRequest() {};
                AudioRequest(AudioRequest&&) = default;

                int64_t seconds = -1;
                std::promise<AudioData> promise;

                std::vector<AudioLayerData> layerData;
            };

            struct Mutex
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
                bool otioTimelineChanged = false;
                std::list<std::shared_ptr<VideoRequest> > videoRequests;
                std::list<std::shared_ptr<AudioRequest> > audioRequests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;
            struct Thread
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;
                std::list<std::shared_ptr<VideoRequest> > videoRequestsInProgress;
                std::list<std::shared_ptr<AudioRequest> > audioRequestsInProgress;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
                std::chrono::steady_clock::time_point logTimer;
            };
            Thread thread;
        };
    }
}
