
#if !defined(ENGINE_IMATERIALSOURCE_H)
#define ENGINE_IMATERIALSOURCE_H

namespace EvilTemple {

/**
Provides access to materials.
*/
class IMaterialSource {
public:

    /**
    Gets a material from this material source by giving a filename.
    */
    virtual SharedMaterialState &getMaterial(const QByteArray &filename) = 0;

    /**
    Returns the material that is used for unknown or invalid materials.
    */
    virtual SharedMaterialState &getInvalidMaterial() = 0;

protected:
    /**
    Polymorphic destruction is not desired.
    */
    virtual ~IMaterialSource();
};

}

#endif // ENGINE_IMATERIALSOURCE_H
