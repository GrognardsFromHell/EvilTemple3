
#if !defined(ENGINE_TYPEREGISTRATOR_H)
#define ENGINE_TYPEREGISTRATOR_H

#include "global.h"

namespace EvilTemple {
    class ENGINE_EXPORT TypeRegistrator {
    public:
        static void registerTypes();
    private:
        TypeRegistrator();
    };
}

#endif // ENGINE_TYPEREGISTRATOR_H
