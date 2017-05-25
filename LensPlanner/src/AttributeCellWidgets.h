#pragma once

#include "Dependencies.h"
#include "AttributeCellDelegate.h"
#include "ImageLibrary.h"

/// Data related to an editor cell.
template<typename AttribType, typename... DataTypes>
class AttributeCellWidgetTypedBase: public AttributeCellWidgetBase
{
public:
    /// Getter and setter function types.
    using Getter = std::function<AttribType()>;
    using Setter = std::function<void (AttribType)>;

    /// Empty default conclassor.
    AttributeCellWidgetTypedBase()
    {}

    /// Conclasss a cell data object with the specified name, a pointer to the
    /// attribute and the specified data.
    AttributeCellWidgetTypedBase(const QString& name, const QString& description,
        AttribType* attrib, DataTypes... data):
            m_name(name),
            m_getter{[=]() { return *attrib; }},
            m_setter{[=](const auto& value) { *attrib = value; }},
            m_data(std::forward<DataTypes>(data)...)
    {}

    /// Conclasss a cell data object with the specified name, a getter/setter
    /// pair and the specified data.
    AttributeCellWidgetTypedBase(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, DataTypes... data):
            m_name(name),
            m_getter(getter),
            m_setter(setter),
            m_data(std::forward<DataTypes>(data)...)
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
/// Enumeration attribute cell type.
template<typename T>
class AttributeCellEnum: public AttributeCellWidgetTypedBase<T, QStringList>
{
public:
    AttributeCellEnum():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellEnum(const QString& name, const QString& description, 
        T* attrib, const QStringList& names):
            AttributeCellWidgetTypedBase(name, description, attrib, names)
    {}

    AttributeCellEnum(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, const QStringList& names):
            AttributeCellWidgetTypedBase(name, description, getter, setter, names)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        const QStringList& names = std::get<0>(m_data);
        QString text = names[(int) m_getter()];
        model->setData(index, QVariant::fromValue(text), Qt::DisplayRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        const QStringList& names = std::get<0>(m_data);
        QString text = qvariant_cast<QString>(model->data(index, Qt::DisplayRole));
        auto it = std::find(names.constBegin(), names.constEnd(), text);
        int id = (it == names.constEnd()) ? 0 : it - names.constBegin();
        m_setter((T) id);
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
    {
        const QStringList& names = std::get<0>(m_data);
        QComboBox* comboBox = new QComboBox(parent);
        comboBox->addItems(names);

        return comboBox;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        auto elemType = m_getter();

        QComboBox* comboBox = (QComboBox*) editor;
        comboBox->setCurrentIndex((int) elemType);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        const QStringList& names = std::get<0>(m_data);
        QComboBox* comboBox = (QComboBox*) editor;
        int current = comboBox->currentIndex();
        auto elemType = (T) current;

        m_setter(elemType);
        model->setData(index, names[current], Qt::DisplayRole);
    }
};

/// Optical system element type attribute type.
using AttributeCellElementType = 
    AttributeCellEnum<OLEF::OpticalSystemElement::ElementType>;

////////////////////////////////////////////////////////////////////////////////
/// Bool attribute cell type.
class AttributeCellBool: public AttributeCellWidgetTypedBase<bool>
{
public:
    AttributeCellBool():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellBool(const QString& name, const QString& description,
         bool* attrib):
            AttributeCellWidgetTypedBase(name, description, attrib)
    {}

    AttributeCellBool(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter):
            AttributeCellWidgetTypedBase(name, description, getter, setter)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, QVariant::fromValue(m_getter()), Qt::DisplayRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        m_setter(qvariant_cast<int>(model->data(index, Qt::DisplayRole)));
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
    {
        QCheckBox* checkBox = new QCheckBox(parent);
        
        return checkBox;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        bool value = m_getter();

        QCheckBox* checkBox = (QCheckBox*) editor;
        checkBox->setChecked(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        QCheckBox* checkBox = (QCheckBox*) editor;
        bool value = checkBox->isChecked();

        m_setter(value);
        model->setData(index, value, Qt::DisplayRole);
    }
};

////////////////////////////////////////////////////////////////////////////////
/// Integer attribute cell type.
class AttributeCellInt: public AttributeCellWidgetTypedBase<int, int, int, int>
{
public:
    AttributeCellInt():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellInt(const QString& name, const QString& description,
         int* attrib, int min, int max, int step):
            AttributeCellWidgetTypedBase(name, description, attrib, min, max, step)
    {}

