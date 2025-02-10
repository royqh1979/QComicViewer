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
    QSize imageSize() const;

    const QColor &background() const;
    void setBackground(const QColor &newBackground);

signals:
    void ratioChanged();
    void fitTypeChanged();
    void imageChanged();
    void requestPrevImage();
    void requestNextImage();
private:
    void resetScrollBars(bool forceRatio=false);
    void scrollImageByMouseMove(QMouseEvent *event);
private:
    float mRatio;
    AutoFitType mFitType;
    AutoFitType mWorkingFitType;
    QPixmap mImage;
    QPixmap mCacheImage;
    QColor mBackground;
    int mScrollAngle;
    QPoint mOldMousePos;
    bool mMovingImage;

    // QWidget interface
protected:
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

#endif // IMAGEWIDGET_H
