#include "LensFlarePreviewer.h"

////////////////////////////////////////////////////////////////////////////////
// Access the GL functions
static QOpenGLFunctions_3_3_Core* getGLFunctions()
{
    return QOpenGLContext::currentContext()->
        versionFunctions<QOpenGLFunctions_3_3_Core>();
}

////////////////////////////////////////////////////////////////////////////////
LensFlarePreviewer::LensFlarePreviewer(OLEF::OpticalSystem* system, QWidget* parent):
    QOpenGLWidget(parent),
    m_lightPolar{ glm::radians(90.0f), glm::radians(90.0f) },
    m_prevMouse{ -1, -1 },
    m_opticalSystem(system),
    m_ghosts(system),
    m_starburstAlgorithm(nullptr),
    m_ghostAlgorithm(nullptr),
    m_imageLibrary(new ImageLibrary(this, this))
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
    fmt.setSamples(4);

    setFormat(fmt);

    // Set a minimum size.
    setMinimumSize(QSize(400, 400));
    
    // Enable mouse interaction
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
}

////////////////////////////////////////////////////////////////////////////////
LensFlarePreviewer::~LensFlarePreviewer()
{
    // Access the context.
    makeCurrent();

    // Also call glew init
    glewInit();

    // Release the starburst renderer object.
    if (m_starburstAlgorithm)
    {
        delete m_starburstAlgorithm;
    }

    // Release the ghost renderer object.
    if (m_ghostAlgorithm)
    {
        delete m_ghostAlgorithm;
    }

    // Release the created textures
    m_imageLibrary->releaseTextures();

    // Release the context
    doneCurrent();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::update()
{
    QOpenGLWidget::update();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::opticalSystemChanged()
{
    // There is only one ghost algorithm right now, so this is safe
    OLEF::RayTraceGhostAlgorithm* ghost =
        (OLEF::RayTraceGhostAlgorithm*) m_ghostAlgorithm;

    // Refresh the ghost list.
    m_ghosts = OLEF::GhostList(m_opticalSystem);
    ghost->setGhostList(m_ghosts);

    // Regenerate the image
    update();
}

////////////////////////////////////////////////////////////////////////////////
GLuint LensFlarePreviewer::uploadTexture(const QImage& image)
{
    // Access the context.
    makeCurrent();

    // Access the GL functions
    auto f = getGLFunctions();

    // Upload the texture
    GLuint tex;
    f->glGenTextures(1, &tex);
    f->glBindTexture(GL_TEXTURE_2D, tex);
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, 
        GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
    f->glBindTexture(GL_TEXTURE_2D, 0);

    // Release the context
    doneCurrent();
    return tex;
}

////////////////////////////////////////////////////////////////////////////////
void cleanUp(void* data)
{
    delete[] (GLubyte*) data;
}

QImage LensFlarePreviewer::loadTexture(GLuint texture)
{
    // Access the context.
    makeCurrent();

    // Access the GL functions
    auto f = getGLFunctions();

	// Bind the image and get its dimensions
	f->glBindTexture(GL_TEXTURE_2D, texture);

    GLint w, h;
    f->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    f->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    
	// Allocate memory for the pixels
	GLubyte* pixels = new GLubyte[w * h * 4];

	// Extract the image
	f->glPixelStorei(GL_PACK_ALIGNMENT, 1);
	f->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	f->glBindTexture(GL_TEXTURE_2D, 0);

	f->glBindTexture(GL_TEXTURE_2D, texture);

    doneCurrent();
    return { pixels, w, h, QImage::Format_RGBA8888, cleanUp, pixels };
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::releaseTexture(GLuint texture)
{
    // Access the context.
    makeCurrent();

    // Access the GL functions
    auto f = getGLFunctions();

    // Delete the texture
    f->glDeleteTextures(1, &texture);

    // Release the context
    doneCurrent();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::mousePressEvent(QMouseEvent* event)
{
    m_prevMouse[0] = event->x();
    m_prevMouse[1] = event->y();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::mouseMoveEvent(QMouseEvent* event)
{
    // Check if left button is pressed
    if ((event->buttons() & Qt::LeftButton) != 0)
    {
        // Compute delta values
        int dx = event->x() - m_prevMouse[0];
        int dy = event->y() - m_prevMouse[1];
        m_prevMouse[0] = event->x();
        m_prevMouse[1] = event->y();

        // Don't do anything on the first use
        if (m_prevMouse[0] == -1)
            return;

        // Update the angles
        m_lightPolar[0] -= dx * glm::radians(0.5f);
        m_lightPolar[1] += dy * glm::radians(0.5f);

        // Update the light angle
        /*
        m_lightSource.setIncidenceDirection(glm::vec3(
            glm::cos(m_lightPolar[0]) * glm::sin(m_lightPolar[1]), 
            glm::cos(m_lightPolar[1]),
            glm::sin(m_lightPolar[0]) * glm::sin(m_lightPolar[1])));
        */

        // Update the view
        update();
    }
}
    
////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::wheelEvent(QWheelEvent* event)
{}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::initializeGL()
{
    // Init glew for OpenLensFlare to work.
    glewInit();

    //TODO: the objects are initialized here, to make sure the proper objects
    //      are created within the proper context. Find a way to safely extract
    //      these objects into the MainWindow instance, and manage them from there

    // Create the starburst renderer.
    OLEF::DiffractionStarburstAlgorithm* diffStarburst = 
        new OLEF::DiffractionStarburstAlgorithm(m_opticalSystem, 0.15f, 
            40.0f);

    /// Create the ray trace ghost algorithm
    OLEF::RayTraceGhostAlgorithm* rayTraceGhost = 
        new OLEF::RayTraceGhostAlgorithm(m_opticalSystem, m_ghosts);

    // Initialize the light source.
    //m_lightSource.setScreenPosition(glm::vec2(0.0f));
    //m_lightSource.setIncidenceDirection(glm::vec3(0.0f, 0.0f, 1.0f));
    //m_lightSource.setDiffuseColor(glm::vec3(1.0f));
    //m_lightSource.setDiffuseIntensity(1.0f);
    
    // Store the created objects.
    m_starburstAlgorithm = diffStarburst;
    m_ghostAlgorithm = rayTraceGhost;
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::resizeGL(int w, int h)
{}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::paintGL()
{
    // Get to the OGL functions
    auto f = getGLFunctions();

    // Also call glew init
    glewInit();

    // Clear the background
    f->glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), 
        m_backgroundColor.blueF(), m_backgroundColor.alphaF());
    f->glClear(GL_COLOR_BUFFER_BIT);

    // Render the starburst
    //m_starburstAlgorithm->renderStarburst(m_lightSource);
    //m_ghostAlgorithm->renderAllGhosts(m_lightSource);
}