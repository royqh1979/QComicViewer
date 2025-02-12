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

#include "pagesnavigator.h"

#include <QDir>
#include <QPainter>
#include <QDebug>
#include <QSet>
#include <QImageReader>
#include "folderarchivereader.h"
#include "ziparchivereader.h"
#include "rararchivereader.h"

QList<std::shared_ptr<ArchiveReader>> PagesNavigator::mArchiveReaders;
QSet<QString> PagesNavigator::mImageSuffice;

PagesNavigator::PagesNavigator(QObject *parent) : QObject(parent),
    mCurrentPage{-1},
    mDoublePagesStart{-1},
    mDoublePagesEnd{-1},
    mDisplayDoublePages{false},
    mDisplayPagesLeftToRight{false},
    mThumbnailSize{256},
    mLoadingThumbnail{false}
{
    if (mImageSuffice.isEmpty()) {
        foreach(const QString& type, QImageReader::supportedImageFormats())
            mImageSuffice.insert(type.toLower());
        //qDebug()<<ImageSuffice;
        mArchiveReaders.append(std::make_shared<FolderArchiveReader>());
        mArchiveReaders.append(std::make_shared<RarArchiveReader>());
        mArchiveReaders.append(std::make_shared<ZipArchiveReader>());
    }
}

PagesNavigator::~PagesNavigator()
{
    emit destroyed();
}

void PagesNavigator::gotoPage(int page)
{
    setCurrentPage(page);
}

void PagesNavigator::toNextPage()
{
    int page = currentPage() + 1;
    if (mDisplayDoublePages && page>=mDoublePagesStart && page<mDoublePagesEnd) {
        page += (page - mDoublePagesStart) % 2;
    }
    setCurrentPage(page);
}

void PagesNavigator::toPrevPage()
{
    int page = currentPage()-1;
    setCurrentPage(page);
}

void PagesNavigator::toLastPage()
{
    int page = pageCount()-1;
    setCurrentPage(page);
}

void PagesNavigator::toFirstPage()
{
    setCurrentPage(0);
}

QPixmap PagesNavigator::currentImage()
{
    if (mPageList.isEmpty())
        return QPixmap();
    if (mCurrentPage == -1)
        return QPixmap();
    if (!mDisplayDoublePages) {
        return getPageImage(mCurrentPage);
    } else {
        if (mCurrentPage+1 >= pageCount()
                || mCurrentPage < mDoublePagesStart
                || mCurrentPage >= mDoublePagesEnd)
            return getPageImage(mCurrentPage);
        else {
            QPixmap image1 = getPageImage(mCurrentPage);
            QPixmap image2 = getPageImage(mCurrentPage+1);
            int width = image1.width()+image2.width();
            int height = std::max(image1.height(), image2.height());
            QPixmap img(width,height);
            QPainter painter(&img);
            painter.fillRect(0, 0, width, height, Qt::transparent);
            if (mDisplayPagesLeftToRight) {
                painter.drawPixmap(0,0,image1);
                painter.drawPixmap(image1.width(),0,image2);
            } else {
                painter.drawPixmap(0,0,image2);
                painter.drawPixmap(image2.width(),0,image1);
            }
            return img;
        }
    }
}

QString PagesNavigator::currentPageName()
{
    if (mCurrentPage<0 || mCurrentPage>=pageCount())
        return QString();
    return mPageList[mCurrentPage];
}

void PagesNavigator::loadThumbnails()
{
    PageThumbnailLoader *loader=new PageThumbnailLoader(this);
    connect(loader, &PageThumbnailLoader::thumbnailLoaded,
            this, &PagesNavigator::setThumbnail);
    connect(loader, &PageThumbnailLoader::loadFinished,
            this, &PagesNavigator::onThumbnailLoadingFinished);
    loader->start();
}


QPixmap PagesNavigator::thumbnail(int page)
{
    QMutexLocker locker(&mThumbnailMutex);
    if (pageCount()>0 && mThumbnailCache.isEmpty() && !mLoadingThumbnail) {
        mLoadingThumbnail=true;
        loadThumbnails();
    }
    QPixmap defaultThumb{mThumbnailSize,mThumbnailSize};
    return mThumbnailCache.value(page, defaultThumb);
}

