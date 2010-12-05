
#if !defined(QTMONO_QOBJECTWRAPPER_H)
#define QTMONO_QOBJECTWRAPPER_H

#include <QtCore/QObject>

namespace mono {
#include "mono/jit/jit.h"
#include "mono/metadata/object.h"
#include "mono/metadata/assembly.h"
#include "mono/utils/mono-publib.h"
#include "mono/metadata/metadata.h"
#include "mono/metadata/debug-helpers.h"
}

class QObjectMonoWrapperFactory {    
public:
    /**
    Creates a QObjectWrapper factory.
    @param domain The mono domain to use for creating new mono objects.
     */
    QObjectMonoWrapperFactory(mono::MonoDomain *domain);
    ~QObjectMonoWrapperFactory();

    const QString &error() const;

    mono::MonoObject *create(QObject *qObject);
private:
    class Data;
    Data *d;
};

#endif // QTMONO_QOBJECTWRAPPER_H
