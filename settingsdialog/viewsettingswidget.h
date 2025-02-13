#ifndef VIEWSETTINGSWIDGET_H
#define VIEWSETTINGSWIDGET_H

#include "settingswidget.h"

namespace Ui {
class ViewSettingsWidget;
}

class ViewSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    explicit ViewSettingsWidget(const QString& name, QWidget *parent = nullptr);
    ~ViewSettingsWidget();

private:
    Ui::ViewSettingsWidget *ui;

    // SettingsWidget interface
protected:
    void doLoad() override;
    void doSave() override;
};

#endif // VIEWSETTINGSWIDGET_H
