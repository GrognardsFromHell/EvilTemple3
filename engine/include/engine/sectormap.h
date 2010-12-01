#ifndef SECTORMAP_H
#define SECTORMAP_H

#include <QObject>
#include <QScopedPointer>
#include <QPolygon>

#include "renderable.h"
#include "texture.h"
#include "navigationmesh.h"

#include <QGLBuffer>

namespace EvilTemple {

class Scene;
class SectorMapData;

class Sector : public Renderable
{
Q_OBJECT
public:
    Sector();

    void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL);

    const Box3d &boundingBox();

    void setNavigationMesh(const SharedNavigationMesh &navigationMesh);

    void setLayer(const RegionLayer &layer);

    void setBaseColor(const Vector4 &baseColor);
private:
    void buildBuffers();

    SharedNavigationMesh mNavigationMesh;

    RegionLayer mLayer;

    bool mBuffersInvalid;
    QGLBuffer mVertexBuffer;
    QGLBuffer mColorBuffer;
    QGLBuffer mIndexBuffer;
    QGLBuffer mPortalVertexBuffer;

    Box3d mBoundingBox;

    Vector4 mBaseColor; // Used as fallback if no special color is known for the layer
};

inline void Sector::setBaseColor(const Vector4 &baseColor)
{
    mBaseColor = baseColor;
}

inline void Sector::setLayer(const RegionLayer &layer)
{
    mLayer = layer;
}

class SectorMap : public QObject
{
Q_OBJECT
public:
    SectorMap(Scene *scene);
    ~SectorMap();

public slots:
    bool load(const QString &filename) const;

    bool createDebugView() const;

    bool createFlyableDebugView() const;

    bool createDebugLayer(const QString &layerName) const;

    bool createDebugLayer(const QString &layerName, const Vector4 &baseColor) const;

    QVector<Vector4> findPath(const Vector4 &start, const Vector4 &end) const;

    bool hasLineOfSight(const Vector4 &from, const Vector4 &to) const;

    QVariant regionTag(const QString &layer, const Vector4 &at) const;

private:
    QScopedPointer<SectorMapData> d;
};

}

Q_DECLARE_METATYPE(EvilTemple::SectorMap*)

#endif // SECTORMAP_H
