#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QAbstractScrollArea>

class ImageWidget : public QAbstractScrollArea
{
    Q_OBJECT
public:
    enum class AutoFitType {
        None,
        Width,
        Height,
        Page
    };
    explicit ImageWidget(QWidget *parent = nullptr);

    float ratio() const;
    void setRatio(float newRatio);

    AutoFitType fitType() const;
    void setFitType(AutoFitType newFitType);
    const QPixmap &image() const;
    void setImage(const QPixmap &newImage);

    const QColor &background() const;
    void setBackground(const QColor &newBackground);

signals:
    void ratioChanged();
    void fitTypeChanged();
    void imageChanged();
    void requestPrevImage();
    void requestNextImage();
private:
    void resetScrollBars();
private:
    float mRatio;
    AutoFitType mFitType;
    QPixmap mImage;
    QPixmap mCacheImage;
    QColor mBackground;
    int mScrollAngle;

    // QWidget interface
protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

#endif // IMAGEWIDGET_H
