// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlIO/SequenceIOReadPrivate.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#include <fseq.h>

#include <cstring>
#include <sstream>

namespace tl
{
    namespace io
    {
        void ISequenceRead::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);

            TLRENDER_P();

            const std::string& number = path.getNumber();
            if (!number.empty())
            {
                if (!_memory.empty())
                {
                    std::stringstream ss(number);
                    ss >> _startFrame;
                    _endFrame = _startFrame + _memory.size() - 1;
                }
                else
                {
                    std::string directory = path.getDirectory();
                    if (directory.empty())
                    {
                        directory = ".";
                    }
                    const std::string& baseName = path.getBaseName();
                    const std::string& extension = path.getExtension();
                    FSeqDirOptions dirOptions;
                    fseqDirOptionsInit(&dirOptions);
                    dirOptions.sequence = FSEQ_TRUE;
                    FSeqBool error = FSEQ_FALSE;
                    auto dirList = fseqDirList(directory.c_str(), &dirOptions, &error);
                    if (FSEQ_FALSE == error)
                    {
                        for (auto entry = dirList; entry; entry = entry->next)
                        {
                            if (strlen(entry->fileName.number) > 0 &&
                                0 == strcmp(entry->fileName.base, baseName.c_str()) &&
                                0 == strcmp(entry->fileName.extension, extension.c_str()))
                            {
                                _startFrame = entry->frameMin;
                                _endFrame = entry->frameMax;
                                break;
                            }
                        }
                    }
                    fseqDirListDel(dirList);
                }
            }

