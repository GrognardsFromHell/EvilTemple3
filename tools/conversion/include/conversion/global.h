#ifndef CONVERSION_GLOBAL_H
#define CONVERSION_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CONVERSION_LIBRARY)
#  define CONVERSIONSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CONVERSIONSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CONVERSION_GLOBAL_H
