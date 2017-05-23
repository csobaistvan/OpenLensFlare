#include "AttributeCellDelegate.h"

////////////////////////////////////////////////////////////////////////////////
AttributeCellDelegate::AttributeCellDelegate(QWidget* parent): 
    QStyledItemDelegate(parent)
{}

////////////////////////////////////////////////////////////////////////////////
void AttributeCellDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, 
    const QModelIndex& index) const
{
    /*
    const auto& data = index.data(Qt::UserRole + 1);
    if (data.canConvert<AttributeCellWidgetBase*>())
    {
        AttributeCellWidgetBase* cellData = 
            qvariant_cast<AttributeCellWidgetBase*>(data);

        cellData->paint(painter, option, index);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
    */
    QStyledItemDelegate::paint(painter, option, index);
}

////////////////////////////////////////////////////////////////////////////////
QWidget* AttributeCellDelegate::createEditor(QWidget* parent, 
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto& data = index.data(Qt::UserRole + 1);
    if (data.canConvert<AttributeCellWidgetBase*>())
    {
        AttributeCellWidgetBase* cellData = 
            qvariant_cast<AttributeCellWidgetBase*>(data);

        return cellData->createEditor(parent, option, index);
    }
    else
    {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

////////////////////////////////////////////////////////////////////////////////
void AttributeCellDelegate::setEditorData(QWidget* editor, 
    const QModelIndex& index) const
{
    const auto& data = index.data(Qt::UserRole + 1);
    if (data.canConvert<AttributeCellWidgetBase*>())
    {
        AttributeCellWidgetBase* cellData = 
            qvariant_cast<AttributeCellWidgetBase*>(data);

        cellData->setEditorData(editor, index);
    }
    else
    {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

////////////////////////////////////////////////////////////////////////////////
void AttributeCellDelegate::setModelData(QWidget* editor, 
    QAbstractItemModel* model, const QModelIndex& index) const
{
    const auto& data = index.data(Qt::UserRole + 1);
    if (data.canConvert<AttributeCellWidgetBase*>())
    {
        AttributeCellWidgetBase* cellData = 
            qvariant_cast<AttributeCellWidgetBase*>(data);

        cellData->setModelData(editor, model, index);
    }
    else
    {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}