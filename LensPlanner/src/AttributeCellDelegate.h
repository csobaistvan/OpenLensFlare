#pragma once

#include "Dependencies.h"

/// Base class for a widget that corresponds to an attribute residing in a
/// view cell.
struct AttributeCellWidgetBase
{
    virtual void refreshName(QAbstractItemModel* model, const QModelIndex& index) = 0;
    virtual void refreshView(QAbstractItemModel* model, const QModelIndex& index) = 0;
    virtual void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) = 0;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) = 0;
    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) = 0;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) = 0;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) = 0;
};
Q_DECLARE_METATYPE(AttributeCellWidgetBase*);

/// Qt cell delegate object that manages the manippulation process of an
/// attribute that has a getter and setter.
class AttributeCellDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    AttributeCellDelegate(QWidget* parent = 0);

    /// QStyledItemDelegate callbacks.
    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
};