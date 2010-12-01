#ifndef UTIL_H
#define UTIL_H

#include <GL/glew.h>

#include <limits>

#include <gamemath.h>
#include "gamemath_streams.h"

namespace EvilTemple
{
    /**
      Custom deleter for QScopedPointer that uses aligned free.
      */
    struct AlignedDeleter
    {
        static inline void cleanup(void *pointer)
        {
            ALIGNED_FREE(pointer);
        }
    };

    const float Pi = 3.14159265358979323846f;

    /**
      The original game applies an additional base rotation to everything in order to align it
      with the isometric grid. This is the radians value of that rotation.
      */
    const float LegacyBaseRotation = 0.77539754f;

    /**
      Converts radians to degree.
      */
    inline float rad2deg(float rad)
    {
        return rad * 180.f / Pi;
    }

    /**
      Converts radians to degree.
      */
    inline float deg2rad(float deg)
    {
        return deg / 180.f * Pi;
    }

    /**
      This class represents an md5 hash that is used to identify a file.
      It is NOT used to actually create md5 hashes, use QCryptographicHash for that purpose
      instead.
      */
    class Md5Hash {
    friend inline uint qHash(const Md5Hash &key);
    public:
        bool operator ==(const Md5Hash &other) const;
        bool operator !=(const Md5Hash &other) const;
    private:
        int mHash[4];
    };

    inline bool Md5Hash::operator ==(const Md5Hash &other) const
    {
        return mHash[0] == other.mHash[0] &&
               mHash[1] == other.mHash[1] &&
               mHash[2] == other.mHash[2] &&
               mHash[3] == other.mHash[3];
    }

    inline uint qHash(const Md5Hash &key)
    {
        return key.mHash[0] ^ key.mHash[1] ^ key.mHash[2] ^ key.mHash[3];
    }

    inline bool Md5Hash::operator !=(const Md5Hash &other) const
    {
        return !this->operator==(other);
    }

    /*
        OpenGL error handling code:
        Every GL function call should be wrapped in this macro. This allows the application to insert
        pre and post debugging code.

        In no-debug mode, this macro simply calls the supplied function without any checks.
    */
    inline void __gl_errorHandler(const char *file, int line, const char *statement) {
        for(int _errorCode = glGetError(); _errorCode != GL_NO_ERROR; _errorCode = glGetError()) {
            qWarning("GL call '%s' failed @ %s:%d with %s.", statement, file, line, gluErrorString(_errorCode));
        }
    }

#if defined(QT_NO_DEBUG)
    #define SAFE_GL(x) x
#else
    #define SAFE_GL(x) x, __gl_errorHandler(__FILE__, __LINE__, #x)
#endif

}

#endif // UTIL_H
