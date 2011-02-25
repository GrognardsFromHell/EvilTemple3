#ifndef ZONETEMPLATE_H
#define ZONETEMPLATE_H

#include "troikaformatsglobal.h"

#include "objectfilereader.h"

#include <QObject>
#include <QQuaternion>
#include <QVector3D>

#include "util.h"

namespace Troika
{
    /**
      The side length of a square sector in tiles.
      */
    const static int SectorSidelength = 64;

    class ZoneTemplateData;
    class ZoneBackgroundMap;

    class TROIKAFORMATS_EXPORT GeometryObject {
    public:
        GeometryObject(const QVector3D &position, float rotation, const QString &mesh) :
                mPosition(position), mRotation(rotation), mScale(1, 1, 1), mMesh(mesh) {
        }

        GeometryObject(const QVector3D &position,
                       float rotation,
                       const QVector3D &scale,
                       const QString &mesh) :
                mPosition(position), mRotation(rotation), mScale(scale), mMesh(mesh) {}

        const QVector3D &position() const { return mPosition; }
        float rotation() const { return mRotation; }
        const QVector3D &scale() const { return mScale; }
        const QString &mesh() const { return mMesh; }
    private:

        QVector3D mPosition;
        float mRotation; // in degrees
        QVector3D mScale;
        QString mMesh;
    };

    struct Light
    {
        bool day;
        quint64 handle;
        quint32 type;
        quint8 r, g, b; // Diffuse/Specular
        quint8 unknown;
        quint8 ur, ug, ub, ua; // Unknown color
        QVector4D position;
        float dirX, dirY, dirZ;
        float range;
        float phi;
    };

    struct ParticleSystem
    {
        Light light;
        quint32 hash;
        quint32 id;
    };

    enum TileFlags
    {
        TILE_BLOCKS = 1 << 0,
        TILE_SINKS = 1 << 1,
        TILE_CAN_FLY_OVER = 1 << 2,
        TILE_ICY = 1 << 3,
        TILE_NATURAL = 1 << 4,
        TILE_SOUNDPROOF = 1 << 5,
        TILE_INDOOR = 1 << 6,
        TILE_REFLECTIVE = 1 << 7,
        TILE_BLOCKS_VISION = 1 << 8,
        TILE_BLOCKS_UL = 1 << 9,
        TILE_BLOCKS_UM = 1 << 10,
        TILE_BLOCKS_UR = 1 << 11,
        TILE_BLOCKS_ML = 1 << 12,
        TILE_BLOCKS_MM = 1 << 13,
        TILE_BLOCKS_MR = 1 << 14,
        TILE_BLOCKS_LL = 1 << 15,
        TILE_BLOCKS_LM = 1 << 16,
        TILE_BLOCKS_LR = 1 << 17,
        TILE_FLYOVER_UL = 1 << 18,
        TILE_FLYOVER_UM = 1 << 19,
        TILE_FLYOVER_UR = 1 << 20,
        TILE_FLYOVER_ML = 1 << 21,
        TILE_FLYOVER_MM = 1 << 22,
        TILE_FLYOVER_MR = 1 << 23,
        TILE_FLYOVER_LL = 1 << 24,
        TILE_FLYOVER_LM = 1 << 25,
        TILE_FLYOVER_LR = 1 << 26,
        TILE_FLYOVER_COVER = 1 << 27,
    };

    struct SectorTile
    {
        quint8 footstepsSound;
        quint8 unknown1[3];
        quint32 bitfield;
        quint64 unknown2;
        uchar visibility[3][3];

        bool isVisionFlagSet(uint sx, uint sy, uint flag) const {
            return (visibility[sx][sy] & flag) == flag;
        }

        bool isVisionExtend(uint sx, uint sy) const {
            return isVisionFlagSet(sx, sy, 1);
        }

        bool isVisionEnd(uint sx, uint sy) const {
            return isVisionFlagSet(sx, sy, 2);
        }

        bool isVisionBase(uint sx, uint sy) const {
            return isVisionFlagSet(sx, sy, 4);
        }

        bool isVisionArchway(uint sx, uint sy) const {
            return isVisionFlagSet(sx, sy, 8);
        }
    };

