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
#include "settingsdialog/settingsdialog.h"
#include "settings.h"
#include <QStyleFactory>
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mPageInfo = new QLabel(this);
    mPageInfo->setText(" ");
    ui->statusbar->addPermanentWidget(mPageInfo);
    mZoomFactor = new QSpinBox(this);
    mZoomFactor->setSuffix("%");
    mZoomFactor->setRange(1,1000);
    connect(mZoomFactor, qOverload<int>(&QSpinBox::valueChanged),
            this, &MainWindow::onZoomFactorChanged);
    ui->statusbar->addPermanentWidget(mZoomFactor);
    mImageSizeInfo = new QLabel(this);
    mImageSizeInfo->setText(" ");
    ui->statusbar->addPermanentWidget(mImageSizeInfo);


    QHBoxLayout *layout = new QHBoxLayout(ui->centralwidget);
    layout->setMargin(0);
    mImageWidget = new ImageWidget(ui->centralwidget);
    layout->addWidget(mImageWidget);
    connect(mImageWidget, &ImageWidget::requestNextImage,
            this, &MainWindow::on_actionNext_Page_triggered);
    connect(mImageWidget, &ImageWidget::requestPrevImage,
            this, &MainWindow::gotoPrevPage);
    connect(mImageWidget, &ImageWidget::imageUpdated,
            this, &MainWindow::updateStatusBar);
    connect(mImageWidget, &QWidget::customContextMenuRequested,
            this , &MainWindow::onImageWidgetContextMenuRequested);
    mImageWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    mPagesNavigator = new PagesNavigator(this);
    connect(mPagesNavigator, &PagesNavigator::currentImageChanged,
            this, &MainWindow::onCurrentPageChanged);
    connect(mPagesNavigator, &PagesNavigator::currentPageChanged,
            this, &MainWindow::updateStatusBar);

    mBookPagesModel = new BookPagesModel(mPagesNavigator, this);
    ui->pagesView->setModel(mBookPagesModel);
    connect(ui->pagesView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onPageViewCurrentChanged);

    ui->actionPrev_Page->setShortcuts({
                                          tr("PgUp"),
                                      });
    ui->actionNext_Page->setShortcuts({
                                          tr("PgDown"),
                                          tr("Space")
                                      });
    QActionGroup *fitActionGroup = new QActionGroup(this);
    fitActionGroup->addAction(ui->actionFit_Width);
    fitActionGroup->addAction(ui->actionFit_Height);
    fitActionGroup->addAction(ui->actionFit_Page);
    fitActionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
    //updateImageFitType();

    connect(ui->actionFit_Width, &QAction::triggered,
            this, &MainWindow::updateImageFitType);
    connect(ui->actionFit_Height, &QAction::triggered,
            this, &MainWindow::updateImageFitType);
    connect(ui->actionFit_Page, &QAction::triggered,
            this, &MainWindow::updateImageFitType);

    QActionGroup *pageModeActionGroup = new QActionGroup(this);
    pageModeActionGroup->addAction(ui->actionSingle_Pages);
    pageModeActionGroup->addAction(ui->actionDouble_Pages);
    pageModeActionGroup->addAction(ui->actionDouble_Pages_with_Front_Cover);
    ui->actionSingle_Pages->setChecked(true);

    connect(ui->actionSingle_Pages, &QAction::triggered,
            this, &MainWindow::updatePageMode);
    connect(ui->actionDouble_Pages, &QAction::triggered,
            this, &MainWindow::updatePageMode);
    connect(ui->actionDouble_Pages_with_Front_Cover, &QAction::triggered,
            this, &MainWindow::updatePageMode);

    ui->actionRight_to_Left->setChecked(true);

    setWindowIcon(QPixmap(":/icons/comic-book.png"));
    setWindowTitle(tr("QComicsViewer %1").arg(APP_VERSION));
    resize(pSettings->ui().mainWindowWidth(), pSettings->ui().mainWindowHeight());
    move(pSettings->ui().mainWindowLeft(), pSettings->ui().mainWindowTop());
    resizeDocks({ui->dockPages},{pSettings->ui().contentsPanelWidth()},Qt::Orientation::Horizontal);
    ui->dockPages->setVisible(pSettings->ui().showContentsPanel());

    qApp->setStyle(QStyleFactory::create("fusion"));

    setAcceptDrops(true);
    applySettings();

    mInFullScreen = false;
    mMaximizedBeforeFullScreen = false;

    addActions(menuBar()->actions());
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::openBook(const QString &bookPath)
{
    if (QFile::exists(bookPath)) {
        mPagesNavigator->setBookPath(bookPath);
    } else {
        mPagesNavigator->setBookPath("");
    }
    return true;
}

