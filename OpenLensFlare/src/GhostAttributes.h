#pragma once

#include "Dependencies.h"

/// Instances of this class holds various information about a partical ghost in
/// a particular optical system.
class GhostAttributes
{
public:
    /// Represents a 2D bounding rect, by storing the lower left corner and 
    /// the length of its sides
    using BoundingRect = std::array<glm::vec2, 2>;
    
    /// Default constructs the attributes object.
    GhostAttributes():
        m_pupilBounds{ glm::vec2(-1.0f), glm::vec2(2.0f) },
        m_sensorBounds{ glm::vec2(-1.0f), glm::vec2(2.0f) },
        m_minChannels(3),
        m_optimalChannels(3),
        m_minRays(129),
        m_optimalRays(129)
    {}

    /// Returns the tightest bounding quad on the pupil element from which rays 
    /// can hit the sensor, normalized to [-1, 1].
    BoundingRect getPupilBounds() const { return m_pupilBounds; }
    
    /// Returns the tightest bounding quad of the ghost image, on the sensor.
    /// This is normalized to [-1, 1].
    BoundingRect getSensorBounds() const { return m_sensorBounds; }
    
    /// Returns the minimum number of channels needed to render the ghost.
    int getMinimumChannels() const { return m_minChannels; }

    /// Returns the optimal number of channels needed to render the ghost.
    int getOptimalAhannels() const { return m_optimalChannels; }
    
    /// Returns the minimum number of rays needed to render the ghost.
    int getMinimumRays() const { return m_minRays; }

    /// Returns the optimal number of rays needed to render the ghost.
    int getOptimalRays() const { return m_optimalRays; }

    /// Sets the pupil bounding quad of the gost.
    void setPupilBounds(BoundingRect value) { m_pupilBounds = value; }
    
    /// Sets the sensor bounding quad of the ghost.
    void setSensorBounds(BoundingRect value) { m_sensorBounds = value; }
    
    /// Sets the minimum number of channels needed to render the ghost.
    void setMinimumChannels(int value) { m_minChannels = value; }

    /// Sets the optimal number of channels needed to render the ghost.
    void setOptimalAhannels(int value) { m_optimalChannels = value; }
    
    /// Sets the minimum number of rays needed to render the ghost.
    void setMinimumRays(int value) { m_minRays = value; }

    /// Sets the optimal number of rays needed to render the ghost.
    void setOptimalRays(int value) { m_optimalRays = value; }

private:
    /// Bounds of the ghost on the pupil (a.k.a. a quad from where rays
    /// originating will actually reach the sensor).
    BoundingRect m_pupilBounds;
    
    /// Bounds of the ghost on the sensor.
    BoundingRect m_sensorBounds;
    
    /// Minimum number of channels to render.
    int m_minChannels;

    /// Optimal number of channels to render.
    int m_optimalChannels;
    
    /// Minimum number of rays to render with.
    int m_minRays;

    /// Optimal number of rays to render with.
    int m_optimalRays;
};