
#include "zonetemplate.h"
#include "zonebackgroundmap.h"
#include "util.h"

namespace Troika
{

    class ZoneTemplateData {
    public:
        ZoneTemplateData()
            : dayBackground(0),
            nightBackground(0),
            tutorialMap(false),
            menuMap(false),
            unfogged(false),
            outdoor(false),
            dayNightTransfer(false),
            bedrest(false) {}

        ~ZoneTemplateData()
        {
            qDeleteAll(staticObjects);
            qDeleteAll(mobiles);
        }

        quint32 id;
        ZoneBackgroundMap *dayBackground;
        ZoneBackgroundMap *nightBackground;
        QList<GeometryObject*> staticGeometry;
        QList<GameObject*> staticObjects;
        QList<GameObject*> mobiles;
        QList<GeometryObject*> clippingGeometry;
        QList<uint> soundSchemes;

        QList<ParticleSystem> particleSystems;
        QList<Light> lights;
        QList<TileSector> tileSectors;

        QPoint startPosition;
        quint32 movie; // Movie to play when entering

        bool tutorialMap;
        bool menuMap;
        bool unfogged;
        bool outdoor;
        bool dayNightTransfer;
        bool bedrest;

        Box3D scrollBox;
        QString directory;
        QString name;

        /**
          Global lighting, globalLight is read from global.lit,
          while the following four lists are compiled from daylight.mes.

          If any daylight transition is set, globalLight is only used for its
          lighting direction.
          */
        Light globalLight;
        QList<LightKeyframe> lightingKeyframesDay2d;
        QList<LightKeyframe> lightingKeyframesDay3d;
        QList<LightKeyframe> lightingKeyframesNight2d;
        QList<LightKeyframe> lightingKeyframesNight3d;
    };

    ZoneTemplate::ZoneTemplate(quint32 id, QObject *parent) :
            QObject(parent), d_ptr(new ZoneTemplateData)
    {
        d_ptr->id = id;
        d_ptr->dayBackground = NULL;
        d_ptr->nightBackground = NULL;
    }

    ZoneTemplate::~ZoneTemplate()
    {
    }

    quint32 ZoneTemplate::id() const
    {
        return d_ptr->id;
    }

    ZoneBackgroundMap*ZoneTemplate::dayBackground() const
    {
        return d_ptr->dayBackground;
    }

    ZoneBackgroundMap *ZoneTemplate::nightBackground() const
    {
        return d_ptr->nightBackground;
    }

    const QList<GeometryObject*> &ZoneTemplate::staticGeometry() const
    {
        return d_ptr->staticGeometry;
    }

    const QList<GeometryObject*> &ZoneTemplate::clippingGeometry() const
    {
        return d_ptr->clippingGeometry;
    }

    void ZoneTemplate::setDayBackground(ZoneBackgroundMap* backgroundMap)
    {
        d_ptr->dayBackground = backgroundMap;
    }

    void ZoneTemplate::setNightBackground(ZoneBackgroundMap* backgroundMap)
    {
        d_ptr->nightBackground = backgroundMap;
    }

    void ZoneTemplate::addStaticGeometry(GeometryObject *object)
    {
        d_ptr->staticGeometry.append(object);
    }

    void ZoneTemplate::addClippingGeometry(GeometryObject *object)
    {
        d_ptr->clippingGeometry.append(object);
    }

    const QPoint &ZoneTemplate::startPosition() const
    {
        return d_ptr->startPosition;
    }

    quint32 ZoneTemplate::movie()
    {
        return d_ptr->movie;
    }

    bool ZoneTemplate::isTutorialMap() const
    {
        return d_ptr->tutorialMap;
    }

    bool ZoneTemplate::isMenuMap() const
    {
        return d_ptr->menuMap;
    }

    bool ZoneTemplate::isUnfogged() const
    {
        return d_ptr->unfogged;
    }

    bool ZoneTemplate::isOutdoor() const
    {
        return d_ptr->outdoor;
    }

    bool ZoneTemplate::hasDayNightTransfer() const
    {
        return d_ptr->dayNightTransfer;
    }

    bool ZoneTemplate::allowsBedrest() const
    {
        return d_ptr->bedrest;
    }

    void ZoneTemplate::setStartPosition(const QPoint &startPosition)
    {
        d_ptr->startPosition = startPosition;
    }

    void ZoneTemplate::setMovie(quint32 movie)
    {
        d_ptr->movie = movie;
    }

