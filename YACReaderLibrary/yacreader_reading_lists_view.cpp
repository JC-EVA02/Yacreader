#include "yacreader_reading_lists_view.h"

#include "reading_list_item.h"
#include "reading_list_model.h"

YACReaderReadingListsView::YACReaderReadingListsView(QWidget *parent)
    :YACReaderTreeView(parent)
{
    setItemDelegate(new YACReaderReadingListsViewItemDeletegate(this));
    setUniformRowHeights(false);
}


//----------------------------------------------------------------------

YACReaderReadingListsViewItemDeletegate::YACReaderReadingListsViewItemDeletegate(QObject *parent)
    :QStyledItemDelegate(parent)
{

}

void YACReaderReadingListsViewItemDeletegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ReadingListModel::TypeList typeList = (ReadingListModel::TypeList)index.data(ReadingListModel::TypeListsRole).toInt();

    if(typeList == ReadingListModel::Separator)
    {
        return;
    }

    /*if(!item->data(FolderModel::Completed).toBool())
    {
        painter->save();
#ifdef Q_OS_MAC
        painter->setBrush(QBrush(QColor(85,95,127)));
#else
        painter->setBrush(QBrush(QColor(237,197,24)));
#endif
        painter->setPen(QPen(QBrush(),0));
        painter->drawRect(0,option.rect.y(),2,option.rect.height());
        painter->restore();
    }*/

    QStyledItemDelegate::paint(painter, option, index);
}

QSize YACReaderReadingListsViewItemDeletegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ReadingListModel::TypeList typeList = (ReadingListModel::TypeList)index.data(ReadingListModel::TypeListsRole).toInt();

    if(typeList == ReadingListModel::Separator)
    {
        QSize newSize = QStyledItemDelegate::sizeHint(option, index);
        newSize.setHeight(7);
        return newSize;
    }

    return QStyledItemDelegate::sizeHint(option, index);
}
