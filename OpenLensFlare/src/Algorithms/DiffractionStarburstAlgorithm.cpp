#include "DiffractionStarburstAlgorithm.h"
#include "GLHelpers.h"

#include "Common_Functions.glsl.h"
#include "Common_ColorSpace.glsl.h"
#include "DiffractionStarburstAlgorithm_Generate_VertexShader.glsl.h"
#include "DiffractionStarburstAlgorithm_Generate_FragmentShader.glsl.h"
#include "DiffractionStarburstAlgorithm_Render_VertexShader.glsl.h"
#include "DiffractionStarburstAlgorithm_Render_FragmentShader.glsl.h"

namespace OLEF
{

////////////////////////////////////////////////////////////////////////////////
DiffractionStarburstAlgorithm::DiffractionStarburstAlgorithm(OpticalSystem* opticalSystem):
    m_opticalSystem(opticalSystem),
    m_size(0.0f),
    m_intensity(0.0f),
    m_texture(0),
    m_external(false),
    m_generateShader(0),
    m_renderShader(0),
    m_vao(0)
{
    // Create the generate shader
    GLHelpers::ShaderSource generateSource;
    
    generateSource.m_source =
    {
        {
            GL_VERTEX_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
                Shaders::DiffractionStarburstAlgorithm_Generate_VertexShader,
            }
        },
        {
            GL_FRAGMENT_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
                Shaders::DiffractionStarburstAlgorithm_Generate_FragmentShader,
            }
        },
    };
    m_generateShader = GLHelpers::createShader(generateSource);
    
    // Create the render shader
    GLHelpers::ShaderSource renderSource;
    
    renderSource.m_source =
    {
        {
            GL_VERTEX_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
                Shaders::DiffractionStarburstAlgorithm_Render_VertexShader,
            }
        },
        {
            GL_FRAGMENT_SHADER, 
            {
                Shaders::Common_Functions,
                Shaders::Common_ColorSpace,
                Shaders::DiffractionStarburstAlgorithm_Render_FragmentShader,
            }
        },
    };
    m_renderShader = GLHelpers::createShader(renderSource);

    // Generate a dummy vertex array.
    glGenVertexArrays(1, &m_vao);
}

DiffractionStarburstAlgorithm::~DiffractionStarburstAlgorithm()
{
    // Generate a dummy vertex array.
    glDeleteVertexArrays(1, &m_vao);

    // Release the shaders
    glDeleteProgram(m_generateShader);
    glDeleteProgram(m_renderShader);

    // Release the generated texture, if any.
    if (m_texture != 0 && m_external == false)
    {
        glDeleteTextures(1, &m_texture);
    }
}

////////////////////////////////////////////////////////////////////////////////
bool DiffractionStarburstAlgorithm::generateTexture(TextureGenerationParameters parameters)
{
    // The aperture FT texture
    GLuint apertureFT = 0;
    GLfloat apertureDist = 0.0f;
    for (const auto& lens: m_opticalSystem->getElements())
    {
        if (lens.getType() == OpticalSystemElement::ElementType::APERTURE_STOP)
        {
            apertureFT = lens.getTextureFT();
            break;
        }

        apertureDist += lens.getThickness();
    }

    // Release any previously generated texture.
    if (m_texture != 0 && m_external == false)
    {
        glDeleteTextures(1, &m_texture);
    }

    // It's no longer an external texture.
    m_external = false;

	// Generate a texture object if it doesn't exist
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, parameters.m_textureWidth, 
        parameters.m_textureHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
    glBindTexture(GL_TEXTURE_2D, 0);

	// Create the FBO
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture, 0);

	// Bind the shader and set the viewport
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_BLEND);
	glViewport(0, 0, parameters.m_textureWidth, parameters.m_textureHeight);
	glUseProgram(m_generateShader);

	// Compute the scale of the texture
    GLHelpers::uploadUniform(m_generateShader, "fMinLambda", parameters.m_minWavelength);
    GLHelpers::uploadUniform(m_generateShader, "fMaxLambda", parameters.m_maxWavelength);
    GLHelpers::uploadUniform(m_generateShader, "fLambdaStep", parameters.m_wavelengthStep);
    GLHelpers::uploadUniform(m_generateShader, "fTextureLambda", 570.0f);
    GLHelpers::uploadUniform(m_generateShader, "fApertureDistance", apertureDist);

	// Bind the aperture FFT texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, apertureFT);
    GLHelpers::uploadUniform(m_generateShader, "sApertureFT", 0);
    
	// Render the starburst
    glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Delete the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
    
	// Make sure the texture has been updated
	glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    
	// Allocate memory for the pixels
	glm::vec4* floatPixels = 
        new glm::vec4[parameters.m_textureWidth * parameters.m_textureHeight];

	// Extract the image
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, floatPixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Upload the new image data the image
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, parameters.m_textureWidth, 
        parameters.m_textureHeight, 0, GL_RGBA, GL_FLOAT, floatPixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Delete the pixel data
	delete[] floatPixels;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
void DiffractionStarburstAlgorithm::renderStarburst(const LightSource& light)
{
    // Bind the starburst shader
    glUseProgram(m_renderShader);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Upload the rendering parameters
    float fnumber = m_opticalSystem->getFnumber();
    float aspect = m_opticalSystem->getAspectRatio();

    // Upload the rendering parameters
    glm::vec2 position = light.getScreenPosition();
    glm::vec2 scale = glm::vec2(m_size, m_size * aspect) * fnumber;
    glm::vec3 color = (glm::vec3(m_intensity) /glm::pow(fnumber, 4.0f)) *
        light.getDiffuseColor() * light.getDiffuseIntensity();

    GLHelpers::uploadUniform(m_renderShader, "vPosition", position);
    GLHelpers::uploadUniform(m_renderShader, "vScale", scale);
    GLHelpers::uploadUniform(m_renderShader, "vColor", color);

    // Bind the pre-generated texture.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    GLHelpers::uploadUniform(m_renderShader, "sStarburst", 0);

    // Render
    glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

}