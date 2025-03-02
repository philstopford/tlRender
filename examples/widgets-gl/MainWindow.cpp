// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "BasicWidgets.h"
#include "Charts.h"
#include "DragAndDrop.h"
#include "GridLayouts.h"
#include "NumericWidgets.h"
#include "RowLayouts.h"
#include "ScrollAreas.h"

#include <tlUI/ButtonGroup.h>
#include <tlUI/ListButton.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/StackLayout.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            struct MainWindow::Private
            {
                std::map<std::string, std::shared_ptr<IExampleWidget> > widgets;
                std::map<std::string, std::shared_ptr<ui::ListButton> > buttons;
                std::shared_ptr<ui::ButtonGroup> buttonGroup;
                std::shared_ptr<ui::RowLayout> layout;
                std::shared_ptr<ui::StackLayout> stackLayout;
            };

            void MainWindow::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init("MainWindow", context, parent);
                TLRENDER_P();

                setBackgroundRole(ui::ColorRole::Window);

                std::shared_ptr<IExampleWidget> widget = nullptr;
                widget = BasicWidgets::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = Charts::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = DragAndDrop::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = GridLayouts::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = NumericWidgets::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = RowLayouts::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = ScrollAreas::create(context);
                p.widgets[widget->getExampleName()] = widget;

                p.buttonGroup = ui::ButtonGroup::create(
                    ui::ButtonGroupType::Click,
                    context);
                for (const auto& i : p.widgets)
                {
                    auto button = ui::ListButton::create(context);
                    const std::string& exampleName = i.second->getExampleName();
                    button->setText(exampleName);
                    p.buttons[exampleName] = button;
                    p.buttonGroup->addButton(button);
                }
                p.buttonGroup->setClickedCallback(
                    [this](int value)
                    {
                        _p->stackLayout->setCurrentIndex(value);
                    });

                p.layout = ui::HorizontalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                p.layout->setSpacingRole(ui::SizeRole::Spacing);
                auto scrollWidget = ui::ScrollWidget::create(
                    context,
                    ui::ScrollType::Vertical,
                    p.layout);
                auto buttonLayout = ui::VerticalLayout::create(context);
                scrollWidget->setWidget(buttonLayout);
                buttonLayout->setSpacingRole(ui::SizeRole::None);
                for (auto button : p.buttons)
                {
                    button.second->setParent(buttonLayout);
                }
                p.stackLayout = ui::StackLayout::create(context, p.layout);
                p.stackLayout->setHStretch(ui::Stretch::Expanding);
                for (auto widget : p.widgets)
                {
                    scrollWidget = ui::ScrollWidget::create(
                        context,
                        ui::ScrollType::Both,
                        p.stackLayout);
                    scrollWidget->setWidget(widget.second);
                }

                //p.stackLayout->setCurrentIndex(4);
            }

            MainWindow::MainWindow() :
                _p(new Private)
            {}

            MainWindow::~MainWindow()
            {}

            std::shared_ptr<MainWindow> MainWindow::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MainWindow>(new MainWindow);
                out->_init(context, parent);
                return out;
            }

            void MainWindow::setGeometry(const math::Box2i& value)
            {
                IWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        }
    }
}
