#include "OpticalSystemEditor.h"

////////////////////////////////////////////////////////////////////////////////
/// List of all the optical system attributes.
static QVector<OpticalSystemEditorCellData> getOpticalSystemAttributes(OLEF::OpticalSystem& opticalSystem)
{
    return
    {
        {
            "Name",
            std::bind(&OLEF::OpticalSystem::getName, std::ref(opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setName, std::ref(opticalSystem), std::placeholders::_1),
        },
        {
            "F-number",
            std::bind(&OLEF::OpticalSystem::getFnumber, std::ref(opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setFnumber, std::ref(opticalSystem), std::placeholders::_1),
            0.0f,
            64.0f,
        },
        {
            "Focal Length",
            std::bind(&OLEF::OpticalSystem::getEffectiveFocalLength, std::ref(opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setEffectiveFocalLength, std::ref(opticalSystem), std::placeholders::_1),
            0.0f,
            500.0f,
        },
        {
            "Field of View", 
            std::bind(&OLEF::OpticalSystem::getFieldOfView, std::ref(opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setFieldOfView, std::ref(opticalSystem), std::placeholders::_1),
            0.0f,
            180.0f
        },
        {
            "Film Width",
            std::bind(&OLEF::OpticalSystem::getFilmWidth, std::ref(opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setFilmWidth, std::ref(opticalSystem), std::placeholders::_1),
            0.0f,
            200.0f,
        },
        {
            "Film Height",
            std::bind(&OLEF::OpticalSystem::getFilmHeight, std::ref(opticalSystem)), 
            std::bind(&OLEF::OpticalSystem::setFilmHeight, std::ref(opticalSystem), std::placeholders::_1),
            0.0f,
            200.0f,
        },
    };
}

////////////////////////////////////////////////////////////////////////////////
/// List of all the optical element attributes.
static QVector<OpticalSystemEditorCellData> getOpticalElementAttributes(OLEF::OpticalSystemElement& element)
{
    return
    {
        { 
            "Type",
            std::bind(&OLEF::OpticalSystemElement::getType, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setType, std::ref(element), std::placeholders::_1),
        },
        {
            "Height",
            std::bind(&OLEF::OpticalSystemElement::getHeight, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setHeight, std::ref(element), std::placeholders::_1),
            0.0f,
            1000.0f,
        },
        {
            "Thickness",
            std::bind(&OLEF::OpticalSystemElement::getThickness, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setThickness, std::ref(element), std::placeholders::_1),
            0.0f,
            1000.0f,
        },
        {
            "Radius",
            std::bind(&OLEF::OpticalSystemElement::getRadiusOfCurvature, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setRadiusOfCurvature, std::ref(element), std::placeholders::_1),
            -10000,
            10000,
        },
        {
            "Refractive Index",
            std::bind(&OLEF::OpticalSystemElement::getIndexOfRefraction, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setIndexOfRefraction, std::ref(element), std::placeholders::_1),
            0.0f,
            10.0f,
        },
        {
            "Abbe Number",
            std::bind(&OLEF::OpticalSystemElement::getAbbeNumber, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setAbbeNumber, std::ref(element), std::placeholders::_1),
            0.0f,
            1000.0f,
        },
        {
            "Coating Wavelength",
            std::bind(&OLEF::OpticalSystemElement::getCoatingLambda, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setCoatingLambda, std::ref(element), std::placeholders::_1),
            0.0f,
            1000.0f,
        },
        {
            "Coating Refractive Index",
            std::bind(&OLEF::OpticalSystemElement::getCoatingIor, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setCoatingIor, std::ref(element), std::placeholders::_1),
            0.0f,
            10.0f
        },
        {
            "Mask Texture",
            std::bind(&OLEF::OpticalSystemElement::getTexture, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setTexture, std::ref(element), std::placeholders::_1),
        },
        {
            "Mask Texture FT",
            std::bind(&OLEF::OpticalSystemElement::getTextureFT, std::ref(element)),
            std::bind(&OLEF::OpticalSystemElement::setTextureFT, std::ref(element), std::placeholders::_1),
        },
    };
}

/// Textual representation of all the element types.
const QStringList s_elemTypes =
{
    "Lens (Spherical)",
    "Lens (Aspherical)",
    "Aperture",
    "Sensor"
};

////////////////////////////////////////////////////////////////////////////////
OpticalSystemEditorDelegate::OpticalSystemEditorDelegate(ImageLibrary* imgLib,
    QWidget* parent): 
        QStyledItemDelegate(parent),
        m_imageLibrary(imgLib)
{}

////////////////////////////////////////////////////////////////////////////////
QWidget* OpticalSystemEditorDelegate::createEditor(QWidget* parent, 
    const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto& data = index.data(Qt::UserRole + 1);
    if (data.canConvert<OpticalSystemEditorCellData>())
    {
        // Extract the editor data
        OpticalSystemEditorCellData cellData = 
            qvariant_cast<OpticalSystemEditorCellData>(data);

        // Switch on its type
        switch (cellData.m_type)
        {
        // Element type
        case OpticalSystemEditorCellData::TYPE:
        {
            QComboBox* comboBox = new QComboBox(parent);
            comboBox->addItems(s_elemTypes);

            return comboBox;
        }

        // Float value
        case OpticalSystemEditorCellData::FLOAT:
        {
            QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
            spinBox->setMinimum(cellData.m_min);
            spinBox->setMaximum(cellData.m_max);
            //spinBox->setFrame(false);
            
            return spinBox;
        }

        // String value
        case OpticalSystemEditorCellData::STRING:
        {
            QLineEdit* lineEdit = new QLineEdit(parent);
            
            return lineEdit;
        }
        break;

        // Texture value
        case OpticalSystemEditorCellData::TEXTURE:
        {
            QWidget* widget = new QWidget(parent);
            
            QLineEdit* lineEdit = new QLineEdit(widget);
            lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            lineEdit->setReadOnly(true);
            QPushButton* browseButton = new QPushButton("...", widget);
            browseButton->resize(1, browseButton->height());
            browseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            connect(browseButton, &QPushButton::pressed, [=]()
            {
                auto filePath = m_imageLibrary->loadImageDialog(widget, "Load " + cellData.m_name);
                lineEdit->setText(filePath);
            });

            QHBoxLayout* layout = new QHBoxLayout(widget);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(lineEdit);
            layout->addWidget(browseButton);
            widget->setLayout(layout);
            browseButton->resize(16, browseButton->height());

            return widget;
        }
        break;
        }

        return QStyledItemDelegate::createEditor(parent, option, index);
    }
    else
    {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditorDelegate::setEditorData(QWidget* editor, 
    const QModelIndex& index) const
{
    const auto& data = index.data(Qt::UserRole + 1);
    if (data.canConvert<OpticalSystemEditorCellData>())
    {
        OpticalSystemEditorCellData cellData = 
            qvariant_cast<OpticalSystemEditorCellData>(data);

        // Switch on its type
        switch (cellData.m_type)
        {
        // Element type
        case OpticalSystemEditorCellData::TYPE:
        {
            OLEF::OpticalSystemElement::ElementType elemType = 
                cellData.m_typeGetter();

            QComboBox* comboBox = (QComboBox*) editor;
            comboBox->setCurrentIndex((int) elemType);
        }
        break;

        // Float value
        case OpticalSystemEditorCellData::FLOAT:
        {
            float value = cellData.m_floatGetter();

            QDoubleSpinBox* spinBox = (QDoubleSpinBox*) editor;
            spinBox->setValue(value);
        }
        break;

        // String value
        case OpticalSystemEditorCellData::STRING:
        {
            const std::string& value = cellData.m_stringGetter();

            QLineEdit* lineEdit = (QLineEdit*) editor;
            lineEdit->setText(QString::fromStdString(value));
        }
        break;

        // Texture value
        case OpticalSystemEditorCellData::TEXTURE:
        {
            GLuint value = cellData.m_textureGetter();

            QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
            lineEdit->setText(m_imageLibrary->lookUpTextureName(value));
        }
        break;
        }
    }
    else
    {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemEditorDelegate::setModelData(QWidget* editor, 
    QAbstractItemModel* model, const QModelIndex& index) const
{
    const auto& data = index.data(Qt::UserRole + 1);
    if (data.canConvert<OpticalSystemEditorCellData>())
    {
        OpticalSystemEditorCellData cellData = 
            qvariant_cast<OpticalSystemEditorCellData>(data);

        // Switch on its type
        switch (cellData.m_type)
        {
        // Element type
        case OpticalSystemEditorCellData::TYPE:
        {
            QComboBox* comboBox = (QComboBox*) editor;
            int current = comboBox->currentIndex();
            OLEF::OpticalSystemElement::ElementType elemType = 
                (OLEF::OpticalSystemElement::ElementType) current;

            cellData.m_typeSetter(elemType);
            model->setData(index, s_elemTypes[current], Qt::EditRole);
        }
        break;

        // Float value
        case OpticalSystemEditorCellData::FLOAT:
        {
            QDoubleSpinBox* spinBox = (QDoubleSpinBox*) editor;
            float value = spinBox->value();

            cellData.m_floatSetter(value);
            model->setData(index, value, Qt::EditRole);
        }
        break;

        // String value
        case OpticalSystemEditorCellData::STRING:
        {
            QLineEdit* lineEdit = (QLineEdit*) editor;
            QString value = lineEdit->text();

            cellData.m_stringSetter(value.toStdString());
            model->setData(index, value, Qt::EditRole);
        }
        break;

        // Texture value
        case OpticalSystemEditorCellData::TEXTURE:
        {
            QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
            QString value = lineEdit->text();
            QFileInfo fileInfo(value);

            cellData.m_textureSetter(m_imageLibrary->uploadTexture(value));
            model->setData(index, fileInfo.fileName(), Qt::EditRole);
        }
        break;
        }
    }
    else
    {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

////////////////////////////////////////////////////////////////////////////////
OpticalSystemEditor::OpticalSystemEditor(ImageLibrary* imageLibrary,
    OLEF::OpticalSystem* system, QWidget* parent):
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
void OpticalSystemEditor::updateTreeAttribute(QStandardItem* item, OpticalSystemEditorCellData attrib)
{
    // Store the cell data
    QVariant data;
    data.setValue(attrib);
    item->setData(data, Qt::UserRole + 1);

    // Set its current value
    switch (attrib.m_type)
    {
    // Element type
    case OpticalSystemEditorCellData::TYPE:
    {
        OLEF::OpticalSystemElement::ElementType elemType = attrib.m_typeGetter();
        item->setData(s_elemTypes[(int) elemType], Qt::EditRole);
    }
    break;

    // Float value
    case OpticalSystemEditorCellData::FLOAT:
    {
        float floatVal = attrib.m_floatGetter();
        item->setData(floatVal, Qt::EditRole);
    }
    break;

    // String value
    case OpticalSystemEditorCellData::STRING:
    {
        const std::string& stringVal = attrib.m_stringGetter();
        item->setData(QString::fromStdString(stringVal), Qt::EditRole);
    }
    break;

    // Texture value
    case OpticalSystemEditorCellData::TEXTURE:
    {
        GLuint value = attrib.m_textureGetter();
        QFileInfo fileInfo(m_imageLibrary->lookUpTextureName(value));
        item->setData(fileInfo.fileName(), Qt::EditRole);
    }
    break;
    }
}

void OpticalSystemEditor::update()
{
    /// Extract the root item
    QStandardItem* root = m_treeModel->invisibleRootItem();
    
    /// Extract the attributes header
    QStandardItem* attributesHeader = root->child(0);

    // Refresh the system attributes
    const auto& systemAttributes = getOpticalSystemAttributes(*m_opticalSystem);
    for (int attribId = 0; attribId < systemAttributes.size(); ++attribId)
    {
        updateTreeAttribute(attributesHeader->child(attribId, 1), 
            systemAttributes[attribId]);
    }

    // Refresh the elements
    for (int elemId = 0; elemId < m_opticalSystem->getElementCount(); ++elemId)
    {
        // Extract the appropriate root item.
        QStandardItem* header = root->child(elemId + 1);
        header->setText(QString("Element #%0").arg(elemId + 1));

        // Add the attributes.
        const auto& lensAttributes = 
            getOpticalElementAttributes((*m_opticalSystem)[elemId]);

        for (int attribId = 0; attribId < lensAttributes.size(); ++attribId)
        {
            updateTreeAttribute(header->child(attribId, 1), 
                lensAttributes[attribId]);
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
    const auto& systemAttributes = getOpticalSystemAttributes(*m_opticalSystem);
    for (auto attrib: systemAttributes)
    {
        // Create the name item
        QStandardItem* name = new QStandardItem(attrib.m_name);
        name->setEditable(false);

        // Create the value item
        QStandardItem* value = new QStandardItem();
        QVariant data;
        data.setValue(attrib);
        value->setData(data, Qt::UserRole + 1);

        // Set its current value
        switch (attrib.m_type)
        {
        // Element type
        case OpticalSystemEditorCellData::TYPE:
        {
            OLEF::OpticalSystemElement::ElementType elemType = attrib.m_typeGetter();
            value->setData(s_elemTypes[(int) elemType], Qt::EditRole);
        }
        break;

        // Float value
        case OpticalSystemEditorCellData::FLOAT:
        {
            float floatVal = attrib.m_floatGetter();
            value->setData(floatVal, Qt::EditRole);
        }
        break;
        }

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
    QStandardItem* elementHeader = 
        new QStandardItem(QString("Element #%0").arg(elemId + 1));
    elementEmpty->setEditable(false);
    elementHeader->setEditable(false);
    root->insertRow(elemId + 1, { elementHeader, elementEmpty });

    // Add the attributes.
    const auto& lensAttributes = 
        getOpticalElementAttributes((*m_opticalSystem)[elemId]);

    for (auto attrib: lensAttributes)
    {
        // Create the name and value items
        QStandardItem* name = new QStandardItem(attrib.m_name);
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
    m_tree->setItemDelegate(new OpticalSystemEditorDelegate(m_imageLibrary, this));
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