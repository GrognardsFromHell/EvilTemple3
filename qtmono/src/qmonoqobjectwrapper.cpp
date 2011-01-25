
#include <QPointer>
#include <QMetaType>
#include <QMetaMethod>
#include <QColor>

#include "qmonoconnectionmanager.h"
#include "qmonoqobjectwrapper.h"

static QMonoQObjectWrapper *instance = NULL;

QMonoQObjectWrapper *QMonoQObjectWrapper::getInstance()
{
    if (!instance)
        instance = new QMonoQObjectWrapper;

    return instance;
}

class QMonoQObjectWrapper::Data {
public:
    Data() 
    : initialized(false), 
    monoDomain(NULL), 
    managedWrapperClass(NULL), 
    managedWrapperClassCtor(NULL), 
    connectionManager(NULL),    
    managedEventHelperClass(NULL),
    managedEventHelperClassCtor(NULL) {}

    bool initialized;
    mono::MonoDomain *monoDomain;
    mono::MonoClass *managedWrapperClass;
    mono::MonoMethod *managedWrapperClassCtor;
    mono::MonoClass *managedEventHelperClass;
    mono::MonoMethod *managedEventHelperClassCtor;
    mono::MonoMethod *colorCtor;
    mono::MonoClass *colorClass;

    mono::MonoClass *objectDictClass;
    mono::MonoMethod *objectDictCtor;
    mono::MonoMethod *objectDictAdd;

    mono::MonoClass *objectListClass;
    mono::MonoMethod *objectListCtor;
    mono::MonoMethod *objectListAdd;

    void findGenericClasses(monopp::MonoImage monoImage);

    QMonoConnectionManager *connectionManager;
    QString error;        
};

void QMonoQObjectWrapper::Data::findGenericClasses(monopp::MonoImage monoImage)
{
    auto genericHelperClass = monoImage.findClass("Bootstrap", "GenericHelper");

    Q_ASSERT(genericHelperClass);

    auto objectListField = mono::mono_class_get_field_from_name(genericHelperClass, "objectList");
    objectListClass = mono::mono_class_from_mono_type (mono::mono_field_get_type(objectListField));
    objectListCtor = mono::mono_class_get_method_from_name(objectListClass, ".ctor", 0);
    objectListAdd = mono::mono_class_get_method_from_name(objectListClass, "Add", 1);

    auto objectDictField = mono::mono_class_get_field_from_name(genericHelperClass, "objectDict");
    objectDictClass = mono::mono_class_from_mono_type (mono::mono_field_get_type(objectDictField));
    objectDictCtor = mono::mono_class_get_method_from_name(objectDictClass, ".ctor", 0);
    objectDictAdd = mono::mono_class_get_method_from_name(objectDictClass, "Add", 2);
}

QMonoQObjectWrapper::QMonoQObjectWrapper() : d(new Data)
{
}

QMonoQObjectWrapper::~QMonoQObjectWrapper()
{
    delete d;
}

const QString &QMonoQObjectWrapper::error() const {
    return d->error;
}

bool QMonoQObjectWrapper::isInitialized() const {
    return d->initialized;
}

/**
  Retrieve a numeric value from a mono-object. This is used to auto-convert different number-types to a target type.
  If a 32-bit integer is required, a 16-bit integer can still be used.
*/
template<typename T>
T numberFromMonoObject(mono::MonoObject *object, bool *ok = NULL) {
    mono::MonoClass *klass = mono::mono_object_get_class(object);

    if (ok)
        *ok = true;

    if (objectClass == mono::mono_get_int32_class()) {
        return monopp::fromMono<qint32>(object);
    } else if (objectClass == mono::mono_get_uint32_class()) {
        return monopp::fromMono<quint32>(object);
    } else if (objectClass == mono::mono_get_int16_class()) {
        return monopp::fromMono<qint16>(object);
    } else if (objectClass == mono::mono_get_uint16_class()) {
        return monopp::fromMono<quint16>(object);
    } else if (objectClass == mono::mono_get_sbyte_class()) {
        return monopp::fromMono<qint8>(object);
    } else if (objectClass == mono::mono_get_byte_class()) {
        return monopp::fromMono<quint8>(object);
    } else if (objectClass == mono::mono_get_boolean_class()) {
        return monopp::fromMono<bool>(object);
    } else if (objectClass == mono::mono_get_double_class()) {
        return monopp::fromMono<double>(object);
    } else if (objectClass == mono::mono_get_single_class()) {
        return monopp::fromMono<float>(object);
    } else {
        if (ok)
            *ok = false;
        return 0;
    }
}

