// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace tl
{
    namespace timeline
    {
        //! Input video levels.
        enum class InputVideoLevels
        {
            FromFile,
            FullRange,
            LegalRange,

            Count,
            First = FromFile
        };
        TLRENDER_ENUM(InputVideoLevels);
        TLRENDER_ENUM_SERIALIZE(InputVideoLevels);

        //! Alpha channel blending.
        //!
        //! References:
        //! - https://microsoft.github.io/Win2D/html/PremultipliedAlpha.htm
        enum class AlphaBlend
        {
            None,
            Straight,
            Premultiplied,

            Count,
            First = None
        };
        TLRENDER_ENUM(AlphaBlend);
        TLRENDER_ENUM_SERIALIZE(AlphaBlend);

        //! Image filtering.
        enum class ImageFilter
        {
            Nearest,
            Linear,

            Count,
            First = Nearest
        };
        TLRENDER_ENUM(ImageFilter);
        TLRENDER_ENUM_SERIALIZE(ImageFilter);

        //! Image filters.
        struct ImageFilters
        {
            ImageFilter minify = ImageFilter::Linear;
            ImageFilter magnify = ImageFilter::Linear;

            bool operator == (const ImageFilters&) const;
            bool operator != (const ImageFilters&) const;
        };

        //! Image options.
        struct ImageOptions
        {
            InputVideoLevels videoLevels = InputVideoLevels::FromFile;
            AlphaBlend       alphaBlend = AlphaBlend::Straight;
            ImageFilters     imageFilters;

            bool operator == (const ImageOptions&) const;
            bool operator != (const ImageOptions&) const;
        };
    }
}

#include <tlTimeline/ImageOptionsInline.h>
