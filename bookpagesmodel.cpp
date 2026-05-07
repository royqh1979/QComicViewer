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

#include "bookpagesmodel.h"

#include <QDir>
#include <QPainter>
#include <QDebug>
#include <QSet>
#include <QImageReader>
#include <QFileSystemWatcher>
#include "folderarchivereader.h"
#include "ziparchivereader.h"
#include "rararchivereader.h"

QList<std::shared_ptr<ArchiveReader>> BookPagesModel::mArchiveReaders;
QSet<QString> BookPagesModel::mImageSuffice;

BookPagesModel::BookPagesModel(QObject *parent) : QAbstractListModel(parent),
    mDisplayPage{-1},
    mCurrentPage{-1},
    mDoublePagesStart{-1},
    mDoublePagesEnd{-1},
    mDisplayDoublePages{false},
    mDoublePagesRightToLeft{false},
    mThumbnailSize{256},
    mLoadingThumbnails{false}
{
    mFileSystemWatcher = new QFileSystemWatcher(this);
    connect(mFileSystemWatcher, &QFileSystemWatcher::directoryChanged,
            this, &BookPagesModel::onDirChanged);
    connect(mFileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &BookPagesModel::onFileChanged);
    if (mImageSuffice.isEmpty()) {
        foreach(const QString& type, QImageReader::supportedImageFormats())
            mImageSuffice.insert(type.toLower());
        //qDebug()<<ImageSuffice;
        mArchiveReaders.append(std::make_shared<FolderArchiveReader>());
        mArchiveReaders.append(std::make_shared<RarArchiveReader>());
        mArchiveReaders.append(std::make_shared<ZipArchiveReader>());
    }
}

BookPagesModel::~BookPagesModel()
{
    emit destroyed();
}

void BookPagesModel::gotoPage(int page)
{
    setCurrentPage(page);
}

void BookPagesModel::toNextPage()
{
    int page = currentPage() + 1;
    int oldPage = currentPage();
    if (mDisplayDoublePages
            && oldPage>=mDoublePagesStart && oldPage<mDoublePagesEnd
            && page>=mDoublePagesStart && page<mDoublePagesEnd)
        page+=1;
    setCurrentPage(page);
}

void BookPagesModel::toPrevPage()
{
    int page = currentPage()-1;
    int oldPage = currentPage();
    if (mDisplayDoublePages
            && oldPage>=mDoublePagesStart && oldPage<mDoublePagesEnd
            && page>=mDoublePagesStart && page<mDoublePagesEnd)
        page-=1;
    setCurrentPage(page);
}

void BookPagesModel::toLastPage()
{
    int page = pageCount()-1;
    setCurrentPage(page);
}

void BookPagesModel::toFirstPage()
{
    setCurrentPage(0);
}

QPixmap BookPagesModel::currentImage() const
{
    if (mPageList.isEmpty())
        return QPixmap();
    if (mDisplayPage == -1)
        return QPixmap();
    if (!mDisplayDoublePages) {
        return getPageImage(mDisplayPage);
    } else {
        if (mDisplayPage+1 >= pageCount()
                || mDisplayPage < mDoublePagesStart
                || mDisplayPage >= mDoublePagesEnd)
            return getPageImage(mDisplayPage);
        else {
            QPixmap image1 = getPageImage(mDisplayPage);
            QPixmap image2 = getPageImage(mDisplayPage+1);
            int width = image1.width()+image2.width();
            int height = std::max(image1.height(), image2.height());
            QPixmap img(width,height);
            QPainter painter(&img);
            painter.fillRect(0, 0, width, height, Qt::transparent);
            if (mDoublePagesRightToLeft) {
                painter.drawPixmap(0,0,image2);
                painter.drawPixmap(image2.width(),0,image1);
            } else {
                painter.drawPixmap(0,0,image1);
                painter.drawPixmap(image1.width(),0,image2);
            }
            return img;
        }
    }
}

QString BookPagesModel::currentPageName() const
{
    if (mDisplayPage<0 || mDisplayPage>=pageCount())
        return QString();
    return mPageList[mCurrentPage];
}