void MainWindow::applySettings()
{
    QFont font{pSettings->ui().fontName(), pSettings->ui().fontSize()};
    qApp->setFont(font);
    setFont(font);

    if (pSettings->view().pageMode() == "DoublePages")
        ui->actionDouble_Pages->setChecked(true);
    else if (pSettings->view().pageMode() == "DoublePagesWithCover")
        ui->actionDouble_Pages_with_Front_Cover->setChecked(true);
    else
        ui->actionSingle_Pages->setChecked(true);
    ui->actionFit_Width->setChecked(pSettings->view().fitMode() == "Width");
    ui->actionFit_Height->setChecked(pSettings->view().fitMode() == "Height");
    ui->actionFit_Page->setChecked(pSettings->view().fitMode() == "Page");

    updateImageFitType();
    updatePageMode();
    ui->actionRight_to_Left->setChecked(pSettings->view().rightToLeft());
    ui->actionSwap_Left_Right_Key->setChecked(pSettings->view().swapLeftRightKey());
    mPagesNavigator->setThumbnailSize(pSettings->view().thumbnailSize());
}

void MainWindow::updateStatusBar()
{
    if (mPagesNavigator->pageCount()<=0) {
        mZoomFactor->blockSignals(true);
        mZoomFactor->setValue(100);
        mZoomFactor->blockSignals(false);
        mImageSizeInfo->setText(" ");
        mPageInfo->setText(" ");
        ui->statusbar->clearMessage();

        ui->actionClose->setEnabled(false);
        ui->actionFirst_Page->setEnabled(false);
        ui->actionLast_Page->setEnabled(false);
        ui->actionNext_Page->setEnabled(false);
        ui->actionPrev_Page->setEnabled(false);
        ui->actionRotate_90_Clockwise->setEnabled(false);
        ui->actionRotate_90_Counter_Clockwise->setEnabled(false);
        ui->actionHorizontal_Flip->setEnabled(false);
        ui->actionVertical_Flip->setEnabled(false);
    } else {
        mZoomFactor->blockSignals(true);
        mZoomFactor->setValue(mImageWidget->ratio()*100);
        mZoomFactor->blockSignals(false);
        mImageSizeInfo->setText(QString(" %1x%2 ").arg(mImageWidget->imageSize().width()).arg(mImageWidget->imageSize().height()));
        mPageInfo->setText(QString("%1/%2").arg(mPagesNavigator->currentPage()+1)
                               .arg(mPagesNavigator->pageCount()));
        ui->statusbar->showMessage(mPagesNavigator->currentPageName());

        ui->actionClose->setEnabled(true);
        ui->actionFirst_Page->setEnabled(mPagesNavigator->currentPage()!=0);
        ui->actionLast_Page->setEnabled(mPagesNavigator->currentPage()!=mPagesNavigator->pageCount()-1);
        ui->actionPrev_Page->setEnabled(mPagesNavigator->currentPage()!=0);
        ui->actionNext_Page->setEnabled(mPagesNavigator->currentPage()!=mPagesNavigator->pageCount()-1);
        ui->actionRotate_90_Clockwise->setEnabled(!mImageWidget->image().isNull());
        ui->actionRotate_90_Counter_Clockwise->setEnabled(!mImageWidget->image().isNull());
        ui->actionHorizontal_Flip->setEnabled(!mImageWidget->image().isNull());
        ui->actionVertical_Flip->setEnabled(!mImageWidget->image().isNull());

    }
}

