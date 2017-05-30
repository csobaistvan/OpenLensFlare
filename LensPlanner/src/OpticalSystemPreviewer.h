#pragma once

#include "Dependencies.h"

/// The optical system previewer widget.
///
/// TODO: display invalid elements with a special color
class OpticalSystemPreviewer: public QOpenGLWidget
{
    Q_OBJECT;

public:
    explicit OpticalSystemPreviewer(OLEF::OpticalSystem* system, QWidget* parent = nullptr);
    ~OpticalSystemPreviewer();

    /// Parameters for a ray to visualize.
    struct RayBatch
    {
        /// The ghost to render.
        OLEF::Ghost m_ghost;

        /// Color of the corresponding lines.
        QColor m_color;

        /// Width of the line strips.
        float m_width;

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

    /// Getters/setters for the preview attributes.
    OLEF::OpticalSystem* getOpticalSystem() const { return m_opticalSystem; }
    QColor getBackgroundColor() const { return m_backgroundColor; };
    QColor getGridColor() const { return m_gridColor; };
    float getGridLineWidth() const { return m_gridLineWidth; };
    QColor getIrisColor() const { return m_irisColor; };
    float getIrisLineWidth() const { return m_irisLineWidth; };
    QColor getLensColor() const { return m_lensColor; };
    float getLensLineWidth() const { return m_lensLineWidth; };
    QColor getAxisColor() const { return m_axisColor; };
    float getAxisLineWidth() const { return m_axisLineWidth; };
    int getLensResolution() const { return m_lensResolution; };
    const QVector<RayBatch>& getRayBatchParameters() const { return m_raysToDraw; }

    void setBackgroundColor(QColor value) { m_backgroundColor = value; };
    void setGridColor(QColor value) { m_gridColor = value; };
    void setGridLineWidth(float value) { m_gridLineWidth = value; };
    void setIrisColor(QColor value) { m_irisColor = value; };
    void setIrisLineWidth(float value) { m_irisLineWidth = value; };
    void setLensColor(QColor value) { m_lensColor = value; };
    void setLensLineWidth(float value) { m_lensLineWidth = value; };
    void setAxisColor(QColor value) { m_axisColor = value; };
    void setAxisLineWidth(float value) { m_axisLineWidth = value; };
    void setLensResolution(int value) { m_lensResolution = value; };
    void setRayBatchParameters(const QVector<RayBatch>& value) { m_raysToDraw = value; }

    /// Invalidates the current preview.
    void invalidate();

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

    /// Holds the index of the starting vertex of each line strip, and the 
    /// number of vertices that correspond to said strips.
    struct LineStripData
    {
        /// Color of the line strip. Stored here (instead of with the batch)
        /// to allow per-strip color, should it become needed.
        QColor m_color;

        /// Width of the line strip.
        float m_width;

        /// Index of the starting vertex.
        int m_start;

        /// Number of vertices.
        int m_length;
    };

    /// Generates geometry for rendering.
    void storeLineStrip(QColor color, float width, const QVector<glm::vec2>& vertices);
    void traceRay(const RayBatch& ray, int id);
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

    /// Width of the grid lines.
    float m_gridLineWidth;

    /// Color of the iris.
    QColor m_irisColor;

    /// Width of the iris lines.
    float m_irisLineWidth;

    /// Color of the lenses.
    QColor m_lensColor;

    /// Width of the lens lines.
    float m_lensLineWidth;

    /// Color of the optical axis.
    QColor m_axisColor;

    /// Width of the axis lines.
    float m_axisLineWidth;

    /// Resolution of the lens strips.
    int m_lensResolution;

    /// Whether we should generate new geometry for the optical system.
    bool m_generateSystemGeometry;

    /// Whether we should generate new geometry for the ray batches.
    bool m_generateRayGeometry;

    /// Ray batches to trace through the system.
    QVector<RayBatch> m_raysToDraw;

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
    QVector<glm::vec2> m_vertices;

    /// The list of all the line strips to render.
    QVector<LineStripData> m_lineStrips;
};