void BookPagesModel::loadThumbnails() const
{
    PageThumbnailsLoader *loader=new PageThumbnailsLoader(this);
    connect(loader, &PageThumbnailsLoader::thumbnailLoaded,
            this, &BookPagesModel::setThumbnail);
    connect(loader, &PageThumbnailsLoader::loadFinished,
            this, &BookPagesModel::onThumbnailsLoadingFinished);
    mLoadingThumbnails=true;
    loader->start();
}

void BookPagesModel::loadThumbnail(int page, const QString& pagePath) const
{
    DirImageThumbnailLoader *loader=new DirImageThumbnailLoader(mBookPath,page,mThumbnailSize,pagePath);
    connect(loader, &DirImageThumbnailLoader::thumbnailLoaded,
            this, &BookPagesModel::setThumbnail);
    loader->start();
}


QPixmap BookPagesModel::thumbnail(int page) const
{
    QMutexLocker locker(&mThumbnailMutex);
    if (QFileInfo{mBookPath}.isDir()) {
        if (page>=0 && page<pageCount() && !mLoadingThumbnails) {
            QString pagePath = mPageList[page];
            if (!mThumbnailCache.contains(page) || mThumbnailPagePath.value(page)!=pagePath)
                loadThumbnail(page,pagePath);
        }
    }
    QPixmap defaultThumb{mThumbnailSize,mThumbnailSize};
    return mThumbnailCache.value(page, defaultThumb);
}

QString BookPagesModel::bookTitle() const
{
    if (pageCount()<=0)
        return QString();
    QDir dir(mBookPath);
    return dir.dirName();
}

const QString &BookPagesModel::bookPath() const
{
    return mBookPath;
}

void BookPagesModel::setBookPath(QString newBookPath)
{
    QString fileName;
    QFileInfo info{newBookPath};
    bool supportPath = false;
    if (info.exists()) {
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
    }
    if (mBookPath != newBookPath) {
        if (!mBookPath.isEmpty())
            mFileSystemWatcher->removePath(mBookPath);
        mBookPath = newBookPath;
        beginResetModel();
        QMutexLocker locker(&mThumbnailMutex);
        mThumbnailCache.clear();
        mThumbnailPagePath.clear();

        mCurrentPage = -1;
        QStringList newPageList;
        foreach (const std::shared_ptr<ArchiveReader> &archiveReader, mArchiveReaders) {
            if (archiveReader->supportArchive(newBookPath)) {
                newPageList = archiveReader->pageList(newBookPath, mImageSuffice);
                break;
            }
        }
        mPageList = newPageList;

        mDoublePagesStart = 0;
        mDoublePagesEnd = pageCount();
        endResetModel();
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

        mFileSystemWatcher->addPath(mBookPath);
        loadThumbnails();
    }
}

const QStringList &BookPagesModel::pageList() const
{
    return mPageList;
}

int BookPagesModel::pageCount() const
{
    return mPageList.count();
}

int BookPagesModel::currentPage() const
{
    return mCurrentPage;
}

void BookPagesModel::setCurrentPage(int newCurrentPage)
{
    if (newCurrentPage<0)
        newCurrentPage = 0;
    if (newCurrentPage>=pageCount())
        newCurrentPage = pageCount()-1;
    if (mCurrentPage!=newCurrentPage) {
        mCurrentPage = newCurrentPage;
        emit currentPageChanged();
    }
    int newDisplayPage = ensureDoublePages(newCurrentPage);
    if (newDisplayPage!=mDisplayPage) {
        mDisplayPage = newDisplayPage;
        emit currentImageChanged();
    }
}

bool BookPagesModel::displayDoublePages() const
{
    return mDisplayDoublePages;
}

void BookPagesModel::setDisplayDoublePages(bool newDisplayDoublePages)
{
    if (mDisplayDoublePages!=newDisplayDoublePages) {
        mDisplayDoublePages = newDisplayDoublePages;
        setCurrentPage(currentPage());
        emit currentImageChanged();
    }
}

bool BookPagesModel::doublePagesRightToLeft() const
{
    return mDoublePagesRightToLeft;
}

void BookPagesModel::setDoublePagesRightToLeft(bool newRightToLeft)
{
    if (mDoublePagesRightToLeft!=newRightToLeft) {
        mDoublePagesRightToLeft = newRightToLeft;
        if (mDisplayDoublePages)
            emit currentImageChanged();
    }
}

