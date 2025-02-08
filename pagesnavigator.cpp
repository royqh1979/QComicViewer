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

static const QSet<QString> ImageSuffice {
    "jpg",
    "jpge",
    "png"
} ;

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
    result.sort();
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
    result.sort();
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
    result.sort();
    return result;
}

PagesNavigator::PagesNavigator(QObject *parent) : QObject(parent),
    mBookType{BookType::Folder},
    mCurrentPage{-1},
    mDoublePagesStart{-1},
    mDoublePagesEnd{-1},
    mDisplayDoublePages{false},
    mDisplayPagesLeftToRight{false}
{

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
        QPixmap image = mPageList[mCurrentPage];
        return image;
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
    if (QFileInfo{newBookPath}.exists()
            && QFileInfo{newBookPath}.isFile()
            && !newBookPath.endsWith(".zip")
            && !newBookPath.endsWith(".rar")) {
        QFileInfo info{newBookPath};
        newBookPath = info.absolutePath();
        fileName = info.fileName();
    }
    if (mBookPath != newBookPath) {
        mBookPath = newBookPath;
        if (QFileInfo{mBookPath}.exists()) {
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

PagesNavigator::BookType PagesNavigator::bookType() const
{
    return mBookType;
}

QPixmap PagesNavigator::getPageImage(int page)
{
    if (mBookType==BookType::Folder) {
        QDir dir{mBookPath};
        QString imagePath = dir.absoluteFilePath(mPageList[page]);
        return QPixmap(imagePath);
    } else if (mBookType == BookType::Zip) {
        QuaZip zip{mBookPath};
        if (!zip.open(QuaZip::mdUnzip))
            return QPixmap();
        if (!zip.setCurrentFile(mPageList[page]))
            return QPixmap();
        QuaZipFile unzipfile(&zip);
        if (!unzipfile.open(QIODevice::ReadOnly))
            return QPixmap();
        QImageReader reader{&unzipfile};
        QPixmap result = QPixmap::fromImageReader(&reader);
        unzipfile.close();
        zip.close();
        return result;
    } else if (mBookType == BookType::RAR) {
        QtRAR archive{mBookPath};
        if (!archive.open(QtRAR::OpenModeExtract))
            return QPixmap();
        QtRARFile unrarFile{&archive};
        unrarFile.setFileName(mPageList[page]);
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