void MainWindow::onImageWidgetContextMenuRequested(const QPoint &pos)
{
    QMenu *menu=new QMenu(this);
    menu->addAction(ui->actionFull_Screen);
    menu->addSeparator();
    menu->addAction(ui->actionSingle_Pages);
    menu->addAction(ui->actionDouble_Pages);
    menu->addAction(ui->actionDouble_Pages_with_Front_Cover);
    menu->addAction(ui->actionRight_to_Left);
    menu->addSeparator();
    menu->addAction(ui->actionFit_Page);
    menu->addAction(ui->actionFit_Width);
    menu->addAction(ui->actionFit_Height);
    menu->popup(mImageWidget->mapToGlobal(pos));
}

void MainWindow::onPageViewCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!current.isValid())
        return;
    mPagesNavigator->gotoPage(current.row());
}

void MainWindow::onZoomFactorChanged(int value)
{
    mImageWidget->setRatio(value/100.0);
}

void MainWindow::updatePageMode()
{
    if (ui->actionSingle_Pages->isChecked()) {
        mPagesNavigator->setDisplayDoublePages(false);
    } else if (ui->actionDouble_Pages->isChecked()) {
        mPagesNavigator->setDisplayDoublePages(true);
        mPagesNavigator->setDoublePagesStart(0);
    } else if (ui->actionDouble_Pages_with_Front_Cover->isChecked()) {
        mPagesNavigator->setDisplayDoublePages(true);
        mPagesNavigator->setDoublePagesStart(1);
    }
}

void MainWindow::onCurrentPageChanged()
{
    QPixmap image = mPagesNavigator->currentImage();
    mImageWidget->setImage(image);
    if (mPagesNavigator->pageCount()>0) {
        QModelIndex index = mBookPagesModel->index(mPagesNavigator->currentPage(),0);
        ui->pagesView->setCurrentIndex(index);
        ui->pagesView->scrollTo(index);
    }
    updateStatusBar();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    pSettings->ui().setShowContentsPanel(ui->actionShow_Contents->isChecked());
    pSettings->ui().setMainWindowWidth(width());
    pSettings->ui().setMainWindowHeight(height());
    pSettings->ui().setMainWindowLeft(pos().x());
    pSettings->ui().setMainWindowTop(pos().y());
    pSettings->ui().setContentsPanelWidth(ui->dockPages->width());
    if (ui->actionDouble_Pages->isChecked())
        pSettings->view().setPageMode("DoublePages");
    else if (ui->actionDouble_Pages_with_Front_Cover->isChecked())
        pSettings->view().setPageMode("DoublePagesWithCover");
    else
        pSettings->view().setPageMode("SinglePage");
    if (ui->actionFit_Width->isChecked())
        pSettings->view().setFitMode("Width");
    else if (ui->actionFit_Height->isChecked())
        pSettings->view().setFitMode("Height");
    else if (ui->actionFit_Page->isChecked())
        pSettings->view().setFitMode("Page");
    else
        pSettings->view().setFitMode("None");

    pSettings->view().setRightToLeft(ui->actionRight_to_Left->isChecked());
    pSettings->view().setSwapLeftRightKey(ui->actionSwap_Left_Right_Key->isChecked());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->urls().count()==1) {
        QList<QUrl> urlList = mimeData->urls();
        QString fileName = urlList.first().toLocalFile();
        if (mPagesNavigator->canHandle(fileName))
            event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->urls().count()==1) {
        QList<QUrl> urlList = mimeData->urls();
        QString fileName = urlList.first().toLocalFile();
        if (mPagesNavigator->canHandle(fileName)) {
            openBook(fileName);
            event->acceptProposedAction();
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && event->modifiers() == Qt::NoModifier
            && mInFullScreen) {
        on_actionFull_Screen_triggered();
    }
}

