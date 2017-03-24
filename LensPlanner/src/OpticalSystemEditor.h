#pragma once

#include "Dependencies.h"
#include "ImageLibrary.h"

/// Data related to an editor cell.
struct OpticalSystemEditorCellData
{
    /// TODO: use std::variant to implement this.

    /// Data type of the cell
    enum CellType
    {
        TYPE,
        FLOAT,
        TEXTURE,
        STRING
    };

    /// Getter and setter function types for element types
    using GetterType = std::function<OLEF::OpticalSystemElement::ElementType()>;
    using SetterType = std::function<void (OLEF::OpticalSystemElement::ElementType)>;

    /// Getter and setter function types for floats
    using GetterFloat = std::function<float()>;
    using SetterFloat = std::function<void(float)>;

    /// Getter and setter function types for textures
    using GetterTexture = std::function<GLuint()>;
    using SetterTexture = std::function<void (GLuint)>;

    /// Getter and setter function types for strings
    using GetterString = std::function<std::string()>;
    using SetterString = std::function<void (std::string)>;

    /// Empty default constructor.
    OpticalSystemEditorCellData()
    {}

    /// Type data constructor
    OpticalSystemEditorCellData(const QString& name, const GetterType& getter, 
        const SetterType& setter):
            m_name(name),
            m_type(TYPE),
            m_typeGetter(getter),
            m_typeSetter(setter)
    {}

    /// Float data constructor
    OpticalSystemEditorCellData(const QString& name, const GetterFloat& getter, 
        const SetterFloat& setter, float min, float max):
            m_name(name),
            m_type(FLOAT),
            m_min(min),
            m_max(max),
            m_floatGetter(getter),
            m_floatSetter(setter)
    {}

    /// Texture data constructor
    OpticalSystemEditorCellData(const QString& name, const GetterTexture& getter, 
        const SetterTexture& setter):
            m_name(name),
            m_type(TEXTURE),
            m_textureGetter(getter),
            m_textureSetter(setter)
    {}

    /// String data constructor
    OpticalSystemEditorCellData(const QString& name, const GetterString& getter, 
        const SetterString& setter):
            m_name(name),
            m_type(STRING),
            m_stringGetter(getter),
            m_stringSetter(setter)
    {}

    /// Name of the cell.
    QString m_name;

    /// Type of the cell.
    CellType m_type;

    /// Minimum value for the cell.
    float m_min;

    /// Maximum value for the cell.
    float m_max;

    /// Its getter.
    GetterType m_typeGetter;
    GetterFloat m_floatGetter;
    GetterTexture m_textureGetter;
    GetterString m_stringGetter;

    /// Its setter.
    SetterType m_typeSetter;
    SetterFloat m_floatSetter;
    SetterTexture m_textureSetter;
    SetterString m_stringSetter;

};
Q_DECLARE_METATYPE(OpticalSystemEditorCellData);

/// Editor for an optical system editor row.
class OpticalSystemEditorDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    OpticalSystemEditorDelegate(ImageLibrary* library, QWidget* parent = 0);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;

private:
    /// The image library that holds our images.
    ImageLibrary* m_imageLibrary;
};

/// The optical system editor widget.
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
    /// Creates the top toolbar
    void createToolBar();

    /// Updates the values in a tree cell.
    void updateTreeAttribute(QStandardItem* item, OpticalSystemEditorCellData attrib);

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