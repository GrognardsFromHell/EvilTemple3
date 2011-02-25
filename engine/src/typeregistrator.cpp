
#include <QMetaType>

#include "engine/typeregistrator.h"
#include "engine/modelinstance.h"
#include "engine/materials.h"

namespace EvilTemple {       

    void TypeRegistrator::registerTypes()
    {
        qRegisterMetaType<EvilTemple::ModelInstance*>();
        qRegisterMetaType<EvilTemple::Materials*>();
        qRegisterMetaType<EvilTemple::SharedModel>();
        qRegisterMetaType<EvilTemple::SharedMaterialState>();
    }

}