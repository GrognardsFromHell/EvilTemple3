
#if !defined(__LAUNCHER_QTMONO_H__)
#define __LAUNCHER_QTMONO_H__

#include <assert.h>

namespace QtMono {

    template<typename T>
    T monoUnbox(mono::MonoObject* obj)
    {
        return *reinterpret_cast<T*>(mono::mono_object_unbox(obj));
    }

    template<typename T>
    T fromMono(mono::MonoObject*) {
        static_assert(false, "No specializations is available for this type.");
    }

    // Unwraps an integer
    template<>
    int fromMono(mono::MonoObject *obj) {
        assert(obj != NULL);
        assert(mono::mono_object_get_class(obj) == mono::mono_get_int32_class());

        return monoUnbox<int>(obj);
    }

    // Unwraps a 32-bit unsigned integer
    template<>
    uint fromMono(mono::MonoObject *obj) {
        assert(obj != NULL);
        assert(mono::mono_object_get_class(obj) == mono::mono_get_uint32_class());

        return monoUnbox<uint>(obj);
    }

    // Unwraps a single
    template<>
    float fromMono(mono::MonoObject *obj) {
        assert(obj != NULL);
        assert(mono::mono_object_get_class(obj) == mono::mono_get_single_class());

        return monoUnbox<float>(obj);
    }

    // Unwraps a double
    template<>
    double fromMono(mono::MonoObject *obj) {
        assert(obj != NULL);
        assert(mono::mono_object_get_class(obj) == mono::mono_get_double_class());

        return monoUnbox<double>(obj);
    }
}

#endif
