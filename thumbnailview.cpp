#include "thumbnailview.h"

#include <QPainter>
#include <QDebug>
#include <QApplication>

ThumbnailDelegate::ThumbnailDelegate(QObject *parent):
    QStyledItemDelegate{parent}
{
    mThumbnailSize=300;
}

void ThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 1. 初始化 option，这会正确同步 State_Selected、State_HasFocus 等状态
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget *widget = opt.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    painter->save();

    //style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);
    QFontMetrics fm(option.font);
    if (opt.state & QStyle::State_Selected) {
        QPalette::ColorGroup cg = (opt.state & QStyle::State_HasFocus)
                                      ? QPalette::Active
                                      : QPalette::Inactive;
        QColor backgroundBrush = opt.palette.color(cg, QPalette::Highlight);
        painter->fillRect(option.rect.left(),option.rect.top(),mThumbnailSize,mThumbnailSize+fm.lineSpacing(), backgroundBrush);
    }
    painter->setPen(opt.palette.color(QPalette::Text));
    QPixmap icon = index.data(Qt::DecorationRole).value<QPixmap>();
    if (!icon.isNull()) {
        painter->drawPixmap(option.rect.left()+std::max(0,mThumbnailSize-icon.width())/2,
                            option.rect.top()+std::max(0,mThumbnailSize-icon.height())/2,icon);
        painter->drawRect(option.rect.left()+std::max(0,mThumbnailSize-icon.width())/2,
                          option.rect.top()+std::max(0,mThumbnailSize-icon.height())/2,icon.width(),icon.height());
    }

    // 2. 根据选中状态设置正确的文本颜色
    QRect textRect(option.rect.left(),
                   option.rect.top()+mThumbnailSize,
                   mThumbnailSize,
                   fm.lineSpacing());
    if (opt.state & QStyle::State_Selected) {
        // 关键：区分有焦点和无焦点
        QPalette::ColorGroup cg = (opt.state & QStyle::State_HasFocus)
                                      ? QPalette::Active
                                      : QPalette::Inactive;
        painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(opt.palette.color(QPalette::Text));
    }

    QString text = index.data(Qt::DisplayRole).toString();
    QString elided = fm.elidedText(text, Qt::ElideRight, textRect.width());

    painter->drawText(textRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextSingleLine, elided);

    painter->restore();
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