    AttributeCellInt(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, int min, int max, int step):
            AttributeCellWidgetTypedBase(name, description, getter, setter, min, max, step)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, QVariant::fromValue(m_getter()), Qt::DisplayRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        m_setter(qvariant_cast<int>(model->data(index, Qt::DisplayRole)));
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
    {
        QSpinBox* spinBox = new QSpinBox(parent);
        spinBox->setMinimum(std::get<0>(m_data));
        spinBox->setMaximum(std::get<1>(m_data));
        spinBox->setSingleStep(std::get<2>(m_data));
        
        return spinBox;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        int value = m_getter();

        QSpinBox* spinBox = (QSpinBox*) editor;
        spinBox->setValue(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        QSpinBox* spinBox = (QSpinBox*) editor;
        int value = spinBox->value();

        m_setter(value);
        model->setData(index, value, Qt::DisplayRole);
    }
};

////////////////////////////////////////////////////////////////////////////////
/// Floating point attribute cell type.
class AttributeCellFloat: public AttributeCellWidgetTypedBase<float, float, float, float>
{
public:
    AttributeCellFloat():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellFloat(const QString& name, const QString& description,
         float* attrib, float min, float max, float step):
            AttributeCellWidgetTypedBase(name, description, attrib, min, max, step)
    {}

    AttributeCellFloat(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, float min, float max, float step):
            AttributeCellWidgetTypedBase(name, description, getter, setter, min, max, step)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, QVariant::fromValue(m_getter()), Qt::DisplayRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        m_setter(qvariant_cast<float>(model->data(index, Qt::DisplayRole)));
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
    {
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
        spinBox->setMinimum(std::get<0>(m_data));
        spinBox->setMaximum(std::get<1>(m_data));
        spinBox->setSingleStep(std::get<2>(m_data));
        
        return spinBox;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        float value = m_getter();

        QDoubleSpinBox* spinBox = (QDoubleSpinBox*) editor;
        spinBox->setValue(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        QDoubleSpinBox* spinBox = (QDoubleSpinBox*) editor;
        float value = spinBox->value();

        m_setter(value);
        model->setData(index, value, Qt::DisplayRole);
    }
};

////////////////////////////////////////////////////////////////////////////////
/// Texture attribute cell type.
class AttributeCellTexture: public AttributeCellWidgetTypedBase<GLuint, ImageLibrary*>
{
public:
    AttributeCellTexture():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellTexture(const QString& name, const QString& description,
         GLuint* attrib, ImageLibrary* imageLibrary):
            AttributeCellWidgetTypedBase(name, description, attrib, imageLibrary)
    {}

    AttributeCellTexture(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, ImageLibrary* imageLibrary):
            AttributeCellWidgetTypedBase(name, description, getter, setter, imageLibrary)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        GLuint value = m_getter();
        QFileInfo fileInfo(std::get<0>(m_data)->lookUpTextureName(value));
        model->setData(index, fileInfo.fileName(), Qt::DisplayRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        QString filePath = qvariant_cast<QString>(model->data(index, Qt::DisplayRole));
        m_setter(std::get<0>(m_data)->uploadTexture(filePath));
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
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

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        GLuint value = m_getter();

        QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
        lineEdit->setText(std::get<0>(m_data)->lookUpTextureName(value));
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
        QString value = lineEdit->text();
        QFileInfo fileInfo(value);

        m_setter(std::get<0>(m_data)->uploadTexture(value));
        model->setData(index, fileInfo.fileName(), Qt::DisplayRole);
    }
};

////////////////////////////////////////////////////////////////////////////////
/// String attribute cell type.
// TODO: this should use a QString, rather than an std::string (works like this
// because the optical system name is embedded into the optical system itself)
class AttributeCellString: public AttributeCellWidgetTypedBase<std::string>
{
public:
    AttributeCellString():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellString(const QString& name, const QString& description, 
        std::string* attrib):
            AttributeCellWidgetTypedBase(name, description, attrib)
    {}

    AttributeCellString(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter):
            AttributeCellWidgetTypedBase(name, description, getter, setter)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, QVariant::fromValue(QString::fromStdString(m_getter())), Qt::DisplayRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        m_setter(qvariant_cast<QString>(model->data(index, Qt::DisplayRole)).toStdString());
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
    {
        QLineEdit* lineEdit = new QLineEdit(parent);
        
        return lineEdit;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        const std::string& value = m_getter();

        QLineEdit* lineEdit = (QLineEdit*) editor;
        lineEdit->setText(QString::fromStdString(value));
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        QLineEdit* lineEdit = (QLineEdit*) editor;
        QString value = lineEdit->text();

        m_setter(value.toStdString());
        model->setData(index, value, Qt::DisplayRole);
    }
};

////////////////////////////////////////////////////////////////////////////////
/// Color attribute cell type.
class AttributeCellColor: public AttributeCellWidgetTypedBase<QColor>
{
public:
    AttributeCellColor():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellColor(const QString& name, const QString& description,
         QColor* attrib):
            AttributeCellWidgetTypedBase(name, description, attrib)
    {}

