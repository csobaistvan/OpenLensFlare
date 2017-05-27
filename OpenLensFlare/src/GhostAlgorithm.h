#pragma once

#include "Dependencies.h"
#include "OpticalSystem.h"
#include "Ghost.h"
#include "LightSource.h"

namespace OLEF
{

/// Represents a physically-based ghost rendering algorithm.
class GhostAlgorithm
{
public:
    /// Polymorphic destructor.
    virtual ~GhostAlgorithm() {}

    /// Renders the ghosts corresponding to the parameter light source.
    virtual void renderGhosts(const LightSource& light, const GhostList& ghosts) = 0;
};

}