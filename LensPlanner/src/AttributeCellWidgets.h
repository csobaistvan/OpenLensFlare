#pragma once

#include "Dependencies.h"
#include "AttributeCellDelegate.h"
#include "ImageLibrary.h"

/// Data related to an editor cell.
template<typename AttribType, typename... DataTypes>
struct AttributeCellWidgetTypedBase: public AttributeCellWidgetBase
{
    /// Getter and setter function types.
    using Getter = std::function<AttribType()>;
    using Setter = std::function<void (AttribType)>;

    /// Empty default constructor.
    AttributeCellWidgetTypedBase()
    {}

    /// Constructs a cell data object with the specified name, a pointer to the
    /// attribute and the specified data.
    AttributeCellWidgetTypedBase(const QString& name, const QString& description,
        AttribType* attrib, DataTypes... data):
            m_name(name),
            m_getter{[=]() { return *attrib; }},
            m_setter{[=](const auto& value) { *attrib = value; }},
            m_data(std::forward<DataTypes>(data)...)
    {}

    /// Constructs a cell data object with the specified name, a getter/setter
    /// pair and the specified data.
    AttributeCellWidgetTypedBase(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, DataTypes... data):
            m_name(name),
            m_getter(getter),
            m_setter(setter),
            m_data(std::forward<DataTypes>(data)...)
    {}

    // AttributeCellWidgetBase methods
    virtual void refreshName(QAbstractItemModel* model, const QModelIndex& index) override
    {
        //item->setData(m_name, Qt::DisplayRole);
    }

    virtual void refreshView(QAbstractItemModel* model, const QModelIndex& index) override
    {
        //item->setData(QVariant::fromValue(m_getter()), Qt::EditRole);
    }

    virtual void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) override
    {
        //m_setter(qvariant_cast<AttribType>(item->data(Qt::EditRole)));
    }

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) override
    {}

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) override
    {
        return nullptr;
    }

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) override
    {}

    virtual void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) override
    {}

    /// Name of the attribute held in the cell.
    QString m_name;

    /// Description of the cell attribute.
    QString m_description;

    /// Getter of the attribute.
    Getter m_getter;

    /// Setter of the attribute.
    Setter m_setter;

    /// Extra data being held in the object.
    std::tuple<DataTypes...> m_data;
};

////////////////////////////////////////////////////////////////////////////////
/// The actual attribute cell widget template, without specialization.
template<typename AttribType, typename... DataTypes>
struct AttributeCellWidget
{};

////////////////////////////////////////////////////////////////////////////////
/// Optical system element type attribute functions.
template<>
struct AttributeCellWidget<OLEF::OpticalSystemElement::ElementType>: 
    public AttributeCellWidgetTypedBase<OLEF::OpticalSystemElement::ElementType>
{
    /// Textual representation of all the element types.
    static QStringList typeNames()
    {
        static const QStringList s_typeNames =  
        {
            "Lens (Spherical)",
            "Lens (Aspherical)",
            "Aperture",
            "Sensor"
        };

        return s_typeNames;
    }

    AttributeCellWidget():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellWidget(const QString& name, const QString& description, 
        OLEF::OpticalSystemElement::ElementType* attrib):
            AttributeCellWidgetTypedBase(name, description, attrib)
    {}

    AttributeCellWidget(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter):
            AttributeCellWidgetTypedBase(name, description, getter, setter)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) override
    {
        QString text = typeNames()[(int) m_getter()];
        model->setData(index, QVariant::fromValue(text), Qt::EditRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) override
    {
        const auto& names = typeNames();
        QString text = qvariant_cast<QString>(model->data(index, Qt::EditRole));
        auto it = std::find(names.constBegin(), names.constEnd(), text);
        int id = (it == names.constEnd()) ? 0 : it - names.constBegin();
        m_setter((OLEF::OpticalSystemElement::ElementType) id);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) override
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) override
    {
        QComboBox* comboBox = new QComboBox(parent);
        comboBox->addItems(typeNames());

        return comboBox;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) override
    {
        auto elemType = m_getter();

        QComboBox* comboBox = (QComboBox*) editor;
        comboBox->setCurrentIndex((int) elemType);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) override
    {
        QComboBox* comboBox = (QComboBox*) editor;
        int current = comboBox->currentIndex();
        auto elemType = (OLEF::OpticalSystemElement::ElementType) current;

        m_setter(elemType);
        model->setData(index, typeNames()[current], Qt::EditRole);
    }
};

/// Optical system element type attribute data type.
using AttributeCellElementType = 
    AttributeCellWidget<OLEF::OpticalSystemElement::ElementType>;
