#ifndef APPEARANCESETTINGSWIDGET_H
#define APPEARANCESETTINGSWIDGET_H

#include <QWidget>
#include "settingswidget.h"

namespace Ui {
class AppearanceSettingsWidget;
}

class AppearanceSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    explicit AppearanceSettingsWidget(const QString& name, QWidget *parent = nullptr);
    ~AppearanceSettingsWidget();

private:
    Ui::AppearanceSettingsWidget *ui;

    // SettingsWidget interface
protected:
    void doLoad() override;
    void doSave() override;
};

#endif // APPEARANCESETTINGSWIDGET_H
