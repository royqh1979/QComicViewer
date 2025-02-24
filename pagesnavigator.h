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

#ifndef PAGESNAVIGATOR_H
#define PAGESNAVIGATOR_H

#include <QObject>
#include <QPixmap>
#include <QAbstractListModel>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <memory>

class QFileSystemWatcher;
class ArchiveReader;
class PagesNavigator;
class BookPagesModel: public QAbstractListModel{
    Q_OBJECT
public:
    BookPagesModel(PagesNavigator *bookNavigator, QObject *parent);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
private slots:
    void onBookChanged(QString newBookPath);
    void invalidateAllThumbnails();
    void onThumbnailReady(int page);
private:
    PagesNavigator *mBookNavigator;
};

class PagesNavigator : public QObject
{
    Q_OBJECT
public:
    explicit PagesNavigator(QObject *parent = nullptr);
    ~PagesNavigator();

    void gotoPage(int page);
    void toNextPage();
    void toPrevPage();
    void toLastPage();
    void toFirstPage();
    QPixmap currentImage();
    QString currentPageName();
    void loadThumbnails();
    QPixmap thumbnail(int page);

    QString bookTitle() const;
    const QString &bookPath() const;
    void setBookPath(QString newBookPath);
    const QStringList &pageList() const;
    int pageCount() const;
    int currentPage() const;
    bool displayDoublePages() const;
    void setDisplayDoublePages(bool newDisplayDoublePages);
    bool doublePagesRightToLeft() const;
    void setDoublePagesRightToLeft(bool newValue);
    int doublePagesStart() const;
    void setDoublePagesStart(int newDoublePagesStart);
    int doublePagesEnd() const;
    void setDoublePagesEnd(int newDoublePagesEnd);

    static QPixmap getBookPageImage(QString bookPath, QString file);
    int thumbnailSize() const;
    void setThumbnailSize(int newThumbnailSize);

    bool canHandle(const QString& filePath);

signals:
    void currentImageChanged();
    void currentPageChanged();
    void bookChanged(QString newBookPath);
    void thumbnailsCleared();
    void destoryed();
    void thumbnailReady(int page);
    void thumbnailRemoved(int page);
private slots:
    void setThumbnail(QString bookPath, int page, QPixmap thumbnail);
    void onThumbnailLoadingFinished(QString bookPath);
    void onDirChanged(const QString& path);
    void onFileChanged(const QString& path);
private:
    QPixmap getPageImage(int page);
    void setCurrentPage(int newCurrentPage);
    int ensureDoublePages(int page);
private:
    QString mBookPath;
    QStringList mPageList;
    int mDisplayPage;
    int mCurrentPage;
    int mDoublePagesStart;
    int mDoublePagesEnd;
    bool mDisplayDoublePages;
    bool mDoublePagesRightToLeft;
    int mThumbnailSize;
    bool mLoadingThumbnail;
    QMap<int, QPixmap> mThumbnailCache;
    QRecursiveMutex mThumbnailMutex;
    QFileSystemWatcher *mFileSystemWatcher;
    static QList<std::shared_ptr<ArchiveReader>> mArchiveReaders;
    static QSet<QString> mImageSuffice;
};

class PageThumbnailLoader: public QThread {
    Q_OBJECT
public:
    PageThumbnailLoader(PagesNavigator *navigator, QObject* parent=nullptr);
signals:
    void thumbnailLoaded(QString bookPath, int page, QPixmap thumbnail);
    void loadFinished(QString bookPath);
private slots:
    void onBookChanged(QString newBookPath);
    void stopLoader();
private:
    QString mBookPath;
    QStringList mPageList;
    bool mStop;
    int mThumbnailSize;

    // QThread interface
protected:
    void run() override;
};

#endif // PAGESNAVIGATOR_H