Q_DECLARE_METATYPE(AttributeCellElementType);
Q_DECLARE_METATYPE(AttributeCellElementType*);

////////////////////////////////////////////////////////////////////////////////
/// Integer attribute functions.
template<>
struct AttributeCellWidget<int, int, int>: 
    public AttributeCellWidgetTypedBase<int, int, int>
{
    AttributeCellWidget():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellWidget(const QString& name, const QString& description,
         int* attrib, int min, int max):
            AttributeCellWidgetTypedBase(name, description, attrib, min, max)
    {}

    AttributeCellWidget(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, int min, int max):
            AttributeCellWidgetTypedBase(name, description, getter, setter, min, max)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, QVariant::fromValue(m_getter()), Qt::EditRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) override
    {
        m_setter(qvariant_cast<int>(model->data(index, Qt::EditRole)));
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) override
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) override
    {
        QSpinBox* spinBox = new QSpinBox(parent);
        spinBox->setMinimum(std::get<0>(m_data));
        spinBox->setMaximum(std::get<1>(m_data));
        
        return spinBox;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) override
    {
        int value = m_getter();

        QSpinBox* spinBox = (QSpinBox*) editor;
        spinBox->setValue(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) override
    {
        QSpinBox* spinBox = (QSpinBox*) editor;
        int value = spinBox->value();

        m_setter(value);
        model->setData(index, value, Qt::EditRole);
    }
};

/// Integer attribute data type.
using AttributeCellInt = 
    AttributeCellWidget<int, int, int>;
Q_DECLARE_METATYPE(AttributeCellInt);
Q_DECLARE_METATYPE(AttributeCellInt*);

////////////////////////////////////////////////////////////////////////////////
/// Floating point attribute functions.
template<>
struct AttributeCellWidget<float, float, float>: 
    public AttributeCellWidgetTypedBase<float, float, float>
{
    AttributeCellWidget():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellWidget(const QString& name, const QString& description,
         float* attrib, float min, float max):
            AttributeCellWidgetTypedBase(name, description, attrib, min, max)
    {}

    AttributeCellWidget(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, float min, float max):
            AttributeCellWidgetTypedBase(name, description, getter, setter, min, max)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, QVariant::fromValue(m_getter()), Qt::EditRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) override
    {
        m_setter(qvariant_cast<float>(model->data(index, Qt::EditRole)));
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) override
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) override
    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
        spinBox->setMinimum(std::get<0>(m_data));
        spinBox->setMaximum(std::get<1>(m_data));
        
        return spinBox;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) override
    {
        float value = m_getter();

        QDoubleSpinBox* spinBox = (QDoubleSpinBox*) editor;
        spinBox->setValue(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) override
    {
        QDoubleSpinBox* spinBox = (QDoubleSpinBox*) editor;
        float value = spinBox->value();

        m_setter(value);
        model->setData(index, value, Qt::EditRole);
    }
};

/// Floating point attribute data type.
using AttributeCellFloat = 
    AttributeCellWidget<float, float, float>;
Q_DECLARE_METATYPE(AttributeCellFloat);
Q_DECLARE_METATYPE(AttributeCellFloat*);

////////////////////////////////////////////////////////////////////////////////
/// Texture attribute functions.
template<>
struct AttributeCellWidget<GLuint, ImageLibrary*>: 
    public AttributeCellWidgetTypedBase<GLuint, ImageLibrary*>
{
    AttributeCellWidget():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellWidget(const QString& name, const QString& description,
         GLuint* attrib, ImageLibrary* imageLibrary):
            AttributeCellWidgetTypedBase(name, description, attrib, imageLibrary)
    {}

    AttributeCellWidget(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, ImageLibrary* imageLibrary):
            AttributeCellWidgetTypedBase(name, description, getter, setter, imageLibrary)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) override
    {
        GLuint value = m_getter();
        QFileInfo fileInfo(std::get<0>(m_data)->lookUpTextureName(value));
        model->setData(index, fileInfo.fileName(), Qt::EditRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) override
    {
        QString filePath = qvariant_cast<QString>(model->data(index, Qt::EditRole));
        m_setter(std::get<0>(m_data)->uploadTexture(filePath));
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) override
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) override
    {
        // Create an empty root widget
        QWidget* widget = new QWidget(parent);
        
        // Create a text widget that holds the name of the texture.
        QLineEdit* lineEdit = new QLineEdit(widget);
        lineEdit->setReadOnly(true);
        lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

        // Create the browse button
        QPushButton* browseButton = new QPushButton("...", widget);
        browseButton->setFixedWidth(32);
        browseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
        QObject::connect(browseButton, &QPushButton::pressed, [=]()
        {
            auto filePath = std::get<0>(m_data)->loadImageDialog(
                widget, "Load " + m_name);
            lineEdit->setText(filePath);
        });

        // Put them into a horizontal layout
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(lineEdit);
        layout->addWidget(browseButton);
        widget->setLayout(layout);

        return widget;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) override
    {
        GLuint value = m_getter();

        QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
        lineEdit->setText(std::get<0>(m_data)->lookUpTextureName(value));
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) override
    {
        QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
        QString value = lineEdit->text();
        QFileInfo fileInfo(value);

        m_setter(std::get<0>(m_data)->uploadTexture(value));
        model->setData(index, fileInfo.fileName(), Qt::EditRole);
    }
};

