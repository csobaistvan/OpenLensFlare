#pragma once

#include "Dependencies.h"

namespace OLEF
{

/// Represents a ghost.
class Ghost
{
public:
    /// Constructs an empty ghost.
    Ghost():
        m_length(0)
    {}

    /// Constructs a ghost with the parameter vector.
    Ghost(const std::vector<int>& interfaces):
        m_length(interfaces.size())
    {
        std::copy(interfaces.begin(), interfaces.end(), m_interfaces.begin());
    }

    /// Constructs a ghost with the parameter initializer list.
    Ghost(const std::initializer_list<int>& interfaces):
        m_length(interfaces.size())
    {
        std::copy(interfaces.begin(), interfaces.end(), m_interfaces.begin());
    }

    /// Returns the length of the ghost.
    int getLength() const { return m_length; }

    /// Sets the length of the ghost.
    void setLength(int value) { m_length = value;}

    /// Accesses the ith interface.
    int& operator[](size_t i) { return m_interfaces[i]; }
    
    /// Accesses the ith interface.
    int operator[](size_t i) const { return m_interfaces[i]; }

private:
    /// Length of the sequence.
    int m_length;

    /// The interfaces that the ghost is reflected by.
    std::array<int, 8> m_interfaces;
};

}