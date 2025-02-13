/*
 * Copyright (C) 2020-2022 Roy Qu (royqh1979@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "../mainwindow.h"
#include "../settings.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "settingswidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QModelIndex>
#include "appearancesettingswidget.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    setWindowFlag(Qt::WindowContextHelpButtonHint,false);
    ui->setupUi(this);

    connect(ui->widgetsView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SettingsDialog::onWidgetsViewCurrentChanged);

    ui->btnApply->setEnabled(false);

    mAppShouldQuit = false;
    resize(pSettings->ui().settingsDialogWidth(),pSettings->ui().settingsDialogHeight());

    QList<int> sizes = ui->splitter->sizes();
    int tabWidth = pSettings->ui().settingsDialogSplitterPos();
    int totalSize = sizes[0] + sizes[1];
    sizes[0] = tabWidth;
    sizes[1] = std::max(1,totalSize - sizes[0]);
    ui->splitter->setSizes(sizes);
}

SettingsDialog::~SettingsDialog()
{
    foreach (SettingsWidget* p, mSettingWidgets) {
        p->setParent(nullptr);
        delete p;
    }
    delete ui;
}

void SettingsDialog::addWidget(SettingsWidget *pWidget)
{
    pWidget->init();
    mSettingWidgets.append(pWidget);
    connect(pWidget, &SettingsWidget::settingsChanged,
            this , &SettingsDialog::widget_settings_changed);

    QListWidgetItem *pItem = new QListWidgetItem();
    pItem->setText(pWidget->name());
    ui->widgetsView->addItem(pItem);
}

void SettingsDialog::selectFirstWidget()
{
    auto widgetIndex = ui->widgetsView->model()->index(0,0);
    ui->widgetsView->selectionModel()->setCurrentIndex(
                widgetIndex,
                QItemSelectionModel::Select | QItemSelectionModel::Current
                );
    showWidget(widgetIndex);
}

PSettingsDialog SettingsDialog::optionDialog(QWidget *parent)
{
    PSettingsDialog dialog = std::make_shared<SettingsDialog>(parent);

    dialog->setWindowTitle(tr("Options"));

    SettingsWidget* widget;
    widget = new AppearanceSettingsWidget(tr("Appearance"));
    dialog->addWidget(widget);

//    widget = new EnvironmentAppearanceWidget(tr("Appearance"),tr("Environment"));
//    dialog->addWidget(widget);

//    widget = new EnvironmentShortcutWidget(tr("Shortcuts"),tr("Environment"));
//    dialog->addWidget(widget);

    dialog->selectFirstWidget();

    return dialog;
}

bool SettingsDialog::setCurrentWidget(const QString &widgetName)
{
    for (int i=0;i<mSettingWidgets.count();i++) {
        if (mSettingWidgets[i]->name() == widgetName) {
            QModelIndex idx = ui->widgetsView->model()->index(i,0);
            ui->widgetsView->setCurrentIndex(idx);
            showWidget(idx);
            return true;
        }
    }
    return false;
}

void SettingsDialog::widget_settings_changed(bool value)
{
    ui->btnApply->setEnabled(value);
}

void SettingsDialog::onWidgetsViewCurrentChanged(const QModelIndex &index, const QModelIndex &/*previous*/)
{
    showWidget(index);
}

void SettingsDialog::on_btnCancel_pressed()
{
    this->close();
}

void SettingsDialog::on_btnApply_pressed()
{
    saveCurrentPageSettings(false);
}

void SettingsDialog::on_btnOk_pressed()
{
    saveCurrentPageSettings(false);
    this->close();
}

void SettingsDialog::saveCurrentPageSettings(bool confirm)
{
    if (ui->scrollArea->widget()==ui->scrollAreaWidgetContents)
        return;
    SettingsWidget* pWidget = (SettingsWidget*) ui->scrollArea->widget();
    if (!pWidget->isSettingsChanged())
        return;
    if (confirm) {
        if (QMessageBox::warning(this,tr("Save Changes"),
               tr("There are changes in the settings, do you want to save them before swtich to other page?"),
               QMessageBox::Yes, QMessageBox::No)!=QMessageBox::Yes) {
            return;
        }
    }
    pWidget->save();
    emit settingsChanged();
}

void SettingsDialog::closeEvent(QCloseEvent *event)
{
    pSettings->ui().setSettingsDialogWidth(width());
    pSettings->ui().setSettingsDialogHeight(height());

    QList<int> sizes = ui->splitter->sizes();
    pSettings->ui().setSettingsDialogSplitterPos(sizes[0]);

    QDialog::closeEvent(event);
}

bool SettingsDialog::appShouldQuit() const
{
    return mAppShouldQuit;
}

void SettingsDialog::closeAndQuit()
{
    mAppShouldQuit = true;
    close();
}

void SettingsDialog::showWidget(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    int i = index.row();
    if (i>=0) {
        saveCurrentPageSettings(true);
        SettingsWidget* pWidget = mSettingWidgets[i];
        if (ui->scrollArea->widget()!=nullptr) {
            QWidget* w = ui->scrollArea->takeWidget();
            w->setParent(nullptr);
        }
        ui->scrollArea->setWidget(pWidget);
        ui->lblWidgetCaption->setText(QString("%1").arg(pWidget->name()));

        ui->btnApply->setEnabled(false);
    }
}
