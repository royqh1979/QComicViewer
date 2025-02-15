/*
 * Copyright (C) 2025 Roy Qu (royqh1979@gmail.com)
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QListView>
#include <QMainWindow>
#include <QSpinBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ImageWidget;
class PagesNavigator;
class BookPagesModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void applySettings();
private slots:
    void updateStatusBar();
    void onPageViewCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onZoomFactorChanged(int value);
    void updatePageMode();
    void updateImageFitType();
    void on_actionNext_Page_triggered();

    void on_actionPrev_Page_triggered();

    void on_actionLast_Page_triggered();

    void on_actionFirst_Page_triggered();

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionAbout_triggered();

    void on_actionRight_to_Left_toggled(bool arg1);

    void on_dockPages_visibilityChanged(bool visible);

    void on_actionShow_Contents_toggled(bool arg1);

    void on_actionOptions_triggered();

    void on_actionSwap_Left_Right_Key_toggled(bool arg1);

    void on_actionRotate_90_Clockwise_triggered();

    void on_actionRotate_90_Counter_Clockwise_triggered();

    void on_actionHorizontal_Flip_triggered();

    void on_actionVertical_Flip_triggered();

    void on_actionClose_triggered();

private:
    void onCurrentPageChanged();
private:
    Ui::MainWindow *ui;
    ImageWidget *mImageWidget;
    PagesNavigator *mPagesNavigator;
    BookPagesModel *mBookPagesModel;
    QSpinBox *mZoomFactor;
    QLabel *mImageSizeInfo;
    QLabel *mFilenameInfo;
    QLabel *mPageInfo;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
