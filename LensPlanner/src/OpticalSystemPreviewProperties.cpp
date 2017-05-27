#include "OpticalSystemPreviewProperties.h"

////////////////////////////////////////////////////////////////////////////////
/// List of all the global attributes.
// TODO: these leak!
QVector<AttributeCellWidgetBase*> OpticalSystemPreviewProperties::getGlobalAttributes()
{
    return
    {
        new AttributeCellColor
        {
            "Background Color",
            "",
            std::bind(&OpticalSystemPreviewer::getBackgroundColor, std::ref(*m_previewer)), 
            std::bind(&OpticalSystemPreviewer::setBackgroundColor, std::ref(*m_previewer), std::placeholders::_1),
        },
        new AttributeCellColor
        {
            "Iris Color",
            "",
            std::bind(&OpticalSystemPreviewer::getIrisColor, std::ref(*m_previewer)), 
            std::bind(&OpticalSystemPreviewer::setIrisColor, std::ref(*m_previewer), std::placeholders::_1),
        },
        /*new AttributeCellFloat
        {
            "Iris Line Width",
            "",
            std::bind(&OpticalSystemPreviewer::getIrisLineWidth, std::ref(*m_previewer)), 
            std::bind(&OpticalSystemPreviewer::setIrisLineWidth, std::ref(*m_previewer), std::placeholders::_1),
            1.0f,
            32.0f,
            1.0f,
        },
        */
        new AttributeCellColor
        {
            "Lens Color", 
            "",
            std::bind(&OpticalSystemPreviewer::getLensColor, std::ref(*m_previewer)), 
            std::bind(&OpticalSystemPreviewer::setLensColor, std::ref(*m_previewer), std::placeholders::_1),
        },
        /*new AttributeCellFloat
        {
            "Lens Width",
            "",
            std::bind(&OpticalSystemPreviewer::getLensLineWidth, std::ref(*m_previewer)), 
            std::bind(&OpticalSystemPreviewer::setLensLineWidth, std::ref(*m_previewer), std::placeholders::_1),
            1.0f,
            32.0f,
            1.0f,
        },
        */
        new AttributeCellColor
        {
            "Axis Color",
            "",
            std::bind(&OpticalSystemPreviewer::getAxisColor, std::ref(*m_previewer)), 
            std::bind(&OpticalSystemPreviewer::setAxisColor, std::ref(*m_previewer), std::placeholders::_1),
        },
        /*new AttributeCellFloat
        {
            "Axis Width",
            "",
            std::bind(&OpticalSystemPreviewer::getAxisLineWidth, std::ref(*m_previewer)), 
            std::bind(&OpticalSystemPreviewer::setAxisLineWidth, std::ref(*m_previewer), std::placeholders::_1),
            1.0f,
            32.0f,
            1.0f,
        },
        */
        new AttributeCellInt
        {
            "Lens Resolution",
            "",
            std::bind(&OpticalSystemPreviewer::getLensResolution, std::ref(*m_previewer)), 
            std::bind(&OpticalSystemPreviewer::setLensResolution, std::ref(*m_previewer), std::placeholders::_1),
            1,
            128,
            1,
        },
    };
}

////////////////////////////////////////////////////////////////////////////////
/// List of all the batch attributes.
// TODO: these leak!
QVector<AttributeCellWidgetBase*> OpticalSystemPreviewProperties::getBatchAttributes(int batchId)
{
    return
    {
        new AttributeCellBool
        {
            "Visualize Batch",
            "",
            &m_batches[batchId].m_enabled,
        },
        new AttributeCellInt
        {
            "First Ghost",
            "",
            &m_batches[batchId].m_firstGhost,
            1,
            1000,
            1,
        },
        new AttributeCellInt
        {
            "Number of Ghosts",
            "",
            &m_batches[batchId].m_numGhosts,
            0,
            1000,
            1,
        },
        new AttributeCellInt
        {
            "Number of Reflections",
            "",
            &m_batches[batchId].m_numReflections,
            0,
            10,
            2,
        },
        new AttributeCellBool
        {
            "Aperture Cross",
            "",
            &m_batches[batchId].m_apertureCross,
        },
        new AttributeCellColor
        {
            "Ray Color",
            "",
            &m_batches[batchId].m_color,
        },
        /*new AttributeCellFloat
        {
            "Line Width",
            "",
            &m_batches[batchId].m_lineWidth,
            1.0f,
            32.0f,
            1.0f,
        },
        */
        new AttributeCellInt
        {
            "Number of Rays",
            "",
            &m_batches[batchId].m_rayCount,
            1,
            256,
            1,
        },
        new AttributeCellFloat
        {
            "Starting Distance",
            "",
            &m_batches[batchId].m_startDistance,
            0,
            1000.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "First Height",
            "",
            &m_batches[batchId].m_startHeight,
            -1000.0f,
            1000.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Last Height",
            "",
            &m_batches[batchId].m_endHeight,
            -1000.0f,
            1000.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Incident Angle",
            "",
            &m_batches[batchId].m_angle,
            -180.0f,
            180.0f,
            1.0f,
        },
    };
}

