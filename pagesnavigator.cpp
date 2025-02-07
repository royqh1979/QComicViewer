#include "pagesnavigator.h"

#include <QDir>
#include <QPainter>
#include <QDebug>


static QStringList readDir(const QString &path)
{
    QDir dir{path};
    QStringList result;
    for(const QString &file : dir.entryList(QDir::Filter::Files)) {
        result.append(dir.absoluteFilePath(file));
    }
    return result;
}

PagesNavigator::PagesNavigator(QObject *parent) : QObject(parent),
    mCurrentPage{-1},
    mDoublePagesStart{-1},
    mDoublePagesEnd{-1},
    mDisplayDoublePages{false},
    mDisplayPagesLeftToRight{true}
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
        if (mCurrentPage+1 >= pageCount())
            return QPixmap(mPageList[mCurrentPage]);
        else {
            QPixmap image1 = QPixmap(mPageList[mCurrentPage]);
            QPixmap image2 = QPixmap(mPageList[mCurrentPage+1]);
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

void PagesNavigator::setBookPath(const QString &newBookPath)
{
    if (mBookPath != newBookPath) {
        mBookPath = newBookPath;
        mPageList = readDir(mBookPath);
        mDoublePagesStart = 0;
        mDoublePagesEnd = pageCount();
        toFirstPage();
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
        emit currentPageChanged();
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
        emit currentPageChanged();
    }
}

bool PagesNavigator::displayPagesLeftToRight() const
{
    return mDisplayPagesLeftToRight;
}

void PagesNavigator::setDisplayPagesLeftToRight(bool newDisplayPagesLeftToRight)
{
    mDisplayPagesLeftToRight = newDisplayPagesLeftToRight;
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
    }
}

int PagesNavigator::ensureDoublePages(int page)
{
    if (mDisplayDoublePages && page>=mDoublePagesStart && page<mDoublePagesEnd) {
        page -= (page - mDoublePagesStart) % 2;
    }
    return page;
}

