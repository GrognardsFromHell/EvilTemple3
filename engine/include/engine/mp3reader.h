#ifndef MP3READER_H
#define MP3READER_H

#include "engine/global.h"

#include "isound.h"

#include <QString>

namespace EvilTemple {

class MP3Reader
{
public:
    static ISound *read(const QString &filename);
};

}

#endif // MP3READER_H
