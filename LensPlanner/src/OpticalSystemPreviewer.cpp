#include "OpticalSystemPreviewer.h"

#include <QIODevice>
#include <QTextStream>
#include <QByteArray>

////////////////////////////////////////////////////////////////////////////////
// Access the GL functions
static QOpenGLFunctions_3_3_Core* getGLFunctions()
{
    return QOpenGLContext::currentContext()->
        versionFunctions<QOpenGLFunctions_3_3_Core>();
}

////////////////////////////////////////////////////////////////////////////////
OpticalSystemPreviewer::OpticalSystemPreviewer(OLEF::OpticalSystem* system, QWidget* parent):
    QOpenGLWidget(parent),
    m_opticalSystem(system),
    m_viewCenter(0.0f, 0.0f),
    m_viewArea(30.0f),
    m_zoomSensitivity(1.0f / 64.0f),
    m_dragSensitivity(1.0f / 128.0f),
    m_backgroundColor(223, 223, 223),
    m_gridColor(128, 128, 128),
    m_gridLineWidth(1.0f),
    m_irisColor(0, 0, 0),
    m_irisLineWidth(1.0f),
    m_lensColor(0, 0, 0),
    m_lensLineWidth(1.0f),
    m_axisColor(0, 0, 0),
    m_axisLineWidth(1.0f),
    m_lensResolution(16),
    m_generateSystemGeometry(true),
    m_generateRayGeometry(true),
    m_renderShader(0)
{
    // Initialize the GL format
    QSurfaceFormat fmt;

    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setDepthBufferSize(24);
    fmt.setRedBufferSize(8);
    fmt.setGreenBufferSize(8);
    fmt.setBlueBufferSize(8);
    fmt.setAlphaBufferSize(8);
    fmt.setSamples(16);

    setFormat(fmt);

    // Set a minimum size.
    setMinimumSize(QSize(300, 300));
}

