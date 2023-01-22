// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlIO/FFmpegReadPrivate.h>

#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

extern "C"
{
#include <libavutil/opt.h>

} // extern "C"

namespace tl
{
    namespace ffmpeg
    {
        AVIOBufferData::AVIOBufferData()
        {}

        AVIOBufferData::AVIOBufferData(const uint8_t* p, size_t size) :
            p(p),
            pCurrent(p),
            size(size)
        {}

        int avIOBufferRead(void* opaque, uint8_t* buf, int bufSize)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            const size_t remaining = (bufferData->p + bufferData->size) - bufferData->pCurrent;
            bufSize = std::min(static_cast<size_t>(bufSize), remaining);
            if (!bufSize)
            {
                return AVERROR_EOF;
            }

            memcpy(buf, bufferData->pCurrent, bufSize);
            bufferData->pCurrent += bufSize;

            return bufSize;
        }

        int64_t avIOBufferSeek(void* opaque, int64_t offset, int whence)
        {
            AVIOBufferData* bufferData = static_cast<AVIOBufferData*>(opaque);

            if (whence & AVSEEK_SIZE)
            {
                return bufferData->size;
            }

            bufferData->pCurrent = bufferData->p + offset;

            return offset;
        }

        void Read::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);

            TLRENDER_P();

            auto i = options.find("ffmpeg/YUVToRGBConversion");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.yuvToRGBConversion;
            }
            i = options.find("ffmpeg/AudioChannelCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                size_t channelCount = 0;
                ss >> channelCount;
                p.options.audioConvertInfo.channelCount = std::min(channelCount, static_cast<size_t>(255));
            }
            i = options.find("ffmpeg/AudioDataType");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.dataType;
            }
            i = options.find("ffmpeg/AudioSampleRate");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioConvertInfo.sampleRate;
            }
            i = options.find("ffmpeg/ThreadCount");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.threadCount;
            }
            i = options.find("ffmpeg/RequestTimeout");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.requestTimeout;
            }
            i = options.find("ffmpeg/VideoBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.videoBufferSize;
            }
            i = options.find("ffmpeg/AudioBufferSize");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.options.audioBufferSize;
            }

            p.videoThread.running = true;
            p.videoThread.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        p.readVideo = std::make_shared<ReadVideo>(path.get(), _memory, p.options);
                        p.info.video.push_back(p.readVideo->getInfo());
                        p.info.videoTime = p.readVideo->getTimeRange();
                        p.info.tags = p.readVideo->getTags();

                        p.readAudio = std::make_shared<ReadAudio>(path.get(), _memory, p.info.videoTime.duration().rate(), p.options);
                        p.info.audio = p.readAudio->getInfo();
                        p.info.audioTime = p.readAudio->getTimeRange();
                        for (const auto& tag : p.readAudio->getTags())
                        {
                            p.info.tags[tag.first] = tag.second;
                        }

                        p.infoPromise.set_value(p.info);

                        p.audioThread.running = true;
                        p.audioThread.thread = std::thread(
                            [this, path]
                            {
                                TLRENDER_P();
                                try
                                {
                                    _audioThread();
                                }
                                catch (const std::exception& e)
                                {
                                    if (auto logSystem = _logSystem.lock())
                                    {
                                        //! \todo How should this be handled?
                                        const std::string id = string::Format("tl::io::ffmpeg::Read ({0}: {1})").
                                            arg(__FILE__).
                                            arg(__LINE__);
                                        logSystem->print(id, string::Format("{0}: {1}").
                                            arg(_path.get()).
                                            arg(e.what()),
                                            log::Type::Error);
                                    }
                                }
                            });

                        try
                        {
                            _videoThread();
                        }
                        catch (const std::exception& e)
                        {
                            if (auto logSystem = _logSystem.lock())
                            {
                                //! \todo How should this be handled?
                                const std::string id = string::Format("tl::io::ffmpeg::Read ({0}: {1})").
                                    arg(__FILE__).
                                    arg(__LINE__);
                                logSystem->print(id, string::Format("{0}: {1}").
                                    arg(_path.get()).
                                    arg(e.what()),
                                    log::Type::Error);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id = string::Format("tl::io::ffmpeg::Read ({0}: {1})").
                                arg(__FILE__).
                                arg(__LINE__);
                            logSystem->print(id, string::Format("{0}: {1}").
                                arg(_path.get()).
                                arg(e.what()),
                                log::Type::Error);
                        }
                        p.infoPromise.set_value(io::Info());
                    }

                    {
                        std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                        p.videoMutex.stopped = true;
                    }
                    _cancelVideoRequests();
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        p.audioMutex.stopped = true;
                    }
                    _cancelAudioRequests();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            TLRENDER_P();
            p.videoThread.running = false;
            p.audioThread.running = false;
            if (p.videoThread.thread.joinable())
            {
                p.videoThread.thread.join();
            }
            if (p.audioThread.thread.joinable())
            {
                p.audioThread.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, logSystem);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            return _p->infoPromise.get_future();
        }

        std::future<io::VideoData> Read::readVideo(
            const otime::RationalTime& time,
            uint16_t)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                if (!p.videoMutex.stopped)
                {
                    valid = true;
                    p.videoMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.videoThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }

        std::future<io::AudioData> Read::readAudio(const otime::TimeRange& time)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->time = time;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                if (!p.audioMutex.stopped)
                {
                    valid = true;
                    p.audioMutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.audioThread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::AudioData());
            }
            return future;
        }

        void Read::cancelRequests()
        {
            _cancelVideoRequests();
            _cancelAudioRequests();
        }

        void Read::_videoThread()
        {
            TLRENDER_P();
            p.videoThread.currentTime = p.info.videoTime.start_time();
            p.readVideo->start();
            p.videoThread.logTimer = std::chrono::steady_clock::now();
            while (p.videoThread.running)
            {
                // Check requests.
                {
                    std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                    if (p.videoThread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(p.options.requestTimeout),
                        [this]
                        {
                            return
                                !_p->videoMutex.requests.empty() ||
                                _p->videoMutex.currentRequest ||
                                !_p->readVideo->isBufferFull();
                        }))
                    {
                        if (!p.videoMutex.currentRequest && !p.videoMutex.requests.empty())
                        {
                            p.videoMutex.currentRequest = p.videoMutex.requests.front();
                            p.videoMutex.requests.pop_front();
                        }
                    }
                }

                // Seek.
                {
                    bool seek = false;
                    {
                        std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                        if (p.videoMutex.currentRequest)
                        {
                            if (!time::compareExact(
                                p.videoMutex.currentRequest->time,
                                p.videoThread.currentTime))
                            {
                                seek = true;
                                p.videoThread.currentTime = p.videoMutex.currentRequest->time;
                            }
                        }
                    }
                    if (seek)
                    {
                        p.readVideo->seek(p.videoThread.currentTime);
                    }
                }

                // Process.
                _p->readVideo->process(p.videoThread.currentTime);

                // Handle requests.
                {
                    std::shared_ptr<Private::VideoRequest> request;
                    {
                        std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                        if ((p.videoMutex.currentRequest && !p.readVideo->isBufferEmpty()) ||
                            (p.videoMutex.currentRequest && !p.readVideo->isValid()) ||
                            (p.videoMutex.currentRequest && p.readVideo->isEOF()))
                        {
                            request = std::move(p.videoMutex.currentRequest);
                        }
                    }
                    if (request)
                    {
                        io::VideoData data;
                        data.time = request->time;
                        if (!p.readVideo->isBufferEmpty())
                        {
                            data.image = p.readVideo->popBuffer();
                        }
                        request->promise.set_value(data);

                        p.videoThread.currentTime += otime::RationalTime(1.0, p.info.videoTime.duration().rate());
                    }
                }

                // Logging.
                if (auto logSystem = _logSystem.lock())
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.videoThread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.videoThread.logTimer = now;
                        const std::string id = string::Format("tl::io::ffmpeg::Read {0}").arg(this);
                        size_t requestsSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                            requestsSize = p.videoMutex.requests.size();
                        }
                        logSystem->print(id, string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Video requests: {1}").
                            arg(_path.get()).
                            arg(requestsSize));
                    }
                }
            }
        }

        void Read::_audioThread()
        {
            TLRENDER_P();
            p.audioThread.currentTime = p.info.audioTime.start_time();
            p.readAudio->start();
            p.audioThread.logTimer = std::chrono::steady_clock::now();
            while (p.audioThread.running)
            {
                // Check requests.
                {
                    std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                    if (p.audioThread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(p.options.requestTimeout),
                        [this]
                        {
                            return
                                !_p->audioMutex.requests.empty() ||
                                _p->audioMutex.currentRequest ||
                                !_p->readAudio->isBufferFull();
                        }))
                    {
                        if (!p.audioMutex.currentRequest && !p.audioMutex.requests.empty())
                        {
                            p.audioMutex.currentRequest = p.audioMutex.requests.front();
                            p.audioMutex.requests.pop_front();
                        }
                    }
                }

                // Seek.
                {
                    bool seek = false;
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        if (p.audioMutex.currentRequest)
                        {
                            if (!time::compareExact(
                                p.audioMutex.currentRequest->time.start_time(),
                                p.audioThread.currentTime))
                            {
                                seek = true;
                                p.audioThread.currentTime = p.audioMutex.currentRequest->time.start_time();
                            }
                        }
                    }
                    if (seek)
                    {
                        p.readAudio->seek(p.audioThread.currentTime);
                    }
                }

                // Process.
                _p->readAudio->process(p.audioThread.currentTime);

                // Handle requests.
                {
                    const size_t bufferSize = p.readAudio->getBufferSize();
                    std::shared_ptr<Private::AudioRequest> request;
                    {
                        std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                        if ((p.audioMutex.currentRequest &&
                            p.audioMutex.currentRequest->time.duration().rescaled_to(p.info.audio.sampleRate).value() <= bufferSize) ||
                            (p.audioMutex.currentRequest && !p.readAudio->isValid()) ||
                            (p.audioMutex.currentRequest && p.readAudio->isEOF()))
                        {
                            request = std::move(p.audioMutex.currentRequest);
                        }
                    }
                    if (request)
                    {
                        io::AudioData data;
                        data.time = request->time.start_time();
                        data.audio = audio::Audio::create(p.info.audio, request->time.duration().value());
                        data.audio->zero();
                        size_t offset = 0;
                        if (data.time < p.info.audioTime.start_time())
                        {
                            offset = (p.info.audioTime.start_time() - data.time).value() * p.info.audio.getByteCount();
                        }
                        p.readAudio->bufferCopy(data.audio->getData() + offset, data.audio->getByteCount() - offset);
                        request->promise.set_value(data);

                        p.audioThread.currentTime += request->time.duration();
                    }
                }

                // Logging.
                if (auto logSystem = _logSystem.lock())
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.audioThread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.audioThread.logTimer = now;
                        const std::string id = string::Format("tl::io::ffmpeg::Read {0}").arg(this);
                        size_t requestsSize = 0;
                        {
                            std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                            requestsSize = p.audioMutex.requests.size();
                        }
                        logSystem->print(id, string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Audio requests: {1}").
                            arg(_path.get()).
                            arg(requestsSize));
                    }
                }
            }
        }

        void Read::_cancelVideoRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::VideoRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                requests = std::move(p.videoMutex.requests);
                if (p.videoMutex.currentRequest)
                {
                    requests.push_front(p.videoMutex.currentRequest);
                    p.videoMutex.currentRequest.reset();
                }
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::VideoData());
            }
        }

        void Read::_cancelAudioRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::AudioRequest> > requests;
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                requests = std::move(p.audioMutex.requests);
                if (p.audioMutex.currentRequest)
                {
                    requests.push_front(p.audioMutex.currentRequest);
                    p.audioMutex.currentRequest.reset();
                }
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::AudioData());
            }
        }
    }
}