int BookPagesModel::doublePagesStart() const
{
    return mDoublePagesStart;
}

void BookPagesModel::setDoublePagesStart(int newDoublePagesStart)
{
    if (mDoublePagesStart != newDoublePagesStart) {
        mDoublePagesStart = newDoublePagesStart;
        setCurrentPage(currentPage());
        emit currentImageChanged();
    }
}

int BookPagesModel::doublePagesEnd() const
{
    return mDoublePagesEnd;
}

void BookPagesModel::setDoublePagesEnd(int newDoublePagesEnd)
{
    if (mDoublePagesEnd != newDoublePagesEnd) {
        mDoublePagesEnd = newDoublePagesEnd;
        setCurrentPage(currentPage());
        emit currentImageChanged();
    }
}

int BookPagesModel::ensureDoublePages(int page)
{
    if (mDisplayDoublePages && page>=mDoublePagesStart && page<mDoublePagesEnd) {
        page -= (page - mDoublePagesStart) % 2;
    }
    return page;
}

void BookPagesModel::setPageList(const QStringList &newPageList)
{
    mPageList = newPageList;

    mDoublePagesStart = 0;
    mDoublePagesEnd = pageCount();
}


void BookPagesModel::clearThumbnails()
{
    QMutexLocker locker(&mThumbnailMutex);
    mThumbnailCache.clear();
    mThumbnailPagePath.clear();
    QModelIndex top=createIndex(0,0);
    QModelIndex bottom = createIndex(pageCount()-1,0);
    emit dataChanged(top,bottom);
}

int BookPagesModel::thumbnailSize() const
{
    return mThumbnailSize;
}

void BookPagesModel::setThumbnailSize(int newThumbnailSize)
{
    if (mThumbnailSize!=newThumbnailSize) {
        mThumbnailSize = newThumbnailSize;
        clearThumbnails();
        loadThumbnails();
    }
}

bool BookPagesModel::canHandle(const QString &filePath)
{
    foreach (const std::shared_ptr<ArchiveReader> &archiveReader, mArchiveReaders) {
        if (archiveReader->supportArchive(filePath)) {
            return true;
        }
    }
    QString suffix = QFileInfo(filePath).suffix().toLower();
    return mImageSuffice.contains(suffix);
}

void BookPagesModel::setThumbnail(const QString &bookPath, int page, const QString& pagePath, QPixmap thumbnail)
{
    if (mBookPath == bookPath){
        QMutexLocker locker(&mThumbnailMutex);
        mThumbnailCache.insert(page, thumbnail);
        mThumbnailPagePath.insert(page, pagePath);
        QModelIndex index = createIndex(page,0);
        emit dataChanged(index,index);
    }
}

void BookPagesModel::onThumbnailsLoadingFinished(const QString &bookPath)
{
    QMutexLocker locker(&mThumbnailMutex);
    if (bookPath == mBookPath)
        mLoadingThumbnails = false;
}

void BookPagesModel::onDirChanged(const QString &path)
{
    if (QFileInfo{path}.isDir()) {
        FolderArchiveReader reader;
        QStringList newPageList = reader.pageList(path, mImageSuffice);
        clearThumbnails();
        QString oldCurrentPagePath;
        if (mCurrentPage>=0 && mCurrentPage<mPageList.count())
            oldCurrentPagePath = mPageList[mCurrentPage];
        QStringList oldPageList = mPageList;
        int oldCurrentPage = mCurrentPage;
        if (oldCurrentPage==-1) {
            beginResetModel();
            setPageList(newPageList);
            endResetModel();
        } else {
            if (oldPageList.count() < newPageList.count()) {
                beginInsertRows(QModelIndex(),oldPageList.count()-1, newPageList.count()-1);
                setPageList(newPageList);
                endInsertRows();
            } else {
                int deleteCount = oldPageList.count()-newPageList.count();
                beginRemoveRows(QModelIndex(),0,deleteCount-1);
                setPageList(newPageList);
                endRemoveRows();
            }
        }

        int newCurrentPage = mPageList.indexOf(oldCurrentPagePath);
        if (newCurrentPage!=-1)
            gotoPage(newCurrentPage);
        else
            toFirstPage();

    }
}