////////////////////////////////////////////////////////////////////////////////
OpticalSystemPreviewer::~OpticalSystemPreviewer()
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Release the created OpenGL resources.
    if (m_renderShader != 0)
    {
        f->glDeleteBuffers(1, &m_vbo);
        f->glDeleteBuffers(1, &m_vao);
        f->glDeleteProgram(m_renderShader);
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::invalidate()
{
    m_generateSystemGeometry = m_generateRayGeometry = true;
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::update()
{
    QOpenGLWidget::update();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::opticalSystemChanged()
{
    // Indicate that the geometry needs to be regeneretated.
    m_generateSystemGeometry = true;

    // Update ourselves
    update();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::mousePressEvent(QMouseEvent* event)
{
    // Store the position of the mouse click
    if (event->button() == Qt::LeftButton)
    {
        m_grabX = event->x();
        m_grabY = event->y();
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::mouseMoveEvent(QMouseEvent* event)
{
    // Store the position of the mouse click
    if ((event->buttons() & Qt::LeftButton) != 0)
    {
        // Compute the delta values
        int dx = event->x() - m_grabX;
        int dy = event->y() - m_grabY;

        // Aspect ratio to correct the movement
        float aspect = (float) size().width() / (float) size().height();

        // Update the view center
        m_viewCenter.x -= dx * m_dragSensitivity * glm::sqrt(m_viewArea);
        m_viewCenter.y += dy * m_dragSensitivity * glm::sqrt(m_viewArea) * aspect;

        // Store the current mouse coordinates
        m_grabX = event->x();
        m_grabY = event->y();

        // Reconstruct the view matrix
        computeProjection();

        // Update the view
        update();
    }
}
    
////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::wheelEvent(QWheelEvent* event)
{
    // Update the view area
    int delta = event->delta();
    m_viewArea = glm::max(0.0f, m_viewArea - delta * m_zoomSensitivity);

    // Reconstruct the view matrix
    computeProjection();

    // Update the view
    update();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::initializeGL()
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Generate the vertex buffer and vertex array objects
    f->glGenBuffers(1, &m_vbo);
    f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);

    f->glGenVertexArrays(1, &m_vao);
    f->glBindVertexArray(m_vao);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    f->glBindVertexArray(0);

    // Also load the shader
    GLuint vs, fs, program;

    // Create the GL objects
    vs = f->glCreateShader(GL_VERTEX_SHADER);
    fs = f->glCreateShader(GL_FRAGMENT_SHADER);
    program = f->glCreateProgram();

    // Load the vertex shader source
    QFile vsFile(":/LensPlanner/Shaders/OpticalSystemPreviewer_vs.glsl");
    vsFile.open(QIODevice::ReadOnly);
    QTextStream vsStream(&vsFile);
    QString vsText = vsStream.readAll();
    QByteArray vsTextRaw = vsText.toLatin1();

    // Load the fragment shader source.
    QFile fsFile(":/LensPlanner/Shaders/OpticalSystemPreviewer_fs.glsl");
    fsFile.open(QIODevice::ReadOnly);
    QTextStream fsStream(&fsFile);
    QString fsText = fsStream.readAll();
    QByteArray fsTextRaw = fsText.toLatin1();

    // Set the shader sources
    const GLchar* vsSrc[] = { vsTextRaw.data() };
    const GLchar* fsSrc[] = { fsTextRaw.data() };
    f->glShaderSource(vs, sizeof(vsSrc) / sizeof(vsSrc[0]), vsSrc, NULL);
    f->glShaderSource(fs, sizeof(fsSrc) / sizeof(fsSrc[0]), fsSrc, NULL);

    // Compile the shaders
    f->glCompileShader(vs);
    f->glCompileShader(fs);

    // Make sure they compiled successfully
    static GLchar s_errorBuffer[4098];

    GLint vsStatus, fsStatus;
    f->glGetShaderiv(vs, GL_COMPILE_STATUS, &vsStatus);
    f->glGetShaderiv(fs, GL_COMPILE_STATUS, &fsStatus);

    if (vsStatus == GL_FALSE)
    {
        f->glGetShaderInfoLog(vs, 
            sizeof(s_errorBuffer) / sizeof(GLchar), NULL, s_errorBuffer);
        qCritical() << s_errorBuffer;
    }

    if (fsStatus == GL_FALSE)
    {
        f->glGetShaderInfoLog(fs, 
            sizeof(s_errorBuffer) / sizeof(GLchar), NULL, s_errorBuffer);
        qCritical() << s_errorBuffer;
    }

    // Attach the shaders, link, detach and delete the shaders
    f->glAttachShader(program, vs);
    f->glAttachShader(program, fs);
    f->glLinkProgram(program);
    f->glDetachShader(program, vs);
    f->glDetachShader(program, fs);
    f->glDeleteShader(vs);
    f->glDeleteShader(fs);

    // Make sure it linked successfully.
    GLint linkStatus;
    f->glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == GL_FALSE)
    {
        f->glGetProgramInfoLog(program, 
            sizeof(s_errorBuffer) / sizeof(GLchar), NULL, s_errorBuffer);
        qCritical() << s_errorBuffer;
    }

    // Store the generated shader.
    m_renderShader = program;
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::storeLineStrip(QColor color, float width, const QVector<glm::vec2>& vertices)
{
    // Store the line strip
    LineStripData lineStrip;

    lineStrip.m_color = color;
    lineStrip.m_width = width;
    lineStrip.m_start = m_vertices.size();
    lineStrip.m_length = vertices.size();

    m_lineStrips.push_back(lineStrip);

    // Store the line strip the vertices.
    m_vertices.append(vertices);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::traceRay(const RayBatch& ray, int rayId)
{
    // Various optical system attributes
    float maxHeight = m_opticalSystem->getTotalHeight();
    float filmHeight = m_opticalSystem->getFilmSize().y;
	float apertureHeight = m_opticalSystem->getEffectiveApertureHeight();
	float sensorDistance = m_opticalSystem->getSensorDistance();

    // Ray attributes
    float heightStep = (ray.m_endHeight - ray.m_startHeight) / (ray.m_rayCount - 1);

    // Vertex data for the current ray
    QVector<glm::vec2> vertices;
    vertices.reserve(m_opticalSystem->getElementCount() + 1);

    // Distance of the lens along the optical axis.
    float lensDistance = 0.0f;

    // Direction of the ray.
    glm::vec2 dir = glm::vec2(glm::cos(ray.m_angle), glm::sin(ray.m_angle));

    // Position of the ray.
    glm::vec2 pos{ -ray.m_startDistance, ray.m_startHeight + rayId * heightStep };

    // Intersection normal.
    glm::vec2 normal;

    // Store the starting position
    vertices.push_back(pos);
    
    // Go through each lens element
    int d = 1, e = 0;
    bool blocked = false;
    for (size_t i = 0; i < m_opticalSystem->getElementCount() && ! blocked; i += d)
    {
        // Flip trace direction upon hitting an interesting interface
        bool reflect = e < ray.m_ghost.getLength() && i == ray.m_ghost[e];
        if (reflect)
        {
            ++e;
            d *= -1;
        }

        // Extract the current lens
        const auto& lens = (*m_opticalSystem)[i];

        // Radius of the virtual sphere
        float radius = lens.getRadiusOfCurvature();
        float absRadius = glm::abs(radius);

        // Compute the center of the sphere
        glm::vec2 center = glm::vec2(lensDistance + radius, 0.0f);

        // Ray-plane intersection for flat surfaces
        if (absRadius < 0.00001f)
        {
            pos = pos + (dir * ((center.x - pos.x) / dir.x));
            normal = glm::vec2(dir.x > 0.0f ? -1.0f : 1.0f, 0.0f);
        }
        
        // Ray-sphere for spherical surfaces
        else
        {
            // Advance the ray
            //pos = pos + 2.0f * dir;
            
            // Vector pointing from the ray to the sphere center
            glm::vec2 D = pos - center;
            float B = glm::dot(D, dir);
            float C = glm::dot(D, D) - (absRadius * absRadius);
            
            // Discriminant
            float B2_C = B * B - C;
            
            // No hit if the discriminant is negative
            if (B2_C < 0.0)
                break;
            
            // The ray is inside the virtual sphere if multiplying its X 
            // coordinate by the lens radius yields a positive value, and it 
            // is outside, if the result is negative.
            float inside = -glm::sign(lens.getRadiusOfCurvature() * dir.x);
            float outside = -inside;

            // '-B - sqrt(B2_C)' if the ray is outside,
            // '-B + sqrt(B2_C)' if the ray is inside.
            float t = -B + glm::sqrt(B2_C) * inside;
            
            // Compute the hit position
            pos = pos + t * dir;
            
            // Compute the normal and flip it if the ray is inside the sphere
            normal = glm::normalize(pos - center) * -inside;
        }

        // Cull the ray if it missed the interface
        switch (lens.getType())
        {
            case OLEF::OpticalSystemElement::ElementType::LENS_SPHERICAL:
            case OLEF::OpticalSystemElement::ElementType::LENS_ASPHERICAL:
            {
                blocked = blocked || glm::abs(pos.y) > lens.getHeight();
            }
            break;
            
            case OLEF::OpticalSystemElement::ElementType::APERTURE_STOP:
            {
                blocked = blocked || glm::abs(pos.y) > apertureHeight;
            }
            break;
            
            case OLEF::OpticalSystemElement::ElementType::SENSOR:
            {
                blocked = blocked || glm::abs(pos.y) > filmHeight;
            }
            break;
        }

        // Extract the corresponding refraction indices.
        float n0, n2;
        if (d > 0)
        {
            n0 = (i == 0 ? 1.0f : (*m_opticalSystem)[i - 1].getIndexOfRefraction());
            n2 = (*m_opticalSystem)[i].getIndexOfRefraction();
        }
        else
        {
            n0 = (*m_opticalSystem)[i].getIndexOfRefraction();
            n2 = (*m_opticalSystem)[i - 1].getIndexOfRefraction();
        }

        // Reflect or refract the ray
        if (reflect)
        {
            dir = glm::reflect(dir, normal);
        }
        else
        {
            // Refract the ray
            dir = glm::refract(dir, normal, n0 / n2);
        }

        // Mark it as blocked upon experiencing total internal reflection
        if (dir == glm::vec2(0.0f))
        {
            blocked = true;
        }

        // Decrease the lens distance
        if (d > 0)
        {
            lensDistance = lensDistance + (*m_opticalSystem)[i].getThickness();
        }
        else
        {
            lensDistance = lensDistance - (*m_opticalSystem)[i - 1].getThickness();
        }

        // Store the position
        vertices.push_back(pos);
    }
    
    // Store the line strip
    storeLineStrip(ray.m_color, ray.m_width, vertices);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::generateRayGeometry()
{
    // Process each ray batch
	for (const auto& ray: m_raysToDraw)
    {
        // Make sure it references a valid ghost
        if (m_opticalSystem->isValidGhost(ray.m_ghost))
        {
            // Trace each individual ray
            for (int r = 0; r < ray.m_rayCount; ++r)
            {
                traceRay(ray, r);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::generateGridGeometry()
{
    // TODO: add support for grids
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::generateLensGeometry()
{
    // Extra length on the optical axis
    static const float OPTICAL_SYSTEM_SLACK = 0.20f;

	// Various optical system attributes
	float sensorDistance = m_opticalSystem->getSensorDistance();
    float housingHeight = m_opticalSystem->getTotalHeight();
    float filmHeight = m_opticalSystem->getFilmSize().y;
	float apertureHeight = m_opticalSystem->getEffectiveApertureHeight();

    // Generate the optical axis line
    storeLineStrip(m_axisColor, m_axisLineWidth,
    {
        glm::vec2(-sensorDistance * OPTICAL_SYSTEM_SLACK, 0),
        glm::vec2(sensorDistance, 0),
    });

    // Generate geometry for the actual lenses
    float lensDistance = 0.0f;
    for (const auto& lens: m_opticalSystem->getElements())
    {
        switch (lens.getType())
        {
            case OLEF::OpticalSystemElement::ElementType::LENS_SPHERICAL:
            case OLEF::OpticalSystemElement::ElementType::LENS_ASPHERICAL:
            {
                // Radius of the virtual sphere
                float height = lens.getHeight();
                float radius = lens.getRadiusOfCurvature();
                float absRadius = glm::abs(lens.getRadiusOfCurvature());
                bool isFlat = (absRadius < 0.00001f);
                float sphereRadius = isFlat ? height : radius;
                glm::vec2 sphereCenter = glm::vec2(lensDistance + radius, 0.0f);
                float resolution = isFlat ? 1 : m_lensResolution;
                float maxAngle = isFlat ? glm::radians(90.0f) : 
                    glm::asin(height / radius);
                float curAngle = glm::radians(180.0f) - maxAngle;
                float angleStep = (maxAngle * 2.0f) / resolution;

                // Vertices of the lens geometry
                QVector<glm::vec2> vertices(resolution + 1);

                // Generate geometry
                for (int i = 0; i <= resolution; ++i, curAngle += angleStep)
                {
                    glm::vec2 sphereVert{ glm::cos(curAngle), glm::sin(curAngle) };
                    vertices[i] = sphereCenter + sphereRadius * sphereVert;
                }

                // Store the lens data
                storeLineStrip(m_lensColor, m_lensLineWidth, vertices);
            }
            break;
            
            case OLEF::OpticalSystemElement::ElementType::APERTURE_STOP:
            {
                // Store the upper section
                storeLineStrip(m_irisColor, m_irisLineWidth,
                {
                    glm::vec2(lensDistance, apertureHeight),
                    glm::vec2(lensDistance, housingHeight),
                });

                // Store the lower section
                storeLineStrip(m_irisColor, m_irisLineWidth,
                {
                    glm::vec2(lensDistance, -apertureHeight),
                    glm::vec2(lensDistance, -housingHeight),
                });
            }
            break;

            case OLEF::OpticalSystemElement::ElementType::SENSOR:
            {
                // Store the sensor line
                storeLineStrip(m_axisColor, m_axisLineWidth,
                {
                    glm::vec2(lensDistance, filmHeight),
                    glm::vec2(lensDistance, -filmHeight),
                });
            }
            break;
        }

        lensDistance += lens.getThickness();
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::generateGeometry()
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Clear the data vectors
    m_vertices.clear();
    m_lineStrips.clear();

    // Generate the needed geometry
    generateGridGeometry();
    generateLensGeometry();
    generateRayGeometry();

    // Upload the vertex data
    f->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec2),
        m_vertices.data(), GL_DYNAMIC_DRAW);
    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::computeProjection()
{
    float aspectRatio = (float) size().width() / (float) size().height();
    m_projection = glm::ortho(
        (m_viewCenter.x - m_viewArea) * aspectRatio,
        (m_viewCenter.x + m_viewArea) * aspectRatio,
        m_viewCenter.y - m_viewArea,
        m_viewCenter.y + m_viewArea, 
        -1.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::paintGL()
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Clear the background
    f->glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), 
        m_backgroundColor.blueF(), m_backgroundColor.alphaF());
    f->glClear(GL_COLOR_BUFFER_BIT);
    f->glDisable(GL_BLEND);
    //f->glEnable(GL_LINE_SMOOTH);

    /// Check if we need to generate new geometry.
    if (m_generateSystemGeometry || m_generateRayGeometry)
    {
        generateGeometry();
        m_generateSystemGeometry = false;
        m_generateRayGeometry = false;
    }

    // Bind the rendering shader
    f->glUseProgram(m_renderShader);

    // Extract the needed uniform locations
    GLuint xformLoc = f->glGetUniformLocation(m_renderShader, "mTransform");
    GLuint colorLoc = f->glGetUniformLocation(m_renderShader, "vLineColor");

    /// Render each strip
    f->glBindVertexArray(m_vao);
    for (auto strip: m_lineStrips)
    {
        // Set the required uniforms.
        glm::vec4 color = glm::vec4(
            strip.m_color.redF(), 
            strip.m_color.greenF(), 
            strip.m_color.blueF(), 
            strip.m_color.alphaF());
        
        f->glLineWidth(strip.m_width);
        f->glUniformMatrix4fv(xformLoc, 1, GL_FALSE, glm::value_ptr(m_projection));
        f->glUniform4fv(colorLoc, 1, glm::value_ptr(color));

        f->glDrawArrays(GL_LINE_STRIP, strip.m_start, strip.m_length);
    }
    f->glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::resizeGL(int w, int h)
{
    computeProjection();
}