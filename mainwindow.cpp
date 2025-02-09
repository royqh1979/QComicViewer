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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QStyle>
#include "imagewidget.h"
#include "pagesnavigator.h"
#include "aboutdialog.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QHBoxLayout *layout = new QHBoxLayout(ui->centralwidget);
    layout->setMargin(0);
    mImageWidget = new ImageWidget(ui->centralwidget);
    layout->addWidget(mImageWidget);
    connect(mImageWidget, &ImageWidget::requestNextImage,
            this, &MainWindow::on_actionNext_Page_triggered);
    connect(mImageWidget, &ImageWidget::requestPrevImage,
            this, &MainWindow::on_actionPrev_Page_triggered);

    mPagesNavigator = new PagesNavigator(this);
    connect(mPagesNavigator, &PagesNavigator::currentImageChanged,
            this, &MainWindow::onCurrentPageChanged);

    mBookPagesModel = new BookPagesModel(mPagesNavigator, this);
    ui->pagesView->setModel(mBookPagesModel);
    connect(ui->pagesView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onPageViewCurrentChanged);

    ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->actionPrev_Page->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    ui->actionNext_Page->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
    ui->actionFirst_Page->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->actionLast_Page->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->actionShow_Contents->setIcon(style()->standardIcon(QStyle::SP_DesktopIcon));
    ui->actionPrev_Page->setShortcuts({
                                          tr("PgUp"),
                                          tr("Left"),
                                          tr("Up")
                                      });
    ui->actionNext_Page->setShortcuts({
                                          tr("PgDown"),
                                          tr("Right"),
                                          tr("Down"),
                                          tr("Space"),
                                          tr("Enter"),
                                          tr("Return")
                                      });
    QActionGroup *fitActionGroups = new QActionGroup(this);
    fitActionGroups->addAction(ui->actionFit_Width);
    fitActionGroups->addAction(ui->actionFit_Height);
    fitActionGroups->addAction(ui->actionFit_Page);
    fitActionGroups->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
    ui->actionFit_Page->setChecked(true);

    ui->actionShow_Double_Pages->setChecked(true);
    ui->actionLeft_To_Right->setChecked(false);

    setWindowIcon(QPixmap(":/icons/comic-book.png"));
    setWindowTitle(tr("QComicsViewer %1").arg(APP_VERSION));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onPageViewCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (!current.isValid())
        return;
    mPagesNavigator->gotoPage(current.row());
}

void MainWindow::onCurrentPageChanged()
{
    QModelIndex index = mBookPagesModel->index(mPagesNavigator->currentPage(),0);
    ui->pagesView->selectionModel()->select(index,
                                            QItemSelectionModel::SelectionFlag::Select | QItemSelectionModel::SelectionFlag::Current);
    ui->pagesView->scrollTo(index);
    QPixmap image = mPagesNavigator->currentImage();
    mImageWidget->setImage(image);
    if (mPagesNavigator->pageCount()<=0)
        ui->statusbar->clearMessage();
    else
        ui->statusbar->showMessage(QString("%1/%2").arg(mPagesNavigator->currentPage()+1)
                               .arg(mPagesNavigator->pageCount()));
}

void MainWindow::on_actionShow_Double_Pages_toggled(bool arg1)
{
    mPagesNavigator->setDisplayDoublePages(arg1);
}


void MainWindow::on_actionNext_Page_triggered()
{
    mPagesNavigator->toNextPage();
}


void MainWindow::on_actionPrev_Page_triggered()
{
    mPagesNavigator->toPrevPage();
}


void MainWindow::on_actionLast_Page_triggered()
{
    mPagesNavigator->toLastPage();
}


void MainWindow::on_actionFirst_Page_triggered()
{
    mPagesNavigator->toFirstPage();
}


void MainWindow::on_actionExit_triggered()
{
    close();
}


void MainWindow::on_actionOpen_triggered()
{
    QString file = QFileDialog::getOpenFileName(
                this, tr("Open File/Folder"));
    if (QFile::exists(file)) {
        mPagesNavigator->setBookPath(file);
    } else {
        mPagesNavigator->setBookPath("");
    }
    if (mPagesNavigator->pageCount()>0) {
        mPagesNavigator->setDoublePagesStart(ui->actionSingle_First_Page->isChecked()?1:0);
        setWindowTitle(tr("QComicsViewer %1 [%2]").arg(APP_VERSION).arg(mPagesNavigator->bookTitle()));
    } else {
        setWindowTitle(tr("QComicsViewer %1").arg(APP_VERSION));
    }
}


void MainWindow::on_actionFit_Width_toggled(bool arg1)
{
    Q_UNUSED(arg1);
    setImageFitType();
}

void MainWindow::setImageFitType()
{
    if (ui->actionFit_Height->isChecked())
        mImageWidget->setFitType(ImageWidget::AutoFitType::Height);
    else if (ui->actionFit_Width->isChecked())
        mImageWidget->setFitType(ImageWidget::AutoFitType::Width);
    else if (ui->actionFit_Page->isChecked())
        mImageWidget->setFitType(ImageWidget::AutoFitType::Page);
    else
        mImageWidget->setFitType(ImageWidget::AutoFitType::None);
}


void MainWindow::on_actionFit_Height_toggled(bool arg1)
{
    Q_UNUSED(arg1);
    setImageFitType();
}


void MainWindow::on_actionFit_Page_toggled(bool arg1)
{
    Q_UNUSED(arg1);
    setImageFitType();
}

void MainWindow::on_actionSingle_First_Page_toggled(bool arg1)
{
    Q_UNUSED(arg1);
    if (ui->actionSingle_First_Page->isChecked())
        mPagesNavigator->setDoublePagesStart(1);
    else
        mPagesNavigator->setDoublePagesStart(0);
}


void MainWindow::on_actionLeft_To_Right_toggled(bool arg1)
{
    Q_UNUSED(arg1);
    mPagesNavigator->setDisplayPagesLeftToRight(ui->actionLeft_To_Right->isChecked());
}


void MainWindow::on_actionShow_Contents_triggered()
{
    ui->dockPages->show();
}


void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}

