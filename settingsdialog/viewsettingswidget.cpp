#include "viewsettingswidget.h"
#include "ui_viewsettingswidget.h"

ViewSettingsWidget::ViewSettingsWidget(const QString& name, QWidget *parent) :
    SettingsWidget{name, parent},
    ui(new Ui::ViewSettingsWidget)
{
    ui->setupUi(this);
}

ViewSettingsWidget::~ViewSettingsWidget()
{
    delete ui;
}
