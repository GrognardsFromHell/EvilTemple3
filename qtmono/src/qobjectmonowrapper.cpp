
#include <QObject>
#include <QPointer>
#include <QMetaObject>
#include <QMetaMethod>
#include <QVector>

#include "qobjectmonowrapper.h"
#include <cstring>

class QObjectMonoWrapperFactory::Data {
public:
    Data() : initialized(false), domain(NULL), wrapperClass(NULL) {}

    bool initialize();

    bool initialized;
    mono::MonoDomain *domain;
    mono::MonoClass *wrapperClass;
    mono::MonoMethod *wrapperClassCtor;
    QString error;
};

QObjectMonoWrapperFactory::QObjectMonoWrapperFactory(mono::MonoDomain *domain)
    : d(new Data) {
    d->domain = domain;
}

QObjectMonoWrapperFactory::~QObjectMonoWrapperFactory()
{
    delete d;
}

const QString &QObjectMonoWrapperFactory::error() const
{
    return d->error;
}

mono::MonoObject *QObjectMonoWrapperFactory::create(QObject *obj)
{
    // Lazily initialize the managed part of QtMono
    if (!d->initialized && !d->initialize())
        return NULL;

    QPointer<QObject> *handle = new QPointer<QObject>(obj);

    mono::MonoObject *result = mono::mono_object_new(mono::mono_domain_get(), d->wrapperClass);

    qDebug("Initializing wrapper class for QObject %x", handle);

    void *params[1] = {&handle};
    mono::MonoObject *exc = NULL;
    mono::mono_runtime_invoke(d->wrapperClassCtor, result, params, &exc);

    // TODO: Handle exception

    return result;
}

inline bool isSameName(const char *signature, const char *name, int nameLength)
{
    int lengthOfName = std::strchr(signature, '(') - signature;

    if (lengthOfName != nameLength)
        return false;

    return memcmp(signature, name, nameLength) == 0;
}

/**
  Checks whether the two methods have compatible signatures.
  @param expected This is the signature that is expected by the caller.
  @param actual This is the actual signature.
 */
inline bool hasCompatibleSignature(const QMetaMethod &expected, const QList<mono::MonoType> &monoTypes)
{
    if (expected.parameterTypes().isEmpty())
        return true;

    if (expected.parameterTypes().size() != monoTypes.size())
        return false;

}

QVector<mono::MonoType*> getMonoTypesFromMethod(mono::MonoMethod *method)
{
    auto signature = mono::mono_method_signature(method);
    Q_ASSERT(signature);

    auto argCount = mono::mono_signature_get_param_count(signature);

    QVector<mono::MonoType*> types(argCount);

    void *iter;

    auto argType = mono::mono_signature_get_params(signature, &iter);    

    while (argType) {
        types.append(argType);
        argType = mono::mono_signature_get_params(signature, &iter);
    }
    
    return types;
}

QVector<mono::MonoType*> getMonoTypesFromArguments(mono::MonoArray *arguments)
{
    auto length = mono::mono_array_length(arguments);

    QVector<mono::MonoType*> types(length);

    for (int i = 0; i < length; ++i) {
        auto element = mono_array_get(arguments, mono::MonoObject*, i);
        auto objClass = mono::mono_object_get_class(element);
        types[i] = mono::mono_class_get_type(objClass);
    }

    return types;
}

