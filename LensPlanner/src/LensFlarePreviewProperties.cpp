#include "LensFlarePreviewProperties.h"

////////////////////////////////////////////////////////////////////////////////
/// List of all the global attributes.
// TODO: these leak!
QVector<AttributeCellWidgetBase*> LensFlarePreviewProperties::getGlobalAttributes()
{
    return
    {
    };
}

////////////////////////////////////////////////////////////////////////////////
/// List of all the light source attributes.
// TODO: these leak!
QVector<AttributeCellWidgetBase*> LensFlarePreviewProperties::getLightSourceAttributes(int lightSourceId)
{
    return
    {
        new AttributeCellVec2
        {
            "Position",
            "",
            &m_lightSources[lightSourceId].m_position,
            glm::vec2(-1.0f),
            glm::vec2(1.0f),
            glm::vec2(0.01f),
        },
        new AttributeCellVec3
        {
            "Incidence Direction",
            "",
            &m_lightSources[lightSourceId].m_direction,
            glm::vec3(-1.0f),
            glm::vec3(1.0f),
            glm::vec3(0.01f),
        },
        new AttributeCellColor
        {
            "Color",
            "",
            &m_lightSources[lightSourceId].m_color,
        },
        new AttributeCellFloat
        {
            "Intensity",
            "",
            &m_lightSources[lightSourceId].m_intensity,
            0.0f,
            1000.0f,
            0.01f,
        },
    };
}

