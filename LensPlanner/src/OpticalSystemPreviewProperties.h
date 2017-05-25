#pragma once

#include "Dependencies.h"
#include "OpticalSystemPreviewer.h"
#include "AttributeCellWidgets.h"

/// Manages the preview options for the optical system editor widget.
class OpticalSystemPreviewProperties: public QWidget
{
    Q_OBJECT;

public:
    explicit OpticalSystemPreviewProperties(OpticalSystemPreviewer* previewer, 
        QWidget* parent = nullptr);
    ~OpticalSystemPreviewProperties();

    /// Update function.
    void update();

public slots:
    /// Slot, used to indicate that the underlying optical system changed.
    void opticalSystemChanged();

signals:
    /// Signal, called when any of the batch parameters is changed.
    void batchParametersChangedSignal();

private:
    /// Parameters for a ray to visualize.
    struct RayBatch
    {
        /// Whether the ray should be shown or not.
        bool m_enabled = true;

        /// Index of the first ghost to render.
        int m_firstGhost = 1;

        /// Number of ghosts to render.
        int m_numGhosts = 0;

        /// The number of reflections.
        int m_numReflections = 0;

        /// Whether aperture crossing is allowed or not.
        bool m_apertureCross;

        /// Color of the corresponding lines.
        QColor m_color = QColor(Qt::white);

        /// Width of the ray line strip.s.
        float m_lineWidth = 1.0f;

        /// Number of rays.
        int m_rayCount = 0;

        /// Distance from the first element from where the rays should originate.
        float m_startDistance = 0.0f;

        /// Offset of the first ray, from the top.
        float m_startHeight = 0.0f;

        /// Offset of the last ray, from the top.
        float m_endHeight = 0.0f;

        /// Angle of the incoming ray.
        float m_angle = 0.0f;
    };

    /// Constructs global attributes for the previewer.
    QVector<AttributeCellWidgetBase*> getGlobalAttributes();

    /// Constructs batch attributes for the parameter batch id.
    QVector<AttributeCellWidgetBase*> getBatchAttributes(int batchId);

    /// Generates ray batches for the preview widget.
    void generateRayBatches();

    /// Creates the top toolbar
    void createToolBar();

    /// Updates the values in a tree cell.
    void updateTreeAttribute(AttributeCellWidgetBase* attrib,
        QStandardItem* name, QStandardItem* value);

    /// Creates the attributes tree item.
    void createTreeAttributes();

    /// Creates a tree item for the specified element.
    void createTreeItem(int batchId);

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
    OpticalSystemPreviewer* m_previewer;

    /// The toolbar at the top.
    QToolBar* m_toolBar;

    /// The tree model that holds the items.
    QStandardItemModel* m_treeModel;

    /// The editor tree.
    QTreeView* m_tree;

    /// The layout holding together the items.
    QLayout* m_layout;

    /// The ray batches to render.
    QVector<RayBatch> m_batches;
};