QString PagesNavigator::bookTitle() const
{
    if (pageCount()<=0)
        return QString();
    QDir dir(mBookPath);
    return dir.dirName();
}

const QString &PagesNavigator::bookPath() const
{
    return mBookPath;
}

void PagesNavigator::setBookPath(QString newBookPath)
{
    QString fileName;
    QFileInfo info{newBookPath};
    bool supportPath = false;
    if (!info.exists())
        return;
    foreach (const std::shared_ptr<ArchiveReader> &archiveReader, mArchiveReaders) {
        if (archiveReader->supportArchive(newBookPath)) {
            supportPath = true;
            break;
        }
    }
    if (!supportPath) {
        newBookPath = info.absolutePath();
        fileName = info.fileName();
    }
    if (mBookPath != newBookPath) {
        mBookPath = newBookPath;

        QMutexLocker locker(&mThumbnailMutex);
        mThumbnailCache.clear();

        foreach (const std::shared_ptr<ArchiveReader> &archiveReader, mArchiveReaders) {
            if (archiveReader->supportArchive(newBookPath)) {
                mPageList = archiveReader->pageList(newBookPath, mImageSuffice);
            }
        }

        mDoublePagesStart = 0;
        mDoublePagesEnd = pageCount();
        emit bookChanged(mBookPath);
        int page = -1;
        if (!fileName.isEmpty()) {
            page = mPageList.indexOf(fileName);
        }
        int oldPage = mCurrentPage;
        if (page == -1)
            toFirstPage();
        else
            gotoPage(page);
        if (oldPage == mCurrentPage)
            emit currentImageChanged();
    }
}

const QStringList &PagesNavigator::pageList() const
{
    return mPageList;
}

int PagesNavigator::pageCount() const
{
    return mPageList.count();
}

int PagesNavigator::currentPage() const
{
    return mCurrentPage;
}

void PagesNavigator::setCurrentPage(int newCurrentPage)
{
    if (newCurrentPage<0)
        newCurrentPage = 0;
    if (newCurrentPage>=pageCount())
        newCurrentPage = pageCount()-1;
    newCurrentPage = ensureDoublePages(newCurrentPage);
    if (newCurrentPage!=mCurrentPage) {
        mCurrentPage = newCurrentPage;
        emit currentImageChanged();
    }
}

bool PagesNavigator::displayDoublePages() const
{
    return mDisplayDoublePages;
}

void PagesNavigator::setDisplayDoublePages(bool newDisplayDoublePages)
{
    if (mDisplayDoublePages!=newDisplayDoublePages) {
        mDisplayDoublePages = newDisplayDoublePages;
        setCurrentPage(currentPage());
        emit currentImageChanged();
    }
}

bool PagesNavigator::displayPagesLeftToRight() const
{
    return mDisplayPagesLeftToRight;
}

void PagesNavigator::setDisplayPagesLeftToRight(bool newDisplayPagesLeftToRight)
{
    if (mDisplayPagesLeftToRight!=newDisplayPagesLeftToRight) {
        mDisplayPagesLeftToRight = newDisplayPagesLeftToRight;
        if (mDisplayDoublePages)
            emit currentImageChanged();
    }
}

int PagesNavigator::doublePagesStart() const
{
    return mDoublePagesStart;
}

void PagesNavigator::setDoublePagesStart(int newDoublePagesStart)
{
    if (mDoublePagesStart != newDoublePagesStart) {
        mDoublePagesStart = newDoublePagesStart;
        setCurrentPage(currentPage());
        emit currentImageChanged();
    }
}

int PagesNavigator::doublePagesEnd() const
{
    return mDoublePagesEnd;
}

void PagesNavigator::setDoublePagesEnd(int newDoublePagesEnd)
{
    if (mDoublePagesEnd != newDoublePagesEnd) {
        mDoublePagesEnd = newDoublePagesEnd;
        setCurrentPage(currentPage());
        emit currentImageChanged();
    }
}

