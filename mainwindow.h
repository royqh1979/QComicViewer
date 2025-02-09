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

#include <QListView>
#include <QMainWindow>

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
private slots:
    void onPageViewCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void on_actionShow_Double_Pages_toggled(bool arg1);

    void on_actionNext_Page_triggered();

    void on_actionPrev_Page_triggered();

    void on_actionLast_Page_triggered();

    void on_actionFirst_Page_triggered();

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionFit_Width_toggled(bool arg1);

    void on_actionFit_Height_toggled(bool arg1);

    void on_actionFit_Page_toggled(bool arg1);

    void on_actionSingle_First_Page_toggled(bool arg1);

    void on_actionLeft_To_Right_toggled(bool arg1);

    void on_actionShow_Contents_triggered();

private:
    void setImageFitType();
    void onCurrentPageChanged();
private:
    Ui::MainWindow *ui;
    ImageWidget *mImageWidget;
    PagesNavigator *mPagesNavigator;
    BookPagesModel *mBookPagesModel;
};
#endif // MAINWINDOW_H
