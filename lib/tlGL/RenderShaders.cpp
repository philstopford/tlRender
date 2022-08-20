// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlGL/RenderPrivate.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace gl
    {
        std::string colorFunctionName() { return "OCIODisplay"; }

        std::string colorFunctionNoOp()
        {
            return
                "vec4 OCIODisplay(vec4 inPixel)\n"
                "{\n"
                "    return inPixel;\n"
                "}\n";
        }

        std::string vertexSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec3 vPos;\n"
                "in vec2 vTexture;\n"
                "out vec2 fTexture;\n"
                "\n"
                "struct Transform\n"
                "{\n"
                "    mat4 mvp;\n"
                "};\n"
                "\n"
                "uniform Transform transform;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                "    fTexture = vTexture;\n"
                "}\n";
        }

        std::string meshFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 fColor;\n"
                "\n"
                "uniform vec4 color;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    fColor = color;\n"
                "}\n";
        }

        std::string textFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 fColor;\n"
                "\n"
                "uniform vec4 color;\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    fColor.r = color.r;\n"
                "    fColor.g = color.g;\n"
                "    fColor.b = color.b;\n"
                "    fColor.a = texture(textureSampler, fTexture).r;\n"
                "}\n";
        }

        std::string textureFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 fColor;\n"
                "\n"
                "uniform vec4 color;\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    fColor = texture(textureSampler, fTexture) * color;\n"
                "}\n";
        }

        namespace
        {
            const std::string pixelType =
                "// enum tl::imaging::PixelType\n"
                "const uint PixelType_None         = 0;\n"
                "const uint PixelType_L_U8         = 1;\n"
                "const uint PixelType_L_U16        = 2;\n"
                "const uint PixelType_L_U32        = 3;\n"
                "const uint PixelType_L_F16        = 4;\n"
                "const uint PixelType_L_F32        = 5;\n"
                "const uint PixelType_LA_U8        = 6;\n"
                "const uint PixelType_LA_U32       = 7;\n"
                "const uint PixelType_LA_U16       = 8;\n"
                "const uint PixelType_LA_F16       = 9;\n"
                "const uint PixelType_LA_F32       = 10;\n"
                "const uint PixelType_RGB_U8       = 11;\n"
                "const uint PixelType_RGB_U10      = 12;\n"
                "const uint PixelType_RGB_U16      = 13;\n"
                "const uint PixelType_RGB_U32      = 14;\n"
                "const uint PixelType_RGB_F16      = 15;\n"
                "const uint PixelType_RGB_F32      = 16;\n"
                "const uint PixelType_RGBA_U8      = 17;\n"
                "const uint PixelType_RGBA_U16     = 18;\n"
                "const uint PixelType_RGBA_U32     = 19;\n"
                "const uint PixelType_RGBA_F16     = 20;\n"
                "const uint PixelType_RGBA_F32     = 21;\n"
                "const uint PixelType_YUV_420P_U8  = 22;\n"
                "const uint PixelType_YUV_422P_U8  = 23;\n"
                "const uint PixelType_YUV_444P_U8  = 24;\n"
                "const uint PixelType_YUV_420P_U16 = 25;\n"
                "const uint PixelType_YUV_422P_U16 = 26;\n"
                "const uint PixelType_YUV_444P_U16 = 27;\n";

            const std::string videoLevels =
                "// enum tl::imaging::VideoLevels\n"
                "const uint VideoLevels_FullRange  = 0;\n"
                "const uint VideoLevels_LegalRange = 1;\n";

            const std::string sampleTexture =
                "vec4 sampleTexture("
                "    vec2 textureCoord,\n"
                "    int pixelType,\n"
                "    int videoLevels,\n"
                "    vec4 yuvCoefficients,\n"
                "    int imageChannels,\n"
                "    sampler2D s0,\n"
                "    sampler2D s1,\n"
                "    sampler2D s2)\n"
                "{\n"
                "    vec4 c;\n"
                "    if (PixelType_YUV_420P_U8 == pixelType ||\n"
                "        PixelType_YUV_422P_U8 == pixelType ||\n"
                "        PixelType_YUV_444P_U8 == pixelType ||\n"
                "        PixelType_YUV_420P_U16 == pixelType ||\n"
                "        PixelType_YUV_422P_U16 == pixelType ||\n"
                "        PixelType_YUV_444P_U16 == pixelType)\n"
                "    {\n"
                "        if (VideoLevels_FullRange == videoLevels)\n"
                "        {\n"
                "            float y  = texture(s0, textureCoord).r;\n"
                "            float cb = texture(s1, textureCoord).r - 0.5;\n"
                "            float cr = texture(s2, textureCoord).r - 0.5;\n"
                "            c.r = y + (yuvCoefficients.x * cr);\n"
                "            c.g = y - (yuvCoefficients.z * cb) - (yuvCoefficients.w * cr);\n"
                "            c.b = y + (yuvCoefficients.y * cb);\n"
                "        }\n"
                "        else if (VideoLevels_LegalRange == videoLevels)\n"
                "        {\n"
                "            float y  = (texture(s0, textureCoord).r - (16.0 / 255.0)) * (255.0 / (235.0 - 16.0));\n"
                "            float cb = (texture(s1, textureCoord).r - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0)) - 0.5;\n"
                "            float cr = (texture(s2, textureCoord).r - (16.0 / 255.0)) * (255.0 / (240.0 - 16.0)) - 0.5;\n"
                "            c.r = y + (yuvCoefficients.x * cr);\n"
                "            c.g = y - (yuvCoefficients.z * cb) - (yuvCoefficients.w * cr);\n"
                "            c.b = y + (yuvCoefficients.y * cb);\n"
                "        }\n"
                "        c.a = 1.0;\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        c = texture(s0, textureCoord);\n"
                "\n"
                "        // Swizzle for the image channels.\n"
                "        if (1 == imageChannels)\n"
                "        {\n"
                "            c.g = c.b = c.r;\n"
                "            c.a = 1.0;\n"
                "        }\n"
                "        else if (2 == imageChannels)\n"
                "        {\n"
                "            c.a = c.g;\n"
                "            c.g = c.b = c.r;\n"
                "        }\n"
                "        else if (3 == imageChannels)\n"
                "        {\n"
                "            c.a = 1.0;\n"
                "        }\n"
                "    }\n"
                "    return c;\n"
                "}\n";
        }

        std::string imageFragmentSource()
        {
            return string::Format(
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 fColor;\n"
                "\n"
                "{0}\n"
                "\n"
                "{1}\n"
                "\n"
                "{2}\n"
                "\n"
                "uniform vec4      color;\n"
                "uniform int       pixelType;\n"
                "uniform int       videoLevels;\n"
                "uniform vec4      yuvCoefficients;\n"
                "uniform int       imageChannels;\n"
                "uniform int       flipX;\n"
                "uniform int       flipY;\n"
                "uniform sampler2D textureSampler0;\n"
                "uniform sampler2D textureSampler1;\n"
                "uniform sampler2D textureSampler2;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec2 t = fTexture;\n"
                "    if (1 == flipX)\n"
                "    {\n"
                "        t.x = 1.0 - t.x;\n"
                "    }\n"
                "    if (1 == flipY)\n"
                "    {\n"
                "        t.y = 1.0 - t.y;\n"
                "    }\n"
                "    fColor = sampleTexture("
                "        t,\n"
                "        pixelType,\n"
                "        videoLevels,\n"
                "        yuvCoefficients,\n"
                "        imageChannels,\n"
                "        textureSampler0,\n"
                "        textureSampler1,\n"
                "        textureSampler2) *\n"
                "        color;\n"
                "}\n").
                arg(pixelType).
                arg(videoLevels).
                arg(sampleTexture);
        }

        std::string displayFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 fColor;\n"
                "\n"
                "// enum tl::render::Channels\n"
                "const uint Channels_Color = 0;\n"
                "const uint Channels_Red   = 1;\n"
                "const uint Channels_Green = 2;\n"
                "const uint Channels_Blue  = 3;\n"
                "const uint Channels_Alpha = 4;\n"
                "\n"
                "struct Levels\n"
                "{\n"
                "    float inLow;\n"
                "    float inHigh;\n"
                "    float gamma;\n"
                "    float outLow;\n"
                "    float outHigh;\n"
                "};\n"
                "\n"
                "struct Exposure\n"
                "{\n"
                "    float v;\n"
                "    float d;\n"
                "    float k;\n"
                "    float f;\n"
                "    float g;\n"
                "};\n"
                "\n"
                "// enum tl::render::OutputRange\n"
                "const uint OutputRange_Full  = 0;\n"
                "const uint OutputRange_Video = 1;\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "uniform int      channels;\n"
                "uniform int      mirrorX;\n"
                "uniform int      mirrorY;\n"
                "uniform bool     colorEnabled;\n"
                "uniform vec3     colorAdd;\n"
                "uniform mat4     colorMatrix;\n"
                "uniform bool     colorInvert;\n"
                "uniform bool     levelsEnabled;\n"
                "uniform Levels   levels;\n"
                "uniform bool     exposureEnabled;\n"
                "uniform Exposure exposure;\n"
                "uniform float    softClip;\n"
                "uniform int      outputRange;\n"
                "\n"
                "vec4 colorFunc(vec4 value, vec3 add, mat4 m)\n"
                "{\n"
                "    vec4 tmp;\n"
                "    tmp[0] = value[0] + add[0];\n"
                "    tmp[1] = value[1] + add[1];\n"
                "    tmp[2] = value[2] + add[2];\n"
                "    tmp[3] = 1.0;\n"
                "    tmp *= m;\n"
                "    tmp[3] = value[3];\n"
                "    return tmp;\n"
                "}\n"
                "\n"
                "vec4 levelsFunc(vec4 value, Levels data)\n"
                "{\n"
                "    vec4 tmp;\n"
                "    tmp[0] = (value[0] - data.inLow) / data.inHigh;\n"
                "    tmp[1] = (value[1] - data.inLow) / data.inHigh;\n"
                "    tmp[2] = (value[2] - data.inLow) / data.inHigh;\n"
                "    if (tmp[0] >= 0.0)\n"
                "        tmp[0] = pow(tmp[0], data.gamma);\n"
                "    if (tmp[1] >= 0.0)\n"
                "        tmp[1] = pow(tmp[1], data.gamma);\n"
                "    if (tmp[2] >= 0.0)\n"
                "        tmp[2] = pow(tmp[2], data.gamma);\n"
                "    value[0] = tmp[0] * data.outHigh + data.outLow;\n"
                "    value[1] = tmp[1] * data.outHigh + data.outLow;\n"
                "    value[2] = tmp[2] * data.outHigh + data.outLow;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "vec4 softClipFunc(vec4 value, float softClip)\n"
                "{\n"
                "    float tmp = 1.0 - softClip;\n"
                "    if (value[0] > tmp)\n"
                "        value[0] = tmp + (1.0 - exp(-(value[0] - tmp) / softClip)) * softClip;\n"
                "    if (value[1] > tmp)\n"
                "        value[1] = tmp + (1.0 - exp(-(value[1] - tmp) / softClip)) * softClip;\n"
                "    if (value[2] > tmp)\n"
                "        value[2] = tmp + (1.0 - exp(-(value[2] - tmp) / softClip)) * softClip;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "float knee(float value, float f)\n"
                "{\n"
                "    return log(value * f + 1.0) / f;\n"
                "}\n"
                "\n"
                "vec4 exposureFunc(vec4 value, Exposure data)\n"
                "{\n"
                "    value[0] = max(0.0, value[0] - data.d) * data.v;\n"
                "    value[1] = max(0.0, value[1] - data.d) * data.v;\n"
                "    value[2] = max(0.0, value[2] - data.d) * data.v;\n"
                "    if (value[0] > data.k)\n"
                "        value[0] = data.k + knee(value[0] - data.k, data.f);\n"
                "    if (value[1] > data.k)\n"
                "        value[1] = data.k + knee(value[1] - data.k, data.f);\n"
                "    if (value[2] > data.k)\n"
                "        value[2] = data.k + knee(value[2] - data.k, data.f);\n"
                "    value[0] *= 0.332;\n"
                "    value[1] *= 0.332;\n"
                "    value[2] *= 0.332;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "// $color"
                "\n"
                "void main()\n"
                "{\n"
                "    vec2 t = fTexture;\n"
                "    if (1 == mirrorX)\n"
                "    {\n"
                "        t.x = 1.0 - t.x;\n"
                "    }\n"
                "    if (1 == mirrorY)\n"
                "    {\n"
                "        t.y = 1.0 - t.y;\n"
                "    }\n"
                "\n"
                "    fColor = texture(textureSampler, t);\n"
                "\n"
                "    // Apply color transformations.\n"
                "    if (colorEnabled)\n"
                "    {\n"
                "        fColor = colorFunc(fColor, colorAdd, colorMatrix);\n"
                "    }\n"
                "    if (colorInvert)\n"
                "    {\n"
                "        fColor.r = 1.0 - fColor.r;\n"
                "        fColor.g = 1.0 - fColor.g;\n"
                "        fColor.b = 1.0 - fColor.b;\n"
                "    }\n"
                "    if (levelsEnabled)\n"
                "    {\n"
                "        fColor = levelsFunc(fColor, levels);\n"
                "    }\n"
                "    if (exposureEnabled)\n"
                "    {\n"
                "        fColor = exposureFunc(fColor, exposure);\n"
                "    }\n"
                "    if (softClip > 0.0)\n"
                "    {\n"
                "        fColor = softClipFunc(fColor, softClip);\n"
                "    }\n"
                "\n"
                "    // Apply color management.\n"
                "    fColor = OCIODisplay(fColor);\n"
                "\n"
                "    // Swizzle for the channels display.\n"
                "    if (Channels_Red == channels)\n"
                "    {\n"
                "        fColor.g = fColor.r;\n"
                "        fColor.b = fColor.r;\n"
                "    }\n"
                "    else if (Channels_Green == channels)\n"
                "    {\n"
                "        fColor.r = fColor.g;\n"
                "        fColor.b = fColor.g;\n"
                "    }\n"
                "    else if (Channels_Blue == channels)\n"
                "    {\n"
                "        fColor.r = fColor.b;\n"
                "        fColor.g = fColor.b;\n"
                "    }\n"
                "    else if (Channels_Alpha == channels)\n"
                "    {\n"
                "        fColor.r = fColor.a;\n"
                "        fColor.g = fColor.a;\n"
                "        fColor.b = fColor.a;\n"
                "    }\n"
                "\n"
                "    // Output range.\n"
                "    if (OutputRange_Video == outputRange)\n"
                "    {\n"
                "        const float scale = (940.0 - 64.0) / 1023.0;\n"
                "        const float offset = 64.0 / 1023.0;\n"
                "        fColor.r = fColor.r * scale + offset;\n"
                "        fColor.g = fColor.g * scale + offset;\n"
                "        fColor.b = fColor.b * scale + offset;\n"
                "        fColor.a = fColor.a * scale + offset;\n"
                "    }\n"
                "}\n";
        }

        std::string dissolveFragmentSource()
        {
            return string::Format(
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 fColor;\n"
                "\n"
                "{0}\n"
                "\n"
                "{1}\n"
                "\n"
                "{2}\n"
                "\n"
                "uniform float transition;\n"
                "\n"
                "uniform int       pixelType;\n"
                "uniform int       videoLevels;\n"
                "uniform vec4      yuvCoefficients;\n"
                "uniform int       imageChannels;\n"
                "uniform int       flipX;\n"
                "uniform int       flipY;\n"
                "uniform vec2      textureRangeU;\n"
                "uniform vec2      textureRangeV;\n"
                "uniform sampler2D textureSampler0;\n"
                "uniform sampler2D textureSampler1;\n"
                "uniform sampler2D textureSampler2;\n"
                "\n"
                "uniform int       pixelTypeB;\n"
                "uniform int       videoLevelsB;\n"
                "uniform vec4      yuvCoefficientsB;\n"
                "uniform int       imageChannelsB;\n"
                "uniform int       flipBX;\n"
                "uniform int       flipBY;\n"
                "uniform vec2      textureRangeBU;\n"
                "uniform vec2      textureRangeBV;\n"
                "uniform sampler2D textureSamplerB0;\n"
                "uniform sampler2D textureSamplerB1;\n"
                "uniform sampler2D textureSamplerB2;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 c = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "    if (fTexture.x >= textureRangeU.x &&\n"
                "        fTexture.x <= textureRangeU.y &&\n"
                "        fTexture.y >= textureRangeV.x &&\n"
                "        fTexture.y <= textureRangeV.y)\n"
                "    {\n"
                "        vec2 t = vec2(\n"
                "            (fTexture.x - textureRangeU.x) / (textureRangeU.y - textureRangeU.x),\n"
                "            (fTexture.y - textureRangeV.x) / (textureRangeV.y - textureRangeV.x));\n"
                "        if (1 == flipX)\n"
                "        {\n"
                "            t.x = 1.0 - t.x;\n"
                "        }\n"
                "        if (1 == flipY)\n"
                "        {\n"
                "            t.y = 1.0 - t.y;\n"
                "        }\n"
                "        c = sampleTexture(\n"
                "            t,\n"
                "            pixelType,\n"
                "            videoLevels,\n"
                "            yuvCoefficients,\n"
                "            imageChannels,\n"
                "            textureSampler0,\n"
                "            textureSampler1,\n"
                "            textureSampler2);\n"
                "    }\n"
                "    vec4 cB = vec4(0.0, 0.0, 0.0, 0.0);\n"
                "    if (fTexture.x >= textureRangeBU.x &&\n"
                "        fTexture.x <= textureRangeBU.y &&\n"
                "        fTexture.y >= textureRangeBV.x &&\n"
                "        fTexture.y <= textureRangeBV.y)\n"
                "    {\n"
                "        vec2 t = vec2(\n"
                "            (fTexture.x - textureRangeBU.x) / (textureRangeBU.y - textureRangeBU.x),\n"
                "            (fTexture.y - textureRangeBV.x) / (textureRangeBV.y - textureRangeBV.x));\n"
                "        if (1 == flipBX)\n"
                "        {\n"
                "            t.x = 1.0 - t.x;\n"
                "        }\n"
                "        if (1 == flipBY)\n"
                "        {\n"
                "            t.y = 1.0 - t.y;\n"
                "        }\n"
                "        cB = sampleTexture(\n"
                "            t,\n"
                "            pixelTypeB,\n"
                "            videoLevelsB,\n"
                "            yuvCoefficientsB,\n"
                "            imageChannelsB,\n"
                "            textureSamplerB0,\n"
                "            textureSamplerB1,\n"
                "            textureSamplerB2);\n"
                "    }\n"
                "    fColor = c * (1.0 - transition) + cB * transition;\n"
                "}\n").
                arg(pixelType).
                arg(videoLevels).
                arg(sampleTexture);
        }

        std::string differenceFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 fColor;\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "uniform sampler2D textureSamplerB;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 c = texture(textureSampler, fTexture);\n"
                "    vec4 cB = texture(textureSamplerB, fTexture);\n"
                "    fColor.r = abs(c.r - cB.r);\n"
                "    fColor.g = abs(c.g - cB.g);\n"
                "    fColor.b = abs(c.b - cB.b);\n"
                "    fColor.a = max(c.a, cB.a);\n"
                "}\n";
        }
    }
}
