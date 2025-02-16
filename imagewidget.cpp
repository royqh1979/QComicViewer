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
#include <QTransform>

ImageWidget::ImageWidget(QWidget *parent) :
  QAbstractScrollArea{parent},
  mRatio{1.0},
  mFitType{AutoFitType::Page},
  mWorkingFitType{AutoFitType::Page},
  mBackground{Qt::gray},
  mScrollAngleX{0},
  mScrollAngleY{0},
  mMovingImage{false},
  mSwapLeftRightWhenTurnPage{false}
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
        updateImage(true);
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
        updateImage();
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
        updateImage();
        horizontalScrollBar()->setValue(0);
        verticalScrollBar()->setValue(0);
    }
}

QSize ImageWidget::imageSize() const
{
    return mImage.size();
}

void ImageWidget::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateImage();
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

void ImageWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Down
            && verticalScrollBar()->value()==verticalScrollBar()->maximum()) {
        emit requestNextImage();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Up
            && verticalScrollBar()->value()==verticalScrollBar()->minimum()) {
        emit requestPrevImage(true);
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Left) {
        if (!mSwapLeftRightWhenTurnPage && horizontalScrollBar()->value()==horizontalScrollBar()->minimum()) {
            emit requestPrevImage(false);
            event->accept();
            return;
        }
        if (mSwapLeftRightWhenTurnPage && horizontalScrollBar()->value()==horizontalScrollBar()->maximum()) {
            emit requestNextImage();
            event->accept();
            return;
        }
    }
    if (event->key() == Qt::Key_Right) {
        if (!mSwapLeftRightWhenTurnPage && horizontalScrollBar()->value()==horizontalScrollBar()->maximum()) {
            emit requestNextImage();
            event->accept();
            return;
        }
        if (mSwapLeftRightWhenTurnPage && horizontalScrollBar()->value()==horizontalScrollBar()->minimum()) {
            emit requestPrevImage(false);
            event->accept();
            return;
        }

    }
    QAbstractScrollArea::keyPressEvent(event);
}

void ImageWidget::updateImage(bool forceRatio)
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
        verticalScrollBar()->setSingleStep(mCacheImage.height()/10);
        horizontalScrollBar()->setSingleStep(mCacheImage.width()/10);
    } else {
        verticalScrollBar()->setRange(0,0);
        horizontalScrollBar()->setRange(0,0);
    }
    emit imageUpdated();
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

bool ImageWidget::swapLeftRightWhenTurnPage() const
{
    return mSwapLeftRightWhenTurnPage;
}

void ImageWidget::setSwapLeftRightWhenTurnPage(bool newSwapLeftRightWhenTurnPage)
{
    if (mSwapLeftRightWhenTurnPage == newSwapLeftRightWhenTurnPage)
        return;
    mSwapLeftRightWhenTurnPage = newSwapLeftRightWhenTurnPage;
    emit swapLeftRightWhenTurnPageChanged();
}

void ImageWidget::rotate(int degree)
{
    if (mImage.isNull())
        return;
    mImage = mImage.transformed(QTransform().rotate(degree));
    updateImage();
}

void ImageWidget::horizontalFlip()
{
    if (mImage.isNull())
        return;
    mImage = mImage.transformed(QTransform().scale(-1,1));
    updateImage();
}

void ImageWidget::verticalFlip()
{
    if (mImage.isNull())
        return;
    mImage = mImage.transformed(QTransform().scale(1,-1));
    updateImage();
}

void ImageWidget::scrollToTop()
{
    verticalScrollBar()->setValue(verticalScrollBar()->minimum());
}

void ImageWidget::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ImageWidget::scrollToLeft()
{
    horizontalScrollBar()->setValue(horizontalScrollBar()->minimum());
}

void ImageWidget::scrollToRight()
{
    horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
}

void ImageWidget::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() == Qt::KeyboardModifier::ControlModifier) {
        if (e->angleDelta().y()*mScrollAngleY<0)
            mScrollAngleY = 0;
        mScrollAngleY += e->angleDelta().y();
        if (mScrollAngleY>=120) {
            setRatio(mRatio - 0.05);
            while (mScrollAngleY >= 120)
                mScrollAngleY -= 120;
        } else if (mScrollAngleY <= -120) {
            setRatio(mRatio + 0.05);
            while (mScrollAngleY <= -120)
                mScrollAngleY += 120;
        }
        e->accept();
        return;
    }

    if (std::abs(e->angleDelta().y())>=std::abs(e->angleDelta().x())) {
        if (e->angleDelta().y()*mScrollAngleY<0)
            mScrollAngleY = 0;
        mScrollAngleY += e->angleDelta().y();
        if (mScrollAngleY>=120) {
            if (verticalScrollBar()->value()==verticalScrollBar()->minimum())
                emit requestPrevImage(true);
            else
                verticalScrollBar()->setValue(verticalScrollBar()->value()-verticalScrollBar()->singleStep());
            mScrollAngleY %= 120;
        } else if (mScrollAngleY <= -120) {
            if (verticalScrollBar()->value()==verticalScrollBar()->maximum())
                emit requestNextImage();
            else
                verticalScrollBar()->setValue(verticalScrollBar()->value()+verticalScrollBar()->singleStep());
            mScrollAngleY = - ((-mScrollAngleY) % 120);
        }
        e->accept();
    } else {
        if (e->angleDelta().x()*mScrollAngleX<0)
            mScrollAngleX = 0;
        mScrollAngleX += e->angleDelta().y();
        if (mScrollAngleX>=120) {
            if (!mSwapLeftRightWhenTurnPage && horizontalScrollBar()->value()==horizontalScrollBar()->minimum())
                emit requestPrevImage(false);
            else if (mSwapLeftRightWhenTurnPage && horizontalScrollBar()->value()==horizontalScrollBar()->maximum())
                emit requestNextImage();
            else
                horizontalScrollBar()->setValue(horizontalScrollBar()->value()-horizontalScrollBar()->singleStep());
            mScrollAngleX %= 120;
        } else if (mScrollAngleX <= -120) {
            if (!mSwapLeftRightWhenTurnPage && horizontalScrollBar()->value()==horizontalScrollBar()->maximum())
                emit requestNextImage();
            else if (mSwapLeftRightWhenTurnPage && horizontalScrollBar()->value()==horizontalScrollBar()->minimum())
                emit requestPrevImage(false);
            else
                horizontalScrollBar()->setValue(horizontalScrollBar()->value()+horizontalScrollBar()->singleStep());
            mScrollAngleX = - ((-mScrollAngleX) % 120);
        }
        e->accept();
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
        updateImage();
        event->accept();
    }
}

void ImageWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mOldMousePos=event->pos();
        mMovingImage = true;
        setCursor(Qt::CursorShape::SizeAllCursor);
        event->accept();
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        scrollImageByMouseMove(event);
        mMovingImage = false;
        setCursor(Qt::CursorShape::ArrowCursor);
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
