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
#include <QMimeData>
#include <QTimer>
#include <QSemaphore>
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
    mAutoSinglePage{false},
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
    int oldPage = calcDisplayPage(currentPage());
    int page = calcDisplayPage(currentPage()+1);
    if (page == oldPage)
        page = currentPage()+2;
    setCurrentPage(page);
}

void BookPagesModel::toPrevPage()
{
    int oldPage = calcDisplayPage(currentPage());
    int page = calcDisplayPage(currentPage()-1);
    if (page == oldPage)
        page = currentPage()-2;
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
    QMutexLocker locker{&mMutex};
    if (mPageList.isEmpty())
        return QPixmap();
    if (mDisplayPage == -1)
        return QPixmap();
    if (!mDisplayDoublePages
            || (mAutoSinglePage &&
                (isPageShouldSingle(mDisplayPage)
                 || isPageShouldSingle(mDisplayPage+1)))) {
        return getPageImage(mDisplayPage);
    } else {
        if (mDisplayPage+1 >= pageCount()
                || mDisplayPage < mDoublePagesStart
                || mDisplayPage >= mDoublePagesEnd)
            return getPageImage(mDisplayPage);
        else {
            QPixmap image1 = getPageImage(mDisplayPage);
            QPixmap image2 = getPageImage(mDisplayPage+1);
            int height = std::max(image1.height(), image2.height());
            if (image1.height() < height)
                image1 = image1.scaledToHeight(height);
            if (image2.height() < height)
                image2 = image2.scaledToHeight(height);
            int width = image1.width()+image2.width();
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
    QMutexLocker locker{&mMutex};
    return mPageList[mCurrentPage];
}

QString BookPagesModel::imagePageNames() const
{
    if (mDisplayPage<0 || mDisplayPage>=pageCount())
        return QString();
    QMutexLocker locker{&mMutex};
    if (mDisplayPage+1 >= pageCount()
            || mDisplayPage < mDoublePagesStart
            || mDisplayPage >= mDoublePagesEnd) {
        return mPageList[mDisplayPage];
    } else {
        if (mDisplayPage == mCurrentPage)
            return mPageList[mDisplayPage] + "*/" + mPageList[mDisplayPage+1];
        else
            return mPageList[mDisplayPage] + "/" + mPageList[mDisplayPage+1] + "*";
    }
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
    QMutexLocker locker(&mMutex);
    QPixmap defaultThumb;
    if (page<0 || page>=pageCount())
        return defaultThumb;
    QString pagePath = mPageList[page];
    if (QFileInfo{mBookPath}.isDir()) {
        if (!mLoadingThumbnails) {
            if (!mThumbnailCache.contains(pagePath))
                loadThumbnail(page,pagePath);
        }
    }
    return mThumbnailCache.value(pagePath, defaultThumb);
}

QString BookPagesModel::pagePath(int page) const
{
    QMutexLocker locker(&mMutex);
    return mPageList[page];
}

QString BookPagesModel::bookTitle() const
{
    if (pageCount()<=0)
        return QString();
    QFileInfo info(mBookPath);
    return info.fileName();
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
        QMutexLocker locker(&mMutex);
        mThumbnailCache.clear();
        mIsPageShouldSingle.clear();

        mCurrentPage = -1;
        QStringList newPageList;
        foreach (const std::shared_ptr<ArchiveReader> &archiveReader, mArchiveReaders) {
            if (archiveReader->supportArchive(newBookPath)) {
                newPageList = archiveReader->pageList(newBookPath, mImageSuffice);
                break;
            }
        }
        mPageList = newPageList;

        //mDoublePagesStart = 0;
        mDoublePagesEnd = pageCount();
        endResetModel();
        emit bookChanged(mBookPath);

        if (!mBookPath.isEmpty())
            mFileSystemWatcher->addPath(mBookPath);
        loadThumbnails();
    }
    if (!fileName.isEmpty()){
        int page = mPageList.indexOf(fileName);
        int oldPage = mCurrentPage;

        if (page == -1)
            toFirstPage();
        else
            gotoPage(page);
        if (oldPage == mCurrentPage)
            emit currentImageChanged();
    } else {
        toFirstPage();
        emit currentImageChanged();
    }
}

QStringList BookPagesModel::pageList() const
{
    QMutexLocker locker{&mMutex};
    return mPageList;
}

int BookPagesModel::pageCount() const
{
    QMutexLocker locker{&mMutex};
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
    int newDisplayPage = calcDisplayPage(newCurrentPage);
    if (mAutoSinglePage) {
        QPixmap pageThumb = thumbnail(newCurrentPage);
        if (!pageThumb.isNull() && pageThumb.width()>pageThumb.height()) {
            newDisplayPage = newCurrentPage;
        }
    }
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

int BookPagesModel::calcDisplayPage(int page)
{
    if (mDisplayDoublePages && page>=mDoublePagesStart && page<mDoublePagesEnd) {
        int p=mDoublePagesStart;
        while (p<page) {
            if (mAutoSinglePage &&
                    (isPageShouldSingle(p)
                    || isPageShouldSingle(p+1)))
                p+=1;
            else if (p+2<=page)
                p+=2;
            else
                return p;
        }
    }
    return page;
}

bool BookPagesModel::isPageShouldSingle(int page) const
{
    QMutexLocker locker(&mMutex);
    return mIsPageShouldSingle.value(page,false);
}

void BookPagesModel::setPageList(const QStringList &newPageList)
{
    QMutexLocker locker{&mMutex};
    mPageList = newPageList;

    mDoublePagesStart = 0;
    mDoublePagesEnd = pageCount();
}


void BookPagesModel::clearThumbnails()
{
    QMutexLocker locker(&mMutex);
    mThumbnailCache.clear();
    mIsPageShouldSingle.clear();
    QModelIndex top=createIndex(0,0);
    QModelIndex bottom = createIndex(pageCount()-1,0);
    emit dataChanged(top,bottom);
}

bool BookPagesModel::autoSinglePage() const
{
    return mAutoSinglePage;
}

void BookPagesModel::setAutoSinglePage(bool newAutoSinglePage)
{
    mAutoSinglePage = newAutoSinglePage;
}

QStringList BookPagesModel::mimeTypes() const
{
    return QStringList{"text/uri-list"};
}

QMimeData *BookPagesModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mime = new QMimeData();
    QDir dir{mBookPath};
    if (dir.exists()) {
        QStringList texts;
        QList<QUrl> urls;
        for (const QModelIndex &idx : indexes) {
            QString path = dir.absoluteFilePath(pagePath(idx.row()));
            texts << path;
            urls.append(QUrl::fromLocalFile(path));
        }
        mime->setUrls(urls);
        mime->setText(texts.join("\n"));
    }
    return mime;

}

Qt::ItemFlags BookPagesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    if (!QFileInfo{mBookPath}.isDir())
        return QAbstractListModel::flags(index);
    return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled;
}