            auto i = options.find("SequenceIO/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.threadCount;
            }
            i = options.find("SequenceIO/DefaultSpeed");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> _defaultSpeed;
            }

            p.thread.running = true;
            p.thread.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        p.info = _getInfo(
                            path.get(), 
                            !_memory.empty() ? &_memory[0] : nullptr);
                        p.addTags(p.info);
                        _thread();
                    }
                    catch (const std::exception& e)
                    {
                        //! \todo How should this be handled?
                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id = string::Format("tl::io::ISequenceRead ({0}: {1})").
                                arg(__FILE__).
                                arg(__LINE__);
                            logSystem->print(id, string::Format("{0}: {1}").
                                arg(_path.get()).
                                arg(e.what()),
                                log::Type::Error);
                        }
                    }
                    _finishRequests();
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    _cancelRequests();
                });
        }

        ISequenceRead::ISequenceRead() :
            _p(new Private)
        {}

        ISequenceRead::~ISequenceRead()
        {}

        std::future<Info> ISequenceRead::getInfo()
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::InfoRequest>();
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(Info());
            }
            return future;
        }

        std::future<VideoData> ISequenceRead::readVideo(
            const otime::RationalTime& time,
            uint16_t layer)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            request->layer = layer;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(VideoData());
            }
            return future;
        }

        void ISequenceRead::cancelRequests()
        {
            _cancelRequests();
        }

        void ISequenceRead::_finish()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        void ISequenceRead::_thread()
        {
            TLRENDER_P();
            p.thread.logTimer = std::chrono::steady_clock::now();
            while (p.thread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        sequenceRequestTimeout,
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.videoRequests.empty() ||
                                !_p->thread.videoRequestsInProgress.empty();
                        }))
                    {
                        infoRequests = std::move(p.mutex.infoRequests);
                        while (!p.mutex.videoRequests.empty() &&
                            (p.thread.videoRequestsInProgress.size() + videoRequests.size()) < p.threadCount)
                        {
                            videoRequests.push_back(p.mutex.videoRequests.front());
                            p.mutex.videoRequests.pop_front();
                        }
                    }
                }

                // Information rquests.
                for (const auto& request : infoRequests)
                {
                    request->promise.set_value(p.info);
                }

                // Initialize video requests.
                while (!videoRequests.empty())
                {
                    auto request = videoRequests.front();
                    videoRequests.pop_front();

                    //std::cout << "request: " << request.time << std::endl;
                    bool seq = false;
                    if (!_path.getNumber().empty())
                    {
                        seq = true;
                        request->fileName = _path.get(static_cast<int>(request->time.value()));
                    }
                    else
                    {
                        request->fileName = _path.get();
                    }
                    const std::string fileName = request->fileName;
                    const otime::RationalTime time = request->time;
                    const uint16_t layer = request->layer;
                    request->future = std::async(
                        std::launch::async,
                        [this, seq, fileName, time, layer]
                        {
                            VideoData out;
                            try
                            {
                                const int64_t frame = time.value();
                                if (!seq || (seq && frame >= _startFrame && frame <= _endFrame))
                                {
                                    const int64_t memoryIndex = seq ? (frame - _startFrame) : 0;
                                    out = _readVideo(
                                        fileName,
                                        memoryIndex >= 0 && memoryIndex < _memory.size() ? &_memory[memoryIndex] : nullptr,
                                        time,
                                        layer);
                                }
                            }
                            catch (const std::exception&)
                            {
                                //! \todo How should this be handled?
                            }
                            return out;
                        });
                    p.thread.videoRequestsInProgress.push_back(request);
                }

                // Check for finished video requests.
                //if (!p.thread.videoRequestsInProgress.empty())
                //{
                //    std::cout << "in progress: " << p.thread.videoRequestsInProgress.size() << std::endl;
                //}
                auto requestIt = p.thread.videoRequestsInProgress.begin();
                while (requestIt != p.thread.videoRequestsInProgress.end())
                {
                    if ((*requestIt)->future.valid() &&
                        (*requestIt)->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    {
                        //std::cout << "finished: " << requestIt->time << std::endl;
                        auto data = (*requestIt)->future.get();
                        (*requestIt)->promise.set_value(data);
                        requestIt = p.thread.videoRequestsInProgress.erase(requestIt);
                        continue;
                    }
                    ++requestIt;
                }

                // Logging.
                if (auto logSystem = _logSystem.lock())
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.thread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.thread.logTimer = now;
                        const std::string id = string::Format("tl::io::ISequenceRead {0}").arg(this);
                        size_t requestsSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.mutex.mutex);
                            requestsSize = p.mutex.videoRequests.size();
                        }
                        logSystem->print(id, string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Requests: {1}, {2} in progress\n"
                            "    Thread count: {3}").
                            arg(_path.get()).
                            arg(requestsSize).
                            arg(p.thread.videoRequestsInProgress.size()).
                            arg(p.threadCount));
                    }
                }
            }
        }

        void ISequenceRead::_finishRequests()
        {
            TLRENDER_P();
            for (auto& request : p.thread.videoRequestsInProgress)
            {
                VideoData data;
                data.time = request->time;
                if (request->future.valid())
                {
                    data = request->future.get();
                }
                request->promise.set_value(data);
            }
            p.thread.videoRequestsInProgress.clear();
        }

        void ISequenceRead::_cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::VideoRequest> > videoRequests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                videoRequests = std::move(p.mutex.videoRequests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(Info());
            }
            for (auto& request : videoRequests)
            {
                request->promise.set_value(VideoData());
            }
        }

        void ISequenceRead::Private::addTags(Info& info)
        {
            if (!info.video.empty())
            {
                {
                    std::stringstream ss;
                    ss << info.video[0].size.w << " " << info.video[0].size.h;
                    info.tags["Video Resolution"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << info.video[0].size.pixelAspectRatio;
                    info.tags["Video Pixel Aspect Ratio"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.video[0].pixelType;
                    info.tags["Video Pixel Type"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.video[0].videoLevels;
                    info.tags["Video Levels"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.videoTime.start_time().to_timecode();
                    info.tags["Video Start Time"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.videoTime.duration().to_timecode();
                    info.tags["Video Duration"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << info.videoTime.start_time().rate() << " FPS";
                    info.tags["Video Speed"] = ss.str();
                }
            }
        }
    }
}