static bool __stdcall invokeMember(QPointer<QObject> *handle, 
    mono::MonoString *name, mono::MonoArray *args, mono::MonoObject **result)
{
    auto success = false;
    auto calledName = mono::mono_string_to_utf8(name);
    auto calledNameLength = mono::mono_string_length(name);
        
    auto obj = handle->data();

    QVector<mono::MonoType*> types = getMonoTypesFromArguments(args);

    if (!obj) {
        qWarning("Invoking %s on NULL Pointer for handle %x", calledName, handle);
    } else {             
        const QMetaObject *metaObj = obj->metaObject();

        for (int i = 0; i < metaObj->methodCount(); ++i) {
            QMetaMethod method = metaObj->method(i);
                        
            if (!isSameName(method.signature(), calledName, calledNameLength))
                continue;

            // Check that it's a compatible overload
            // DUMP typenames to console            
            foreach (auto type, types) {
                auto name = mono::mono_type_get_name(type);
                auto typeId = mono::mono_type_get_type(type);

                mono::MonoClass *cl;

                switch (typeId) {
                case mono::MONO_TYPE_VOID:
                    qDebug("Void");
                    break;
                case mono::MONO_TYPE_BOOLEAN:
                    qDebug("Boolean");
                    break;
                case mono::MONO_TYPE_CHAR:
                    qDebug("Char");
                    break;
                case mono::MONO_TYPE_I1:
                    qDebug("int8");
                    break;
                case mono::MONO_TYPE_U1:
                    qDebug("uint8");
                    break;
                case mono::MONO_TYPE_I2:
                    qDebug("int16");
                    break;
                case mono::MONO_TYPE_U2:
                    qDebug("uint16");
                    break;
                case mono::MONO_TYPE_I4:
                    qDebug("int32");
                    break;
                case mono::MONO_TYPE_U4:
                    qDebug("uint32");
                    break;
                case mono::MONO_TYPE_I8:
                    qDebug("int64");
                    break;
                case mono::MONO_TYPE_U8:
                    qDebug("uint64");
                    break;
                case mono::MONO_TYPE_R4:
                    qDebug("float");
                    break;
                case mono::MONO_TYPE_R8:
                    qDebug("double");
                    break;
                case mono::MONO_TYPE_STRING:
                    qDebug("string");
                    break;
                case mono::MONO_TYPE_PTR:
                    qDebug("pointer");
                    break;
                case mono::MONO_TYPE_BYREF:
                    qDebug("ref");
                    break;
                case mono::MONO_TYPE_VALUETYPE:
                    qDebug("valuetype");
                    break;
                case mono::MONO_TYPE_CLASS:
                    qDebug("class");
                    cl = mono::mono_type_get_class(type);
                    while (cl) {
                        qDebug("Classname: %s", mono::mono_class_get_name(cl));
                        cl = mono_class_get_parent(cl);
                    }
                    break;
                case mono::MONO_TYPE_VAR:
                    qDebug("var");
                    break;
                case mono::MONO_TYPE_ARRAY:
                    qDebug("array");
                    break;
                case mono::MONO_TYPE_GENERICINST:
                    qDebug("genericinst");
                    break;
                case mono::MONO_TYPE_TYPEDBYREF:
                    qDebug("type by ref");
                    break;
                case mono::MONO_TYPE_I:
                    qDebug("i");
                    break;
                case mono::MONO_TYPE_U:
                    qDebug("u");
                    break;
                case mono::MONO_TYPE_FNPTR:
                    qDebug("fn ptr");
                    break;
                case mono::MONO_TYPE_OBJECT:
                    qDebug("object");
                    break;
                case mono::MONO_TYPE_SZARRAY:
                    qDebug("sz array");
                    break;
                case mono::MONO_TYPE_MVAR:
                    qDebug("mvar");
                    break;
                case mono::MONO_TYPE_CMOD_REQD:
                    qDebug("cmod reqd");
                    break;
                case mono::MONO_TYPE_CMOD_OPT:
                    qDebug("cmod opt");
                    break;
                case mono::MONO_TYPE_INTERNAL:
                    qDebug("internal");
                    break;
                case mono::MONO_TYPE_ENUM:
                    qDebug("enum");
                    break;
                }
            }
        }
    }

    mono::mono_free(calledName);

    return success;
}

static void __stdcall freeHandle(QPointer<QObject> *handle)
{
    qDebug("Freeing QObjectWrapper handle %x", handle);
    delete handle;
}

bool QObjectMonoWrapperFactory::Data::initialize()
{
    error.clear();

    mono::MonoImageOpenStatus status;
    mono::MonoAssembly *assembly = mono::mono_assembly_open("Bootstrap.dll", &status);

    switch (status) {
    default:
    case mono::MONO_IMAGE_OK:
        break;

    case mono::MONO_IMAGE_ERROR_ERRNO:
        error = QString("MONO_IMAGE_ERROR_ERRNO: %1").arg(errno);
        return false;

    case mono::MONO_IMAGE_MISSING_ASSEMBLYREF:
        error = "MONO_IMAGE_MISSING_ASSEMBLYREF";
        return false;

    case mono::MONO_IMAGE_IMAGE_INVALID:
        error = "MONO_IMAGE_IMAGE_INVALID";
        return false;
    }

    mono::MonoImage *image = mono_assembly_get_image(assembly);

    wrapperClass = mono::mono_class_from_name(image, "Bootstrap", "QObjectWrapper");

    if (!wrapperClass) {
        error = "Unable to find Bootstrap.QObjectWrapper";
        return false;
    }

    // Find Ctor in class
    wrapperClassCtor = mono::mono_class_get_method_from_name(wrapperClass, ".ctor", 1);

    if (!wrapperClassCtor) {
        error = "Unable to find Bootstrap.QObjectWrapper constructor.";
        return false;
    }
    
    mono::mono_add_internal_call("Bootstrap.QObjectWrapper::FreeHandle", freeHandle);
    mono::mono_add_internal_call("Bootstrap.QObjectWrapper::InvokeMember", invokeMember);

    initialized = true;    
    return initialized;
}