bool __stdcall QMonoQObjectWrapper::InvokeMember(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoArray *args, mono::MonoObject **result)
{
    bool success = false;
    char *nameUtf8 = mono::mono_string_to_utf8(name);
    int nameUtf8Length = strlen(nameUtf8);

    QObject *obj = handle->data();

    auto wrapper = getInstance();

    if (!obj) {
        qWarning("Invoking %s on NULL Pointer for handle %x", nameUtf8, handle);
    } else {             
        const QMetaObject *metaObj = obj->metaObject();

        for (int i = 0; i < metaObj->methodCount(); ++i) {
            QMetaMethod method = metaObj->method(i);

            if (method.parameterTypes().size() != mono::mono_array_length(args))
                continue;

            const char *startOfParamList = strchr(method.signature(), '(');
            if (!startOfParamList) {
                qWarning("Cannot find parameter list in method signature: %s", method.signature());
                continue;
            }

            int nameLength = startOfParamList - method.signature();

            if (nameUtf8Length != nameLength)
                continue;

            if (memcmp(method.signature(), nameUtf8, nameLength))
                continue;

            if (method.parameterTypes().size() > 10) {
                qWarning("Methods with more than 10 parameters cannot be called.");
                continue;
            }

            if (method.methodType() == QMetaMethod::Slot) {
                
                if (method.typeName()[0]) {
                    // Method has a return type
                    int returnTypeId = QMetaType::type(method.typeName());
                    void *returnArgValue = QMetaType::construct(returnTypeId);

                    QGenericReturnArgument returnArg(method.typeName(), returnArgValue);
                    QGenericArgument genericArgs[10];
                    
                    for (int i = 0; i < method.parameterTypes().size(); ++i) {
                        auto monoArg = mono_array_get(args, mono::MonoObject*, i);

                        // Pull all necessary arguments from the given argument list and try to convert them if possible
                        auto parameterType = method.parameterTypes()[i];
                        auto parameterTypeId = QMetaType::type(parameterType);

                        void *data = NULL;

                        switch (parameterTypeId) {
                        case QMetaType::QVariant:
                            data = new QVariant(wrapper->convertObjectToVariant(monoArg));
                            break;
                        // TODO: Add other parameter-types that should be supported here.
                        }
                        
                        if (data) {
                            genericArgs[i] = QGenericArgument(parameterType, data);
                        }
                    }

                    method.invoke(obj, returnArg, genericArgs[0], genericArgs[1], genericArgs[2], genericArgs[3],
                        genericArgs[4], genericArgs[5], genericArgs[6], genericArgs[7], genericArgs[8], genericArgs[9]);

                    // Clean up the argument array and free previously allocated memory
                    for (int i = 0; i < method.parameterTypes().size(); ++i) {
                        if (genericArgs[i].data()) {
                            QMetaType::destroy(QMetaType::type(genericArgs[i].name()), genericArgs[i].data());
                        }
                    }

                    *result = NULL;

                    // Convert the return-type back.
                    switch (returnTypeId) {
                    case QMetaType::QVariant:
                        *result = wrapper->convertVariantToObject(*reinterpret_cast<QVariant*>(returnArgValue));
                        // TODO: Add other return-types that should be supported here.
                    }

                    QMetaType::destroy(returnTypeId, returnArgValue);                    
                } else {
                    method.invoke(obj);
                    *result = NULL;
                }
                
                success = true;
                break;
            }

            break;
        }
    }

    mono::mono_free(nameUtf8);

    return success;
}

template<typename T>
inline mono::MonoObject *boxValue(T value, mono::MonoDomain *domain, mono::MonoClass *klass)
{
    return mono::mono_value_box(domain, klass, &value);
}

