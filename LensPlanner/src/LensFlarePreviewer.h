#pragma once

#include "Dependencies.h"
#include "ImageLibrary.h"

/// The lens flare visualizer, previewer object.
class LensFlarePreviewer: public QOpenGLWidget, public TextureAccessor
{
    Q_OBJECT;

public:
    explicit LensFlarePreviewer(OLEF::OpticalSystem* system, QWidget* parent = nullptr);
    ~LensFlarePreviewer();

    /// Returns the object's image library.
    ImageLibrary* getImageLibrary() const;

    /// The algorithm to render the starburst.
    OLEF::StarburstAlgorithm* getStarburstAlgorithm() const;

    /// The algorithm to render ghosts.
    OLEF::GhostAlgorithm* getGhostAlgorithm() const;

    /// TextureAccessor interface
    GLuint uploadTexture(const QImage& image);
    QImage loadTexture(GLuint texture);
    void releaseTexture(GLuint texture);

    /// Update hook.
    void update();

    /// QGLWidget callbacks.
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    /// QWidget events.
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);

public slots:
    /// Slot, used to indicate that the underlying optical system changed.
    void opticalSystemChanged();

private:
    /// The image library that manages our textures.
    ImageLibrary* m_imageLibrary;

    /// Light polar coordinates
    std::array<float, 2> m_lightPolar;

    // Previous mouse coordinates.
    std::array<int, 2> m_prevMouse;

    /// The optical system to use with the algorithms.
    OLEF::OpticalSystem* m_opticalSystem;

    /// The algorithm to render the starburst.
    OLEF::StarburstAlgorithm* m_starburst;

    /// The algorithm to render ghosts.
    OLEF::GhostAlgorithm* m_ghost;

    /// The light source object used for previewing.
    OLEF::LightSource m_lightSource;

    /// Ghosts to render.
    OLEF::GhostList m_ghosts;

    /// Color of the background.
    QColor m_backgroundColor;
};