#ifndef PAGESNAVIGATOR_H
#define PAGESNAVIGATOR_H

#include <QObject>
#include <QPixmap>

class PagesNavigator : public QObject
{
    Q_OBJECT
public:
    explicit PagesNavigator(QObject *parent = nullptr);

    void gotoPage(int page);
    void toNextPage();
    void toPrevPage();
    void toLastPage();
    void toFirstPage();
    QPixmap currentImage();

    QString bookTitle() const;
    const QString &bookPath() const;
    void setBookPath(const QString &newBookPath);
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

signals:
    void currentPageChanged();
private:
    void setCurrentPage(int newCurrentPage);
    int ensureDoublePages(int page);
private:
    QString mBookPath;
    QStringList mPageList;
    int mCurrentPage;
    int mDoublePagesStart;
    int mDoublePagesEnd;
    bool mDisplayDoublePages;
    bool mDisplayPagesLeftToRight;
};

#endif // PAGESNAVIGATOR_H
