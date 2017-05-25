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

    void generateStarburst(int textureSize, float minWl, float maxWl, float wlStep);

    /// Parameters for a light source to visualize.
    struct LightSource
    {
        /// Screen position of the light source.
        glm::vec2 m_position = glm::vec2(0.0f, 0.0f);

        /// Light source incidence direction.
        glm::vec3 m_direction = glm::vec3(0.0f, 0.0f, -1.0f);

        /// Color of the light source.
        QColor m_color = QColor(Qt::white);

        /// Diffuse intensity of the light source.
        float m_intensity = 1.0f;

        /// Size of the corresponding starburst.
        float m_starburstSize = 0.1f;

        /// Intensity of the corresponding starburst.
        float m_starburstIntensity = 1.0f;
    };

    // Getters and setters for the visualization attributes
    ImageLibrary* getImageLibrary() const { return m_imageLibrary; }
    OLEF::StarburstAlgorithm* getStarburstAlgorithm() const { return m_starburstAlgorithm; }
    OLEF::GhostAlgorithm* getGhostAlgorithm() const { return m_ghostAlgorithm; }
    const QVector<LightSource>& getLightSources() const { return m_lightSources; }
    
    void setLightSources(const QVector<LightSource>& value) { m_lightSources = value; }

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

public slots:
    /// Slot, used to indicate that the underlying optical system changed.
    void opticalSystemChanged();

private:
    /// The optical system to use with the algorithms.
    OLEF::OpticalSystem* m_opticalSystem;

    /// The image library that manages our textures.
    ImageLibrary* m_imageLibrary;

    /// The algorithm to render the starburst.
    OLEF::DiffractionStarburstAlgorithm* m_starburstAlgorithm;

    /// The algorithm to render ghosts.
    OLEF::GhostAlgorithm* m_ghostAlgorithm;

    /// Color of the background.
    QColor m_backgroundColor;

    /// Ghosts to render.
    OLEF::GhostList m_ghosts;

    /// The list of light source objects used for previewing.
    QVector<LightSource> m_lightSources;
};