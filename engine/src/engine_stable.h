
/*
  This is the precompiled header.
 */

#include <QtDeclarative>
#include <QtCore>
#include <QtGui>

// Target windows xp sp1
#define WINVER 0x0502

// Windows headers define min/max macros that kill <limits> support for min/max
#define NOMINMAX

#include <GL/glew.h>

#include <QtOpenGL>

#include <limits>
#include <cmath>
