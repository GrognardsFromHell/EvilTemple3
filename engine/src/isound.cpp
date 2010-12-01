
 #include <QMetaType>

#include "engine/isound.h"

namespace EvilTemple {

ISound::~ISound()
{
}

}

Q_DECLARE_METATYPE(EvilTemple::SharedSound)