int PagesNavigator::ensureDoublePages(int page)
{
    if (mDisplayDoublePages && page>=mDoublePagesStart && page<mDoublePagesEnd) {
        page -= (page - mDoublePagesStart) % 2;
    }
    return page;
}

int PagesNavigator::thumbnailSize() const
{
    return mThumbnailSize;
}

void PagesNavigator::setThumbnailSize(int newThumbnailSize)
{
    mThumbnailSize = newThumbnailSize;
}

void PagesNavigator::setThumbnail(QString bookPath, int page, QPixmap thumbnail)
{
    if (mBookPath == bookPath){
        mThumbnailCache.insert(page, thumbnail);
        emit thumbnailReady(page);
    }
}

void PagesNavigator::onThumbnailLoadingFinished(QString bookPath)
{
    QMutexLocker locker(&mThumbnailMutex);
    if (bookPath != mBookPath) {
        mLoadingThumbnail = true;
        loadThumbnails();
    } else
        mLoadingThumbnail = false;
}

QPixmap PagesNavigator::getBookPageImage(QString bookPath, QString file)
{
    foreach (const std::shared_ptr<ArchiveReader> &archiveReader, mArchiveReaders) {
        if (archiveReader->supportArchive(bookPath)) {
            return archiveReader->pageImage(bookPath, file);
        }
    }
    return QPixmap();
}

QPixmap PagesNavigator::getPageImage(int page)
{
    return getBookPageImage(mBookPath, mPageList[page]);
}


BookPagesModel::BookPagesModel(PagesNavigator *bookNavigator, QObject *parent):
    QAbstractListModel{parent},
    mBookNavigator{bookNavigator}
{
    connect(bookNavigator, &PagesNavigator::bookChanged,
            this, &BookPagesModel::onBookChanged);
    connect(bookNavigator, &PagesNavigator::thumbnailReady,
            this, &BookPagesModel::onThumbnailReady);
}

int BookPagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mBookNavigator->pageCount();
}

QVariant BookPagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int row =index.row();
    if (row<0 || row>=mBookNavigator->pageCount())
        return QVariant();
    switch(role) {
    case Qt::DisplayRole:
        return row+1;
    case Qt::DecorationRole:
        return mBookNavigator->thumbnail(row);
    }
    return QVariant();
}

void BookPagesModel::onBookChanged(QString newBookPath)
{
    Q_UNUSED(newBookPath);
    beginResetModel();
    endResetModel();
}

void BookPagesModel::onThumbnailReady(int page)
{
    QModelIndex index = createIndex(page,0);
    emit dataChanged(index,index);
}

PageThumbnailLoader::PageThumbnailLoader(PagesNavigator *navigator, QObject *parent):
    QThread{parent}
{
    mBookPath = navigator->bookPath();
    mPageList = navigator->pageList();
    mThumbnailSize = navigator->thumbnailSize();
    mStop = false;
    connect(this, &QThread::finished,
            this, &QObject::deleteLater);
    connect(navigator, &PagesNavigator::bookChanged,
            this, &PageThumbnailLoader::onBookChanged);
    connect(navigator, &PagesNavigator::destoryed,
            this, &PageThumbnailLoader::stopLoader);
}

void PageThumbnailLoader::onBookChanged(QString newBookPath)
{
    if (newBookPath!=mBookPath)
        stopLoader();
}

void PageThumbnailLoader::stopLoader()
{
    mStop = true;
}

void PageThumbnailLoader::run()
{
    for(int i=0;i<mPageList.count();i++){
        const QString& file=mPageList[i];
        if (mStop)
            break;
        QPixmap image = PagesNavigator::getBookPageImage(mBookPath, file);
        if (mStop)
            break;
        QPixmap thumbnail;
        if (image.width()>image.height()) {
            thumbnail = image.scaledToWidth(mThumbnailSize);
        } else {
            thumbnail = image.scaledToHeight(mThumbnailSize);
        }
        emit thumbnailLoaded(mBookPath, i, thumbnail);
    }
    emit loadFinished(mBookPath);
}
