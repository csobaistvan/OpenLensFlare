#include "ImageLibrary.h"

////////////////////////////////////////////////////////////////////////////////
ImageLibrary::ImageLibrary(TextureAccessor* accessor, QObject* parent):
    QObject(parent),
    m_textureAccessor(accessor)
{}

////////////////////////////////////////////////////////////////////////////////
ImageLibrary::~ImageLibrary()
{}

////////////////////////////////////////////////////////////////////////////////
QString ImageLibrary::loadImageDialog(QWidget* parent, const QString& caption)
{
    // Generate a filter list
    QStringList mimeTypeFilters;
    for (const auto& mimeTypeName : QImageReader::supportedMimeTypes())
    {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();

    // Open the dialog
    QFileDialog dialog(parent, caption);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    //dialog.setDirectory(QDir::currentPath());
    dialog.setDirectory("D:/Program/Programming/Projects/Cpp/OpenLensFlare/OpenLensFlare/examples/systems");
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    
    // Make sure a file was selected.
    if (!dialog.exec())
        return "";

    // Extract the selected file.
    QString fileName = dialog.selectedFiles().first();

    // Make sure the dialog wasn't cancelled.
    if (fileName.isNull())
        return "";

    return loadImage(fileName) ? fileName : "";
}

////////////////////////////////////////////////////////////////////////////////
QString ImageLibrary::saveImageDialog(QWidget* parent, const QString& caption, 
    const QString& image)
{
    // Generate a filter list
    QStringList mimeTypeFilters;
    for (const auto& mimeTypeName : QImageWriter::supportedMimeTypes())
    {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();

    // Open the dialog
    QFileDialog dialog(parent, caption);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    //dialog.setDirectory(QDir::currentPath());
    dialog.setDirectory("D:/Program/Programming/Projects/Cpp/OpenLensFlare/OpenLensFlare/examples/systems");
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    
    // Make sure a file was selected.
    if (!dialog.exec())
        return "";

    // Extract the selected file.
    QString fileName = dialog.selectedFiles().first();

    // Attempt to save the file
    return !saveImage(image, fileName) ? "" : fileName;
}

////////////////////////////////////////////////////////////////////////////////
QString ImageLibrary::saveImageDialog(QWidget* parent, const QString& caption, 
    GLuint texture)
{
    // Generate a filter list
    QStringList mimeTypeFilters;
    for (const auto& mimeTypeName : QImageWriter::supportedMimeTypes())
    {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();

    // Open the dialog
    QFileDialog dialog(parent, caption);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    //dialog.setDirectory(QDir::currentPath());
    dialog.setDirectory("D:/Program/Programming/Projects/Cpp/OpenLensFlare/OpenLensFlare/examples/systems");
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    
    // Make sure a file was selected.
    if (!dialog.exec())
        return "";

    // Extract the selected file.
    QString fileName = dialog.selectedFiles().first();

    // Attempt to save the file
    return !saveImage(texture, fileName) ? "" : fileName;
}

////////////////////////////////////////////////////////////////////////////////
bool ImageLibrary::loadImage(GLuint texture, bool forceReload)
{
    // Make sure it is not a null image
    if (texture == 0)
        return false;

    // Make sure it isn't loaded already.
    if (m_textures.contains(texture) && !forceReload)
        return true;

    // Load the image
    QImage image = m_textureAccessor->loadTexture(texture);
    if (image.isNull())
        return false;

    // Store the image
    m_textures[texture] = image;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ImageLibrary::loadImage(const QString& path)
{
    // Make sure the filename isn't empty
    if (path.isEmpty())
        return false;

    // Make sure it isn't loaded already.
    if (m_images.contains(path))
        return true;

    // Read the image
    QImageReader imReader(path);
    QImage image = imReader.read();

    if (image.isNull())
        return false;

    // Convert it to the expected format
    image = image.convertToFormat(QImage::Format_RGBA8888);

    /// Store the image.
    m_images[path] = image;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ImageLibrary::saveImage(const QString& image, const QString& path)
{
    // Make sure the filename isn't empty
    if (image.isEmpty() || path.isEmpty())
        return false;
        
    // Make sure it is actually loaded
    if (!m_images.contains(image))
        return false;

    // Save the image
    QImageWriter imWriter(path);
    return imWriter.write(m_images[image]);
}

////////////////////////////////////////////////////////////////////////////////
bool ImageLibrary::saveImage(GLuint texture, const QString& path)
{
    qDebug() << "save - check";
    // Make sure the filename isn't empty
    if (texture == 0 || path.isEmpty())
        return false;
        
    qDebug() << "save - check 2";
    // Look for it in the texture map
    if (m_textures.contains(texture))
    {
        qDebug() << "save - write";
        QImageWriter imWriter(path);
        return imWriter.write(m_textures[texture]);
    }

    qDebug() << "save - check 3";
    // Look for an alias
    if (m_textureNames.contains(texture))
    {
        qDebug() << "save - write";
        QImageWriter imWriter(path);
        return imWriter.write(m_images[m_textureNames[texture]]);
    }

    // No success
    return false;
}

////////////////////////////////////////////////////////////////////////////////
const QImage& ImageLibrary::getImage(const QString& imagePath) const
{
    auto it = m_images.constFind(imagePath);
    return (it != m_images.end()) ? it.value() : m_empty;
}

////////////////////////////////////////////////////////////////////////////////
const QImage& ImageLibrary::getImage(GLuint texture) const
{
    auto it = m_textures.constFind(texture);
    return (it != m_textures.end()) ? it.value() : m_empty;
}

////////////////////////////////////////////////////////////////////////////////
QString ImageLibrary::lookUpTextureName(GLuint texture) const
{
    auto it = m_textureNames.constFind(texture);
    return (it != m_textureNames.end()) ? it.value() : "";
}

////////////////////////////////////////////////////////////////////////////////
GLuint ImageLibrary::uploadTexture(const QString& imagePath)
{
    // Make sure the file path is not empty.
    if (imagePath.isEmpty())
        return 0;

    // Lookup the image.
    if (!m_images.contains(imagePath))
        return 0;

    // Ask the accessor to upload the texture for us.
    auto id = m_textureAccessor->uploadTexture(m_images[imagePath]);
    if (id == 0)
        return 0;

    // Store it.
    m_textureNames[id] = imagePath;

    // Return the updated it.
    return id;
}

////////////////////////////////////////////////////////////////////////////////
void ImageLibrary::releaseTextures()
{
    // Release the uploaded textures
    for (auto it = m_textureNames.begin(); it != m_textureNames.end(); ++it)
        m_textureAccessor->releaseTexture(it.key());
}