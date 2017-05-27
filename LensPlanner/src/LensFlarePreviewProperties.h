#pragma once

#include "Dependencies.h"
#include "LensFlarePreviewer.h"
#include "AttributeCellWidgets.h"

/// Manages the preview options for the lens flare preview widget.
class LensFlarePreviewProperties: public QWidget
{
    Q_OBJECT;

public:
    explicit LensFlarePreviewProperties(LensFlarePreviewer* previewer, 
        QWidget* parent = nullptr);
    ~LensFlarePreviewProperties();

    /// Update function.
    void update();

signals:
    /// Signal, called when any of the light source parameters is changed.
    void lightSourceParametersChangedSignal();

private:
    /// Constructs global attributes for the previewer.
    QVector<AttributeCellWidgetBase*> getGlobalAttributes();

    /// Constructs layer attributes for the previewer.
    QVector<AttributeCellWidgetBase*> getLayerAttributes(int layerId);

    /// Creates the top toolbar
    void createToolBar();

    /// Updates the values in a tree cell.
    void updateTreeAttribute(AttributeCellWidgetBase* attrib,
        QStandardItem* name, QStandardItem* value);

    /// Creates the attributes tree item.
    void createTreeAttributes();

    /// Creates a tree item for the specified element.
    void createTreeItem(int layerId);

    /// Creates the tree structure.
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

    /// The preview object that is edited.
    LensFlarePreviewer* m_previewer;

    /// The toolbar at the top.
    QToolBar* m_toolBar;

    /// The tree model that holds the items.
    QStandardItemModel* m_treeModel;

    /// The editor tree.
    QTreeView* m_tree;

    /// The layout holding together the items.
    QLayout* m_layout;

    /// The light sources to render.
    QVector<LensFlarePreviewer::Layer> m_layers;
};