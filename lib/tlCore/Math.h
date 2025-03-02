// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

namespace tl
{
    //! Math.
    namespace math
    {
        //! Approximate value of PI.
        constexpr float pi = 3.14159265359F;

        //! Approximate value of PI times two.
        constexpr float pi2 = pi * 2.F;

        //! Convert degress to radians.
        constexpr float deg2rad(float) noexcept;

        //! Convert radians to degress.
        constexpr float rad2deg(float) noexcept;

        //! Clamp a value.
        template<typename T>
        constexpr T clamp(T value, T min, T max);

        //! Linear interpolation.
        template<typename T, typename U>
        constexpr T lerp(U value, T min, T max) noexcept;

        //! Smooth step function.
        template<typename T>
        constexpr T smoothStep(T value, T min, T max) noexcept;

        //! Count the number of digits.
        size_t digits(int) noexcept;

        //! Fuzzy double comparison.
        bool fuzzyCompare(double a, double b, double e = .1e-9) noexcept;

        //! Fuzzy float comparison.
        bool fuzzyCompare(float a, float b, float e = .1e-6F) noexcept;
    }
}

#include <tlCore/MathInline.h>
