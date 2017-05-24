#pragma once

#include "Dependencies.h"
#include "AttributeCellWidgets.h"

/// The optical system editor widget.
///
/// TODO: the underlying tree structure is duplicated (currently) 3 times - 
/// consider extracting it somehow.
class OpticalSystemEditor: public QWidget
{
    Q_OBJECT;

public:
    explicit OpticalSystemEditor(ImageLibrary* imgLibrary, 
        OLEF::OpticalSystem* system, QWidget* parent = nullptr);
    ~OpticalSystemEditor();

    /// Update function.
    void update();

public slots:
    /// Slot, used to indicate that the underlying optical system changed.
    void opticalSystemChanged();

signals:
    /// Signal, called when the optical system is changed.
    void opticalSystemChangedSignal();

private:
    /// Constructs system attribute objects.
    QVector<AttributeCellWidgetBase*> getOpticalSystemAttributes();

    /// Constructs elements attributes for the specified element.
    QVector<AttributeCellWidgetBase*> getOpticalElementAttributes(int elemId);

    /// Creates the top toolbar
    void createToolBar();

    /// Updates the values in a tree cell.
    void updateTreeAttribute(AttributeCellWidgetBase* attrib,
        QStandardItem* name, QStandardItem* value);

    /// Creates the attributes tree item.
    void createTreeAttributes();

    /// Creates a tree item for the specified element.
    void createTreeItem(int elemId);

    /// Creates the editor tree
    void createTree();

    /// Creates the layout.
    void createLayout();

    /// Slot for when an item is changed.
    void itemChanged(QStandardItem* item);

    /// Slot for when the 'add' button is pressed.
    void addItem();

    /// Flor for thwn the 'duplicate' button is pressed.
    void duplicateItem();
    
    /// Flor for thwn the 'up' button is pressed.
    void upItem();
    
    /// Flor for thwn the 'down' button is pressed.
    void downItem();

    /// Slot for when the 'delete' button is pressed.
    void deleteItem();

    /// Slot for when the 'clear' button is pressed.
    void clearItems();

    /// The image library that holds our images.
    ImageLibrary* m_imageLibrary;

    /// The optical system to edit.
    OLEF::OpticalSystem* m_opticalSystem;

    /// The toolbar at the top.
    QToolBar* m_toolBar;

    /// The tree model that holds the items.
    QStandardItemModel* m_treeModel;

    /// The editor tree.
    QTreeView* m_tree;

    /// The layout holding together the items.
    QLayout* m_layout;
};