#include "viewsettingswidget.h"
#include "ui_viewsettingswidget.h"
#include "../settings.h"

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

void ViewSettingsWidget::doLoad()
{
    ui->sbThumbsize->setValue(pSettings->view().thumbnailSize());
}

void ViewSettingsWidget::doSave()
{
    pSettings->view().setThumbnailSize(ui->sbThumbsize->value());
}