mono::MonoObject *QMonoQObjectWrapper::convertVariantToObject(const QVariant &variant)
{
    switch (variant.type()) {
    case QVariant::Bool:
        return boxValue<bool>(variant.toBool(), d->monoDomain, mono::mono_get_boolean_class());
    case QVariant::Int:
        return boxValue<qint32>(variant.toInt(), d->monoDomain, mono::mono_get_int32_class());
    case QVariant::UInt:
        return boxValue<quint32>(variant.toUInt(), d->monoDomain, mono::mono_get_uint32_class());
    case QVariant::LongLong:
        return boxValue<qint64>(variant.toLongLong(), d->monoDomain, mono::mono_get_int64_class());
    case QVariant::ULongLong:
        return boxValue<quint64>(variant.toULongLong(), d->monoDomain, mono::mono_get_uint64_class());
    case QVariant::Double:
        return boxValue<double>(variant.toDouble(), d->monoDomain, mono::mono_get_double_class());
    case QVariant::Char:
        return boxValue<QChar>(variant.toChar(), d->monoDomain, mono::mono_get_char_class());
    case QVariant::String:
        // MonoString's are MonoObjects
        return (mono::MonoObject*)monopp::toMonoString(variant.toString());
    case QVariant::Color:
        {
            if (!d->colorCtor) {
                qWarning("System.Drawing.Color constructor wasn't found. Color conversion not possible.");
                break;
            }

            QColor color = variant.value<QColor>();
            int paramValues[4] = {color.alpha(), color.red(), color.green(), color.blue()};
            void* params[4] = {&paramValues[0], &paramValues[1], &paramValues[2], &paramValues[3]};

            mono::MonoObject *exc = NULL;
            mono::MonoObject *result = mono::mono_runtime_invoke(d->colorCtor, NULL, params, &exc);

            if (exc) {
                qWarning("Error while creating System.Drawing.Color object from QColor.");
                monopp::handleMonoException(exc);
                return NULL;
            }

            return result;
        }
        break;
    case QVariant::List:
        {
            QList<QVariant> variantList = variant.toList();
            auto *resultList = mono::mono_object_new(d->monoDomain, d->objectListClass);
            mono::mono_runtime_invoke(d->objectListCtor, resultList, NULL, NULL);

            foreach (QVariant variantListEl, variantList) {
                auto resultListEl = convertVariantToObject(variantListEl);
                void *params[1] = {resultListEl};
                mono::mono_runtime_invoke(d->objectListAdd, resultList, params, NULL);
            }

            return resultList;
        }
        break;
    }

    qWarning("Unable to convert variant type %s to a mono type.", variant.typeName());

    return NULL;
}

bool __stdcall QMonoQObjectWrapper::GetProperty(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoObject **result)
{
    char *nameUtf8 = mono::mono_string_to_utf8(name);
    int nameUtf8Length = strlen(nameUtf8);

    if (handle->isNull()) {
        qDebug("Trying to retrieve property %s of already freed object %x", nameUtf8, handle);
        mono::mono_free(nameUtf8);
        return false;
    }

    QObject *obj = handle->data();

    QVariant value = obj->property(nameUtf8);

    if (!value.isValid()) {
        auto metaObj = obj->metaObject();

        // Search for a signal with the same name. Could be a connection attempt, but we don't know.
        for (int i = 0; i < metaObj->methodCount(); ++i) {
            auto method = metaObj->method(i);

            const char *startOfParamList = strchr(method.signature(), '(');
            if (!startOfParamList) {
                qWarning("Cannot find parameter list in method signature: %s", method.signature());
                continue;
            }

            int nameLength = startOfParamList - method.signature();

            if (nameUtf8Length != nameLength)
                continue;

            if (memcmp(method.signature(), nameUtf8, nameLength))
                continue;

            if (method.methodType() == QMetaMethod::Signal) {
                // return an event-subscription handler for this wrapper & signal
                Data *d = QMonoQObjectWrapper::getInstance()->d;

                mono::MonoObject *eventHelper = mono::mono_object_new(d->monoDomain, d->managedEventHelperClass);

                qDebug("Initializing event helper class for QObject %x", handle);

                void *params[2] = {&handle, name};
                mono::MonoObject *exc = NULL;
                mono::mono_runtime_invoke(d->managedEventHelperClassCtor, eventHelper, params, &exc);

                *result = eventHelper;

                mono::mono_free(nameUtf8);
                return true;
            }
        }

        mono::mono_free(nameUtf8);
        return false;
    }

    // Convert variant to MonoObject
    *result = QMonoQObjectWrapper::getInstance()->convertVariantToObject(value);

    return *result != NULL;
}

