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
    /// Constructs an algorithms by using the parameter texture as the sprite
    /// Constructs a texture generator object with the provided parameters.
    DiffractionStarburstAlgorithm(OpticalSystem* system, float size, float intensity);
    
    /// These objects are not copyable.
    DiffractionStarburstAlgorithm(const DiffractionStarburstAlgorithm& other) = delete;

    /// Releases all the allocated GL objects.
    ~DiffractionStarburstAlgorithm();

    /// These objects are not copyable.
    DiffractionStarburstAlgorithm& operator=(const DiffractionStarburstAlgorithm& other) = delete;
    
    /// Generates the starburst texture. Previous copies are discarded.
    bool generateTexture(int width, int height, float minWl, float maxWl, float wlStep);

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

    /// A dummy vao to use.
    GLuint m_vao;

    /// The shader object used to generate the starburst texture.
    GLuint m_generateShader;
    
    /// The shader object used to render the starburst.
    GLuint m_renderShader;
};

}