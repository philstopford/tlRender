// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/SettingsTool.h>

#include <tlPlayGLApp/App.h>
#include <tlPlayGLApp/Settings.h>
#include <tlPlayGLApp/Style.h>

#include <tlUI/Bellows.h>
#include <tlUI/CheckBox.h>
#include <tlUI/ComboBox.h>
#include <tlUI/DoubleEdit.h>
#include <tlUI/FloatEditSlider.h>
#include <tlUI/GridLayout.h>
#include <tlUI/IntEdit.h>
#include <tlUI/IntEdit.h>
#include <tlUI/Label.h>
#include <tlUI/LineEdit.h>
#include <tlUI/MessageDialog.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_gl
    {
        struct CacheSettingsWidget::Private
        {
            std::shared_ptr<ui::IntEdit> cacheSize;
            std::shared_ptr<ui::DoubleEdit> readAhead;
            std::shared_ptr<ui::DoubleEdit> readBehind;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void CacheSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::CacheSettingsWidget", context, parent);
            TLRENDER_P();

            p.cacheSize = ui::IntEdit::create(context);
            p.cacheSize->setRange(math::IntRange(0, 1024));

            p.readAhead = ui::DoubleEdit::create(context);
            p.readAhead->setRange(math::DoubleRange(0.0, 60.0));
            p.readAhead->setStep(1.0);
            p.readAhead->setLargeStep(10.0);

            p.readBehind = ui::DoubleEdit::create(context);
            p.readBehind->setRange(math::DoubleRange(0.0, 60.0));
            p.readBehind->setStep(1.0);
            p.readBehind->setLargeStep(10.0);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Cache size (GB):", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.cacheSize->setParent(p.layout);
            p.layout->setGridPos(p.cacheSize, 0, 1);
            label = ui::Label::create("Read ahead (seconds):", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.readAhead->setParent(p.layout);
            p.layout->setGridPos(p.readAhead, 1, 1);
            label = ui::Label::create("Read behind (seconds):", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.readBehind->setParent(p.layout);
            p.layout->setGridPos(p.readBehind, 2, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.cacheSize->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Cache/Size", value);
                    }
                });

            p.readAhead->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Cache/ReadAhead", value);
                    }
                });

            p.readBehind->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Cache/ReadBehind", value);
                    }
                });

            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            int value = 0;
                            settings->getValue<int>("Cache/Size", value);
                            p.cacheSize->setValue(value);
                        }
                        {
                            double value = 0.0;
                            settings->getValue<double>("Cache/ReadAhead", value);
                            p.readAhead->setValue(value);
                        }
                        {
                            double value = 0.0;
                            settings->getValue<double>("Cache/ReadBehind", value);
                            p.readBehind->setValue(value);
                        }
                    }
                });
        }

        CacheSettingsWidget::CacheSettingsWidget() :
            _p(new Private)
        {}

        CacheSettingsWidget::~CacheSettingsWidget()
        {}

        std::shared_ptr<CacheSettingsWidget> CacheSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CacheSettingsWidget>(new CacheSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void CacheSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void CacheSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct FileSequenceSettingsWidget::Private
        {
            std::shared_ptr<ui::ComboBox> audioComboBox;
            std::shared_ptr<ui::LineEdit> audioFileNameEdit;
            std::shared_ptr<ui::LineEdit> audioDirectoryEdit;
            std::shared_ptr<ui::IntEdit> maxDigitsEdit;
            std::shared_ptr<ui::IntEdit> threadsEdit;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void FileSequenceSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::FileSequenceSettingsWidget", context, parent);
            TLRENDER_P();

            p.audioComboBox = ui::ComboBox::create(
                timeline::getFileSequenceAudioLabels(),
                context);

            p.audioFileNameEdit = ui::LineEdit::create(context);

            p.audioDirectoryEdit = ui::LineEdit::create(context);

            p.maxDigitsEdit = ui::IntEdit::create(context);

            p.threadsEdit = ui::IntEdit::create(context);
            p.threadsEdit->setRange(math::IntRange(1, 64));

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Audio:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.audioComboBox->setParent(p.layout);
            p.layout->setGridPos(p.audioComboBox, 0, 1);
            label = ui::Label::create("Audio file name:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.audioFileNameEdit->setParent(p.layout);
            p.layout->setGridPos(p.audioFileNameEdit, 1, 1);
            label = ui::Label::create("Audio directory:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.audioDirectoryEdit->setParent(p.layout);
            p.layout->setGridPos(p.audioDirectoryEdit, 2, 1);
            label = ui::Label::create("Maximum digits:", context, p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.maxDigitsEdit->setParent(p.layout);
            p.layout->setGridPos(p.maxDigitsEdit, 3, 1);
            label = ui::Label::create("I/O threads:", context, p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.threadsEdit->setParent(p.layout);
            p.layout->setGridPos(p.threadsEdit, 4, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            timeline::FileSequenceAudio value = timeline::FileSequenceAudio::First;
                            settings->getValue("FileSequence/Audio", value);
                            p.audioComboBox->setCurrentIndex(static_cast<int>(value));
                        }
                        {
                            std::string value;
                            settings->getValue("FileSequence/AudioFileName", value);
                            p.audioFileNameEdit->setText(value);
                        }
                        {
                            std::string value;
                            settings->getValue("FileSequence/AudioDirectory", value);
                            p.audioDirectoryEdit->setText(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("FileSequence/MaxDigits", value);
                            p.maxDigitsEdit->setValue(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("SequenceIO/ThreadCount", value);
                            p.threadsEdit->setValue(value);
                        }
                    }
                });

            p.audioComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "FileSequence/Audio",
                            static_cast<timeline::FileSequenceAudio>(value));
                    }
                });

            p.audioFileNameEdit->setTextCallback(
                [appWeak](const std::string& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("FileSequence/AudioFileName", value);
                    }
                });

            p.audioDirectoryEdit->setTextCallback(
                [appWeak](const std::string& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("FileSequence/AudioDirectory", value);
                    }
                });

            p.maxDigitsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("FileSequence/MaxDigits", value);
                    }
                });

            p.threadsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("SequenceIO/ThreadCount", value);
                    }
                });
        }

        FileSequenceSettingsWidget::FileSequenceSettingsWidget() :
            _p(new Private)
        {}

        FileSequenceSettingsWidget::~FileSequenceSettingsWidget()
        {}

        std::shared_ptr<FileSequenceSettingsWidget> FileSequenceSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileSequenceSettingsWidget>(new FileSequenceSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void FileSequenceSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileSequenceSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

#if defined(TLRENDER_FFMPEG)
        struct FFmpegSettingsWidget::Private
        {
            std::shared_ptr<ui::CheckBox> yuvToRGBCheckBox;
            std::shared_ptr<ui::IntEdit> threadsEdit;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void FFmpegSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::FFmpegSettingsWidget", context, parent);
            TLRENDER_P();

            p.yuvToRGBCheckBox = ui::CheckBox::create(context);

            p.threadsEdit = ui::IntEdit::create(context);
            p.threadsEdit->setRange(math::IntRange(0, 64));

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("YUV to RGB conversion:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.yuvToRGBCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.yuvToRGBCheckBox, 0, 1);
            label = ui::Label::create("I/O threads:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.threadsEdit->setParent(p.layout);
            p.layout->setGridPos(p.threadsEdit, 1, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            bool value = false;
                            settings->getValue("FFmpeg/YUVToRGBConversion", value);
                            p.yuvToRGBCheckBox->setChecked(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("FFmpeg/ThreadCount", value);
                            p.threadsEdit->setValue(value);
                        }
                    }
                });

            p.yuvToRGBCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "FFmpeg/YUVToRGBConversion",
                            value);
                    }
                });

            p.threadsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "FFmpeg/ThreadCount",
                            value);
                    }
                });
        }

        FFmpegSettingsWidget::FFmpegSettingsWidget() :
            _p(new Private)
        {}

        FFmpegSettingsWidget::~FFmpegSettingsWidget()
        {}

        std::shared_ptr<FFmpegSettingsWidget> FFmpegSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FFmpegSettingsWidget>(new FFmpegSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void FFmpegSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FFmpegSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
        struct USDSettingsWidget::Private
        {
            std::shared_ptr<ui::IntEdit> renderWidthEdit;
            std::shared_ptr<ui::FloatEditSlider> complexitySlider;
            std::shared_ptr<ui::ComboBox> drawModeComboBox;
            std::shared_ptr<ui::CheckBox> lightingCheckBox;
            std::shared_ptr<ui::CheckBox> sRGBCheckBox;
            std::shared_ptr<ui::IntEdit> stageCacheEdit;
            std::shared_ptr<ui::IntEdit> diskCacheEdit;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void USDSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::USDSettingsWidget", context, parent);
            TLRENDER_P();

            p.renderWidthEdit = ui::IntEdit::create(context);
            p.renderWidthEdit->setRange(math::IntRange(1, 8192));

            p.complexitySlider = ui::FloatEditSlider::create(context);

            p.drawModeComboBox = ui::ComboBox::create(usd::getDrawModeLabels(), context);

            p.lightingCheckBox = ui::CheckBox::create(context);

            p.sRGBCheckBox = ui::CheckBox::create(context);

            p.stageCacheEdit = ui::IntEdit::create(context);
            p.stageCacheEdit->setRange(math::IntRange(0, 10));

            p.diskCacheEdit = ui::IntEdit::create(context);
            p.diskCacheEdit->setRange(math::IntRange(0, 1024));

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Render width:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.renderWidthEdit->setParent(p.layout);
            p.layout->setGridPos(p.renderWidthEdit, 0, 1);
            label = ui::Label::create("Render complexity:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.complexitySlider->setParent(p.layout);
            p.layout->setGridPos(p.complexitySlider, 1, 1);
            label = ui::Label::create("Draw mode:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.drawModeComboBox->setParent(p.layout);
            p.layout->setGridPos(p.drawModeComboBox, 2, 1);
            label = ui::Label::create("Enable lighting:", context, p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.lightingCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.lightingCheckBox, 3, 1);
            label = ui::Label::create("Enable sRGB color space:", context, p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sRGBCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.sRGBCheckBox, 4, 1);
            label = ui::Label::create("Stage cache size:", context, p.layout);
            p.layout->setGridPos(label, 5, 0);
            p.stageCacheEdit->setParent(p.layout);
            p.layout->setGridPos(p.stageCacheEdit, 5, 1);
            label = ui::Label::create("Disk cache size (GB):", context, p.layout);
            p.layout->setGridPos(label, 6, 0);
            p.diskCacheEdit->setParent(p.layout);
            p.layout->setGridPos(p.diskCacheEdit, 6, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            int value = 1920;
                            settings->getValue("USD/renderWidth", value);
                            p.renderWidthEdit->setValue(value);
                        }
                        {
                            float value = 1.F;
                            settings->getValue("USD/complexity", value);
                            p.complexitySlider->setValue(value);
                        }
                        {
                            usd::DrawMode value = usd::DrawMode::ShadedSmooth;
                            settings->getValue("USD/drawMode", value);
                            p.drawModeComboBox->setCurrentIndex(static_cast<int>(value));
                        }
                        {
                            bool value = true;
                            settings->getValue("USD/enableLighting", value);
                            p.lightingCheckBox->setChecked(value);
                        }
                        {
                            bool value = true;
                            settings->getValue("USD/sRGB", value);
                            p.sRGBCheckBox->setChecked(value);
                        }
                        {
                            size_t value = 10;
                            settings->getValue("USD/stageCacheCount", value);
                            p.stageCacheEdit->setValue(value);
                        }
                        {
                            size_t value = 0;
                            settings->getValue("USD/diskCacheByteCount", value);
                            p.diskCacheEdit->setValue(value / memory::gigabyte);
                        }
                    }
                });

            p.renderWidthEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("USD/renderWidth", value);
                    }
                });

            p.complexitySlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("USD/complexity", value);
                    }
                });

            p.drawModeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        const usd::DrawMode drawMode = static_cast<usd::DrawMode>(value);
                        app->getSettings()->setValue("USD/drawMode", drawMode);
                    }
                });

            p.lightingCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("USD/enableLighting", value);
                    }
                });

            p.sRGBCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("USD/sRGB", value);
                    }
                });

            p.stageCacheEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("USD/stageCacheCount", value);
                    }
                });

            p.diskCacheEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "USD/diskCacheByteCount",
                            value * memory::gigabyte);
                    }
                });
        }

        USDSettingsWidget::USDSettingsWidget() :
            _p(new Private)
        {}

        USDSettingsWidget::~USDSettingsWidget()
        {}

        std::shared_ptr<USDSettingsWidget> USDSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<USDSettingsWidget>(new USDSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void USDSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void USDSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
#endif // TLRENDER_USD

        struct FileBrowserSettingsWidget::Private
        {
            std::shared_ptr<ui::CheckBox> nativeFileDialogCheckBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void FileBrowserSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::FileBrowserSettingsWidget", context, parent);
            TLRENDER_P();

            p.nativeFileDialogCheckBox = ui::CheckBox::create(context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Native file dialog:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.nativeFileDialogCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.nativeFileDialogCheckBox, 0, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            bool value = false;
                            settings->getValue("FileBrowser/NativeFileDialog", value);
                            p.nativeFileDialogCheckBox->setChecked(value);
                        }
                    }
                });

            p.nativeFileDialogCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("FileBrowser/NativeFileDialog", value);
                    }
                });
        }

        FileBrowserSettingsWidget::FileBrowserSettingsWidget() :
            _p(new Private)
        {}

        FileBrowserSettingsWidget::~FileBrowserSettingsWidget()
        {}

        std::shared_ptr<FileBrowserSettingsWidget> FileBrowserSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileBrowserSettingsWidget>(new FileBrowserSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void FileBrowserSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileBrowserSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct PerformanceSettingsWidget::Private
        {
            std::shared_ptr<ui::ComboBox> timerComboBox;
            std::shared_ptr<ui::IntEdit> audioBufferFramesEdit;
            std::shared_ptr<ui::IntEdit> videoRequestsEdit;
            std::shared_ptr<ui::IntEdit> audioRequestsEdit;
            std::shared_ptr<ui::VerticalLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void PerformanceSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::PerformanceSettingsWidget", context, parent);
            TLRENDER_P();

            p.timerComboBox = ui::ComboBox::create(
                timeline::getTimerModeLabels(), context);

            p.audioBufferFramesEdit = ui::IntEdit::create(context);
            p.audioBufferFramesEdit->setRange(math::IntRange(1024, 4096));
            p.audioBufferFramesEdit->setStep(256);
            p.audioBufferFramesEdit->setLargeStep(1024);

            p.videoRequestsEdit = ui::IntEdit::create(context);
            p.videoRequestsEdit->setRange(math::IntRange(1, 64));

            p.audioRequestsEdit = ui::IntEdit::create(context);
            p.audioRequestsEdit->setRange(math::IntRange(1, 64));

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Changes are applied to new files.", context, p.layout);
            auto gridLayout = ui::GridLayout::create(context, p.layout);
            gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            label = ui::Label::create("Timer mode:", context, gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.timerComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.timerComboBox, 0, 1);
            label = ui::Label::create("Audio buffer frames:", context, gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.audioBufferFramesEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.audioBufferFramesEdit, 1, 1);
            label = ui::Label::create("Video requests:", context, gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.videoRequestsEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.videoRequestsEdit, 2, 1);
            label = ui::Label::create("Audio requests:", context, gridLayout);
            gridLayout->setGridPos(label, 3, 0);
            p.audioRequestsEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.audioRequestsEdit, 3, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            timeline::TimerMode value = timeline::TimerMode::First;
                            settings->getValue("Performance/TimerMode", value);
                            p.timerComboBox->setCurrentIndex(static_cast<int>(value));
                        }
                        {
                            int value = 0;
                            settings->getValue("Performance/AudioBufferFrameCount", value);
                            p.audioBufferFramesEdit->setValue(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("Performance/VideoRequestCount", value);
                            p.videoRequestsEdit->setValue(value);
                        }
                        {
                            int value = 0;
                            settings->getValue("Performance/AudioRequestCount", value);
                            p.audioRequestsEdit->setValue(value);
                        }
                    }
                });

            p.timerComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/TimerMode",
                            static_cast<timeline::TimerMode>(value));
                    }
                });

            p.audioBufferFramesEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/AudioBufferFrameCount",
                            value);
                    }
                });

            p.videoRequestsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/VideoRequestCount",
                            value);
                    }
                });

            p.audioRequestsEdit->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue(
                            "Performance/AudioRequestCount",
                            value);
                    }
                });
        }

        PerformanceSettingsWidget::PerformanceSettingsWidget() :
            _p(new Private)
        {}

        PerformanceSettingsWidget::~PerformanceSettingsWidget()
        {}

        std::shared_ptr<PerformanceSettingsWidget> PerformanceSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PerformanceSettingsWidget>(new PerformanceSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void PerformanceSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void PerformanceSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct StyleSettingsWidget::Private
        {
            std::shared_ptr<ui::ComboBox> paletteComboBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void StyleSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::StyleSettingsWidget", context, parent);
            TLRENDER_P();

            p.paletteComboBox = ui::ComboBox::create(getStylePaletteLabels(), context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Palette:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.paletteComboBox->setParent(p.layout);
            p.layout->setGridPos(p.paletteComboBox, 0, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            StylePalette value = StylePalette::First;
                            settings->getValue("Style/Palette", value);
                            p.paletteComboBox->setCurrentIndex(static_cast<int>(value));
                        }
                    }
                });

            p.paletteComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        const StylePalette stylePalette = static_cast<StylePalette>(value);
                        app->getSettings()->setValue("Style/Palette", stylePalette);
                    }
                });
        }

        StyleSettingsWidget::StyleSettingsWidget() :
            _p(new Private)
        {}

        StyleSettingsWidget::~StyleSettingsWidget()
        {}

        std::shared_ptr<StyleSettingsWidget> StyleSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<StyleSettingsWidget>(new StyleSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void StyleSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void StyleSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct MiscSettingsWidget::Private
        {
            std::shared_ptr<ui::CheckBox> toolTipsEnabledCheckBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
        };

        void MiscSettingsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_gl::MiscSettingsWidget", context, parent);
            TLRENDER_P();

            p.toolTipsEnabledCheckBox = ui::CheckBox::create(context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Enable tool tips:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.toolTipsEnabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.toolTipsEnabledCheckBox, 1, 1);

            auto appWeak = std::weak_ptr<App>(app);
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                app->getSettings()->observeValues(),
                [this, appWeak](const std::string&)
                {
                    TLRENDER_P();
                    if (auto app = appWeak.lock())
                    {
                        auto settings = app->getSettings();
                        {
                            bool value = false;
                            settings->getValue("Misc/ToolTipsEnabled", value);
                            p.toolTipsEnabledCheckBox->setChecked(value);
                        }
                    }
                });

            p.toolTipsEnabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getSettings()->setValue("Misc/ToolTipsEnabled", value);
                    }
                });
        }

        MiscSettingsWidget::MiscSettingsWidget() :
            _p(new Private)
        {}

        MiscSettingsWidget::~MiscSettingsWidget()
        {}

        std::shared_ptr<MiscSettingsWidget> MiscSettingsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MiscSettingsWidget>(new MiscSettingsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void MiscSettingsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void MiscSettingsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct SettingsTool::Private
        {
            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<ui::ToolButton> resetButton;
            std::shared_ptr<ui::VerticalLayout> layout;
        };

        void SettingsTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Settings,
                "tl::play_gl::SettingsTool",
                app,
                context,
                parent);
            TLRENDER_P();

            auto cacheWidget = CacheSettingsWidget::create(app, context);
            auto fileSequenceWidget = FileSequenceSettingsWidget::create(app, context);