QVariant QMonoQObjectWrapper::convertObjectToVariant(mono::MonoObject *object)
{
    mono::MonoClass *objectClass = mono::mono_object_get_class(object);

    if (objectClass == mono::mono_get_int32_class()) {
        return monopp::fromMono<qint32>(object);
    } else if (objectClass == mono::mono_get_uint32_class()) {
        return monopp::fromMono<quint32>(object);
    } else if (objectClass == mono::mono_get_int16_class()) {
        return monopp::fromMono<qint16>(object);
    } else if (objectClass == mono::mono_get_uint16_class()) {
        return monopp::fromMono<quint16>(object);
    } else if (objectClass == mono::mono_get_sbyte_class()) {
        return monopp::fromMono<qint8>(object);
    } else if (objectClass == mono::mono_get_byte_class()) {
        return monopp::fromMono<quint8>(object);
    } else if (objectClass == mono::mono_get_string_class()) {
        return monopp::fromMonoString((mono::MonoString*)object);
    } else if (objectClass == mono::mono_get_boolean_class()) {
        return monopp::fromMono<bool>(object);
    } else if (objectClass == mono::mono_get_double_class()) {
        return monopp::fromMono<double>(object);
    } else if (objectClass == mono::mono_get_single_class()) {
        return monopp::fromMono<float>(object);
    } else if (objectClass == d->colorClass) {
        
        quint32 argb = *(quint32*)mono::mono_object_unbox(object);

        quint8 alpha = (argb >> 24) & 0xFF;
        quint8 red = (argb >> 16) & 0xFF;
        quint8 green = (argb >> 8) & 0xFF;
        quint8 blue = argb & 0xFF;
        
        return QColor(red, green, blue, alpha);
    }

    return QVariant();
}

bool __stdcall QMonoQObjectWrapper::SetProperty(QPointer<QObject> *handle, 
    mono::MonoString *name, mono::MonoObject *value)
{
    auto objClass = mono::mono_object_get_class(value);

    auto d = QMonoQObjectWrapper::getInstance()->d;

    if (objClass == d->managedEventHelperClass) {
        qDebug("Re-setting a previously issued event-helper class. Doing nothing");
        return true;
    }

    char *nameUtf8 = mono::mono_string_to_utf8(name);

    auto metaObj = handle->data()->metaObject();

    int id = metaObj->indexOfProperty(nameUtf8);

    if (id == -1) {
        mono::mono_free(nameUtf8);
        return false;
    }

    auto metaProp = metaObj->property(id);
    
    if (!metaProp.isWritable()) {
        qWarning("Attempting to write to a read-only property %s.", nameUtf8);
        mono::mono_free(nameUtf8);
        return false;
    }

    QVariant variantValue = QMonoQObjectWrapper::getInstance()->convertObjectToVariant(value);

    if (!variantValue.isValid()) {
        qWarning("Attempting to set incompatible value on property %s.", nameUtf8);
        mono::mono_free(nameUtf8);
        return false;
    }

    if (!metaProp.write(handle->data(), variantValue)) {
        qWarning("Failed to write value to property %s.", nameUtf8);
        mono::mono_free(nameUtf8);
        return false;
    } else {
        mono::mono_free(nameUtf8);
        return true;
    }
}

void __stdcall QMonoQObjectWrapper::FreeHandle(QPointer<QObject> *handle)
{
    qDebug("Freeing QObjectWrapper handle %x", handle);
    delete handle;
}

mono::MonoObject *QMonoQObjectWrapper::create(QObject *obj)
{
    if (!d->initialized) {
        qWarning("Attempting to create a QMonoObjectWrapper without first initializing the class.");
        return NULL;
    }

    QPointer<QObject> *handle = new QPointer<QObject>(obj);

    mono::MonoObject *result = mono::mono_object_new(d->monoDomain, d->managedWrapperClass);

    qDebug("Initializing wrapper class for QObject %x", handle);

    void *params[1] = {&handle};
    mono::MonoObject *exc = NULL;
    mono::mono_runtime_invoke(d->managedWrapperClassCtor, result, params, &exc);

    return result;
}

