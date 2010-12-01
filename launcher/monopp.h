
#ifndef MONOPP_H
#define MONOPP_H


namespace mono {
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
}

namespace monopp {

class MonoMethodDesc;

class MonoClass {
public:

    /**
    ECMA Specification Type Attributes (21.1.13).
    */
    enum TypeAttributes {
        VisibilityMask = 0x00000007,
        NotPublic = 0x00000000,
        Public = 0x00000001,
        NestedPublic = 0x00000002,
        NestedPrivate = 0x00000003,
        NestedFamily = 0x00000004,
        NestedAssembly = 0x00000005,
        NestedFamAndAssem = 0x00000006,
        NestedFamOrAssem = 0x00000007,

        LayoutMask = 0x00000018,
        AutoLayout = 0x00000000,
        SequentialLayout = 0x00000008,
        ExplicitLayout = 0x00000010,

        ClassSemanticMask = 0x00000020,
        Class = 0x00000000,
        Interface = 0x00000020,

        Abstract = 0x00000080,
        Sealed = 0x00000100,
        SpecialName = 0x00000400,

        Import = 0x00001000,
        Serializable = 0x00002000,

        StringFormatMask = 0x00030000,
        AnsiClass = 0x00000000,
        UnicodeClass = 0x00010000,
        AutoClass = 0x00020000,

        BeforeFieldInit = 0x00100000,
        Forwarder = 0x00200000,

        ReservedMask = 0x00040800,
        RtSpecialName = 0x00000800,
        HasSecurity = 0x00040000,
    };

    MonoClass(mono::MonoClass *klass) : mClass(klass) {}

    bool isValid() const {
        return mClass != NULL;
    }

    operator mono::MonoClass *() {
        return mClass;
    }

    int dataSize() {
        return mono::mono_class_data_size(mClass);
    }

    int arrayElementSize() {
        return mono::mono_class_array_element_size(mClass);
    }

    MonoClass elementClass() {
        return mono::mono_class_get_element_class(mClass);
    }

    TypeAttributes flags() {
        return (TypeAttributes)mono::mono_class_get_flags(mClass);
    }

    const char *name() {
        return mono::mono_class_get_name(mClass);
    }

    const char *nameSpace() {
        return mono::mono_class_get_namespace(mClass);
    }

    bool isAssignableFrom(mono::MonoClass *klass) {
        return mono::mono_class_is_assignable_from(mClass, klass);
    }

    mono::MonoMethod *findMethod(const char *name, int paramCount);

    mono::MonoMethod *findMethod(mono::MonoMethodDesc *desc);

private:
    mono::MonoClass *mClass;
};

class MonoImage {
public:
    MonoImage(mono::MonoImage *image) : mImage(image) {}

    bool isValid() const {
        return mImage != NULL;
    }

    operator mono::MonoImage *() {
        return mImage;
    }

    mono::MonoMethod *findMethod(MonoMethodDesc &methodDesc);

    MonoClass findClass(const char* nameSpace, const char *className)
    {
        return MonoClass(mono::mono_class_from_name(mImage, nameSpace, className));
    }

    MonoClass findClassIgnoreCase(const char* nameSpace, const char *className)
    {
        return MonoClass(mono::mono_class_from_name_case(mImage, nameSpace, className));
    }

private:
    mono::MonoImage *mImage;
};

class MonoAssembly {
public:

    MonoAssembly(mono::MonoAssembly *assembly) : mAssembly(assembly) {}

    MonoImage image() {
        return MonoImage(mono::mono_assembly_get_image(mAssembly));
    }

    operator mono::MonoAssembly *() {
        return mAssembly;
    }

    bool isValid() const {
        return mAssembly != NULL;
    }

private:
    mono::MonoAssembly *mAssembly;
};

class MonoDomain {
public:

    enum RuntimeVersion {
        DotNet4
    };

    explicit MonoDomain(const char *file) : mDomain(mono::mono_jit_init(file)) {}
    
