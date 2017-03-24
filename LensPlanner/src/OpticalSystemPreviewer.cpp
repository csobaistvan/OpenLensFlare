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
    m_ghosts(system),
    m_backgroundColor(0x000000FF),
    m_generateGeometry(true),
    m_zoomFactor(0.3f),
    m_renderShader(0)
{
    // Initialize the GL format
    QSurfaceFormat fmt;

    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    fmt.setRedBufferSize(8);
    fmt.setGreenBufferSize(8);
    fmt.setBlueBufferSize(8);
    fmt.setAlphaBufferSize(8);

    setFormat(fmt);

    // Set a minimum size.
    setMinimumSize(QSize(300, 300));

    // Append the default ray
    m_raysToDraw.push_back({ -1, 2, 8.0f, 16.0f, 0.0f });

    // Set the default colors
    m_rays.m_color = QColor(255, 0, 0);
    m_lenses.m_color = QColor(255, 128, 0);
    m_apertures.m_color = QColor(0, 255, 0);
    m_films.m_color = QColor(255, 255, 255);
}

////////////////////////////////////////////////////////////////////////////////
OpticalSystemPreviewer::~OpticalSystemPreviewer()
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Release the created OpenGL resources.
    if (m_renderShader != 0)
    {
        f->glDeleteBuffers(1, &m_films.m_vbo);
        f->glDeleteBuffers(1, &m_films.m_vao);
        f->glDeleteBuffers(1, &m_lenses.m_vbo);
        f->glDeleteBuffers(1, &m_lenses.m_vao);
        f->glDeleteBuffers(1, &m_apertures.m_vbo);
        f->glDeleteBuffers(1, &m_apertures.m_vao);
        f->glDeleteBuffers(1, &m_rays.m_vbo);
        f->glDeleteBuffers(1, &m_rays.m_vao);
        f->glDeleteProgram(m_renderShader);
    }
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
    m_generateGeometry = true;

    // Refresh the ghost list.
    m_ghosts = OLEF::GhostList(m_opticalSystem);

    // Update ourselves
    update();
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::initializeGL()
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Generate vertex buffers
    f->glGenBuffers(1, &m_films.m_vbo);
    f->glBindBuffer(GL_ARRAY_BUFFER, m_films.m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);

    f->glGenVertexArrays(1, &m_films.m_vao);
    f->glBindVertexArray(m_films.m_vao);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    f->glBindVertexArray(0);

    f->glGenBuffers(1, &m_lenses.m_vbo);
    f->glBindBuffer(GL_ARRAY_BUFFER, m_lenses.m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);

    f->glGenVertexArrays(1, &m_lenses.m_vao);
    f->glBindVertexArray(m_lenses.m_vao);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    f->glBindVertexArray(0);

    f->glGenBuffers(1, &m_apertures.m_vbo);
    f->glBindBuffer(GL_ARRAY_BUFFER, m_apertures.m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);

    f->glGenVertexArrays(1, &m_apertures.m_vao);
    f->glBindVertexArray(m_apertures.m_vao);
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    f->glBindVertexArray(0);

    f->glGenBuffers(1, &m_rays.m_vbo);
    f->glBindBuffer(GL_ARRAY_BUFFER, m_rays.m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);

    f->glGenVertexArrays(1, &m_rays.m_vao);
    f->glBindVertexArray(m_rays.m_vao);
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
void OpticalSystemPreviewer::resizeGL(int w, int h)
{
    /*
    m_proj = glm::ortho(
        0.0f, 
        w * (1.0f + m_zoomFactor), 
        -h * (1.0f + m_zoomFactor * 0.5f), 
        h * (1.0f + m_zoomFactor * 0.5f),
        -1.0f, 
        1.0f);
        */
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::computeSystemBounds()
{
    // Margin around the optical system
    static const float MARGIN = 0.25f;

	// Various optical system attributes
    float sensorDistance = m_opticalSystem->getSensorDistance();
    float maxHeight = m_opticalSystem->getTotalHeight();
    
    m_proj = glm::ortho(
        -sensorDistance * MARGIN, 
        sensorDistance * (1.0f + MARGIN),
        -maxHeight * (1.0f + MARGIN), 
        maxHeight * (1.0f + MARGIN),
        -1.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::generateRayGeometry(const Ray& ray)
{
    // Skip the ray if no corresponding ghost exists
    if (ray.m_ghostIndex >= (int) m_ghosts.getGhostCount())
        return;

    // Various optical system attributes
	float sensorDistance = m_opticalSystem->getSensorDistance();
    float maxHeight = m_opticalSystem->getTotalHeight();
    float filmHeight = m_opticalSystem->getFilmSize().y;
	float effectiveApertureHeight = (m_opticalSystem->getEffectiveFocalLength() /
		 m_opticalSystem->getFnumber()) / 2.0f;

    // Trace the rays
    OLEF::Ghost ghost = (ray.m_ghostIndex < 0 ? OLEF::Ghost() : m_ghosts[ray.m_ghostIndex]);
    for (int r = 0; r < ray.m_incomingRayCount; ++r)
    {
        // Distance of the lens along the optical axis.
        float lensDistance = sensorDistance;

        // Position of the ray.
        glm::vec2 pos = glm::vec2(sensorDistance * 1.05f,
            ray.m_incomingRayOffset - r * ray.m_incomingRaySpacing);

        // Direction of the ray.
        glm::vec2 dir = glm::vec2(
            -glm::cos(ray.m_incomingRayAngle),
            glm::sin(ray.m_incomingRayAngle));

        // Intersection normal.
        glm::vec2 normal;
        
        // Id of the last vertex
        int backId = (int) m_rays.m_vertices.size();

        // Store the starting position
        glm::vec2 vertex = pos;
        vertex.x = sensorDistance - vertex.x;
        m_rays.m_vertices.push_back(vertex);
        
        // Go through each lens element
        int d = 1, phase = 0;
        for (size_t i = 0; i < m_opticalSystem->getElementCount(); i += d)
        {
            // Flip trace direction upon hitting an interesting interface
            bool reflect = phase < ghost.getLength() && i == ghost[phase];
            if (reflect)
            {
                ++phase;
                d = -d;
            }

            // Extract the current lens
            const auto& lens = (*m_opticalSystem)[i];

            // Radius of the virtual sphere
            float absRadius = glm::abs(lens.getRadiusOfCurvature());

            // Compute the center of the sphere
            glm::vec2 center = glm::vec2(
                lensDistance - lens.getRadiusOfCurvature(), 0.0f);

            // Special treatment for flat surfaces
            if (absRadius < 0.00001f)
            {
                pos = pos + (dir * ((center.x - pos.x) / dir.x));
                normal = glm::vec2(dir.x > 0.0f ? -1.0f : 1.0f, 0.0f);
            }
            else
            {
                // Advance the ray
                pos = pos + 2.0f * dir;
                
                // Vector pointing from the ray to the sphere center
                glm::vec2 D = pos - center;
                float B = glm::dot(D, dir);
                float C = glm::dot(D, D) - (absRadius * absRadius);
                
                // Discriminant
                float B2_C = B * B - C;
                
                // No hit if the discriminant is negative
                if (B2_C < 0.0)
                    break;
                
                // The ray is inside the virtual sphere if multiplying its Z coordinate by 
                // the lens radius yields a positive value, and it is outside, if the result
                // is negative.
                float inside = glm::sign(lens.getRadiusOfCurvature() * dir.x);
                float outside = -inside;

                // '-B - sqrt(B2_C)' if the ray is outside,
                // '-B + sqrt(B2_C)' if the ray is inside.
                float t = -B + glm::sqrt(B2_C) * inside;
                
                // Compute the hit position
                pos = pos + t * dir;
                
                // Compute the hit normal - also flip it if the ray is inside the sphere
                normal = glm::normalize(pos - center) * -inside;
            }

            // Cull the ray if it missed the interface
            bool keep = true;
            switch (lens.getType())
            {
                case OLEF::OpticalSystemElement::ElementType::LENS_SPHERICAL:
                case OLEF::OpticalSystemElement::ElementType::LENS_ASPHERICAL:
                {
                    keep = glm::abs(pos.y) <= lens.getHeight();
                }
                break;
                
                case OLEF::OpticalSystemElement::ElementType::APERTURE_STOP:
                {
                    keep = glm::abs(pos.y) <= effectiveApertureHeight;
                }
                break;
                
                case OLEF::OpticalSystemElement::ElementType::SENSOR:
                {
                    keep = glm::abs(pos.y) <= filmHeight;
                }
                break;
            }
            if (!keep)
                break;

            // Reflect or refract the ray
            if (reflect)
            {
                dir = glm::reflect(dir, normal);
            }
            else
            {
                float n0 = (i == 0 ? 1.0f : (*m_opticalSystem)[i - d].getIndexOfRefraction());
                float n2 = lens.getIndexOfRefraction();
                dir = glm::refract(dir, normal, n0 / n2);

                if (dir == glm::vec2(0.0f))
                    break;
            }

            // Decrease the lens distance
            lensDistance = lensDistance - d * lens.getThickness();

            // Store the position
            glm::vec2 vertex = pos;
            vertex.x = sensorDistance - vertex.x;
            m_rays.m_vertices.push_back(vertex);
        }
        
        // Store the indices
        m_rays.m_indices.push_back({ backId, (int) m_rays.m_vertices.size() - backId });
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::generateRayGeometry()
{
	for (const auto& ray: m_raysToDraw)
        generateRayGeometry(ray);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::generateLensGeometry()
{
    // Extra length on the optical axis
    static const float OPTICAL_SYSTEM_SLACK = 0.20f;

	// Various optical system attributes
	float sensorDistance = m_opticalSystem->getSensorDistance();
    float maxHeight = m_opticalSystem->getTotalHeight();
    float filmHeight = m_opticalSystem->getFilmSize().y;
	float effectiveApertureHeight = (m_opticalSystem->getEffectiveFocalLength() /
		 m_opticalSystem->getFnumber()) / 2.0f;

    // Generate the film and optical axis
    m_films.m_vertices = 
    {
        glm::vec2(sensorDistance, filmHeight),
        glm::vec2(sensorDistance, -filmHeight),
        
        glm::vec2(-sensorDistance * OPTICAL_SYSTEM_SLACK, 0),
        glm::vec2(sensorDistance, 0),
    };

    m_films.m_indices = 
    {
        { 0, 2 },
        { 2, 2 },
    };

    // Generate geometry for the actual lenses
    float lensDistance = sensorDistance;
    for (const auto& lens: m_opticalSystem->getElements())
    {
        switch (lens.getType())
        {
            case OLEF::OpticalSystemElement::ElementType::LENS_SPHERICAL:
            case OLEF::OpticalSystemElement::ElementType::LENS_ASPHERICAL:
            {
                // Radius of the virtual sphere
                float absRadius = glm::abs(lens.getRadiusOfCurvature());

                // Special treatment for flat surfaces
                if (absRadius < 0.00001f)
                {
                    glm::vec2 vertices[] = 
                    {
                        glm::vec2(sensorDistance - lensDistance, -lens.getHeight()),
                        glm::vec2(sensorDistance - lensDistance, lens.getHeight()),
                    };
                    int backId = (int) m_lenses.m_vertices.size();

                    m_lenses.m_vertices.insert(m_lenses.m_vertices.end(),
                        vertices, vertices + 2);
                    m_lenses.m_indices.push_back({ backId, 2 });
                }
                else
                {
                    // Number of lens vertices
                    static const int LENS_VERTICES = 65;

                    // Find the angle that corresponds to the lens height
                    float maxAngle = glm::asin(lens.getHeight() / 
                        lens.getRadiusOfCurvature());

                    // Compute the center of the sphere
                    glm::vec2 center = glm::vec2(
                        lensDistance - lens.getRadiusOfCurvature(), 0.0f);

                    // Generate geometry
                    int backId = (int) m_lenses.m_vertices.size();
                    float angle = maxAngle;
                    for (int i = 0; i < LENS_VERTICES; ++i)
                    {
                        glm::vec2 vertex = center +  lens.getRadiusOfCurvature() * 
                            glm::vec2(glm::cos(angle), glm::sin(angle));
                        vertex.x = sensorDistance - vertex.x;
                        m_lenses.m_vertices.push_back(vertex);
                        
                        angle -= maxAngle / (LENS_VERTICES / 2);
                    }
                    m_lenses.m_indices.push_back({ backId, LENS_VERTICES });
                }

            }
            break;
            
            case OLEF::OpticalSystemElement::ElementType::APERTURE_STOP:
            {
                glm::vec2 vertices[] = 
                {
                    glm::vec2(sensorDistance - lensDistance, effectiveApertureHeight),
                    glm::vec2(sensorDistance - lensDistance, maxHeight),
                    glm::vec2(sensorDistance - lensDistance, -effectiveApertureHeight),
                    glm::vec2(sensorDistance - lensDistance, -maxHeight),
                };
                int backId = (int) m_apertures.m_vertices.size();

                m_apertures.m_vertices.insert(m_apertures.m_vertices.end(),
                    vertices, vertices + 4);
                m_apertures.m_indices.push_back({ backId, 2 });
                m_apertures.m_indices.push_back({ backId + 2, 2 });
            }
            break;
        }

        lensDistance -= lens.getThickness();
    }
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::generateGeometry()
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Clear the data vectors.
    m_films.m_vertices.clear();
    m_lenses.m_vertices.clear();
    m_apertures.m_vertices.clear();
    m_rays.m_vertices.clear();
    
    m_films.m_indices.clear();
    m_lenses.m_indices.clear();
    m_apertures.m_indices.clear();
    m_rays.m_indices.clear();

    // Generate the needed geometry
    generateRayGeometry();
    generateLensGeometry();

    // Upload the vertex data
    f->glBindBuffer(GL_ARRAY_BUFFER, m_films.m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, m_films.m_vertices.size() * sizeof(glm::vec2),
        m_films.m_vertices.data(), GL_DYNAMIC_DRAW);
    
    f->glBindBuffer(GL_ARRAY_BUFFER, m_lenses.m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, m_lenses.m_vertices.size() * sizeof(glm::vec2),
        m_lenses.m_vertices.data(), GL_DYNAMIC_DRAW);

    f->glBindBuffer(GL_ARRAY_BUFFER, m_apertures.m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, m_apertures.m_vertices.size() * sizeof(glm::vec2),
        m_apertures.m_vertices.data(), GL_DYNAMIC_DRAW);

    f->glBindBuffer(GL_ARRAY_BUFFER, m_rays.m_vbo);
    f->glBufferData(GL_ARRAY_BUFFER, m_rays.m_vertices.size() * sizeof(glm::vec2),
        m_rays.m_vertices.data(), GL_DYNAMIC_DRAW);

    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemPreviewer::renderLineSet(const LineSet& lineSet)
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Set its uniforms.
    glm::mat4 transform = m_proj * m_transform;
    glm::vec4 color = glm::vec4(lineSet.m_color.redF(), lineSet.m_color.greenF(), 
        lineSet.m_color.blueF(), lineSet.m_color.alphaF());
    
    GLuint transformLoc = f->glGetUniformLocation(m_renderShader, "mTransform");
    f->glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
    GLuint colorLoc = f->glGetUniformLocation(m_renderShader, "vLineColor");
    f->glUniform4f(colorLoc, color.r, color.g, color.b, color.a);

    /// Render each strip
    f->glBindVertexArray(lineSet.m_vao);
    for (auto strips: lineSet.m_indices)
    {
        glDrawArrays(GL_LINE_STRIP, strips[0], strips[1]);
    }
    f->glBindVertexArray(0);
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

    /// Check if we need to generate new geometry.
    if (m_generateGeometry)
    {
        generateGeometry();
        computeSystemBounds();
        m_generateGeometry = false;
    }

    // Bind the rendering shader
    f->glUseProgram(m_renderShader);

    // Render the film and optical axis
    renderLineSet(m_films);
    renderLineSet(m_lenses);
    renderLineSet(m_apertures);
    renderLineSet(m_rays);
}