    void ZoneTemplate::setTutorialMap(bool enabled)
    {
        d_ptr->tutorialMap = enabled;
    }

    void ZoneTemplate::setMenuMap(bool enabled)
    {
        d_ptr->menuMap = enabled;
    }

    void ZoneTemplate::setUnfogged(bool enabled)
    {
        d_ptr->unfogged = enabled;
    }

    void ZoneTemplate::setOutdoor(bool enabled)
    {
        d_ptr->outdoor = enabled;
    }

    void ZoneTemplate::setDayNightTransfer(bool enabled)
    {
        d_ptr->dayNightTransfer = enabled;
    }

    void ZoneTemplate::setBedrest(bool enabled)
    {
        d_ptr->bedrest = enabled;
    }

    const Box3D &ZoneTemplate::scrollBox() const
    {
        return d_ptr->scrollBox;
    }

    void ZoneTemplate::setScrollBox(const Box3D &box)
    {
        d_ptr->scrollBox = box;
    }

    const QString &ZoneTemplate::directory() const
    {
        return d_ptr->directory;
    }

    void ZoneTemplate::setDirectory(const QString &directory)
    {
        d_ptr->directory = directory;
    }

    const QString &ZoneTemplate::name() const
    {
        return d_ptr->name;
    }

    void ZoneTemplate::setName(const QString &name)
    {
        d_ptr->name = name;
    }

    void ZoneTemplate::addLight(const Light &light)
    {
        d_ptr->lights.append(light);
    }

    void ZoneTemplate::addParticleSystem(const ParticleSystem &particleSystem)
    {
        d_ptr->particleSystems.append(particleSystem);
    }

    const QList<Light> &ZoneTemplate::lights() const
    {
        return d_ptr->lights;
    }

    const QList<ParticleSystem> &ZoneTemplate::particleSystems() const
    {
        return d_ptr->particleSystems;
    }

        const Light &ZoneTemplate::globalLight() const
        {
                return d_ptr->globalLight;
        }

        void ZoneTemplate::setGlobalLight(const Light &light)
        {
                d_ptr->globalLight = light;
        }

        const QList<GameObject*> &ZoneTemplate::staticObjects() const
        {
            return d_ptr->staticObjects;
        }

    const QList<GameObject*> &ZoneTemplate::mobiles() const
    {
        return d_ptr->mobiles;
    }

    void ZoneTemplate::addStaticObject(GameObject *gameObject)
    {
        d_ptr->staticObjects.append(gameObject);
    }

    void ZoneTemplate::addMobile(GameObject *gameObject)
    {
        d_ptr->mobiles.append(gameObject);
    }

    void ZoneTemplate::addTileSector(const TileSector &tileSector)
    {
        d_ptr->tileSectors.append(tileSector);
    }

    const QList<TileSector> &ZoneTemplate::tileSectors() const
    {
        return d_ptr->tileSectors;
    }

    const QList<LightKeyframe> &ZoneTemplate::lightingKeyframesDay2d() const
    {
        return d_ptr->lightingKeyframesDay2d;
    }

    const QList<LightKeyframe> &ZoneTemplate::lightingKeyframesDay3d() const
    {
        return d_ptr->lightingKeyframesDay3d;
    }

    void ZoneTemplate::setLightingKeyframesDay(const QList<LightKeyframe> &keyframes2d,
                                 const QList<LightKeyframe> &keyframes3d)
    {
        d_ptr->lightingKeyframesDay2d = keyframes2d;
        d_ptr->lightingKeyframesDay3d = keyframes3d;
    }

    const QList<LightKeyframe> &ZoneTemplate::lightingKeyframesNight2d() const
    {
        return d_ptr->lightingKeyframesNight2d;
    }

    const QList<LightKeyframe> &ZoneTemplate::lightingKeyframesNight3d() const
    {
        return d_ptr->lightingKeyframesNight3d;
    }

    void ZoneTemplate::setLightingKeyframesNight(const QList<LightKeyframe> &keyframes2d,
                                 const QList<LightKeyframe> &keyframes3d)
    {
        d_ptr->lightingKeyframesNight2d = keyframes2d;
        d_ptr->lightingKeyframesNight3d = keyframes3d;
    }

    const QList<uint> &ZoneTemplate::soundSchemes() const
    {
        return d_ptr->soundSchemes;
    }

    void ZoneTemplate::setSoundSchemes(const QList<uint> &soundSchemes)
    {
        d_ptr->soundSchemes = soundSchemes;
    }

}
