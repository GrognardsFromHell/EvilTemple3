#ifndef TROIKAFORMATSGLOBAL_H
#define TROIKAFORMATSGLOBAL_H

#include <QtCore/QtGlobal>

#if defined(TROIKAFORMATS_LIBRARY)
#  define TROIKAFORMATS_EXPORT Q_DECL_EXPORT
#else
#  define TROIKAFORMATS_EXPORT Q_DECL_IMPORT
#endif

#endif // TROIKAFORMATSGLOBAL_H