void MainWindow::on_actionNext_Page_triggered()
{
    mPagesNavigator->toNextPage();
}


void MainWindow::on_actionPrev_Page_triggered()
{
    gotoPrevPage(false);
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
    openBook(file);
    if (mPagesNavigator->pageCount()>0) {
        updatePageMode();
        setWindowTitle(tr("QComicsViewer %1 [%2]").arg(APP_VERSION).arg(mPagesNavigator->bookTitle()));
    } else {
        setWindowTitle(tr("QComicsViewer %1").arg(APP_VERSION));
    }
}

void MainWindow::updateImageFitType()
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

void MainWindow::gotoPrevPage(bool scrollToPageBottom)
{
    mPagesNavigator->toPrevPage();
    if (scrollToPageBottom) {
        mImageWidget->scrollToBottom();
    }
}


void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}


void MainWindow::on_actionRight_to_Left_toggled(bool arg1)
{
    Q_UNUSED(arg1);
    mPagesNavigator->setDoublePagesRightToLeft(ui->actionRight_to_Left->isChecked());
}


void MainWindow::on_dockPages_visibilityChanged(bool visible)
{
    ui->actionShow_Contents->blockSignals(true);
    ui->actionShow_Contents->setChecked(visible);
    ui->actionShow_Contents->blockSignals(false);
}

void MainWindow::on_actionShow_Contents_toggled(bool arg1)
{
    Q_UNUSED(arg1);
    ui->dockPages->setVisible(ui->actionShow_Contents->isChecked());
}



void MainWindow::on_actionOptions_triggered()
{
    PSettingsDialog optionDlg = SettingsDialog::optionDialog(this);
    connect(optionDlg.get(), &SettingsDialog::settingsChanged,
            this, &MainWindow::applySettings);
    optionDlg->exec();
}


void MainWindow::on_actionSwap_Left_Right_Key_toggled(bool arg1)
{
    Q_UNUSED(arg1);
    mImageWidget->setSwapLeftRightWhenTurnPage(ui->actionSwap_Left_Right_Key->isChecked());
}


void MainWindow::on_actionRotate_90_Clockwise_triggered()
{
    mImageWidget->rotate(90);
}


void MainWindow::on_actionRotate_90_Counter_Clockwise_triggered()
{
    mImageWidget->rotate(-90);
}


void MainWindow::on_actionHorizontal_Flip_triggered()
{
    mImageWidget->horizontalFlip();
}


void MainWindow::on_actionVertical_Flip_triggered()
{
    mImageWidget->verticalFlip();
}


void MainWindow::on_actionClose_triggered()
{
    mPagesNavigator->setBookPath("");
}


void MainWindow::on_actionFull_Screen_triggered()
{
    if (mInFullScreen) {
        ui->dockPages->blockSignals(true);
        if (ui->dockPages->isVisible()!=ui->actionShow_Contents->isChecked())
            ui->dockPages->setVisible(ui->actionShow_Contents->isChecked());
        ui->dockPages->blockSignals(false);
        ui->toolBar->setVisible(true);
        ui->menubar->setVisible(true);
        ui->statusbar->setVisible(true);
        if (mMaximizedBeforeFullScreen)
            showMaximized();
        else
            showNormal();
        mInFullScreen = false;
    } else {
        mMaximizedBeforeFullScreen = isMaximized();
        ui->dockPages->blockSignals(true);
        ui->dockPages->setVisible(false);
        ui->dockPages->blockSignals(false);
        ui->toolBar->setVisible(false);
        ui->menubar->setVisible(false);
        ui->statusbar->setVisible(false);
        showFullScreen();
        mInFullScreen = true;
    }
}
