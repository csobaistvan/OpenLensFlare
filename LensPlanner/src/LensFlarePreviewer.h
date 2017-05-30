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

    // Getters for the underlying objects.
    ImageLibrary* getImageLibrary() const 
    { return m_imageLibrary; }
    
    OLEF::DiffractionStarburstAlgorithm* getDiffractionStarburstAlgorithm() const 
    { return m_diffractionStarburstAlgorithm; }
    
    OLEF::RayTraceGhostAlgorithm* getRayTraceGhostAlgorithm() const 
    { return m_rayTraceGhostAlgorithm; }

    /// Parameters for a lens flare layer to render.
    struct Layer
    {
        /// Screen position of the light source.
        glm::vec2 m_lightPosition = glm::vec2(0.0f, 0.0f);

        /// Light source incidence direction.
        glm::vec3 m_lightDirection = glm::vec3(0.0f, 0.0f, 1.0f);

        /// Color of the light source.
        QColor m_lightColor = QColor(Qt::white);

        /// Diffuse intensity of the light source.
        float m_lightIntensity = 1.0f;

        /// Size of the corresponding starburst.
        float m_starburstSize = 0.1f;

        /// Intensity of the corresponding starburst.
        float m_starburstIntensity = 1.0f;

        /// Index of the first ghost to render.
        int m_firstGhost = 1;

        /// The number of ghosts to render.
        int m_numGhosts = 0;

        /// Intensity scaling value used to render the ghost.
        float m_ghostIntensityScale = 1.0f;

        /// Iris distance (mask texture) clip value.
        float m_ghostDistanceClip = 0.95f;

        /// Radius clipping value.
        float m_ghostRadiusClip = 1.0f;

        /// Intensity clipping value.
        float m_ghostIntensityClip = 0.05f;

        /// The render mode used to render these ghosts.
        OLEF::RayTraceGhostAlgorithm::RenderMode m_ghostRenderMode;

        /// The shading mode used to render these ghosts.
        OLEF::RayTraceGhostAlgorithm::ShadingMode m_ghostShadingMode;

        /// Whether precomputed ghost attributes should be used (when available) or not.
        bool m_useGhostAttributes = true;

        /// Whether we should draw wireframes or not.
        bool m_wireframe = false;
    };

    // Getters and setters for the visualization attributes
    int getStarburstTextureSize() const { return m_starburstTextureSize; }
    float getStarburstMinWavelength() const { return m_starburstMinWavelength; }
    float getStarburstMaxWavelength() const { return m_starburstMaxWavelength; }
    float getStarburstWavelengthStep() const { return m_starburstWavelengthStep; }
    const QVector<Layer>& getLayers() const { return m_layers; }
    const QMap<float, OLEF::GhostList>& getPrecomputedGhosts() const { return m_precomputedGhosts; };
    
    void setStarburstTextureSize(int value) { m_starburstTextureSize = value; }
    void setStarburstMinWavelength(float value) { m_starburstMinWavelength = value; }
    void setStarburstMaxWavelength(float value) { m_starburstMaxWavelength = value; }
    void setStarburstWavelengthStep(float value) { m_starburstWavelengthStep = value; }
    void setLayers(const QVector<Layer>& value) { m_layers = value; }
    void setPrecomputedGhosts(const QMap<float, OLEF::GhostList>& value) {m_precomputedGhosts = value; };
    void requestPrecomputation() { m_precompute = true; }
    void requestStarburstGeneration() { m_generateStarburst = true; }

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
    /// Generates the diffraction starbust texture.
    void generateStarburst();

    /// Computes parameters for the rendered ghosts.
    void computeGhostParameters();
    
    /// The optical system to use with the algorithms.
    OLEF::OpticalSystem* m_opticalSystem;

    /// The image library that manages our textures.
    ImageLibrary* m_imageLibrary;

    /// Whether we need to re-generate the starburst texture or not.
    bool m_generateStarburst;

    /// Whether we need to do pre-computation or not.
    bool m_precompute;

    /// Color of the background.
    QColor m_backgroundColor;

    /// Size of the diffraction starburst texture.
    int m_starburstTextureSize;

    /// Starting wavelength of the diffraction starburst texture.
    float m_starburstMinWavelength;
    
    /// Ending wavelength of the diffraction starburst texture.
    float m_starburstMaxWavelength;

    /// Wavelength step of the diffraction starburst texture.
    float m_starburstWavelengthStep;

    /// The diffraction starburst rendering algorithm.
    OLEF::DiffractionStarburstAlgorithm* m_diffractionStarburstAlgorithm;

    /// The ray traced ghost rendering algorithm.
    OLEF::RayTraceGhostAlgorithm* m_rayTraceGhostAlgorithm;

    /// Precomputed ghosts with their attributes.
    QMap<float, OLEF::GhostList> m_precomputedGhosts;

    /// The list of light source objects used for previewing.
    QVector<Layer> m_layers;
};