    class TileSector {
    public:
        uint x;
        uint y;
        SectorTile tiles[SectorSidelength][SectorSidelength];

        bool hasNegativeHeight;
        uchar negativeHeight[192][192];
    };

    /**
      A keyframe entry used for day/night transfer lighting.
      */
    struct LightKeyframe {
        uint hour;
        float red;
        float green;
        float blue;
    };

    class TROIKAFORMATS_EXPORT ZoneTemplate : public QObject
    {
        Q_OBJECT
    public:
        explicit ZoneTemplate(quint32 id, QObject *parent = 0);
        ~ZoneTemplate();

        quint32 id() const;
        ZoneBackgroundMap *dayBackground() const;
        ZoneBackgroundMap *nightBackground() const;
        const QList<GeometryObject*> &staticGeometry() const;
        const QList<GeometryObject*> &clippingGeometry() const;

        const QList<TileSector> &tileSectors() const;

        const QString &directory() const;

        const QPoint &startPosition() const; // Camera start position
        quint32 movie(); // Movie to play when entering
        bool isTutorialMap() const;
        bool isMenuMap() const; // This denotes the first map played in the background of the menu
        bool isUnfogged() const; // No fog of war
        bool isOutdoor() const;
        bool hasDayNightTransfer() const;
        bool allowsBedrest() const;

        const Light &globalLight() const;

        const QList<Light> &lights() const;

        const QList<ParticleSystem> &particleSystems() const;

        const QString &name() const; // Zone name (translated)

        /**
          Returns the visible box of the map. The user can only scroll within the bounds of this box
          on this map.
          */
        const Box3D &scrollBox() const;

        const QList<GameObject*> &staticObjects() const;
        const QList<GameObject*> &mobiles() const;

        /**
          Sets the background to use during the day. If no day/night exchange is used
          for this zone, this background is also used during the night.
          */
        void setDayBackground(ZoneBackgroundMap *backgroundMap);

        /**
          Sets the background to use during the night. If no day/night exchange is used,
          this value is ignored.
          */
        void setNightBackground(ZoneBackgroundMap *backgroundMap);

        /**
          Adds a static geometry object. This could be a tree or a door for instance.
          @param object The geometry that is part of this zone template. The template takes ownership
                        of this pointer.
          */
        void addStaticGeometry(GeometryObject *object);

        /**
          Adds a geometry object that is used to add depth information to the pre rendered background.
          @param object The geometry that is part of this zone template. The template takes ownership
                        of this pointer.
          */
        void addClippingGeometry(GeometryObject *object);

        void addLight(const Light &light);
        void addParticleSystem(const ParticleSystem &particleSystem);

        void setName(const QString &name);
        void setDirectory(const QString &directory);
        void setStartPosition(const QPoint &startPosition);
        void setMovie(quint32 movie);
        void setTutorialMap(bool enabled);
        void setMenuMap(bool enabled);
        void setUnfogged(bool enabled);
        void setOutdoor(bool enabled);
        void setDayNightTransfer(bool enabled);
        void setBedrest(bool enabled);
        void setScrollBox(const Box3D &scrollBox);
                void setGlobalLight(const Light &light);

        void addStaticObject(GameObject *gameObject);
        void addMobile(GameObject *gameObject);

        void addTileSector(const TileSector &tileSector);

        /**
          The following methods map the data from rules\daylight.mes
          */
        const QList<LightKeyframe> &lightingKeyframesDay2d() const;
        const QList<LightKeyframe> &lightingKeyframesDay3d() const;

        void setLightingKeyframesDay(const QList<LightKeyframe> &keyframes2d,
                                     const QList<LightKeyframe> &keyframes3d);

        const QList<LightKeyframe> &lightingKeyframesNight2d() const;
        const QList<LightKeyframe> &lightingKeyframesNight3d() const;

        void setLightingKeyframesNight(const QList<LightKeyframe> &keyframes2d,
                                     const QList<LightKeyframe> &keyframes3d);

        const QList<uint> &soundSchemes() const;
        void setSoundSchemes(const QList<uint> &soundSchemes);

    private:
        QScopedPointer<ZoneTemplateData> d_ptr;

        Q_DISABLE_COPY(ZoneTemplate);
    };

}

#endif // ZONETEMPLATE_H