/// Texture attribute data type.
using AttributeCellTexture = 
    AttributeCellWidget<GLuint, ImageLibrary*>;
Q_DECLARE_METATYPE(AttributeCellTexture*);
Q_DECLARE_METATYPE(AttributeCellTexture);

////////////////////////////////////////////////////////////////////////////////
/// String attribute functions.
template<>
struct AttributeCellWidget<std::string>: 
    public AttributeCellWidgetTypedBase<std::string>
{
    AttributeCellWidget():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellWidget(const QString& name, const QString& description, 
        std::string* attrib):
            AttributeCellWidgetTypedBase(name, description, attrib)
    {}

    AttributeCellWidget(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter):
            AttributeCellWidgetTypedBase(name, description, getter, setter)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, QVariant::fromValue(QString::fromStdString(m_getter())), Qt::EditRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) override
    {
        m_setter(qvariant_cast<QString>(model->data(index, Qt::EditRole)).toStdString());
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) override
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) override
    {
        QLineEdit* lineEdit = new QLineEdit(parent);
        
        return lineEdit;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) override
    {
        const std::string& value = m_getter();

        QLineEdit* lineEdit = (QLineEdit*) editor;
        lineEdit->setText(QString::fromStdString(value));
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) override
    {
        QLineEdit* lineEdit = (QLineEdit*) editor;
        QString value = lineEdit->text();

        m_setter(value.toStdString());
        model->setData(index, value, Qt::EditRole);
    }
};

/// String attribute data type.
using AttributeCellString = 
    AttributeCellWidget<std::string>;
Q_DECLARE_METATYPE(AttributeCellString);
Q_DECLARE_METATYPE(AttributeCellString*);

////////////////////////////////////////////////////////////////////////////////
/// String attribute functions.
template<>
struct AttributeCellWidget<QColor>: 
    public AttributeCellWidgetTypedBase<QColor>
{
    AttributeCellWidget():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellWidget(const QString& name, const QString& description,
         QColor* attrib):
            AttributeCellWidgetTypedBase(name, description, attrib)
    {}

    AttributeCellWidget(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter):
            AttributeCellWidgetTypedBase(name, description, getter, setter)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) override
    {
        model->setData(index, QVariant::fromValue(m_getter().name()), Qt::EditRole);
        model->setData(index, m_getter(), Qt::DecorationRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) override
    {
        m_setter(QColor(qvariant_cast<QString>(model->data(index, Qt::EditRole))));
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, 
        const QModelIndex& index) override
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) override
    {
        // Create an empty root widget
        QWidget* widget = new QWidget(parent);
        
        // Create a text widget that holds the name of the texture.
        QLineEdit* lineEdit = new QLineEdit(widget);
        lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        lineEdit->setReadOnly(true);

        // Create the browse button
        QPushButton* browseButton = new QPushButton("...", widget);
        browseButton->setFixedWidth(32);
        browseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
        QObject::connect(browseButton, &QPushButton::pressed, [=]()
        {
            QColor color = QColorDialog::getColor(m_getter(), widget, 
                "Select" + m_name, QColorDialog::ShowAlphaChannel);
            lineEdit->setText(color.name());
        });

        // Put them into a horizontal layout
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(lineEdit);
        layout->addWidget(browseButton);
        widget->setLayout(layout);

        return widget;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) override
    {
        QColor value = m_getter();

        QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
        lineEdit->setText(value.name());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) override
    {
        QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
        QString value = lineEdit->text();

        m_setter(QColor(value));
        model->setData(index, value, Qt::EditRole);
        model->setData(index, QColor(value), Qt::DecorationRole);
    }
};

/// Color attribute data type.
using AttributeCellColor = 
    AttributeCellWidget<QColor>;
Q_DECLARE_METATYPE(AttributeCellColor);
Q_DECLARE_METATYPE(AttributeCellColor*);