#if defined(TLRENDER_FFMPEG)
            auto ffmpegWidget = FFmpegSettingsWidget::create(app, context);
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            auto usdWidget = USDSettingsWidget::create(app, context);
#endif // TLRENDER_USD
            auto fileBrowserWidget = FileBrowserSettingsWidget::create(app, context);
            auto performanceWidget = PerformanceSettingsWidget::create(app, context);
            auto styleWidget = StyleSettingsWidget::create(app, context);
            auto miscWidget = MiscSettingsWidget::create(app, context);
            auto vLayout = ui::VerticalLayout::create(context);
            vLayout->setSpacingRole(ui::SizeRole::None);
            auto bellows = ui::Bellows::create("Cache", context, vLayout);
            bellows->setWidget(cacheWidget);
            bellows = ui::Bellows::create("File Sequences", context, vLayout);
            bellows->setWidget(fileSequenceWidget);
#if defined(TLRENDER_FFMPEG)
            bellows = ui::Bellows::create("FFmpeg", context, vLayout);
            bellows->setWidget(ffmpegWidget);
#endif // TLRENDER_USD
#if defined(TLRENDER_USD)
            bellows = ui::Bellows::create("USD", context, vLayout);
            bellows->setWidget(usdWidget);
#endif // TLRENDER_USD
            bellows = ui::Bellows::create("File Browser", context, vLayout);
            bellows->setWidget(fileBrowserWidget);
            bellows = ui::Bellows::create("Performance", context, vLayout);
            bellows->setWidget(performanceWidget);
            bellows = ui::Bellows::create("Style", context, vLayout);
            bellows->setWidget(styleWidget);
            bellows = ui::Bellows::create("Miscellaneous", context, vLayout);
            bellows->setWidget(miscWidget);
            p.scrollWidget = ui::ScrollWidget::create(context);
            p.scrollWidget->setWidget(vLayout);
            p.scrollWidget->setVStretch(ui::Stretch::Expanding);

            p.resetButton = ui::ToolButton::create("Default Settings", context);

            p.layout = ui::VerticalLayout::create(context);
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.scrollWidget->setParent(p.layout);
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.resetButton->setParent(hLayout);
            _setWidget(p.layout);

            std::weak_ptr<App> appWeak(app);
            p.resetButton->setClickedCallback(
                [this, appWeak]
                {
                    if (auto context = _context.lock())
                    {
                        if (auto eventLoop = getEventLoop().lock())
                        {
                            if (auto messageDialogSystem = context->getSystem<ui::MessageDialogSystem>())
                            {
                                messageDialogSystem->open(
                                    "Reset preferences to default values?",
                                    eventLoop,
                                    [appWeak](bool value)
                                    {
                                        if (value)
                                        {
                                            if (auto app = appWeak.lock())
                                            {
                                                app->getSettings()->reset();
                                            }
                                        }
                                    });
                            }
                        }
                    }
                });
        }

        SettingsTool::SettingsTool() :
            _p(new Private)
        {}

        SettingsTool::~SettingsTool()
        {}

        std::shared_ptr<SettingsTool> SettingsTool::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<SettingsTool>(new SettingsTool);
            out->_init(app, context, parent);
            return out;
        }
    }
}
