#pragma once

#include "Dependencies.h"

namespace OLEF
{

/// Holds a one frame snapshot of all the light source information that can be 
/// relevant for rendering the various lens flare effects.
class LightSource
{
public:
    /// Constructs an empty light source.
    LightSource():
        LightSource(glm::vec2(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), 0.0f)
    {}
    
    /// Constructs a light source with the parameter values.
    LightSource(glm::vec2 pos, glm::vec3 dir, glm::vec3 color, float intensity):
        m_screenPosition(pos),
        m_incidenceDirection(dir),
        m_diffuseColor(color),
        m_diffuseIntensity(intensity)
    {}

    /// Returns the position of the light source on the screen.
    glm::vec2 getScreenPosition() const { return m_screenPosition; }

    /// Returns a unit length vector describing the incidence direction.
    glm::vec3 getIncidenceDirection() const { return m_incidenceDirection; }
    
    /// Returns a unit length vector pointing toward the light source.
    glm::vec3 getToLightDirection() const { return -m_incidenceDirection; }

    /// Returns the normalized color of the light source
    glm::vec3 getDiffuseColor() const { return m_diffuseColor; }
    
    /// Returns the diffuse intensity of the light source.
    float getDiffuseIntensity() const { return m_diffuseIntensity; }

    /// Position of the light source on the screen.
    void setScreenPosition(glm::vec2 value) { m_screenPosition = value; }

    /// A unit length vector describing the incidence direction.
    void setIncidenceDirection(glm::vec3 value) { m_incidenceDirection = value; }

    /// Color of the light source
    void setDiffuseColor(glm::vec3 value) { m_diffuseColor = value; }
    
    /// Intensity of the light source.
    void setDiffuseIntensity(float value) { m_diffuseIntensity = value; }

private:
    /// Position of the light source on the screen.
    glm::vec2 m_screenPosition;

    /// A unit length vector describing the incidence direction.
    glm::vec3 m_incidenceDirection;

    /// Color of the light source
    glm::vec3 m_diffuseColor;
    
    /// Intensity of the light source.
    float m_diffuseIntensity;
};

}