bool QMonoQObjectWrapper::initialize(QMonoConnectionManager *connectionManager, monopp::MonoDomain &domain) {
    if (d->initialized) {
        d->error = "Already initialized";
        return false;
    }

    d->error.clear();
        
    mono::MonoImageOpenStatus status;
    mono::MonoAssembly *assembly = mono::mono_assembly_open("Bootstrap.dll", &status);

    switch (status) {
    default:
    case mono::MONO_IMAGE_OK:
        break;

    case mono::MONO_IMAGE_ERROR_ERRNO:
        d->error = QString("MONO_IMAGE_ERROR_ERRNO: %1").arg(errno);
        return false;

    case mono::MONO_IMAGE_MISSING_ASSEMBLYREF:
        d->error = "MONO_IMAGE_MISSING_ASSEMBLYREF";
        return false;

    case mono::MONO_IMAGE_IMAGE_INVALID:
        d->error = "MONO_IMAGE_IMAGE_INVALID";
        return false;
    }

    mono::MonoImage *image = mono_assembly_get_image(assembly);

    auto managedWrapperClass = mono::mono_class_from_name(image, "Bootstrap", "QObjectWrapper");

    if (!managedWrapperClass) {
        d->error = "Unable to find Bootstrap.QObjectWrapper";
        return false;
    }

    // Find Ctor in class
    auto managedWrapperClassCtor = mono::mono_class_get_method_from_name(managedWrapperClass, ".ctor", 1);

    if (!managedWrapperClassCtor) {
        d->error = "Unable to find Bootstrap.QObjectWrapper constructor.";
        return false;
    }

    auto managedEventHelperClass = mono::mono_class_from_name(image, "Bootstrap", "EventSubscriptionHelper");

    if (!managedEventHelperClass) {
        d->error = "Unable to find Bootstrap.EventSubscriptionHelper";
        return false;
    }

    auto managedEventHelperClassCtor = mono::mono_class_get_method_from_name(managedEventHelperClass, ".ctor", 2);

    if (!managedEventHelperClassCtor) {
        d->error = "Unable to find Bootstrap.EventSubscriptionHelper constructor";
        return false;
    }

    monopp::MonoImage systemDrawingImage = mono::mono_image_loaded("System.Drawing");

    if (!systemDrawingImage) {
        auto systemDrawingAssemblyName = mono::mono_assembly_name_new("System.Drawing, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a");

        mono::MonoImageOpenStatus status;
        auto systemDrawingAssembly = mono::mono_assembly_load(systemDrawingAssemblyName, NULL, &status);

        if (!systemDrawingAssembly) {
            qWarning("Unable to load System.Drawing assembly. Several classes will not be convertible.");
            return NULL;
        }

        systemDrawingImage = mono::mono_assembly_get_image(systemDrawingAssembly);
    }

    // Find the method directory
    d->colorClass = systemDrawingImage.findClass("System.Drawing", "Color");

    if (!d->colorClass) {
        qWarning("Unable to find System.Drawing.Color, setting QColor properties will not be possible.");
    }
    
    monopp::MonoMethodDesc methodDesc("System.Drawing.Color:FromArgb(int,int,int,int)");
    d->colorCtor = systemDrawingImage.findMethod(methodDesc);
    d->managedWrapperClass = managedWrapperClass;
    d->managedWrapperClassCtor = managedWrapperClassCtor;
    d->managedEventHelperClass = managedEventHelperClass;
    d->managedEventHelperClassCtor = managedEventHelperClassCtor;
    d->connectionManager = connectionManager;
    d->monoDomain = domain;

    mono::mono_add_internal_call("Bootstrap.QObjectWrapper::FreeHandle", FreeHandle);
    mono::mono_add_internal_call("Bootstrap.QObjectWrapper::InvokeMember", InvokeMember);
    mono::mono_add_internal_call("Bootstrap.QObjectWrapper::GetProperty", GetProperty);
    mono::mono_add_internal_call("Bootstrap.QObjectWrapper::SetProperty", SetProperty);
    mono::mono_add_internal_call("Bootstrap.EventSubscriptionHelper::ConnectToSignal", ConnectToSignal);
    mono::mono_add_internal_call("Bootstrap.EventSubscriptionHelper::DisconnectFromSignal", DisconnectFromSignal);

    d->findGenericClasses(image);

    d->initialized = true;    
    return true;
}

void __stdcall QMonoQObjectWrapper::ConnectToSignal(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoObject *handler)
{
    using namespace mono;

    if (handle->isNull()) {
        qWarning("Trying to connect to a signal on an invalid object reference.");
        return;
    }

    QObject *obj = handle->data();

    const char *nameUtf8 = mono::mono_string_to_utf8(name);
    int nameUtf8Length = mono::mono_string_length(name);

    // Find matching signal
    const QMetaObject *metaObj = obj->metaObject();

    auto wrapper = QMonoQObjectWrapper::getInstance();

    for (int i = 0; i < metaObj->methodCount(); ++i) {
        QMetaMethod method = metaObj->method(i);

        const char *startOfParamList = strchr(method.signature(), '(');
        if (!startOfParamList) {
            qWarning("Cannot find parameter list in method signature: %s", method.signature());
            continue;
        }

        int nameLength = startOfParamList - method.signature();

        if (nameUtf8Length != nameLength)
            continue;

        if (memcmp(method.signature(), nameUtf8, nameLength))
            continue;

        if (method.methodType() != QMetaMethod::Signal)
            continue;

        if (!wrapper->d->connectionManager->addSignalHandler(obj, i, handler, Qt::DirectConnection)) {
            continue;
        }
    }
}

void __stdcall QMonoQObjectWrapper::DisconnectFromSignal(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoObject *handler)
{

}
