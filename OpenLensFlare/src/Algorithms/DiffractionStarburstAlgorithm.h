#pragma once

#include "../OpticalSystem.h"
#include "../LightSource.h"
#include "../StarburstAlgorithm.h"

namespace OLEF
{

/// Implements a starburst rendering algorithm that renders a sprite at the 
/// location of the light source.
class DiffractionStarburstAlgorithm: public StarburstAlgorithm
{
public:
    /// Constructs an algorithms by using the parameter texture as the sprite.
    DiffractionStarburstAlgorithm(OpticalSystem* system);
    
    /// These objects are not copyable.
    DiffractionStarburstAlgorithm(const DiffractionStarburstAlgorithm& other) = delete;

    /// Releases all the allocated GL objects.
    ~DiffractionStarburstAlgorithm();

    /// These objects are not copyable.
    DiffractionStarburstAlgorithm& operator=(const DiffractionStarburstAlgorithm& other) = delete;
    
    /// Parameters for generating the starburst texture.
    struct TextureGenerationParameters
    {
        /// Width of the generated texture.
        int m_textureWidth = 512;

        /// Height of the generated texture.
        int m_textureHeight = 512;
        
        /// Starting wavelength for the composition.
        float m_minWavelength = 380.0f;

        /// Ending wavelength for the composition.
        float m_maxWavelength = 780.0f;

        /// Wavelength step size.
        float m_wavelengthStep = 5.0f;
    };

    /// Generates the starburst texture. Previous copies are discarded.
    bool generateTexture(TextureGenerationParameters parameters = {});

    /// Renders the starburst corresponding to the parameter light source.
    void renderStarburst(const LightSource& light);

    /// Returns a pointer to the optical system.
    OpticalSystem* getOpticalSystem() const { return m_opticalSystem; }

    /// Returns a handle to the generated texture, that is used for rendering.
    GLuint getTexture() const { return m_texture; }
    
    /// Size of the starburst. Note that this is scaled by the F-number of the
    /// optical system.
    float getSize() const { return m_size; }
    
    /// Intensity of the starburst. Note that this is scaled by the F-number of
    /// the optical system.
    float getIntensity() const { return m_intensity; }
    
    /// Size of the starburst. Note that this is scaled by the F-number of the
    /// optical system.
    void setSize(float value) { m_size = value; }
    
    /// Intensity of the starburst. Note that this is scaled by the F-number of
    /// the optical system.
    void setIntensity(float value) { m_intensity = value; }

    /// Stores the parameter texture as the starburst texture.
    void setTexture(GLuint texture) { m_texture = texture; m_external = true; }

private:
    /// Pointer to the optical system.
    OpticalSystem* m_opticalSystem;

    /// Size of the starburst. Note that this is scaled by the F-number of the
    /// optical system.
    float m_size;
    
    /// Intensity of the starburst. Note that this is scaled by the F-number of
    /// the optical system.
    float m_intensity;
    
    /// The generated starburst texture.
    GLuint m_texture;

    /// Whether the texture is from an external source or not.
    bool m_external;

    /// A dummy vertex array to use, since OpenGL requires a valid object to be
    /// bound, even if we don't actually use any vertex buffers.
    GLuint m_vao;

    /// The shader object used to generate the starburst texture.
    GLuint m_generateShader;
    
    /// The shader object used to render the starburst.
    GLuint m_renderShader;
};

}