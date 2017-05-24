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

    // Getters and setters for the visualization attributes
    ImageLibrary* getImageLibrary() const { return m_imageLibrary; }
    OLEF::StarburstAlgorithm* getStarburstAlgorithm() const { return m_starburstAlgorithm; }
    OLEF::GhostAlgorithm* getGhostAlgorithm() const { return m_ghostAlgorithm; }
    const QVector<OLEF::LightSource>& getLightSources() const { return m_lightSources; }
    
    void setLightSources(const QVector<OLEF::LightSource>& value) { m_lightSources = value; }

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
    void wheelEvent(QWheelEvent* event);

public slots:
    /// Slot, used to indicate that the underlying optical system changed.
    void opticalSystemChanged();

private:
    /// The optical system to use with the algorithms.
    OLEF::OpticalSystem* m_opticalSystem;

    /// The image library that manages our textures.
    ImageLibrary* m_imageLibrary;

    /// The algorithm to render the starburst.
    OLEF::StarburstAlgorithm* m_starburstAlgorithm;

    /// The algorithm to render ghosts.
    OLEF::GhostAlgorithm* m_ghostAlgorithm;

    // Previous mouse coordinates.
    std::array<int, 2> m_prevMouse;

    /// Light source polar coordinates
    std::array<float, 2> m_lightPolar;

    /// Color of the background.
    QColor m_backgroundColor;

    /// Ghosts to render.
    OLEF::GhostList m_ghosts;

    /// The list of light source objects used for previewing.
    QVector<OLEF::LightSource> m_lightSources;
};