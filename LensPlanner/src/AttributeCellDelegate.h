#pragma once

#include "Dependencies.h"

/// Base class for a widget that corresponds to an attribute residing in a
/// view cell.
class AttributeCellWidgetBase 
{
public:
    virtual void refreshName(QAbstractItemModel* model, 
        const QModelIndex& index) const = 0;
    virtual void refreshView(QAbstractItemModel* model, 
        const QModelIndex& index) const = 0;
    virtual void refreshAttribute(QAbstractItemModel* model, 
        const QModelIndex& index) const = 0;

    // QStyledItemDelegate interface
    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const = 0;
    virtual void setEditorData(QWidget* editor, 
        const QModelIndex& index) const = 0;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const = 0;
};

Q_DECLARE_METATYPE(AttributeCellWidgetBase*)

/// Qt cell delegate object that manages the manippulation process of an
/// attribute that has a getter and setter.
class AttributeCellDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    AttributeCellDelegate(QWidget* parent = 0);

    /// QStyledItemDelegate callbacks.
    bool editorEvent(QEvent* event, QAbstractItemModel* model, 
        const QStyleOptionViewItem& option, const QModelIndex& index) override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, 
        const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) const override;
};