////////////////////////////////////////////////////////////////////////////////
OpticalSystemPreviewProperties::OpticalSystemPreviewProperties(OpticalSystemPreviewer* previewer, QWidget* parent):
    QWidget(parent),
    m_previewer(previewer)
{
    // Create the tree and the layout
    createToolBar();
    createTree();
    createLayout();

    // Call an update to set the item texts.
    update();

    // Set a minimum size.
    setMinimumSize(QSize(350, 300));
}

////////////////////////////////////////////////////////////////////////////////
OpticalSystemPreviewProperties::~OpticalSystemPreviewProperties()
{}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::generateRayBatches()
{
    // Extract the optical system pointer
    OLEF::OpticalSystem* opticalSystem = m_previewer->getOpticalSystem();

    QVector<OpticalSystemPreviewer::RayBatch> batches;
    OLEF::GhostList ghosts[16][2];

    // Reserve space
    int numBatches = 0;
    for (auto batch: m_batches)
    {
        numBatches += batch.m_numGhosts;
    }

    batches.reserve(numBatches);

    // Generate the actual definitions
    for (auto& batch: m_batches)
    {
        // Generate ghosts on first reference
        if (ghosts[batch.m_numReflections][batch.m_apertureCross].empty())
        {
            ghosts[batch.m_numReflections][batch.m_apertureCross] = 
                opticalSystem->generateGhosts(batch.m_numReflections, 
                batch.m_apertureCross);
        }

        // Append a batch for each ghost
        for (int i = 0; i < batch.m_numGhosts; ++i)
        {
            int ghostId = batch.m_firstGhost + i - 1;
            
            // Make sure the ghost ID is valid
            if (ghostId >= ghosts[batch.m_numReflections][batch.m_apertureCross].size())
                break;
            
            // Make sure the batch is enabled
            if (batch.m_enabled == false)
                continue;

            OpticalSystemPreviewer::RayBatch rayBatch;

            rayBatch.m_ghost = ghosts[batch.m_numReflections][batch.m_apertureCross][ghostId];
            rayBatch.m_color = batch.m_color;
            rayBatch.m_width = batch.m_lineWidth;
            rayBatch.m_rayCount = batch.m_rayCount;
            rayBatch.m_startDistance = batch.m_startDistance;
            rayBatch.m_startHeight = batch.m_startHeight;
            rayBatch.m_endHeight = batch.m_endHeight;
            rayBatch.m_angle = glm::radians(batch.m_angle);

            batches.push_back(rayBatch);
        }
    }

    // Store the batch parameters
    m_previewer->setRayBatchParameters(batches);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::updateTreeAttribute(AttributeCellWidgetBase* attrib,
        QStandardItem* name, QStandardItem* value)
{
    value->setData(QVariant::fromValue(attrib), Qt::UserRole + 1);
    attrib->refreshName(name->model(), name->index());
    attrib->refreshView(value->model(), value->index());
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::update()
{
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();
    
    /// Extract the attributes header
    QStandardItem* attributesHeader = root->child(0);

    // Refresh the preview attributes
    const auto& previewerAttributes = getGlobalAttributes();
    for (int attribId = 0; attribId < previewerAttributes.size(); ++attribId)
    {
        updateTreeAttribute(
            previewerAttributes[attribId],
            attributesHeader->child(attribId, 0),
            attributesHeader->child(attribId, 1));
    }

    // Refresh the elements
    for (int batchId = 0; batchId < m_batches.size(); ++batchId)
    {
        // Extract the appropriate root item.
        QStandardItem* header = root->child(batchId + 1);
        header->setText(QString("Batch #%0").arg(batchId + 1));

        // Add the attributes.
        const auto& batchAttributes = getBatchAttributes(batchId);
        for (int attribId = 0; attribId < batchAttributes.size(); ++attribId)
        {
            updateTreeAttribute(batchAttributes[attribId],
                header->child(attribId, 0),
                header->child(attribId, 1));
        }
    }

    // Also expand everything
    m_tree->expandAll();

    // Call the base update function.
    QWidget::update();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::opticalSystemChanged()
{
    generateRayBatches();
    m_previewer->invalidate();
    m_previewer->update();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::itemChanged(QStandardItem* item)
{
    generateRayBatches();
    m_previewer->invalidate();
    m_previewer->update();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::addItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Index where the new element should go.
    // selected.row() is always 1 higher than the element index due to the 
    // first 'Attributes' row, so we can use it to append after the selected 
    // element.
    auto dstId = (selected.isValid()) ? selected.row() : m_batches.size();
    
    // Append to the list
    m_batches.insert(m_batches.begin() + dstId, RayBatch());

    // Create its item.
    createTreeItem(dstId);

    // Redraw
    update();

    // Emit a 'system changed' event
    emit batchParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::duplicateItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() == 0)
        return;
    
    // Append to the list
    m_batches.insert(m_batches.begin() + selected.row(), m_batches[selected.row() - 1]);

    // Create its item.
    createTreeItem(selected.row());

    // Redraw
    update();

    // Emit a 'system changed' event
    emit batchParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::upItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() < 2)
        return;
    
    // Top and bottom indices
    int top = selected.row() - 2, bot = selected.row() - 1;

    // Swap the elements.
    std::iter_swap(m_batches.begin() + top, m_batches.begin() + bot);

    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    // Extract the relevant items
    root->removeRows(top + 1, 2);

    // Create its item.
    createTreeItem(top);

    // Create its item.
    createTreeItem(bot);

    // Redraw
    update();

    // Emit a 'system changed' event
    emit batchParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::downItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() < 1 || selected.row() == m_batches.size())
        return;
    
    // Top and bottom indices
    int top = selected.row() - 1, bot = selected.row();

    // Swap the elements.
    std::iter_swap(m_batches.begin() + top, m_batches.begin() + bot);

    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    // Extract the relevant items
    root->removeRows(top + 1, 2);

    // Create its item.
    createTreeItem(top);

    // Create its item.
    createTreeItem(bot);

    // Redraw
    update();

    // Emit a 'system changed' event
    emit batchParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::deleteItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() < 1)
        return;
    
    // Erase the selected element from the list
    m_batches.erase(m_batches.begin() + selected.row() - 1);
    
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    /// Remove the elements corresponding to the removed elements
    root->removeRow(selected.row());

    // Redraw
    update();

    // Emit a 'system changed' event
    emit batchParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::clearItems()
{
    // Extract the list of selected rows.
    m_batches.clear();
    
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    /// Remove the elements corresponding to the removed elements
    root->removeRows(1, root->rowCount() - 1);

    // Redraw
    update();

    // Emit a 'system changed' event
    emit batchParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::createTreeAttributes()
{
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    // Append the attributes header
    QStandardItem* attributesEmpty = new QStandardItem();
    QStandardItem* attributesHeader = new QStandardItem("Attributes");
    attributesHeader->setEditable(false);
    attributesEmpty->setEditable(false);
    //attributesHeader->setColumnCount(2);
    root->appendRow({ attributesHeader, attributesEmpty });

    // Add the attributes.
    const auto& previewerAttributes = getGlobalAttributes();
    for (auto attrib: previewerAttributes)
    {
        // Create the name item
        QStandardItem* name = new QStandardItem();
        QStandardItem* value = new QStandardItem();
        name->setEditable(false);

        // Append the row
        attributesHeader->appendRow({ name, value });
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::createTreeItem(int batchId)
{
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    // Append the lens element header
    QStandardItem* elementEmpty = new QStandardItem();
    QStandardItem* elementHeader = new QStandardItem();
    elementEmpty->setEditable(false);
    elementHeader->setEditable(false);
    root->insertRow(batchId + 1, { elementHeader, elementEmpty });

    // Add the attributes.
    const auto& batchAttributes = getBatchAttributes(batchId);
    for (auto attrib: batchAttributes)
    {
        // Create the name and value items
        QStandardItem* name = new QStandardItem();
        QStandardItem* value = new QStandardItem();
        name->setEditable(false);

        // Append the row
        elementHeader->appendRow({ name, value });
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::createTree()
{
    // Create the model
    m_treeModel = new QStandardItemModel(this);
    m_treeModel->setColumnCount(2);
    m_treeModel->setHorizontalHeaderLabels({ "Name", "Value" });

    // Create an item for the attributes
    createTreeAttributes();

    // Create items for the elements
    for (size_t i = 0; i < m_batches.size(); ++i)
    {
        createTreeItem((int) i);
    }

    // Create the view
    m_tree = new QTreeView(this);
    m_tree->setModel(m_treeModel);
    m_tree->setItemDelegate(new AttributeCellDelegate(this));
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tree->setEditTriggers(QAbstractItemView::CurrentChanged);
    m_tree->expandAll();
    m_tree->setItemsExpandable(false);
    m_tree->setRootIsDecorated(false);

    auto totalWidth = m_tree->columnWidth(0) + m_tree->columnWidth(1);
    m_tree->setColumnWidth(0, 180);
    m_tree->setColumnWidth(1, totalWidth - 180);
    
    connect(m_treeModel, &QStandardItemModel::itemChanged, 
        this, &OpticalSystemPreviewProperties::itemChanged);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::createToolBar()
{
    // Create the toolbar at the top
    m_toolBar = new QToolBar(this);
    m_toolBar->setIconSize(QSize(m_toolBar->iconSize().width() * 0.7, 
        m_toolBar->iconSize().height() * 0.7));

    // 'Add' action
    QAction* addWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/add128.png"), 
        "Add a new ray batch", this);
    m_toolBar->addAction(addWidgetAction);
    connect(addWidgetAction, &QAction::triggered, 
        this, &OpticalSystemPreviewProperties::addItem);
        
    // 'Duplicate' action
    QAction* duplicateWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/copy128.png"), 
        "Duplicate the selected batch", this);
    m_toolBar->addAction(duplicateWidgetAction);
    connect(duplicateWidgetAction, &QAction::triggered, 
        this, &OpticalSystemPreviewProperties::duplicateItem);
        
    // 'Up' action
    QAction* upWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/up128.png"), 
        "Move the selected batch up", this);
    m_toolBar->addAction(upWidgetAction);
    connect(upWidgetAction, &QAction::triggered, 
        this, &OpticalSystemPreviewProperties::upItem);
        
    // 'Down' action
    QAction* downWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/down128.png"), 
        "Move the selected batch down", this);
    m_toolBar->addAction(downWidgetAction);
    connect(downWidgetAction, &QAction::triggered, 
        this, &OpticalSystemPreviewProperties::downItem);

    // 'Remove' action
    QAction* removeWidgetsAction = new QAction(QIcon(":/LensPlanner/Icons/delete128.png"), 
        "Remove the selected batch", this);
    m_toolBar->addAction(removeWidgetsAction);
    connect(removeWidgetsAction, &QAction::triggered, 
        this, &OpticalSystemPreviewProperties::deleteItem);

    // 'Clear' action
    QAction* clearWidgetsAction = new QAction(QIcon(":/LensPlanner/Icons/trash128.png"), 
        "Remove all batches", this);
    m_toolBar->addAction(clearWidgetsAction);
    connect(clearWidgetsAction, &QAction::triggered, 
        this, &OpticalSystemPreviewProperties::clearItems);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewProperties::createLayout()
{
    // Create the layout object.
    m_layout = new QVBoxLayout(this);

    // Add the widgets
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_tree);

    // Set the layout;
    setLayout(m_layout);
}