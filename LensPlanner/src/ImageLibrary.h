#pragma once

#include "Dependencies.h"

/// An abstract texture accessor object, used to communicate textures with the 
/// graphical API. Note that it is not context aware, meaning that it only
/// manages textures for a single context, and must be manually instantiated
/// for each context.
class TextureAccessor
{
public:
    virtual ~TextureAccessor() {}

    /// Uploads an image as a texture, and returns its id.
    virtual GLuint uploadTexture(const QImage& image) = 0;

    /// Loads the texture with the given id.
    virtual QImage loadTexture(GLuint texture) = 0;

    /// Releases a previously allocated texture.
    virtual void releaseTexture(GLuint texture) = 0;
};

/// Represents a library, which is responsible for mapping texture names to
/// images. It also maps GLuint textures to texture names, allowing to access
/// the OpenLensFlare images through their id's.
///
/// TODO: some of the function names are really cryptic (e.g. the various
///       overloaded 'loadImage' functions), rename them to better names
/// TODO: also, some of the function pairs do not seem to share the same attributes
///       ()
class ImageLibrary: public QObject
{
    Q_OBJECT

public:
    ImageLibrary(TextureAccessor* accessor, QObject* parent = nullptr);
    ~ImageLibrary();

    /// Creates an image loading dialog, and returns the loaded image's path.
    QString loadImageDialog(QWidget* parent, const QString& caption);

    /// Creates an image saving dialog, for saving the parameter image.
    QString saveImageDialog(QWidget* parent, const QString& caption, const QString& image);

    /// Creates an image saving dialog, for saving the parameter image.
    QString saveImageDialog(QWidget* parent, const QString& caption, GLuint texture);

    /// Loads the parameter image.
    bool loadImage(const QString& path);

    /// Loads the parameter texture.
    bool loadImage(GLuint image, bool forceReload = false);

    /// Saves the parameter image.
    bool saveImage(const QString& image, const QString& path);

    /// Saves the parameter image.
    bool saveImage(GLuint image, const QString& path);

    /// Returns the image coressponding to the parameter path.
    const QImage& getImage(const QString& imagePath) const;

    /// Returns the image coressponding to the parameter texture.
    const QImage& getImage(GLuint texture) const;

    /// Uploads the provided texture and returns its new id.
    GLuint uploadTexture(const QString& imagePath);

    /// Gets the texture path of the selected texture.
    QString lookUpTextureName(GLuint texture) const;

    /// Releases all the uploaded textures.
    void releaseTextures();

private:
    /// The texture accessor object, used to load and upload textures.
    TextureAccessor* m_textureAccessor;

    /// Empty image, return if no image is found.
    QImage m_empty;

    /// The list of images.
    QMap<QString, QImage> m_images;

    /// List of texture to image mappings.
    QMap<GLuint, QString> m_textureNames;

    /// List of texture to image mappings.
    QMap<GLuint, QImage> m_textures;
};