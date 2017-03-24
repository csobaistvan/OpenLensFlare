#pragma once

#include "Dependencies.h"

/// The optical system previewer widget.
class OpticalSystemPreviewer: public QOpenGLWidget
{
    Q_OBJECT;

public:
    explicit OpticalSystemPreviewer(OLEF::OpticalSystem* system, QWidget* parent = nullptr);
    ~OpticalSystemPreviewer();

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
    /// Represents a set of line strips, with all the data needed to render them.
    struct LineSet
    {
        /// Color of the lines.
        QColor m_color;

        /// The vertex buffer that holds the vertices.
        GLuint m_vbo;

        /// The vertex buffer that holds the vertices.
        GLuint m_vao;

        /// Line strip vertex positions.
        std::vector<glm::vec2> m_vertices;
        
        /// Line strip start indices and vertex counts.
        std::vector<std::array<int, 2>> m_indices;
    };

    /// Parameters for a ray to visualize.
    struct Ray
    {
        /// Index of the ghost to render.
        int m_ghostIndex;

        /// Number of rays.
        int m_incomingRayCount;

        /// Offset of the first ray, from the top.
        float m_incomingRayOffset;

        /// The spacing between incoming rays.
        float m_incomingRaySpacing;

        /// Angle of the incoming ray.
        float m_incomingRayAngle;
    };

    /// Generates geometry for rendering.
    void computeSystemBounds();
    void generateLensGeometry();
    void generateRayGeometry(const Ray& ray);
    void generateRayGeometry();
    void generateGeometry();

    /// Renders a line set.
    void renderLineSet(const LineSet& lineSet);

    /// The optical system to edit.
    OLEF::OpticalSystem* m_opticalSystem;

    /// All the possible ghosts in the system.
    OLEF::GhostList m_ghosts;

    /// Background color.
    QColor m_backgroundColor;

    /// Whether we should generate new geometry.
    bool m_generateGeometry;

    /// Rays to visualize.
    std::vector<Ray> m_raysToDraw;

    /// Sets of input rays.
    LineSet m_rays;

    /// Set of lenses
    LineSet m_lenses;

    /// Aperture lines
    LineSet m_apertures;

    /// Film lines (including the optical axis)
    LineSet m_films;

    /// The zooming factor.
    float m_zoomFactor;

    /// Transform matrix.
    glm::mat4 m_transform;

    /// Projection matrix.
    glm::mat4 m_proj;

    /// The rendering shader.
    GLuint m_renderShader;
};