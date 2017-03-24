#pragma once

// Standard headers
#include <utility>  // For std::move.
#include <cassert>  // For parameter validations.
#include <string>   // For string handling.
#include <iostream> // For serialization of certain objects
#include <array>    // For statically sized arrays.
#include <vector>   // For dynamic arrays.
#include <map>      // For mapping data to certain ghosts.

// GLM
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// GLEW
#define GLEW_STATIC
#include "glew.h"

// OpenLensFlare (included first, because of GLEW)
#include "OpenLensFlare/OpenLensFlare.h"

// Qt
#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>
#include <QOpenGLFunctions_3_3_Core>
#include <QDebug>