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
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "qtrar/qtrar.h"
#include "qtrar/qtrarfile.h"

static QSet<QString> ImageSuffice;

static QStringList readDir(const QString &path)
{
    QDir dir{path};
    QStringList result;
    QStringList fileList = dir.entryList(QDir::Filter::Files);
    foreach(const QString &file, dir.entryList(QDir::Filter::Files)) {
        QFileInfo info{file};
        QString suffix = info.suffix().toLower();
        if (ImageSuffice.contains(suffix))
            result.append(file);
    }
    result.sort(Qt::CaseInsensitive);
    return result;
}

static QStringList readZip(const QString& path) {
    QStringList result;
    QuaZip zip{path};
    if (zip.open(QuaZip::mdUnzip)) {
        QStringList fileList = zip.getFileNameList();
        foreach(const QString &file, fileList) {
            QFileInfo info{file};
            QString suffix = info.suffix().toLower();
            if (ImageSuffice.contains(suffix))
                result.append(file);
        }
        zip.close();
    }
    result.sort(Qt::CaseInsensitive);
    return result;
}

static QStringList readRar(const QString& path) {
    QStringList result;
    QtRAR archive{path};
    if (archive.open(QtRAR::OpenModeList)) {
        QStringList fileList = archive.fileNameList();
        foreach(const QString &file, fileList) {
            QFileInfo info{file};
            QString suffix = info.suffix().toLower();
            if (ImageSuffice.contains(suffix))
                result.append(file);
        }
        archive.close();
    }
    result.sort(Qt::CaseInsensitive);
    return result;
}

PagesNavigator::PagesNavigator(QObject *parent) : QObject(parent),
    mBookType{BookType::Folder},
    mCurrentPage{-1},
    mDoublePagesStart{-1},
    mDoublePagesEnd{-1},
    mDisplayDoublePages{false},
    mDisplayPagesLeftToRight{false},
    mThumbnailSize{256},
    mLoadingThumbnail{false}
{
    if (ImageSuffice.isEmpty()) {
        foreach(const QString& type, QImageReader::supportedImageFormats())
            ImageSuffice.insert(type.toLower());
        //qDebug()<<ImageSuffice;
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
    if (info.exists()
            && info.isFile()
            && !newBookPath.endsWith(".zip")
            && !newBookPath.endsWith(".rar")) {

        newBookPath = info.absolutePath();
        fileName = info.fileName();
    }
    if (mBookPath != newBookPath) {
        mBookPath = newBookPath;

        QMutexLocker locker(&mThumbnailMutex);
        mThumbnailCache.clear();

        if (QFileInfo::exists(mBookPath)) {
            if (QFileInfo{mBookPath}.isDir()) {
                mPageList = readDir(mBookPath);
                mBookType = BookType::Folder;
            } else if (mBookPath.endsWith(".zip")) {
                mBookType = BookType::Zip;
                mPageList = readZip(mBookPath);
            } else if (mBookPath.endsWith(".rar")) {
                mBookType = BookType::RAR;
                mPageList = readRar(mBookPath);
            }
        }
        mDoublePagesStart = 0;
        mDoublePagesEnd = pageCount();
        emit bookChanged(mBookPath);
        int page = -1;
        if (!fileName.isEmpty()) {
            page = mPageList.indexOf(fileName);
        }
        if (page == -1)
            toFirstPage();
        else
            gotoPage(page);
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

PagesNavigator::BookType PagesNavigator::bookType() const
{
    return mBookType;
}

QPixmap PagesNavigator::getBookPageImage(QString bookPath, QString file, BookType bookType)
{
    if (bookType==BookType::Folder) {
        QDir dir{bookPath};
        QString imagePath = dir.absoluteFilePath(file);
        return QPixmap(imagePath);
    } else if (bookType == BookType::Zip) {
        QuaZip zip{bookPath};
        if (!zip.open(QuaZip::mdUnzip))
            return QPixmap();
        if (!zip.setCurrentFile(file))
            return QPixmap();
        QuaZipFile unzipfile(&zip);
        if (!unzipfile.open(QIODevice::ReadOnly))
            return QPixmap();
        QImageReader reader{&unzipfile};
        QPixmap result = QPixmap::fromImageReader(&reader);
        unzipfile.close();
        zip.close();
        return result;
    } else if (bookType == BookType::RAR) {
        QtRAR archive{bookPath};
        if (!archive.open(QtRAR::OpenModeExtract))
            return QPixmap();
        QtRARFile unrarFile{&archive};
        unrarFile.setFileName(file);
        if (!unrarFile.open(QIODevice::ReadOnly))
            return QPixmap();
        QImageReader reader{&unrarFile};
        QPixmap result = QPixmap::fromImageReader(&reader);
        unrarFile.close();
        archive.close();
        return result;
    }
    return QPixmap();
}

QPixmap PagesNavigator::getPageImage(int page)
{
    return getBookPageImage(mBookPath, mPageList[page], mBookType);
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
        return row;
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
    mBookType = navigator->bookType();
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
        QPixmap image = PagesNavigator::getBookPageImage(mBookPath, file, mBookType);
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
