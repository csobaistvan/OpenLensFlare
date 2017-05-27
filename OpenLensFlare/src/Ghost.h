#pragma once

#include "Dependencies.h"

namespace OLEF
{

/// Represents a ghost. Instances of this class also hold various information 
/// about the ghost, such as bounding data or rendering parameters.
class Ghost
{
public:
    /// Represents a 2D bounding rect, by storing the lower left corner and 
    /// the length of its sides.
    using BoundingRect = std::array<glm::vec2, 2>;

    /// Maximum number of interfaces.
    static const int MAX_INTERFACES = 16;

    /// Constructs an empty ghost.
    Ghost():
        Ghost({})
    {}

    /// Constructs a ghost with the parameter vector.
    Ghost(const std::vector<int>& interfaces):
        Ghost(interfaces.begin(), interfaces.end())
    {}

    /// Constructs a ghost with the parameter initializer list.
    Ghost(const std::initializer_list<int>& interfaces):
        Ghost(interfaces.begin(), interfaces.end())
    {}

    template<typename It>
    Ghost(It begin, It end):
        m_length(std::distance(begin, end)),
        m_pupilBounds{ glm::vec2(-1.0f), glm::vec2(2.0f) },
        m_sensorBounds{ glm::vec2(-1.0f), glm::vec2(2.0f) },
        m_avgIntensity(1.0f),
        m_minChannels(3),
        m_optimalChannels(3),
        m_minRays(32),
        m_optimalRays(32)
    {
        std::copy(begin, end, m_interfaces.begin());
    }

    /// Returns the length of the ghost.
    size_t getLength() const { return m_length; }

    /// Returns the tightest bounding quad on the pupil element from which rays 
    /// can hit the sensor, normalized to [-1, 1].
    BoundingRect getPupilBounds() const { return m_pupilBounds; }
    
    /// Returns the tightest bounding quad of the ghost image, on the sensor.
    /// This is normalized to [-1, 1].
    BoundingRect getSensorBounds() const { return m_sensorBounds; }

    /// Returns the average intensity of the ghost.
    float getAverageIntensity() const { return m_avgIntensity; }
    
    /// Returns the minimum number of channels needed to render the ghost.
    int getMinimumChannels() const { return m_minChannels; }

    /// Returns the optimal number of channels needed to render the ghost.
    int getOptimalChannels() const { return m_optimalChannels; }
    
    /// Returns the minimum number of rays needed to render the ghost.
    int getMinimumRays() const { return m_minRays; }

    /// Returns the optimal number of rays needed to render the ghost.
    int getOptimalRays() const { return m_optimalRays; }

    /// Sets the length of the ghost.
    void setLength(size_t value) { m_length = value;}

    /// Sets the pupil bounding quad of the gost.
    void setPupilBounds(BoundingRect value) { m_pupilBounds = value; }
    
    /// Sets the sensor bounding quad of the ghost.
    void setSensorBounds(BoundingRect value) { m_sensorBounds = value; }
    
    /// Sets the average intensity of the ghost.
    void setAverageIntensity(float value) { m_avgIntensity = value; }
    
    /// Sets the minimum number of channels needed to render the ghost.
    void setMinimumChannels(int value) { m_minChannels = value; }

    /// Sets the optimal number of channels needed to render the ghost.
    void setOptimalChannels(int value) { m_optimalChannels = value; }
    
    /// Sets the minimum number of rays needed to render the ghost.
    void setMinimumRays(int value) { m_minRays = value; }

    /// Sets the optimal number of rays needed to render the ghost.
    void setOptimalRays(int value) { m_optimalRays = value; }

    /// Accesses the ith interface.
    int& operator[](size_t i) { return m_interfaces[i]; }
    
    /// Accesses the ith interface.
    int operator[](size_t i) const { return m_interfaces[i]; }

    /// Standard iterators to the underlying data.
    using iterator = std::array<int, MAX_INTERFACES>::iterator;
    using const_iterator = std::array<int, MAX_INTERFACES>::const_iterator;
    using reverse_iterator = std::array<int, MAX_INTERFACES>::reverse_iterator;
    using const_reverse_iterator = std::array<int, MAX_INTERFACES>::const_reverse_iterator;

    iterator begin() { return m_interfaces.begin(); }

    const_iterator begin() const { return m_interfaces.begin(); }

    const_iterator cbegin() const { return m_interfaces.cbegin(); }

    iterator end() { return m_interfaces.begin() + m_length; }

    const_iterator end() const { return m_interfaces.begin() + m_length; }

    const_iterator cend() const { return m_interfaces.begin() + m_length; }

    reverse_iterator rbegin() { return m_interfaces.rbegin() + (MAX_INTERFACES - m_length); }

    const_reverse_iterator rbegin() const { return m_interfaces.rbegin() + (MAX_INTERFACES - m_length); }

    const_reverse_iterator crbegin() const { return m_interfaces.crbegin() + (MAX_INTERFACES - m_length); }

    reverse_iterator rend() { return m_interfaces.rend(); }

    const_reverse_iterator rend() const { return m_interfaces.rend(); }

    const_reverse_iterator crend() const { return m_interfaces.crend(); }

private:
    /// Length of the sequence.
    size_t m_length;

    /// The interfaces that the ghost is reflected by.
    std::array<int, MAX_INTERFACES> m_interfaces;

    /// Bounds of the ghost on the pupil (a.k.a. a quad from where rays
    /// originating will actually reach the sensor).
    BoundingRect m_pupilBounds;
    
    /// Bounds of the ghost on the sensor.
    BoundingRect m_sensorBounds;

    /// Average of the transported energy.
    float m_avgIntensity;
    
    /// Minimum number of channels to render.
    int m_minChannels;

    /// Optimal number of channels to render.
    int m_optimalChannels;
    
    /// Minimum number of rays to render with.
    int m_minRays;

    /// Optimal number of rays to render with.
    int m_optimalRays;
};

/// A ghost list is just a vector of ghosts.
using GhostList = std::vector<Ghost>;

}