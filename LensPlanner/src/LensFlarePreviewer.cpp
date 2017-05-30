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
    m_opticalSystem(system),
    m_imageLibrary(new ImageLibrary(this, this)),
    m_diffractionStarburstAlgorithm(nullptr),
    m_starburstTextureSize(512),
    m_starburstMinWavelength(390.0f),
    m_starburstMaxWavelength(780.0f),
    m_starburstWavelengthStep(5.0f),
    m_rayTraceGhostAlgorithm(nullptr),
    m_precompute(false),
    m_generateStarburst(false)
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
}

////////////////////////////////////////////////////////////////////////////////
LensFlarePreviewer::~LensFlarePreviewer()
{
    // Access the context.
    makeCurrent();

    // Also call glew init
    glewInit();

    // Release the starburst renderer object.
    if (m_diffractionStarburstAlgorithm)
    {
        delete m_diffractionStarburstAlgorithm;
    }

    // Release the ghost renderer object.
    if (m_rayTraceGhostAlgorithm)
    {
        delete m_rayTraceGhostAlgorithm;
    }

    // Release the created textures
    m_imageLibrary->releaseTextures();

    // Release the context
    doneCurrent();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::generateStarburst()
{
    // Make sure it is not called before the objects are initialized
    if (m_diffractionStarburstAlgorithm == nullptr)
        return;

    // Setup the required parameter object
    OLEF::DiffractionStarburstAlgorithm::TextureGenerationParameters parameters;

    parameters.m_textureWidth = m_starburstTextureSize;
    parameters.m_textureHeight = m_starburstTextureSize;
    parameters.m_minWavelength = m_starburstMinWavelength;
    parameters.m_maxWavelength = m_starburstMaxWavelength;
    parameters.m_wavelengthStep = m_starburstWavelengthStep;

    // Generate the texture
    m_diffractionStarburstAlgorithm->generateTexture(parameters);

    // Update the view
    update();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::computeGhostParameters()
{
    // Generate the starting ghost list
    OLEF::GhostList originalGhosts = m_opticalSystem->generateGhosts(2, false);

    // Process each possible angle
    //
    // TODO: don't use pre-baked angles!
    QMap<float, OLEF::GhostList> rawValues;

    for (int i = 0; i < 181; ++i)
    {
        // Current angle
        float angle = i * 0.5f;

        // Construct the parameter object
        OLEF::RayTraceGhostAlgorithm::GhostAttribComputeParams computeParams;

        computeParams.m_angle = glm::radians(angle);
        computeParams.m_boundingRays = { 32, 32, 32 };
        computeParams.m_rayPresets = { 5, 16, 32, 64, 128 };
        computeParams.m_targetVariance = 0.025f;

        // Compute the ghost attributes
        OLEF::GhostList currentGhosts = 
            m_rayTraceGhostAlgorithm->computeGhostAttributes(
                originalGhosts, computeParams);

        // Store it in the map
        rawValues[angle] = currentGhosts;
    }

    // Use neighbouring values to find looser bounds, to avoid clipping
    m_precomputedGhosts = rawValues;

    for (auto it = rawValues.begin() + 1; it != rawValues.end() - 1; ++it)
    {
        // Extract the current, previous and next ghost lists
        const auto& prevGhosts = (it - 1).value();
        const auto& currentGhosts = it.value();
        const auto& nextGhosts = (it + 1).value();
        auto mergedGhosts = currentGhosts;

        // Process each ghost on the list
        for (size_t ghostId = 0; ghostId < mergedGhosts.size(); ++ghostId)
        {
            // Extract the previous, current and next pupil bounds
            OLEF::Ghost::BoundingRect rawPupilBounds[3] =
            {
                prevGhosts[ghostId].getPupilBounds(),
                currentGhosts[ghostId].getPupilBounds(),
                nextGhosts[ghostId].getPupilBounds(),
            };
            
            // Extract the previous, current and next sensor bounds
            OLEF::Ghost::BoundingRect rawSensorBounds[3] =
            {
                prevGhosts[ghostId].getSensorBounds(),
                currentGhosts[ghostId].getSensorBounds(),
                nextGhosts[ghostId].getSensorBounds(),
            };

            // Output pupil and sensor values
            OLEF::Ghost::BoundingRect outPupilBounds;
            OLEF::Ghost::BoundingRect outSensorBounds;

            // Process the X/W and Y/H channels of both bounds
            outPupilBounds[0] = glm::min(
                rawPupilBounds[0][0], glm::min(
                rawPupilBounds[1][0],
                rawPupilBounds[2][0]
                ));
            outPupilBounds[1] = glm::max(
                rawPupilBounds[0][1], glm::max(
                rawPupilBounds[1][1],
                rawPupilBounds[2][1]
                ));
            outSensorBounds[0] = glm::min(
                rawSensorBounds[0][0], glm::min(
                rawSensorBounds[1][0],
                rawSensorBounds[2][0]
                ));
            outSensorBounds[1] = glm::max(
                rawSensorBounds[0][1], glm::max(
                rawSensorBounds[1][1],
                rawSensorBounds[2][1]
                ));

            // Store the computed values
            mergedGhosts[ghostId].setPupilBounds(outPupilBounds);
            mergedGhosts[ghostId].setSensorBounds(outSensorBounds);
        }

        // Store the merged ghost list
        m_precomputedGhosts[it.key()] = mergedGhosts;
    }

    // Update the view
    update();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::update()
{
    QOpenGLWidget::update();
}

////////////////////////////////////////////////////////////////////////////////
void LensFlarePreviewer::opticalSystemChanged()
{
    // Clear the precomputed attribute set
    m_precomputedGhosts.clear();

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
void LensFlarePreviewer::initializeGL()
{
    // Init glew for OpenLensFlare to work.
    glewInit();

    //TODO: the objects are initialized here, to make sure the proper objects
    //      are created within the proper context. Find a way to safely extract
    //      these objects into the MainWindow instance, and manage them from there

    // Create the starburst renderer.
    m_diffractionStarburstAlgorithm = new OLEF::DiffractionStarburstAlgorithm(
        m_opticalSystem);

    /// Create the ray trace ghost algorithm
    m_rayTraceGhostAlgorithm = new OLEF::RayTraceGhostAlgorithm(m_opticalSystem);
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

    // Compute ghost bounds, if needed.
    if (m_precompute)
    {
        computeGhostParameters();
        m_precompute = false;
    }

    // Generate the starburst texture, if needed.
    if (m_generateStarburst)
    {
        generateStarburst();
        m_generateStarburst = false;
    }

    // Clear the background
    f->glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), 
        m_backgroundColor.blueF(), m_backgroundColor.alphaF());
    f->glClear(GL_COLOR_BUFFER_BIT);

    // Render the starburst
    for (auto layer: m_layers)
    {
        // Turn on wireframe rendering, if requested.
        if (layer.m_wireframe)
        {
		    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        // Construct the corresponding light source object
        OLEF::LightSource lightSource;

        lightSource.setScreenPosition(layer.m_lightPosition);
        lightSource.setIncidenceDirection(glm::normalize(layer.m_lightDirection));
        lightSource.setDiffuseColor(glm::vec3(
            layer.m_lightColor.redF(), 
            layer.m_lightColor.greenF(), 
            layer.m_lightColor.blueF()
        ));
        lightSource.setDiffuseIntensity(layer.m_lightIntensity);

        // Set the per-light starburst parameters
        m_diffractionStarburstAlgorithm->setSize(layer.m_starburstSize);
        m_diffractionStarburstAlgorithm->setIntensity(layer.m_starburstIntensity);
        
        // Render the starbursts
        m_diffractionStarburstAlgorithm->renderStarburst(lightSource);

        // Generate the list of ghosts to render
        OLEF::GhostList allGhosts;

        // Re-use the precomputed values if we can
        if (layer.m_useGhostAttributes)
        {
            float angle = glm::degrees(glm::acos(glm::dot(
                -lightSource.getIncidenceDirection(), 
                glm::vec3(0.0f, 0.0f, -1.0f))));

            auto it = m_precomputedGhosts.lowerBound(angle - 0.1f);
            if (it != m_precomputedGhosts.end() && glm::abs(angle - it.key()) < 1.0f)
                allGhosts = it.value();
        }

        // Generate a fresh list if we couldn't re-use anything
        if (!layer.m_useGhostAttributes || allGhosts.empty())
        {
            allGhosts = m_opticalSystem->generateGhosts(2, false);
        }
        
        int firstGhost = std::min(layer.m_firstGhost - 1, (int) allGhosts.size());
        int lastGhost = std::min(layer.m_firstGhost + layer.m_numGhosts - 1, (int) allGhosts.size());

        OLEF::GhostList ghosts(allGhosts.begin() + firstGhost, 
            allGhosts.begin() + lastGhost);

        // Set the per-light ghost parameters        
        m_rayTraceGhostAlgorithm->setIntensityScale(layer.m_ghostIntensityScale);
        m_rayTraceGhostAlgorithm->setRenderMode(layer.m_ghostRenderMode);
        m_rayTraceGhostAlgorithm->setShadingMode(layer.m_ghostShadingMode);
        m_rayTraceGhostAlgorithm->setDistanceClip(layer.m_ghostDistanceClip);
        m_rayTraceGhostAlgorithm->setRadiusClip(layer.m_ghostRadiusClip);
        m_rayTraceGhostAlgorithm->setIntensityClip(layer.m_ghostIntensityClip);

        // Render the generated ghosts
        m_rayTraceGhostAlgorithm->renderGhosts(lightSource, ghosts);
        
        // Disable wireframe rendering
        if (layer.m_wireframe)
        {
	        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}