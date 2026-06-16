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

#ifndef BOOKPAGESMODEL_H
#define BOOKPAGESMODEL_H

#include <QObject>
#include <QPixmap>
#include <QAbstractListModel>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <memory>

class QFileSystemWatcher;
class ArchiveReader;

class BookPagesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit BookPagesModel(QObject *parent = nullptr);
    ~BookPagesModel();

    void gotoPage(int page);
    void toNextPage();
    void toPrevPage();
    void toLastPage();
    void toFirstPage();
    QPixmap currentImage() const;
    QString currentPageName() const;
    QString imagePageNames() const;
    void loadThumbnails() const;
    void loadThumbnail(int page, const QString &pagePath) const;
    QPixmap thumbnail(int page) const;
    QString pagePath(int page) const;

    QString bookTitle() const;
    const QString &bookPath() const;
    void setBookPath(QString newBookPath);
    QStringList pageList() const;
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
    bool autoSinglePage() const;
    void setAutoSinglePage(bool newAutoSinglePage);
    static QPixmap getBookPageImage(QString bookPath, QString file);
    int thumbnailSize() const;
    void setThumbnailSize(int newThumbnailSize);

    bool canHandle(const QString& filePath);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

signals:
    void currentImageChanged();
    void currentPageChanged();
    void bookChanged(const QString &newBookPath);
    void destoryed();
private slots:
    void setThumbnail(const QString &bookPath, int page, const QString &pagePath, QPixmap thumbnail);
    void onThumbnailsLoadingFinished(const QString &bookPath);
    void onDirChanged(const QString& path);
    void onFileChanged(const QString& path);
private:
    QPixmap getPageImage(int page) const;
    void setCurrentPage(int newCurrentPage);
    int calcDisplayPage(int page);
    bool isPageShouldSingle(int page) const;
    void setPageList(const QStringList &newPageList);
    void clearThumbnails();
private:
    QString mBookPath;
    QStringList mPageList;
    int mDisplayPage;
    int mCurrentPage;
    int mDoublePagesStart;
    int mDoublePagesEnd;
    bool mDisplayDoublePages;
    bool mAutoSinglePage;
    bool mDoublePagesRightToLeft;
    int mThumbnailSize;
    mutable bool mLoadingThumbnails;
    QMap<QString, QPixmap> mThumbnailCache;
    QMap<int, bool> mIsPageShouldSingle;
    mutable QRecursiveMutex mMutex;
    QFileSystemWatcher *mFileSystemWatcher;
    static QList<std::shared_ptr<ArchiveReader>> mArchiveReaders;
    static QSet<QString> mImageSuffice;

    // QAbstractItemModel interface
public:
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDragActions() const override;
};

class PageThumbnailsLoader: public QThread {
    Q_OBJECT
public:
    PageThumbnailsLoader(const BookPagesModel *navigator, QObject* parent=nullptr);
signals:
    void thumbnailLoaded(const QString &bookPath, int page, const QString &pagePath, const QPixmap &thumbnail);
    void loadFinished(const QString &bookPath);
private slots:
    void onBookChanged(const QString &newBookPath);
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

class DirImageThumbnailLoader: public QThread {
    Q_OBJECT
public:
    DirImageThumbnailLoader(const QString &bookPath, int page, int thumbnailSize, const QString &pagePath, QObject* parent=nullptr);
signals:
    void thumbnailLoaded(const QString &bookPath, int page, const QString &pagePath, const QPixmap &thumbnail);
private:
    QString mBookPath;
    int mPage;
    QString mPagePath;
    int mThumbnailSize;

    // QThread interface
protected:
    void run() override;
};

#endif // BOOKPAGESMODEL_H
