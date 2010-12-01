
#if !defined(ENGINE_GLOBAL_H)
#define ENGINE_GLOBAL_H

#include <QtCore/QtGlobal>

#if defined(ENGINE_LIBRARY)
#  define ENGINE_EXPORT Q_DECL_EXPORT
#else
#  define ENGINE_EXPORT Q_DECL_IMPORT
#endif // ENGINE_LIBRARY

#endif // ENGINE_GLOBAL_H
