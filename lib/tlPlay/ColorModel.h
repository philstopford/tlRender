// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ColorConfigOptions.h>
#include <tlTimeline/DisplayOptions.h>
#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/LUTOptions.h>

#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        //! Color model.
        class ColorModel : public std::enable_shared_from_this<ColorModel>
        {
            TLRENDER_NON_COPYABLE(ColorModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            ColorModel();

        public:
            ~ColorModel();

            //! Create a new model.
            static std::shared_ptr<ColorModel> create(const std::shared_ptr<system::Context>&);

            //! Get the color configuration options.
            const timeline::ColorConfigOptions& getColorConfigOptions() const;

            //! Observe the color configuration options.
            std::shared_ptr<observer::IValue<timeline::ColorConfigOptions> > observeColorConfigOptions() const;

            //! Set the color configuration options.
            void setColorConfigOptions(const timeline::ColorConfigOptions&);

            //! Get the LUT options.
            const timeline::LUTOptions& getLUTOptions() const;

            //! Observe the LUT options.
            std::shared_ptr<observer::IValue<timeline::LUTOptions> > observeLUTOptions() const;

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Get the image options.
            const timeline::ImageOptions& getImageOptions() const;

            //! Observe the image options.
            std::shared_ptr<observer::IValue<timeline::ImageOptions> > observeImageOptions() const;

            //! Set the image options.
            void setImageOptions(const timeline::ImageOptions&);

            //! Get the display options.
            const timeline::DisplayOptions& getDisplayOptions() const;

            //! Observe the display options.
            std::shared_ptr<observer::IValue<timeline::DisplayOptions> > observeDisplayOptions() const;

            //! Set the display options.
            void setDisplayOptions(const timeline::DisplayOptions&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
