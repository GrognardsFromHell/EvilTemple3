
#include <QMetaType>

#include "qmonoargumentconverter.h"

inline bool isArrayOfInternalType(monopp::MonoType monoType, mono::MonoTypeEnum desiredElementType) {
    auto internalType = monoType.internalType();

    if (internalType != mono::MONO_TYPE_ARRAY)
        return false;

    auto arrayType = monoType.arrayElementType();

    monopp::MonoType elementType = mono::mono_class_get_type(arrayType->eklass);

    return elementType.internalType() == desiredElementType;
}

bool QMonoArgumentConverter::isAssignableFrom(monopp::MonoType monoType, int qtMetaType)
{
    auto monoTypeInternal = monoType.internalType();

    switch (qtMetaType) {    
    case QMetaType::Bool:
        return monoTypeInternal == mono::MONO_TYPE_BOOLEAN;

    case QMetaType::Int:
        return monoTypeInternal == mono::MONO_TYPE_I4;

    case QMetaType::UInt:
        return monoTypeInternal == mono::MONO_TYPE_U4;

    case QMetaType::LongLong:
        return monoTypeInternal == mono::MONO_TYPE_I8;

    case QMetaType::Double:
        return monoTypeInternal == mono::MONO_TYPE_R8;

    case QMetaType::QChar:
        return monoTypeInternal == mono::MONO_TYPE_CHAR;

    case QMetaType::QString:
        return monoTypeInternal == mono::MONO_TYPE_STRING;

    case QMetaType::QStringList:
        return isArrayOfInternalType(monoType, mono::MONO_TYPE_STRING);

    case QMetaType::QByteArray:
        return isArrayOfInternalType(monoType, mono::MONO_TYPE_U1);

    case QMetaType::QBitArray:
        return isArrayOfInternalType(monoType, mono::MONO_TYPE_BOOLEAN);
    }

    return false;
}

