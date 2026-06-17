#ifndef RESIZE_AWARE_LIST_H
#define RESIZE_AWARE_LIST_H
#include <QListView>

class QTimer;
class ResizeawareListView : public QListView {
    Q_OBJECT
public:
    explicit ResizeawareListView(QWidget *parent = nullptr);
signals:
    void resized();
private slots:
    void onResizeTimerTimeout();
    // QAbstractItemView interface
public:
    void resizeEvent(QResizeEvent *event) override;
private:
    QTimer *mResizeTimer;
};

#endif // RESIZE_AWARE_LIST_H