////////////////////////////////////////////////////////////////////////////////
LensFlarePreviewProperties::LensFlarePreviewProperties(LensFlarePreviewer* previewer, QWidget* parent):
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
LensFlarePreviewProperties::~LensFlarePreviewProperties()
{}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::generateLightSources()
{
    // Extract the optical system pointer
    QVector<OLEF::LightSource> lightSources;
    lightSources.reserve(m_lightSources.size());

    // Generate the actual definitions
    for (auto& lightSource: m_lightSources)
    {
        OLEF::LightSource outLightSource;

        outLightSource.setScreenPosition(lightSource.m_position);
        outLightSource.setIncidenceDirection(lightSource.m_direction);
        outLightSource.setDiffuseColor(glm::vec3(
            lightSource.m_color.redF(), 
            lightSource.m_color.greenF(), 
            lightSource.m_color.blueF()
        ));
        outLightSource.setDiffuseIntensity(lightSource.m_intensity);

        lightSources.push_back(outLightSource);
    }

    // Store the batch parameters
    m_previewer->setLightSources(lightSources);
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::updateTreeAttribute(AttributeCellWidgetBase* attrib,
        QStandardItem* name, QStandardItem* value)
{
    value->setData(QVariant::fromValue(attrib), Qt::UserRole + 1);
    attrib->refreshName(name->model(), name->index());
    attrib->refreshView(value->model(), value->index());
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::update()
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
    for (int lightSourceId = 0; lightSourceId < m_lightSources.size(); ++lightSourceId)
    {
        // Extract the appropriate root item.
        QStandardItem* header = root->child(lightSourceId + 1);
        header->setText(QString("Light Source #%0").arg(lightSourceId + 1));

        // Add the attributes.
        const auto& lightSourceAttributes = getLightSourceAttributes(lightSourceId);
        for (int attribId = 0; attribId < lightSourceAttributes.size(); ++attribId)
        {
            updateTreeAttribute(lightSourceAttributes[attribId],
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
void LensFlarePreviewProperties::itemChanged(QStandardItem* item)
{
    generateLightSources();
    m_previewer->update();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::addItem()
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
    auto dstId = (selected.isValid()) ? selected.row() : m_lightSources.size();
    
    // Append to the list
    m_lightSources.insert(m_lightSources.begin() + dstId, LightSource());

    // Create its item.
    createTreeItem(dstId);

    // Redraw
    update();

    // Emit a 'system changed' event
    emit lightSourceParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::duplicateItem()
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
    m_lightSources.insert(m_lightSources.begin() + selected.row(), m_lightSources[selected.row() - 1]);

    // Create its item.
    createTreeItem(selected.row());

    // Redraw
    update();

    // Emit a 'system changed' event
    emit lightSourceParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::upItem()
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
    std::iter_swap(m_lightSources.begin() + top, m_lightSources.begin() + bot);

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
    emit lightSourceParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::downItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() < 1 || selected.row() == m_lightSources.size())
        return;
    
    // Top and bottom indices
    int top = selected.row() - 1, bot = selected.row();

    // Swap the elements.
    std::iter_swap(m_lightSources.begin() + top, m_lightSources.begin() + bot);

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
    emit lightSourceParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::deleteItem()
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
    m_lightSources.erase(m_lightSources.begin() + selected.row() - 1);
    
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    /// Remove the elements corresponding to the removed elements
    root->removeRow(selected.row());

    // Redraw
    update();

    // Emit a 'system changed' event
    emit lightSourceParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::clearItems()
{
    // Extract the list of selected rows.
    m_lightSources.clear();
    
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    /// Remove the elements corresponding to the removed elements
    root->removeRows(1, root->rowCount() - 1);

    // Redraw
    update();

    // Emit a 'system changed' event
    emit lightSourceParametersChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::createTreeAttributes()
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
void LensFlarePreviewProperties::createTreeItem(int lightSourceId)
{
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    // Append the lens element header
    QStandardItem* elementEmpty = new QStandardItem();
    QStandardItem* elementHeader = new QStandardItem();
    elementEmpty->setEditable(false);
    elementHeader->setEditable(false);
    root->insertRow(lightSourceId + 1, { elementHeader, elementEmpty });

    // Add the attributes.
    const auto& lightSourceAttributes = getLightSourceAttributes(lightSourceId);
    for (auto attrib: lightSourceAttributes)
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
void LensFlarePreviewProperties::createTree()
{
    // Create the model
    m_treeModel = new QStandardItemModel(this);
    m_treeModel->setColumnCount(2);
    m_treeModel->setHorizontalHeaderLabels({ "Name", "Value" });

    // Create an item for the attributes
    createTreeAttributes();

    // Create items for the elements
    for (size_t i = 0; i < m_lightSources.size(); ++i)
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
        this, &LensFlarePreviewProperties::itemChanged);
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::createToolBar()
{
    // Create the toolbar at the top
    m_toolBar = new QToolBar(this);
    m_toolBar->setIconSize(QSize(m_toolBar->iconSize().width() * 0.7, 
        m_toolBar->iconSize().height() * 0.7));

    // 'Add' action
    QAction* addWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/add128.png"), 
        "Add a new light source", this);
    m_toolBar->addAction(addWidgetAction);
    connect(addWidgetAction, &QAction::triggered, 
        this, &LensFlarePreviewProperties::addItem);
        
    // 'Duplicate' action
    QAction* duplicateWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/copy128.png"), 
        "Duplicate the selected light source", this);
    m_toolBar->addAction(duplicateWidgetAction);
    connect(duplicateWidgetAction, &QAction::triggered, 
        this, &LensFlarePreviewProperties::duplicateItem);
        
    // 'Up' action
    QAction* upWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/up128.png"), 
        "Move the selected light source up", this);
    m_toolBar->addAction(upWidgetAction);
    connect(upWidgetAction, &QAction::triggered, 
        this, &LensFlarePreviewProperties::upItem);
        
    // 'Down' action
    QAction* downWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/down128.png"), 
        "Move the selected light source down", this);
    m_toolBar->addAction(downWidgetAction);
    connect(downWidgetAction, &QAction::triggered, 
        this, &LensFlarePreviewProperties::downItem);

    // 'Remove' action
    QAction* removeWidgetsAction = new QAction(QIcon(":/LensPlanner/Icons/delete128.png"), 
        "Remove the selected light source", this);
    m_toolBar->addAction(removeWidgetsAction);
    connect(removeWidgetsAction, &QAction::triggered, 
        this, &LensFlarePreviewProperties::deleteItem);

    // 'Clear' action
    QAction* clearWidgetsAction = new QAction(QIcon(":/LensPlanner/Icons/trash128.png"), 
        "Remove all light sources", this);
    m_toolBar->addAction(clearWidgetsAction);
    connect(clearWidgetsAction, &QAction::triggered, 
        this, &LensFlarePreviewProperties::clearItems);
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewProperties::createLayout()
{
    // Create the layout object.
    m_layout = new QVBoxLayout(this);

    // Add the widgets
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_tree);

    // Set the layout;
    setLayout(m_layout);
}