void BookPagesModel::onFileChanged(const QString &path)
{
    Q_UNUSED(path);
    std::shared_ptr<ArchiveReader> reader;
    foreach (const std::shared_ptr<ArchiveReader> &archiveReader, mArchiveReaders) {
        if (archiveReader->supportArchive(mBookPath)) {
            reader = archiveReader;
            break;
        }
    }
    if (reader) {
        if (reader->archiveType()!="folder") {
            QString oldPath = mBookPath;
            mBookPath = "";
            setBookPath(oldPath);
        } else {
            QString oldPath = mBookPath;
            mBookPath = "";
            setBookPath(oldPath);
        }
    }
}

QPixmap BookPagesModel::getBookPageImage(QString bookPath, QString file)
{
    foreach (const std::shared_ptr<ArchiveReader> &archiveReader, mArchiveReaders) {
        if (archiveReader->supportArchive(bookPath)) {
            return archiveReader->pageImage(bookPath, file);
        }
    }
    return QPixmap();
}

QPixmap BookPagesModel::getPageImage(int page) const
{
    return getBookPageImage(mBookPath, mPageList[page]);
}

int BookPagesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return pageCount();
}

QVariant BookPagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int row =index.row();
    if (row<0 || row>=pageCount())
        return QVariant();
    switch(role) {
    case Qt::DisplayRole:
        return row+1;
    case Qt::DecorationRole:
        return thumbnail(row);
    }
    return QVariant();
}



PageThumbnailsLoader::PageThumbnailsLoader(const BookPagesModel *navigator, QObject *parent):
    QThread{parent}
{
    mBookPath = navigator->bookPath();
    mPageList = navigator->pageList();
    mThumbnailSize = navigator->thumbnailSize();
    mStop = false;
    connect(this, &QThread::finished,
            this, &QObject::deleteLater);
    connect(navigator, &BookPagesModel::bookChanged,
            this, &PageThumbnailsLoader::onBookChanged);
    connect(navigator, &BookPagesModel::destoryed,
            this, &PageThumbnailsLoader::stopLoader);
}

void PageThumbnailsLoader::onBookChanged(const QString &newBookPath)
{
    if (newBookPath!=mBookPath)
        stopLoader();
}

void PageThumbnailsLoader::stopLoader()
{
    mStop = true;
}

void PageThumbnailsLoader::run()
{
    for(int i=0;i<mPageList.count();i++){
        const QString& pagePath=mPageList[i];
        if (mStop)
            break;

        QPixmap image = BookPagesModel::getBookPageImage(mBookPath, pagePath);
        if (mStop)
            break;
        QPixmap thumbnail;
        if (image.width()>image.height()) {
            thumbnail = image.scaledToWidth(mThumbnailSize);
        } else {
            thumbnail = image.scaledToHeight(mThumbnailSize);
        }
        emit thumbnailLoaded(mBookPath, i, pagePath, thumbnail);
    }
    emit loadFinished(mBookPath);
}

DirImageThumbnailLoader::DirImageThumbnailLoader(const QString &bookPath, int page, int thumbnailSize,const QString &pagePath, QObject *parent):
    QThread{parent}
{
    mBookPath = bookPath;
    mThumbnailSize = thumbnailSize;
    mPage = page;
    mPagePath = pagePath;
    connect(this, &QThread::finished,
            this, &QObject::deleteLater);
}

void DirImageThumbnailLoader::run()
{
    FolderArchiveReader reader;
    QPixmap image;
    int delayCount = 0;
    while (true) {
        image = reader.pageImage(mBookPath, mPagePath);
        if (!image.isNull())
            break;
        delayCount++;
        if (delayCount>10) {
            return;
        }
        QThread::msleep(100);
    }
    QPixmap thumbnail;
    if (image.width()>image.height()) {
        thumbnail = image.scaledToWidth(mThumbnailSize);
    } else {
        thumbnail = image.scaledToHeight(mThumbnailSize);
    }
    emit thumbnailLoaded(mBookPath, mPage, mPagePath, thumbnail);
}
