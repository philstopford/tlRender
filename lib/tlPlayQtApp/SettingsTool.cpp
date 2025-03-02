// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/SettingsTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>
#include <tlPlayQtApp/SettingsObject.h>

#include <tlQt/MetaTypes.h>

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        struct CacheSettingsWidget::Private
        {
            QDoubleSpinBox* readAheadSpinBox = nullptr;
            QDoubleSpinBox* readBehindSpinBox = nullptr;
        };

        CacheSettingsWidget::CacheSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.readAheadSpinBox = new QDoubleSpinBox;
            p.readAheadSpinBox->setRange(0.0, 60.0);

            p.readBehindSpinBox = new QDoubleSpinBox;
            p.readBehindSpinBox->setRange(0, 60.0);

            auto layout = new QFormLayout;
            layout->addRow(tr("Read ahead (seconds):"), p.readAheadSpinBox);
            layout->addRow(tr("Read behind (seconds):"), p.readBehindSpinBox);
            setLayout(layout);

            p.readAheadSpinBox->setValue(settingsObject->value("Cache/ReadAhead").toDouble());
            p.readBehindSpinBox->setValue(settingsObject->value("Cache/ReadBehind").toDouble());

            connect(
                p.readAheadSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [settingsObject](double value)
                {
                    settingsObject->setValue("Cache/ReadAhead", value);
                });

            connect(
                p.readBehindSpinBox,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [settingsObject](double value)
                {
                    settingsObject->setValue("Cache/ReadBehind", value);
                });

            connect(
                settingsObject,
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant& value)
                {
                    if (name == "Cache/ReadAhead")
                    {
                        QSignalBlocker signalBlocker(_p->readAheadSpinBox);
                        _p->readAheadSpinBox->setValue(value.toDouble());
                    }
                    else if (name == "Cache/ReadBehind")
                    {
                        QSignalBlocker signalBlocker(_p->readBehindSpinBox);
                        _p->readBehindSpinBox->setValue(value.toDouble());
                    }
                });
        }

        CacheSettingsWidget::~CacheSettingsWidget()
        {}

        struct FileSequenceSettingsWidget::Private
        {
            QComboBox* audioComboBox = nullptr;
            QLineEdit* audioFileName = nullptr;
            QLineEdit* audioDirectory = nullptr;
            QSpinBox* maxDigitsSpinBox = nullptr;
        };

        FileSequenceSettingsWidget::FileSequenceSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.audioComboBox = new QComboBox;
            for (const auto& i : timeline::getFileSequenceAudioLabels())
            {
                p.audioComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.audioFileName = new QLineEdit;

            p.audioDirectory = new QLineEdit;

            p.maxDigitsSpinBox = new QSpinBox;
            p.maxDigitsSpinBox->setRange(0, 255);

            auto layout = new QFormLayout;
            layout->addRow(tr("Audio:"), p.audioComboBox);
            layout->addRow(tr("Audio file name:"), p.audioFileName);
            layout->addRow(tr("Audio directory:"), p.audioDirectory);
            layout->addRow(tr("Maximum digits:"), p.maxDigitsSpinBox);
            setLayout(layout);

            p.audioComboBox->setCurrentIndex(
                static_cast<int>(settingsObject->value("FileSequence/Audio").toInt()));
            p.audioFileName->setText(
                settingsObject->value("FileSequence/AudioFileName").toString());
            p.audioDirectory->setText(
                settingsObject->value("FileSequence/AudioDirectory").toString());
            p.maxDigitsSpinBox->setValue(
                settingsObject->value("FileSequence/MaxDigits").toInt());

            connect(
                p.audioComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [settingsObject](int value)
                {
                    settingsObject->setValue("FileSequence/Audio", value);
                });

            connect(
                p.audioFileName,
                &QLineEdit::textChanged,
                [settingsObject](const QString& value)
                {
                    settingsObject->setValue("FileSequence/AudioFileName", value);
                });

            connect(
                p.audioDirectory,
                &QLineEdit::textChanged,
                [settingsObject](const QString& value)
                {
                    settingsObject->setValue("FileSequence/AudioDirectory", value);
                });

            connect(
                p.maxDigitsSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [settingsObject](int value)
                {
                    settingsObject->setValue("FileSequence/MaxDigits", value);
                });

            connect(
                settingsObject,
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant& value)
                {
                    if (name == "FileSequence/Audio")
                    {
                        QSignalBlocker signalBlocker(_p->audioComboBox);
                        _p->audioComboBox->setCurrentIndex(value.toInt());
                    }
                    else if (name == "FileSequence/AudioFileName")
                    {
                        QSignalBlocker signalBlocker(_p->audioFileName);
                        _p->audioFileName->setText(value.toString());
                    }
                    else if (name == "FileSequence/AudioDirectory")
                    {
                        QSignalBlocker signalBlocker(_p->audioDirectory);
                        _p->audioDirectory->setText(value.toString());
                    }
                    else if (name == "FileSequence/MaxDigits")
                    {
                        QSignalBlocker signalBlocker(_p->maxDigitsSpinBox);
                        _p->maxDigitsSpinBox->setValue(value.toInt());
                    }
                });
        }

        FileSequenceSettingsWidget::~FileSequenceSettingsWidget()
        {}

        struct FileBrowserSettingsWidget::Private
        {
            QCheckBox* nativeFileDialogCheckBox = nullptr;
        };

        FileBrowserSettingsWidget::FileBrowserSettingsWidget(SettingsObject * settingsObject, QWidget * parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.nativeFileDialogCheckBox = new QCheckBox;
            p.nativeFileDialogCheckBox->setText(tr("Native file dialog"));

            auto layout = new QFormLayout;
            layout->addRow(p.nativeFileDialogCheckBox);
            setLayout(layout);

            p.nativeFileDialogCheckBox->setChecked(
                settingsObject->value("FileBrowser/NativeFileDialog").toBool());

            connect(
                p.nativeFileDialogCheckBox,
                &QCheckBox::stateChanged,
                [settingsObject](int value)
                {
                    settingsObject->setValue("FileBrowser/NativeFileDialog", Qt::Checked == value);
                });

            connect(
                settingsObject,
                &SettingsObject::valueChanged,
                [this](const QString& key, const QVariant& value)
                {
                    if ("FileBrowser/NativeFileDialog" == key)
                    {
                        QSignalBlocker signalBlocker(_p->nativeFileDialogCheckBox);
                        _p->nativeFileDialogCheckBox->setChecked(value.toBool());
                    }
                });
        }

        FileBrowserSettingsWidget::~FileBrowserSettingsWidget()
        {}

        struct PerformanceSettingsWidget::Private
        {
            QComboBox* timerModeComboBox = nullptr;
            QSpinBox* audioBufferFrameCountSpinBox = nullptr;
            QSpinBox* videoRequestCountSpinBox = nullptr;
            QSpinBox* audioRequestCountSpinBox = nullptr;
            QSpinBox* sequenceThreadCountSpinBox = nullptr;
            QCheckBox* ffmpegYUVToRGBConversionCheckBox = nullptr;
            QSpinBox* ffmpegThreadCountSpinBox = nullptr;
        };

        PerformanceSettingsWidget::PerformanceSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.timerModeComboBox = new QComboBox;
            for (const auto& i : timeline::getTimerModeLabels())
            {
                p.timerModeComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.audioBufferFrameCountSpinBox = new QSpinBox;
            p.audioBufferFrameCountSpinBox->setRange(1024, 4096);

            p.videoRequestCountSpinBox = new QSpinBox;
            p.videoRequestCountSpinBox->setRange(1, 64);

            p.audioRequestCountSpinBox = new QSpinBox;
            p.audioRequestCountSpinBox->setRange(1, 64);

            p.sequenceThreadCountSpinBox = new QSpinBox;
            p.sequenceThreadCountSpinBox->setRange(1, 64);

            p.ffmpegYUVToRGBConversionCheckBox = new QCheckBox;

            p.ffmpegThreadCountSpinBox = new QSpinBox;
            p.ffmpegThreadCountSpinBox->setRange(0, 64);

            auto layout = new QFormLayout;
            auto label = new QLabel(tr("Changes are applied to new files."));
            label->setWordWrap(true);
            layout->addRow(label);
            layout->addRow(tr("Timer mode:"), p.timerModeComboBox);
            layout->addRow(tr("Audio buffer frames:"), p.audioBufferFrameCountSpinBox);
            layout->addRow(tr("Video requests:"), p.videoRequestCountSpinBox);
            layout->addRow(tr("Audio requests:"), p.audioRequestCountSpinBox);
            layout->addRow(tr("Sequence I/O threads:"), p.sequenceThreadCountSpinBox);
            layout->addRow(tr("FFmpeg YUV to RGB conversion:"), p.ffmpegYUVToRGBConversionCheckBox);
            layout->addRow(tr("FFmpeg I/O threads:"), p.ffmpegThreadCountSpinBox);
            setLayout(layout);

            p.timerModeComboBox->setCurrentIndex(
                settingsObject->value("Performance/TimerMode").toInt());
            p.audioBufferFrameCountSpinBox->setValue(
                settingsObject->value("Performance/AudioBufferFrameCount").toInt());
            p.videoRequestCountSpinBox->setValue(
                settingsObject->value("Performance/VideoRequestCount").toInt());
            p.audioRequestCountSpinBox->setValue(
                settingsObject->value("Performance/AudioRequestCount").toInt());
            p.sequenceThreadCountSpinBox->setValue(
                settingsObject->value("Performance/SequenceThreadCount").toInt());
            p.ffmpegYUVToRGBConversionCheckBox->setChecked(
                settingsObject->value("Performance/FFmpegYUVToRGBConversion").toBool());
            p.ffmpegThreadCountSpinBox->setValue(
                settingsObject->value("Performance/FFmpegThreadCount").toInt());

            connect(
                p.timerModeComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [settingsObject](int value)
                {
                    settingsObject->setValue("Performance/TimerMode", value);
                });

            connect(
                p.audioBufferFrameCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [settingsObject](int value)
                {
                    settingsObject->setValue("Performance/AudioBufferFrameCount", value);
                });

            connect(
                p.videoRequestCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [settingsObject](int value)
                {
                    settingsObject->setValue("Performance/VideoRequestCount", value);
                });

            connect(
                p.audioRequestCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [settingsObject](int value)
                {
                    settingsObject->setValue("Performance/AudioRequestCount", value);
                });

            connect(
                p.sequenceThreadCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [settingsObject](int value)
                {
                    settingsObject->setValue("Performance/SequenceThreadCount", value);
                });

            connect(
                p.ffmpegYUVToRGBConversionCheckBox,
                &QCheckBox::toggled,
                [settingsObject](bool value)
                {
                    settingsObject->setValue("Performance/FFmpegYUVToRGBConversion", value);
                });

            connect(
                p.ffmpegThreadCountSpinBox,
                QOverload<int>::of(&QSpinBox::valueChanged),
                [settingsObject](int value)
                {
                    settingsObject->setValue("Performance/FFmpegThreadCount", value);
                });

            connect(
                settingsObject,
                &SettingsObject::valueChanged,
                [this](const QString& name, const QVariant& value)
                {
                    if (name == "Performance/TimerMode")
                    {
                        QSignalBlocker signalBlocker(_p->timerModeComboBox);
                        _p->timerModeComboBox->setCurrentIndex(value.toInt());
                    }
                    else if (name == "Performance/AudioBufferFrameCount")
                    {
                        QSignalBlocker signalBlocker(_p->audioBufferFrameCountSpinBox);
                        _p->audioBufferFrameCountSpinBox->setValue(value.toInt());
                    }
                    else if (name == "Performance/VideoRequestCount")
                    {
                        QSignalBlocker signalBlocker(_p->videoRequestCountSpinBox);
                        _p->videoRequestCountSpinBox->setValue(value.toInt());
                    }
                    else if (name == "Performance/AudioRequestCount")
                    {
                        QSignalBlocker signalBlocker(_p->audioRequestCountSpinBox);
                        _p->audioRequestCountSpinBox->setValue(value.toInt());
                    }
                    else if (name == "Performance/SequenceThreadCount")
                    {
                        QSignalBlocker signalBlocker(_p->sequenceThreadCountSpinBox);
                        _p->sequenceThreadCountSpinBox->setValue(value.toInt());
                    }
                    else if (name == "Performance/FFmpegYUVToRGBConversion")
                    {
                        QSignalBlocker signalBlocker(_p->ffmpegYUVToRGBConversionCheckBox);
                        _p->ffmpegYUVToRGBConversionCheckBox->setChecked(value.toInt());
                    }
                    else if (name == "Performance/FFmpegThreadCount")
                    {
                        QSignalBlocker signalBlocker(_p->ffmpegThreadCountSpinBox);
                        _p->ffmpegThreadCountSpinBox->setValue(value.toInt());
                    }
                });
        }

        PerformanceSettingsWidget::~PerformanceSettingsWidget()
        {}

        struct MiscSettingsWidget::Private
        {
            QCheckBox* toolTipsCheckBox = nullptr;
        };

        MiscSettingsWidget::MiscSettingsWidget(SettingsObject* settingsObject, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.toolTipsCheckBox = new QCheckBox;
            p.toolTipsCheckBox->setText(tr("Enable tool tips"));

            auto layout = new QFormLayout;
            layout->addRow(p.toolTipsCheckBox);
            setLayout(layout);

            p.toolTipsCheckBox->setChecked(
                settingsObject->value("Misc/ToolTipsEnabled").toBool());

            connect(
                p.toolTipsCheckBox,
                &QCheckBox::stateChanged,
                [settingsObject](int value)
                {
                    settingsObject->setValue("Misc/ToolTipsEnabled", Qt::Checked == value);
                });

            connect(
                settingsObject,
                &SettingsObject::valueChanged,
                [this](const QString& key, const QVariant& value)
                {
                    if ("Misc/ToolTipsEnabled" == key)
                    {
                        QSignalBlocker signalBlocker(_p->toolTipsCheckBox);
                        _p->toolTipsCheckBox->setChecked(value.toBool());
                    }
                });
        }

        MiscSettingsWidget::~MiscSettingsWidget()
        {}

        SettingsTool::SettingsTool(App* app, QWidget* parent) :
            IToolWidget(app, parent)
        {
            auto settingsObject = app->settingsObject();
            addBellows(tr("Cache"), new CacheSettingsWidget(settingsObject));
            addBellows(tr("File Sequences"), new FileSequenceSettingsWidget(settingsObject));
            addBellows(tr("File Browser"), new FileBrowserSettingsWidget(settingsObject));
            addBellows(tr("Performance"), new PerformanceSettingsWidget(settingsObject));
            addBellows(tr("Miscellaneous"), new MiscSettingsWidget(settingsObject));
            auto resetButton = new QToolButton;
            resetButton->setText(tr("Default Settings"));
            resetButton->setAutoRaise(true);
            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(1);
            layout->addWidget(resetButton);
            layout->addStretch();
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);
            addStretch();

            connect(
                resetButton,
                &QToolButton::clicked,
                [settingsObject]
                {
                    settingsObject->reset();
                });
        }

        SettingsDockWidget::SettingsDockWidget(
            SettingsTool* settingsTool,
            QWidget* parent)
        {
            setObjectName("SettingsTool");
            setWindowTitle(tr("Settings"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Settings"));
            dockTitleBar->setIcon(QIcon(":/Icons/Settings.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(settingsTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Settings.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F7));
            toggleViewAction()->setToolTip(tr("Show settings"));
        }
    }
}
