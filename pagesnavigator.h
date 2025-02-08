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

class PagesNavigator : public QObject
{
    Q_OBJECT
public:
    enum class BookType {
        Folder,
        Zip,
    };
    explicit PagesNavigator(QObject *parent = nullptr);

    void gotoPage(int page);
    void toNextPage();
    void toPrevPage();
    void toLastPage();
    void toFirstPage();
    QPixmap currentImage();

    QString bookTitle() const;
    const QString &bookPath() const;
    void setBookPath(QString newBookPath);
    const QStringList &pageList() const;
    int pageCount() const;
    int currentPage() const;
    bool displayDoublePages() const;
    void setDisplayDoublePages(bool newDisplayDoublePages);
    bool displayPagesLeftToRight() const;
    void setDisplayPagesLeftToRight(bool newDisplayPagesLeftToRight);
    int doublePagesStart() const;
    void setDoublePagesStart(int newDoublePagesStart);
    int doublePagesEnd() const;
    void setDoublePagesEnd(int newDoublePagesEnd);

    BookType bookType() const;

signals:
    void currentImageChanged();
private:
    QPixmap getPageImage(int page);
    void setCurrentPage(int newCurrentPage);
    int ensureDoublePages(int page);
private:
    QString mBookPath;
    QStringList mPageList;
    BookType mBookType;
    int mCurrentPage;
    int mDoublePagesStart;
    int mDoublePagesEnd;
    bool mDisplayDoublePages;
    bool mDisplayPagesLeftToRight;
};

#endif // PAGESNAVIGATOR_H
