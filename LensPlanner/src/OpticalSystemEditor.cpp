#include "OpticalSystemEditor.h"

////////////////////////////////////////////////////////////////////////////////
/// List of all the optical system attributes.
// TODO: these leak!
QVector<AttributeCellWidgetBase*> OpticalSystemEditor::getOpticalSystemAttributes()
{
    return
    {
        new AttributeCellString
        {
            "Name",
            "",
            std::bind(&OLEF::OpticalSystem::getName, std::ref(*m_opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setName, std::ref(*m_opticalSystem), std::placeholders::_1),
        },
        new AttributeCellFloat
        {
            "F-number",
            "",
            std::bind(&OLEF::OpticalSystem::getFnumber, std::ref(*m_opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setFnumber, std::ref(*m_opticalSystem), std::placeholders::_1),
            0.0f,
            64.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Focal Length",
            "",
            std::bind(&OLEF::OpticalSystem::getEffectiveFocalLength, std::ref(*m_opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setEffectiveFocalLength, std::ref(*m_opticalSystem), std::placeholders::_1),
            0.0f,
            500.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Field of View", 
            "",
            std::bind(&OLEF::OpticalSystem::getFieldOfView, std::ref(*m_opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setFieldOfView, std::ref(*m_opticalSystem), std::placeholders::_1),
            0.0f,
            180.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Film Width",
            "",
            std::bind(&OLEF::OpticalSystem::getFilmWidth, std::ref(*m_opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setFilmWidth, std::ref(*m_opticalSystem), std::placeholders::_1),
            0.0f,
            200.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Film Height",
            "",
            std::bind(&OLEF::OpticalSystem::getFilmHeight, std::ref(*m_opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setFilmHeight, std::ref(*m_opticalSystem), std::placeholders::_1),
            0.0f,
            200.0f,
            1.0f,
        },
    };
}

////////////////////////////////////////////////////////////////////////////////
/// List of all the optical element attributes.
// TODO: these leak!
QVector<AttributeCellWidgetBase*> OpticalSystemEditor::getOpticalElementAttributes(int elemId)
{
    auto* element = &((*m_opticalSystem)[elemId]);
    return
    {
        new AttributeCellEnum<OLEF::OpticalSystemElement::ElementType>
        { 
            "Type",
            "",
            std::bind(&OLEF::OpticalSystemElement::getType, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setType, std::ref(*element), std::placeholders::_1),
            {
                "Lens (Spherical)",
                "Lens (Aspherical)",
                "Aperture",
                "Sensor"
            },
        },
        new AttributeCellFloat
        {
            "Height",
            "",
            std::bind(&OLEF::OpticalSystemElement::getHeight, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setHeight, std::ref(*element), std::placeholders::_1),
            0.0f,
            1000.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Thickness",
            "",
            std::bind(&OLEF::OpticalSystemElement::getThickness, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setThickness, std::ref(*element), std::placeholders::_1),
            0.0f,
            1000.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Radius",
            "",
            std::bind(&OLEF::OpticalSystemElement::getRadiusOfCurvature, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setRadiusOfCurvature, std::ref(*element), std::placeholders::_1),
            -10000,
            10000,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Refractive Index",
            "",
            std::bind(&OLEF::OpticalSystemElement::getIndexOfRefraction, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setIndexOfRefraction, std::ref(*element), std::placeholders::_1),
            0.0f,
            10.0f,
            0.1f,
        },
        new AttributeCellFloat
        {
            "Abbe Number",
            "",
            std::bind(&OLEF::OpticalSystemElement::getAbbeNumber, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setAbbeNumber, std::ref(*element), std::placeholders::_1),
            0.0f,
            1000.0f,
            0.1f,
        },
        new AttributeCellFloat
        {
            "Coating Wavelength",
            "",
            std::bind(&OLEF::OpticalSystemElement::getCoatingLambda, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setCoatingLambda, std::ref(*element), std::placeholders::_1),
            0.0f,
            1000.0f,
            1.0f,
        },
        new AttributeCellFloat
        {
            "Coating Refractive Index",
            "",
            std::bind(&OLEF::OpticalSystemElement::getCoatingIor, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setCoatingIor, std::ref(*element), std::placeholders::_1),
            0.0f,
            10.0f,
            0.1f,
        },
        new AttributeCellTexture
        {
            "Mask Texture",
            "",
            std::bind(&OLEF::OpticalSystemElement::getTexture, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setTexture, std::ref(*element), std::placeholders::_1),
            m_imageLibrary,
        },
        new AttributeCellTexture
        {
            "Mask Texture FT",
            "",
            std::bind(&OLEF::OpticalSystemElement::getTextureFT, std::ref(*element)),
            std::bind(&OLEF::OpticalSystemElement::setTextureFT, std::ref(*element), std::placeholders::_1),
            m_imageLibrary,
        },
    };
}

////////////////////////////////////////////////////////////////////////////////
OpticalSystemEditor::OpticalSystemEditor(ImageLibrary* imageLibrary, OLEF::OpticalSystem* system, QWidget* parent):
    QWidget(parent),
    m_imageLibrary(imageLibrary),
    m_opticalSystem(system)
{    
    // Setup the UI
    createToolBar();
    createTree();
    createLayout();

    // Call an update to set the item texts.
    update();

    // Set a minimum size.
    setMinimumSize(QSize(350, 300));
}

////////////////////////////////////////////////////////////////////////////////
OpticalSystemEditor::~OpticalSystemEditor()
{}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::updateTreeAttribute(AttributeCellWidgetBase* attrib,
        QStandardItem* name, QStandardItem* value)
{
    value->setData(QVariant::fromValue(attrib), Qt::UserRole + 1);
    attrib->refreshName(name->model(), name->index());
    attrib->refreshView(value->model(), value->index());
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::update()
{
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();
    
    /// Extract the attributes header
    QStandardItem* attributesHeader = root->child(0);

    // Refresh the system attributes
    const auto& systemAttributes = getOpticalSystemAttributes();
    for (int attribId = 0; attribId < systemAttributes.size(); ++attribId)
    {
        updateTreeAttribute(
            systemAttributes[attribId],
            attributesHeader->child(attribId, 0),
            attributesHeader->child(attribId, 1));
    }

    // Refresh the elements
    for (int elemId = 0; elemId < m_opticalSystem->getElementCount(); ++elemId)
    {
        // Extract the appropriate root item.
        QStandardItem* header = root->child(elemId + 1);
        header->setText(QString("Element #%0").arg(elemId + 1));

        // Add the attributes.
        const auto& lensAttributes = getOpticalElementAttributes(elemId);
        for (int attribId = 0; attribId < lensAttributes.size(); ++attribId)
        {
            updateTreeAttribute(lensAttributes[attribId],
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
void OpticalSystemEditor::opticalSystemChanged()
{
    // Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    // Remove the elements corresponding to the old elements
    root->removeRows(1, root->rowCount() - 1);

    // Create the new items
    for (int i = 0; i < m_opticalSystem->getElementCount(); ++i)
        createTreeItem(i);
    
    // Refresh the view
    update();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::itemChanged(QStandardItem* item)
{
    emit opticalSystemChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::createToolBar()
{
    // Create the toolbar at the top
    m_toolBar = new QToolBar(this);
    m_toolBar->setIconSize(QSize(m_toolBar->iconSize().width() * 0.7, 
        m_toolBar->iconSize().height() * 0.7));

    // 'Add' action
    QAction* addWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/add128.png"), 
        "Add a new element", this);
    m_toolBar->addAction(addWidgetAction);
    connect(addWidgetAction, &QAction::triggered, 
        this, &OpticalSystemEditor::addItem);
        
    // 'Duplicate' action
    QAction* duplicateWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/copy128.png"), 
        "Duplicate the selected element", this);
    m_toolBar->addAction(duplicateWidgetAction);
    connect(duplicateWidgetAction, &QAction::triggered, 
        this, &OpticalSystemEditor::duplicateItem);
        
    // 'Up' action
    QAction* upWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/up128.png"), 
        "Move the selected element up", this);
    m_toolBar->addAction(upWidgetAction);
    connect(upWidgetAction, &QAction::triggered, 
        this, &OpticalSystemEditor::upItem);
        
    // 'Down' action
    QAction* downWidgetAction = new QAction(QIcon(":/LensPlanner/Icons/down128.png"), 
        "Move the selected element down", this);
    m_toolBar->addAction(downWidgetAction);
    connect(downWidgetAction, &QAction::triggered, 
        this, &OpticalSystemEditor::downItem);

    // 'Remove' action
    QAction* removeWidgetsAction = new QAction(QIcon(":/LensPlanner/Icons/delete128.png"), 
        "Remove the selected element", this);
    m_toolBar->addAction(removeWidgetsAction);
    connect(removeWidgetsAction, &QAction::triggered, 
        this, &OpticalSystemEditor::deleteItem);

    // 'Clear' action
    QAction* clearWidgetsAction = new QAction(QIcon(":/LensPlanner/Icons/trash128.png"), 
        "Remove all elements", this);
    m_toolBar->addAction(clearWidgetsAction);
    connect(clearWidgetsAction, &QAction::triggered, 
        this, &OpticalSystemEditor::clearItems);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::addItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Extract the element list
    auto elements = m_opticalSystem->getElements();

    // Index where the new element should go.
    // selected.row() is always 1 higher than the element index due to the 
    // first 'Attributes' row, so we can use it to append after the selected 
    // element.
    auto dstId = (selected.isValid()) ? selected.row() : elements.size();
    
    // Create the new element
    OLEF::OpticalSystemElement newElement;
    
    // Append to the list
    elements.insert(elements.begin() + dstId, newElement);

    // Store the new element list
    m_opticalSystem->setElements(elements);

    // Create its item.
    createTreeItem(dstId);

    // Redraw
    update();

    // Emit a 'system changed' event
    emit opticalSystemChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::duplicateItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() == 0)
        return;

    // Extract the element list
    auto elements = m_opticalSystem->getElements();
    
    // Create the new element
    OLEF::OpticalSystemElement newElement = elements[selected.row() - 1];
    
    // Append to the list
    elements.insert(elements.begin() + selected.row(), newElement);

    // Store the new element list
    m_opticalSystem->setElements(elements);

    // Create its item.
    createTreeItem(selected.row());

    // Redraw
    update();

    // Emit a 'system changed' event
    emit opticalSystemChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::upItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() < 2)
        return;

    // Extract the element list
    auto elements = m_opticalSystem->getElements();
    
    // Top and bottom indices
    int top = selected.row() - 2, bot = selected.row() - 1;

    // Swap the elements.
    std::iter_swap(elements.begin() + top, elements.begin() + bot);

    // Store the new element list
    m_opticalSystem->setElements(elements);

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
    emit opticalSystemChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::downItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() < 1 || 
        selected.row() == m_opticalSystem->getElementCount())
        return;

    // Extract the element list
    auto elements = m_opticalSystem->getElements();
    
    // Top and bottom indices
    int top = selected.row() - 1, bot = selected.row();

    // Swap the elements.
    std::iter_swap(elements.begin() + top, elements.begin() + bot);

    // Store the new element list
    m_opticalSystem->setElements(elements);

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
    emit opticalSystemChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::deleteItem()
{
    // Extract the list of selected rows.
    auto selected = m_tree->currentIndex();
    
    // Travel up until the root element.
    for (auto tmp = selected; tmp.isValid(); tmp = tmp.parent())
        selected = tmp;

    // Make sure a valid element is selected
    if (!selected.isValid() || selected.row() < 1)
        return;

    // Extract the element list
    auto elements = m_opticalSystem->getElements();
    
    // Erase the selected element from the list
    elements.erase(elements.begin() + selected.row() - 1);

    // Store the new element list
    m_opticalSystem->setElements(elements);
    
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    /// Remove the elements corresponding to the removed elements
    root->removeRow(selected.row());

    // Redraw
    update();

    // Emit a 'system changed' event
    emit opticalSystemChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::clearItems()
{
    // Extract the list of selected rows.
    m_opticalSystem->setElements({});
    
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    /// Remove the elements corresponding to the removed elements
    root->removeRows(1, root->rowCount() - 1);

    // Redraw
    update();

    // Emit a 'system changed' event
    emit opticalSystemChangedSignal();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::createTreeAttributes()
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
    const auto& systemAttributes = getOpticalSystemAttributes();
    for (auto attrib: systemAttributes)
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
void OpticalSystemEditor::createTreeItem(int elemId)
{
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();

    // Append the lens element header
    QStandardItem* elementEmpty = new QStandardItem();
    QStandardItem* elementHeader = new QStandardItem();
    elementEmpty->setEditable(false);
    elementHeader->setEditable(false);
    root->insertRow(elemId + 1, { elementHeader, elementEmpty });

    // Add the attributes.
    const auto& lensAttributes = getOpticalElementAttributes(elemId);
    for (auto attrib: lensAttributes)
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
void OpticalSystemEditor::createTree()
{
    // Create the model
    m_treeModel = new QStandardItemModel(this);
    m_treeModel->setColumnCount(2);
    m_treeModel->setHorizontalHeaderLabels({ "Name", "Value" });

    // Create an item for the attributes
    createTreeAttributes();

    // Create items for the elements
    for (size_t i = 0; i < m_opticalSystem->getElementCount(); ++i)
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
        this, &OpticalSystemEditor::itemChanged);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditor::createLayout()
{
    // Create the layout object.
    m_layout = new QVBoxLayout(this);

    // Add the widgets
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_tree);

    // Set the layout;
    setLayout(m_layout);
}