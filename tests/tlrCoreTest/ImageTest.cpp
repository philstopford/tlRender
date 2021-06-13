// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/ImageTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Image.h>

using namespace tlr::imaging;

namespace tlr
{
    namespace CoreTest
    {
        ImageTest::ImageTest() :
            ITest("CoreTest::ImageTest")
        {}

        std::shared_ptr<ImageTest> ImageTest::create()
        {
            return std::shared_ptr<ImageTest>(new ImageTest);
        }

        void ImageTest::run()
        {
            _size();
            _enum();
            _util();
            _info();
            _image();
        }
        
        void ImageTest::_size()
        {
            {
                const Size size;
                TLR_ASSERT(0 == size.w);
                TLR_ASSERT(0 == size.h);
                TLR_ASSERT(!size.isValid());
                TLR_ASSERT(0.F == size.getAspect());
            }
            {
                const Size size(1, 2);
                TLR_ASSERT(1 == size.w);
                TLR_ASSERT(2 == size.h);
                TLR_ASSERT(size.isValid());
                TLR_ASSERT(.5F == size.getAspect());
            }
            {
                TLR_ASSERT(Size(1, 2) == Size(1, 2));
                TLR_ASSERT(Size(1, 2) != Size(1, 3));
                TLR_ASSERT(Size(1, 2) < Size(1, 3));
            }
            {
                const Size size(1, 2);
                std::stringstream ss;
                ss << size;
                Size size2;
                ss >> size2;
                TLR_ASSERT(size == size2);
            }
        }
        
        void ImageTest::_enum()
        {
            ITest::_enum<PixelType>("PixelType", getPixelTypeEnums);
        }

        void ImageTest::_info()
        {
            {
                const Info info;
                TLR_ASSERT(Size() == info.size);
                TLR_ASSERT(PixelType::None == info.pixelType);
                TLR_ASSERT(!info.isValid());
            }
            {
                const Info info(Size(1, 2), PixelType::L_U8);
                TLR_ASSERT(Size(1, 2) == info.size);
                TLR_ASSERT(PixelType::L_U8 == info.pixelType);
                TLR_ASSERT(info.isValid());
            }
            {
                const Info info(1, 2, PixelType::L_U8);
                TLR_ASSERT(Size(1, 2) == info.size);
                TLR_ASSERT(PixelType::L_U8 == info.pixelType);
                TLR_ASSERT(info.isValid());
            }
            {
                TLR_ASSERT(Info(1, 2, PixelType::L_U8) == Info(1, 2, PixelType::L_U8));
                TLR_ASSERT(Info(1, 2, PixelType::L_U8) != Info(1, 2, PixelType::L_U16));
            }
        }
        
        void ImageTest::_util()
        {
            for (auto i : getPixelTypeEnums())
            {
                std::stringstream ss;
                ss << i << " channel count: " << int(getChannelCount(i));
                _print(ss.str());
            }
            for (auto i : getPixelTypeEnums())
            {
                std::stringstream ss;
                ss << i << " bit depth: " << int(getBitDepth(i));
                _print(ss.str());
            }
            for (size_t c : { 1, 2, 3, 4 })
            {
                for (size_t b : { 8, 16 })
                {
                    std::stringstream ss;
                    ss << c << "/" << b << " int type: " << getIntType(c, b);
                    _print(ss.str());
                }
            }
            for (size_t c : { 1, 2, 3, 4 })
            {
                for (size_t b : { 16, 32 })
                {
                    std::stringstream ss;
                    ss << c << "/" << b << " float type: " << getFloatType(c, b);
                    _print(ss.str());
                }
            }
            for (const auto& i :
                {
                    Info(1, 2, PixelType::L_U8),
                    Info(1, 2, PixelType::L_U16)
                })
            {
                std::stringstream ss;
                ss << i << " data byte count: " << getDataByteCount(i);
                _print(ss.str());
            }
        }
        
        void ImageTest::_image()
        {
            {
                const Info info(1, 2, PixelType::L_U8);
                auto image = Image::create(info);
                image->zero();
                TLR_ASSERT(image->getInfo() == info);
                TLR_ASSERT(image->getSize() == info.size);
                TLR_ASSERT(image->getWidth() == info.size.w);
                TLR_ASSERT(image->getHeight() == info.size.h);
                TLR_ASSERT(image->getAspect() == .5F);
                TLR_ASSERT(image->getPixelType() == info.pixelType);
                TLR_ASSERT(image->isValid());
                TLR_ASSERT(image->getData());
                TLR_ASSERT(static_cast<const imaging::Image*>(image.get())->getData());
            }
        }
    }
}