    explicit MonoDomain(const char *file, RuntimeVersion version) : mDomain(NULL) {
        const char *versionString = getVersionString(version);
        qDebug("Requesting mono runtime version: %s.", versionString);
        mDomain = mono::mono_jit_init_version(file, versionString);
    }

    static const char *getVersionString(RuntimeVersion version) {
        switch (version) {
        default:
        case DotNet4:
            return "v4.0.30319";        
        };
    }

    ~MonoDomain() {
        if (mDomain)
            mono::mono_jit_cleanup(mDomain);
    }

    bool isValid() const {
        return mDomain != NULL;
    }

    mono::MonoAssembly *openAssembly(const char *filename) {
        return mono::mono_domain_assembly_open(mDomain, filename);
    }

    operator mono::MonoDomain *() {
        return mDomain;
    }

private:
    MonoDomain(MonoDomain&);

    mono::MonoDomain *mDomain;
};

class MonoMethod {
public:

    MonoMethod(mono::MonoMethod *method) : mMethod(method) {}

    MonoMethod(const MonoMethod &method) : mMethod(method.mMethod) {}

    bool isValid() const {
        return mMethod != NULL;
    }

    operator mono::MonoMethod *() {
        return mMethod;
    }

    int runAsMain(int argc, char *argv[]) {
        mono::MonoObject *exc = NULL;
        int result = mono::mono_runtime_run_main(mMethod, argc, argv, &exc);
        if (exc != NULL) {
            mono::mono_print_unhandled_exception(exc);
        }
        return result;
    }

private:

    mono::MonoMethod *mMethod;

};

class MonoMethodDesc {
public:

    explicit MonoMethodDesc(const char *pattern, bool includesNamespace = true)
        : mMethodDesc(mono::mono_method_desc_new(pattern, includesNamespace)) {}

    ~MonoMethodDesc() {
        free();
    }

    void free() {
        if (isValid())
            mono::mono_method_desc_free(mMethodDesc);
    }

    bool isValid() const {
        return mMethodDesc != NULL;
    }

    operator mono::MonoMethodDesc *() {
        return mMethodDesc;
    }

private:
    MonoMethodDesc(MonoMethodDesc&);

    mono::MonoMethodDesc *mMethodDesc;
};

/**
Wraps a mono delegate. The type argument is the function pointer type representing the signature of the
delegate.
 */
template<typename T>
class MonoDelegate {
public:
    explicit MonoDelegate(mono::MonoDelegate* delegate) 
        : mDelegate(delegate), 
          mGcHandle(mono::mono_gchandle_new((mono::MonoObject*)delegate, TRUE)),
          mFunction(NULL) {
    }

    ~MonoDelegate() {
        mono::mono_gchandle_free(mGcHandle);
    }

    operator mono::MonoObject*() {
        return (mono::MonoObject*)mDelegate;
    }

    operator mono::MonoDelegate*() {
        return mDelegate;
    }

    mono::MonoClass *getClass() {
        return mono::mono_object_get_class (*this);
    }

    mono::MonoMethod *getDelegateMethod() {
        return mono::mono_get_delegate_invoke(getClass());
    }
    
    T functionPointer() {
        if (!mFunction) {
            mFunction = reinterpret_cast<T>(mono::mono_method_get_unmanaged_thunk(getDelegateMethod()));
        }
        return mFunction;
    }

private:
    mono::MonoDelegate *mDelegate;
    mono::uint32_t mGcHandle;
    T mFunction;
};

inline mono::MonoMethod *MonoImage::findMethod(MonoMethodDesc &methodDesc)
{
    return mono::mono_method_desc_search_in_image(methodDesc, mImage);
}

inline mono::MonoMethod *MonoClass::findMethod(mono::MonoMethodDesc *methodDesc)
{
    return mono::mono_method_desc_search_in_class(methodDesc, mClass);
}

inline mono::MonoMethod *MonoClass::findMethod(const char *name, int paramCount)
{
    return mono::mono_class_get_method_from_name(mClass, name, paramCount);
}

}

#endif // MONOPP_H
