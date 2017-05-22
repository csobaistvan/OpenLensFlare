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

    /// QWidget events.
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);

public slots:
    /// Slot, used to indicate that the underlying optical system changed.
    void opticalSystemChanged();

private:
    /// Parameters for a ray to visualize.
    struct Ray
    {
        /// Index of the ghost to render.
        OLEF::Ghost m_ghost;

        /// Color of the corresponding lines.
        QColor m_color;

        /// Number of rays.
        int m_rayCount;

        /// Distance from the first element from where the rays should originate.
        float m_startDistance;

        /// Offset of the first ray, from the top.
        float m_startHeight;

        /// Offset of the last ray, from the top.
        float m_endHeight;

        /// Angle of the incoming ray.
        float m_angle;
    };

    /// Holds the index of the starting vertex of each line strip, and the 
    /// number of vertices that correspond to said strips.
    struct LineStripData
    {
        /// Color of the line strip. Stored here (instead of with the batch)
        /// to allow per-strip color, should it become needed.
        QColor m_color;

        /// Index of the starting vertex.
        int m_start;

        /// Number of vertices.
        int m_length;
    };

    /// Generates geometry for rendering.
    void storeLineStrip(QColor color, const std::vector<glm::vec2>& vertices);
    void traceRay(const Ray& ray, int id);
    void generateRayGeometry();
    void generateGeometry();
    void generateLensGeometry();
    void generateGridGeometry();
    void computeProjection();

    /// The optical system to preview.
    OLEF::OpticalSystem* m_opticalSystem;

    /// Center of the viewport.
    glm::vec2 m_viewCenter;

    /// Viewport dimension
    float m_viewArea;

    /// Coordinates for mouse grabbing.
    int m_grabX;
    int m_grabY;

    /// Sensitivity values for zooming and dragging.
    float m_zoomSensitivity;
    float m_dragSensitivity;

    /// Color of the preview canvas.
    QColor m_backgroundColor;

    /// Color of the background grid.
    QColor m_gridColor;

    /// Color of the iris.
    QColor m_irisColor;

    /// Color of the lenses.
    QColor m_lensColor;

    /// Color of the optical axis.
    QColor m_axisColor;

    /// Resolution of the lens strips.
    int m_lensResolution;

    /// Whether we should generate new geometry for the optical system.
    bool m_generateSystemGeometry;

    /// Whether we should generate new geometry for the ray batches.
    bool m_generateRayGeometry;

    /// Ray batches to trace through the system.
    std::vector<Ray> m_raysToDraw;

    /// The shader used to render the preview.
    GLuint m_renderShader;

    /// Projection matrix for projecting the line strips.
    glm::mat4 m_projection;

    /// Vertex buffer that holds the vertices to draw.
    GLuint m_vbo;

    /// Vertex array object used for rendering the line strips.
    GLuint m_vao;

    /// Vertex positions for the line strips that have to be rendered for the 
    /// preview.
    std::vector<glm::vec2> m_vertices;

    /// The list of all the line strips to render.
    std::vector<LineStripData> m_lineStrips;
};