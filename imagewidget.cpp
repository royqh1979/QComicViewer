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

#include "imagewidget.h"
#include <QPainter>
#include <QScrollBar>
#include <QDebug>
#include <QWheelEvent>

ImageWidget::ImageWidget(QWidget *parent) :
  QAbstractScrollArea{parent},
  mRatio{1.0},
  mFitType{AutoFitType::Page},
  mWorkingFitType{AutoFitType::Page},
  mBackground{Qt::gray},
  mScrollAngle{0},
  mMovingImage{false}
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

float ImageWidget::ratio() const
{
    return mRatio;
}

void ImageWidget::setRatio(float newRatio)
{
    newRatio = std::max((float)0.01, newRatio);
    if (newRatio!=mRatio) {
        mRatio = newRatio;
        resetScrollBars(true);
    }
}

ImageWidget::AutoFitType ImageWidget::fitType() const
{
    return mFitType;
}

void ImageWidget::setFitType(AutoFitType newFitType)
{
    if (mFitType!=newFitType) {
        mFitType = newFitType;
        mWorkingFitType = mFitType;
        if (mFitType == AutoFitType::None)
            mRatio = 1;
        resetScrollBars();
        emit fitTypeChanged();
    }
}

const QPixmap &ImageWidget::image() const
{
    return mImage;
}

void ImageWidget::setImage(const QPixmap &newImage)
{
    if (mImage != newImage) {
        mImage = newImage;
        mWorkingFitType = mFitType;
        resetScrollBars();
        horizontalScrollBar()->setValue(0);
        verticalScrollBar()->setValue(0);
        emit imageChanged();
    }
}

QSize ImageWidget::imageSize() const
{
    return mImage.size();
}

void ImageWidget::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    resetScrollBars();
}

void ImageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(viewport());
    painter.fillRect(0,0,viewport()->width(),viewport()->height(),mBackground);
    if (!mImage.isNull()) {
        int img_x = horizontalScrollBar()->value();
        int img_y = verticalScrollBar()->value();
        int x = std::max(0, (viewport()->width() - mCacheImage.width())/2);
        int y = std::max(0, (viewport()->height() - mCacheImage.height())/2);
        painter.drawPixmap(x,y,mCacheImage,img_x,img_y,viewport()->width(), viewport()->height());
    }
}

void ImageWidget::resetScrollBars(bool forceRatio)
{
    if (forceRatio)
        mWorkingFitType = AutoFitType::None;
    if (!mImage.isNull() && viewport()->width()>0 && viewport()->height()>0) {
        switch(mWorkingFitType) {
        case AutoFitType::Page:
        {
            float r1,r2;
            r1 = (float)viewport()->width() / mImage.width();
            r2 = (float)viewport()->height() / mImage.height();
            if (r1 * mImage.height() > viewport()->height())
                mRatio = r2;
            else
                mRatio = r1;
        }   // fall through here
        case AutoFitType::None:
            mCacheImage = mImage.scaled(
                        mImage.width() * mRatio,
                        mImage.height() * mRatio,
                        Qt::KeepAspectRatio);
            break;
        case AutoFitType::Height:
            mCacheImage = mImage.scaledToHeight(viewport()->height());
            mRatio = (float)mCacheImage.height() / mImage.height();
            break;
        case AutoFitType::Width:
            mCacheImage = mImage.scaledToWidth(viewport()->width());
            mRatio = (float)mCacheImage.width() / mImage.width();
            break;
        }
        verticalScrollBar()->setRange(0, mCacheImage.height()-viewport()->height());
        horizontalScrollBar()->setRange(0, mCacheImage.width()-viewport()->width());
    } else {
        verticalScrollBar()->setRange(0,0);
        horizontalScrollBar()->setRange(0,0);
    }
    emit ratioChanged();
    viewport()->update();
}

void ImageWidget::scrollImageByMouseMove(QMouseEvent( *event))
{
    if (!mMovingImage)
        return;
    QPoint delta = event->pos() - mOldMousePos;
    mOldMousePos = event->pos();
    horizontalScrollBar()->setValue(
                horizontalScrollBar()->value() - delta.x());
    verticalScrollBar()->setValue(
                verticalScrollBar()->value() - delta.y());
}

void ImageWidget::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() == Qt::KeyboardModifier::ControlModifier) {
        if (event->angleDelta().y()*mScrollAngle<0)
            mScrollAngle = 0;
        mScrollAngle += event->angleDelta().y();
        if (mScrollAngle>=120) {
            setRatio(mRatio - 0.05);
            while (mScrollAngle >= 120)
                mScrollAngle -= 120;
        } else if (mScrollAngle <= -120) {
            setRatio(mRatio + 0.05);
            while (mScrollAngle <= -120)
                mScrollAngle += 120;
        }
        return;
    }
    if (verticalScrollBar()->isVisible()) {
        mScrollAngle = 0;
        QAbstractScrollArea::wheelEvent(event);
    } else{
        if (event->angleDelta().y()*mScrollAngle<0)
            mScrollAngle = 0;
        mScrollAngle += event->angleDelta().y();
        if (mScrollAngle>=120) {
            emit requestPrevImage();
            while (mScrollAngle >= 120)
                mScrollAngle -= 120;
        } else if (mScrollAngle <= -120) {
            emit requestNextImage();
            while (mScrollAngle <= -120)
                mScrollAngle += 120;
        }
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mMovingImage) {
        scrollImageByMouseMove(event);
        event->accept();
    }
}

void ImageWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (mFitType==AutoFitType::None)
            mWorkingFitType = AutoFitType::Page;
        else
            mWorkingFitType = mFitType;
        resetScrollBars();
        event->accept();
    }
}

void ImageWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mOldMousePos=event->pos();
        mMovingImage = true;
        event->accept();
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        scrollImageByMouseMove(event);
        mMovingImage = false;
        event->accept();
    }
}

const QColor &ImageWidget::background() const
{
    return mBackground;
}

void ImageWidget::setBackground(const QColor &newBackground)
{
    if (mBackground!=newBackground) {
        mBackground = newBackground;
        viewport()->update();
    }
}
