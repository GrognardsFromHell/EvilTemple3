
#ifndef QTMONO_QMONOARGUMENTCONVERTER_H
#define QTMONO_QMONOARGUMENTCONVERTER_H

#include <assert.h>

#include <QtGlobal>
#include <QVarLengthArray>
#include <QMetaType>

#include "monopp.h"

class QMonoArgumentConverter {
public:
    /**
       The assumed number of arguments for an average call.
       If the actual converter is given a number below this constant, no heap allocations will occur for the
       argument arrays.
      */
    static const int AverageArgCount = 10;

    static_assert(sizeof(qint64) >= sizeof(void*), "The pointer size must not exceed 64-bit.");

    /**
      Creates an argument converter using the given Mono domain to create mono objects.
      */
    QMonoArgumentConverter(int argCount, mono::MonoDomain *domain)
        : mDomain(domain) {
        Q_ASSERT(domain != NULL);
        mValueArguments.reserve(argCount);
        mArguments.reserve(argCount);
    }

    /**
      Will retrieve the active Mono domain from the environment. This may be a little slower
      compared to setting it directly.
      */
    QMonoArgumentConverter(int argCount)
        : mDomain(mono::mono_domain_get()) {
        Q_ASSERT(mDomain != NULL);
        mValueArguments.reserve(argCount);
        mArguments.reserve(argCount);
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

    bool add(const void *arg, const QByteArray &sourceType) {
        int sourceTypeId = QMetaType::type(sourceType);

        /*Void = 0, Bool = 1, Int = 2, UInt = 3, LongLong = 4, ULongLong = 5,
            Double = 6, QChar = 7, QVariantMap = 8, QVariantList = 9,
            QString = 10, QStringList = 11, QByteArray = 12,
            QBitArray = 13, QDate = 14, QTime = 15, QDateTime = 16, QUrl = 17,
            QLocale = 18, QRect = 19, QRectF = 20, QSize = 21, QSizeF = 22,
            QLine = 23, QLineF = 24, QPoint = 25, QPointF = 26, QRegExp = 27,
            QVariantHash = 28, QEasingCurve = 29, LastCoreType = QEasingCurve,*/

        void *actualArg = NULL;

        switch (sourceTypeId) {
        case QMetaType::Bool:
            actualArg = appendValueArg<bool>(arg);
            break;
        case QMetaType::UChar:
            actualArg = appendValueArg<uchar>(arg);
            break;
        case QMetaType::Char:
            actualArg = appendValueArg<char>(arg);
            break;
        case QMetaType::UShort:
            actualArg = appendValueArg<ushort>(arg);
            break;
        case QMetaType::Short:
            actualArg = appendValueArg<short>(arg);
            break;
        case QMetaType::Int:
            actualArg = appendValueArg<int>(arg);
            break;
        case QMetaType::UInt:
            actualArg = appendValueArg<uint>(arg);
            break;
        case QMetaType::ULongLong:
            actualArg = appendValueArg<quint64>(arg);
            break;
        case QMetaType::LongLong:
            actualArg = appendValueArg<qint64>(arg);
            break;
        case QMetaType::Float:
            actualArg = appendValueArg<float>(arg);
            break;
        case QMetaType::Double:
            actualArg = appendValueArg<double>(arg);
            break;
        case QMetaType::QChar:
            actualArg = appendValueArg<QChar>(arg);
            break;
        case QMetaType::QString:
            {
                auto qstr = reinterpret_cast<const QString*>(arg);
                actualArg = mono::mono_string_new_utf16(mono::mono_domain_get(), qstr->utf16(), qstr->length());
            }
            break;

        default:
            qWarning("Attempting to convert unsupported QtMetaType (Name: %s, Id: %d) to a mono type",
                sourceType.constData(), sourceTypeId);
            return false;
        }

        mArguments.append(actualArg);
        return true;
    }

    // Convert to C++ types.
    template<typename T>
    T *appendValueArg(const void *arg) {
        static_assert(sizeof(T) <= sizeof(qint64), 
            "The type you are trying to convert doesn't fit into the value argument array.");
        
        mValueArguments.resize(mValueArguments.size() + 1);

        T* result = reinterpret_cast<T*>(mValueArguments.data() + (mValueArguments.size() - 1));

        *result = *reinterpret_cast<const T*>(arg);

        return result;
    }

    void **args() {
        return mArguments.data();
    }

    /**
    Checks whether the given Mono type can be assigned from the given Qt Meta Type (the internal id obtained from
    QMetaType::type
    */
    static bool isAssignableFrom(monopp::MonoType targetType, int sourceType);

private:
    mono::MonoDomain *mDomain;

    QVarLengthArray<qint64, AverageArgCount> mValueArguments;
    QVarLengthArray<void*, AverageArgCount> mArguments;

    Q_DISABLE_COPY(QMonoArgumentConverter)
};

#endif // QTMONO_QMONOARGUMENTCONVERTER_H
