
#ifndef QTMONO_QMONOARGUMENTCONVERTER_H
#define QTMONO_QMONOARGUMENTCONVERTER_H

#include <assert.h>

#include <QtGlobal>
#include <QVarLengthArray>

namespace mono {
#include "mono/jit/jit.h"
#include "mono/metadata/object.h"
#include "mono/utils/mono-publib.h"
}

template<int Args>
class QMonoArgumentConverter {
public:
    static_assert(Args > 0, "Args needs to be at least 1");
    static_assert(sizeof(qint64) >= sizeof(void*), "The pointer size must not exceed 64-bit.");

    QMonoArgumentConverter(mono::MonoDomain *domain) : mDomain(domain) {
        Q_ASSERT(domain != NULL);
    }

    ~QMonoArgumentConverter() {
    }

    /*
      For value types.
     */
    template<typename T>
    void add(const T &value) {
        static_assert(sizeof(T) < sizeof(qint64), "The types added through this function must fit into a qint64.");
        qint64 tmp;
        *reinterpret_cast<T*>(&tmp) = value;
        mValueArguments.append(tmp);
    }

    template<>
    void add<QString>(const QString &arg) {
        // This will be garbage collected by the mono runtime
        mArguments.append(mono::mono_string_new_utf16(mDomain, arg.utf16(), arg.length()));
    }

    void **args() {
        return mArguments.data();
    }

private:
    mono::MonoDomain *mDomain;

    QVarLengthArray<qint64, Args> mValueArguments;
    QVarLengthArray<void*, Args> mArguments;

    Q_DISABLE_COPY(QMonoArgumentConverter)
};

#endif // QTMONO_QMONOARGUMENTCONVERTER_H
