#include "thumbnailview.h"

#include <QPainter>
#include <QDebug>

ThumbnailDelegate::ThumbnailDelegate(QObject *parent):
    QStyledItemDelegate{parent}
{
    mThumbnailSize=300;
}

void ThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->fillRect(option.rect,option.backgroundBrush);
    QPixmap icon = index.data(Qt::DecorationRole).value<QPixmap>();
    if (!icon.isNull()) {
        painter->drawPixmap(option.rect.left()+std::max(0,mThumbnailSize-icon.width())/2,
                            option.rect.top()+std::max(0,mThumbnailSize-icon.height())/2,icon);
        painter->drawRect(option.rect.left()+std::max(0,mThumbnailSize-icon.width())/2,
                          option.rect.top()+std::max(0,mThumbnailSize-icon.height())/2,icon.width(),icon.height());
    }
    QFontMetrics fm(option.font);
    QRect textRect(option.rect.left(),
                   option.rect.top()+mThumbnailSize,
                   mThumbnailSize,
                   fm.lineSpacing());

    QString text = index.data(Qt::DisplayRole).toString();
    QString elided = fm.elidedText(text, Qt::ElideRight, textRect.width());

    painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextSingleLine, elided);
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics fm(option.font);
    return QSize(mThumbnailSize,
                 mThumbnailSize + fm.lineSpacing());
}

void ThumbnailDelegate::setThumbnailSize(int newThumbnailSize)
{
    mThumbnailSize = newThumbnailSize;
}
