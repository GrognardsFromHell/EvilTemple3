#ifndef ISOUNDHANDLE_H
#define ISOUNDHANDLE_H

#include "engine/global.h"

#include <gamemath.h>
using namespace GameMath;

#include <QSharedPointer>

namespace EvilTemple {

/**
  Describes the category of a playing sound. This mostly effects which volume setting is used.
  */
enum SoundCategory {
    SoundCategory_Music = 0,
    SoundCategory_Effect, // Generic sound effect
    SoundCategory_Interface, // Interface sound effects like hovering/clicking a button
    SoundCategory_Ambience, // Ambient sound effects like the chirping of birds
    SoundCategory_Movie, // Movie soundtrack
    SoundCategory_Other,
    SoundCategory_Count, // Don't use this literal. Its only for faster mapping from this enum to values in an array
};

/**
  Describes a sound handle that can be used to control a sound that is being played by the audio engine.
  */
class ENGINE_EXPORT ISoundHandle
{
public:
    virtual ~ISoundHandle();

    /**
      Stops playing this sound. The audio engine will free this handle afterwards.
      */
    virtual void stop() = 0;

    /**
      Marks this sound as looping, it will continue playing at the beginning of the sound,
      even if the end is reached.
      */
    virtual void setLooping(bool looping) = 0;

    /**
      Returns whether this sound will loop when it reaches its end.
      */
    virtual bool looping() const = 0;

    /**
      Sets the volume of this sound on a scala of 0 (mute) to 1 (full volume).

      Please note that the global volume settings will be multiplied with this value.

      @param The volume, which is a floating point number within the range of [0,1]. This value will be
      clamped automatically.
      */
    virtual void setVolume(qreal volume) = 0;

    /**
      Returns the volume settings for this sound sorce.
      */
    virtual qreal volume() const = 0;

    /**
      Returns the category of this sound.
      */
    virtual SoundCategory category() const = 0;

    /**
      Changes the category of this sound handle.
      */
    virtual void setCategory(SoundCategory category) = 0;

    /**
      Changes the position of this sound.
      */
    virtual void setPosition(const Vector4 &position) = 0;

    /**
      Change the maximum distance for this sound.
      */
    virtual void setMaxDistance(float maxDistance) = 0;

    virtual void setReferenceDistance(float distance) = 0;
};

typedef QSharedPointer<ISoundHandle> SharedSoundHandle;

}

#endif // ISOUNDHANDLE_H