    AttributeCellColor(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter):
            AttributeCellWidgetTypedBase(name, description, getter, setter)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, QVariant::fromValue(m_getter().name()), Qt::DisplayRole);
        model->setData(index, m_getter(), Qt::DecorationRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        m_setter(QColor(qvariant_cast<QString>(model->data(index, Qt::DisplayRole))));
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
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

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        QColor value = m_getter();

        QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
        lineEdit->setText(value.name());
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        QLineEdit* lineEdit = (QLineEdit*) (editor->layout()->itemAt(0)->widget());
        QString value = lineEdit->text();

        m_setter(QColor(value));
        model->setData(index, value, Qt::DisplayRole);
        model->setData(index, QColor(value), Qt::DecorationRole);
    }
};

////////////////////////////////////////////////////////////////////////////////
/// Vector attribute functions.
template<int NumComponents> struct AttributeCellVectorTypes {};
template<> struct AttributeCellVectorTypes<2> { using type = glm::vec2; };
template<> struct AttributeCellVectorTypes<3> { using type = glm::vec3; };
template<> struct AttributeCellVectorTypes<4> { using type = glm::vec4; };

template<int NumComponents>
class AttributeCellVector: public AttributeCellWidgetTypedBase<
    typename AttributeCellVectorTypes<NumComponents>::type, 
    typename AttributeCellVectorTypes<NumComponents>::type,
    typename AttributeCellVectorTypes<NumComponents>::type,
    typename AttributeCellVectorTypes<NumComponents>::type>
{
public:
    // The actual vector type
    using VectorType = typename AttributeCellVectorTypes<NumComponents>::type;

    // Names of the labels
    static QStringList labelNames()
    {
        static QStringList s_labelNames = 
        {
            "X", "Y", "Z", "W"
        };
        return s_labelNames;
    }

    // Converts a vector to a string.
    static QString vecToString(VectorType vector)
    {
        QStringList values;
        for (int i = 0; i < NumComponents; ++i)
        {
            values.append(QString::number(vector[i]));
        }

        // Return the result
        return QString("(%1)").arg(values.join(", "));
    }

    /// Converts a string to a vector.
    static VectorType stringToVec(const QString& string)
    {
        // Split the string into parts
        auto parts = string.split(", ", QString::SkipEmptyParts);
        if (parts.size() != NumComponents)
        {
            return VectorType(0.0f);
        }

        VectorType result;

        for (int i = 0; i < NumComponents; ++i)
        {
            qDebug() << i << parts[i];
        }

        return result;
    }

    AttributeCellVector():
        AttributeCellWidgetTypedBase()
    {}

    AttributeCellVector(const QString& name, const QString& description,
         VectorType* attrib, VectorType min, VectorType max, VectorType step):
            AttributeCellWidgetTypedBase(name, description, attrib, min, max, step)
    {}

    AttributeCellVector(const QString& name, const QString& description, 
        const Getter& getter, const Setter& setter, 
        VectorType min, VectorType max, VectorType step):
            AttributeCellWidgetTypedBase(name, description, getter, setter, min, max, step)
    {}

    // AttributeCellWidgetBase methods
    void refreshName(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, m_name, Qt::DisplayRole);
    }

    void refreshView(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, QVariant::fromValue(vecToString(m_getter())), Qt::DisplayRole);
    }

    void refreshAttribute(QAbstractItemModel* model, const QModelIndex& index) const override
    {
        m_setter(stringToVec(qvariant_cast<QString>(model->data(index, Qt::DisplayRole))));
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
    {
        // Min and max values
        VectorType min = std::get<0>(m_data);
        VectorType max = std::get<1>(m_data);
        VectorType step = std::get<2>(m_data);

        // Create an empty root widget
        QWidget* widget = new QWidget(parent);

        // Create the horizontal layout
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        
        // Create the spinboxes
        for (int i = 0; i < NumComponents; ++i)
        {
            // Create a text widget that holds the name of the component.
            QLabel* label = new QLabel(widget);
            label->setText(labelNames()[i]);
            label->setFixedWidth(14);
            label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
            label->setAlignment(Qt::AlignHCenter);

            // Create the spinbox button
            QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
            spinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
            spinBox->setMinimum(min[i]);
            spinBox->setMaximum(max[i]);
            spinBox->setSingleStep(step[i]);

            // Put them into the layout
            layout->addWidget(label);
            layout->addWidget(spinBox);
        }

        // Add the layout to the widget
        widget->setLayout(layout);

        return widget;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        VectorType value = m_getter();

        for (int i = 0; i < NumComponents; ++i)
        {
            QDoubleSpinBox* spinBox = (QDoubleSpinBox*) (editor->layout()->itemAt(i * 2 + 1)->widget());
            spinBox->setValue(value[i]);
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override
    {
        VectorType value;

        for (int i = 0; i < NumComponents; ++i)
        {
            QDoubleSpinBox* spinBox = (QDoubleSpinBox*) (editor->layout()->itemAt(i * 2 + 1)->widget());
            value[i] = spinBox->value();
        }
        m_setter(value);
        model->setData(index, vecToString(value), Qt::DisplayRole);
    }
};

/// Vector attribute data types.
using AttributeCellVec2 = AttributeCellVector<2>;
using AttributeCellVec3 = AttributeCellVector<3>;
using AttributeCellVec4 = AttributeCellVector<4>;