#include "appearancesettingswidget.h"
#include "ui_appearancesettingswidget.h"
#include "../settings.h"

AppearanceSettingsWidget::AppearanceSettingsWidget(const QString& name, QWidget *parent) :
    SettingsWidget(name, parent),
    ui(new Ui::AppearanceSettingsWidget)
{
    ui->setupUi(this);
}

AppearanceSettingsWidget::~AppearanceSettingsWidget()
{
    delete ui;
}

void AppearanceSettingsWidget::doLoad()
{
    ui->cbFontName->setCurrentText(pSettings->ui().fontName());
    ui->sbFontSize->setValue(pSettings->ui().fontSize());
}

void AppearanceSettingsWidget::doSave()
{
    pSettings->ui().setFontName(ui->cbFontName->currentText());
    pSettings->ui().setFontSize(ui->sbFontSize->value());
    pSettings->ui().save();
}