Qt::DropActions BookPagesModel::supportedDragActions() const
{
    return Qt::CopyAction;
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
        QMutexLocker locker(&mMutex);
        mThumbnailCache.insert(pagePath, thumbnail);
        mIsPageShouldSingle.insert(page,thumbnail.width()>thumbnail.height()*1.2);
        QModelIndex index = createIndex(page,0);
        emit dataChanged(index,index);
    }
}

void BookPagesModel::onThumbnailsLoadingFinished(const QString &bookPath)
{
    QMutexLocker locker(&mMutex);
    if (bookPath == mBookPath)
        mLoadingThumbnails = false;
}

void BookPagesModel::onDirChanged(const QString &path)
{
    if (QFileInfo{path}.isDir()) {
        QMutexLocker locker{&mMutex};
        FolderArchiveReader reader;
        QStringList newPageList = reader.pageList(path, mImageSuffice);
        QMap<QString, QPixmap> oldThumbnails = mThumbnailCache;
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
        for(int i=0;i<mPageList.count();i++) {
            QString pagePath = mPageList[i];
            QPixmap thumbnail = oldThumbnails.value(pagePath);
            if (!thumbnail.isNull()) {
                mThumbnailCache.insert(pagePath,thumbnail);
                mIsPageShouldSingle.insert(i,thumbnail.width()>thumbnail.height()*1.2);
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
    QMutexLocker locker{&mMutex};
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
    case Qt::ToolTipRole:
        return pagePath(row);
    case Qt::DisplayRole:
        return QFileInfo{pagePath(row)}.fileName();
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

static QSemaphore thumbnailLoaderSemaphore{5};
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
        thumbnailLoaderSemaphore.acquire(1);
        image = reader.pageImage(mBookPath, mPagePath);
        thumbnailLoaderSemaphore.release(1);
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
