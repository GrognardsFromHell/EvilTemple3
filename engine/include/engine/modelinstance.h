#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include "engine/global.h"

#include <QtCore/QString>
#include <QtCore/QSharedPointer>

#include "modelfile.h"
#include "materialstate.h"
#include "renderable.h"
#include "drawhelper.h"

#include <QtOpenGL/QGLBuffer>

#include <gamemath.h>
using namespace GameMath;

namespace EvilTemple {

class RenderStates;

struct AddMesh {
    QString boneName;
    SharedModel model;
};

/**
    A model instance manages the per-instance state of models. This includes animation state
    and transformed position and normal data.
  */
class ENGINE_EXPORT ModelInstance : public Renderable
{
Q_OBJECT
Q_PROPERTY(EvilTemple::SharedModel model READ model WRITE setModel)
Q_PROPERTY(bool idling READ isIdling)
Q_PROPERTY(QByteArray idleAnimation READ idleAnimation WRITE setIdleAnimation)
Q_PROPERTY(bool drawBehindWalls READ drawsBehindWalls WRITE setDrawsBehindWalls)
public:
    ModelInstance();
    ~ModelInstance();

    const SharedModel &model() const;
    void setModel(const SharedModel &model);

    /**
      This is just a temporary remedy. Instead, skeleton updates should be refactored out of this
      class into a separate class, so other classes can also draw animated skeletons without the
      overhead of this class.
      */
    void draw(RenderStates &renderStates, const CustomDrawHelper<ModelDrawStrategy, ModelBufferSource> &drawHelper);
    void drawNormals() const;
    void render(RenderStates &renderStates, MaterialState *overrideMaterial = NULL);

    IntersectionResult intersect(const Ray3d &ray) const;

    const Box3d &boundingBox();

    const Matrix4 &worldTransform() const;

    void setIdleAnimation(const QByteArray &idleAnimation);
    const QByteArray &idleAnimation() const;

    bool isIdling() const; // No animation is playing

    bool drawsBehindWalls() const;
    void setDrawsBehindWalls(bool drawsBehindWalls);

    const Skeleton *skeleton() const;
    bool hasSkeleton() const;

public slots:
    Matrix4 getBoneSpace(uint boneId);

    void addMesh(const EvilTemple::SharedModel &model);
    void clearAddMeshes();

    bool overrideMaterial(const QByteArray &name, const EvilTemple::SharedMaterialState &state);
    bool clearOverrideMaterial(const QByteArray &name);
    void clearOverrideMaterials();

    /**
      Overrides material properties by name. This feature may be used to specify
      uniform values for materials. A specific use case is the specification of
      skin or hair colors, that may subsequently be used by the material shaders
      to color hair and skin properly.
      */
    void setMaterialPropertyFloat(const QString &name, float value);
    void setMaterialPropertyVec4(const QString &name, const Vector4 &value);

    bool playAnimation(const QByteArray &name, bool loop = false);
    void stopAnimation();

    void elapseTime(float elapsedSeconds);
    void elapseDistance(float distance);
    void elapseRotation(float rotation);

signals:
    void animationFinished(const QString &name, bool canceled);
    void animationEvent(int type, const QString &content);

private:

    bool advanceFrame();

    void animateVertices(const SharedModel &model,
        Vector4 *transformedPositions,
        Vector4 *transformedNormals,
        VertexBufferObject *positionBuffer,
        VertexBufferObject *normalBuffer,
        const QVector<uint> &boneMapping);

    void playIdleAnimation();
    void updateBones();

    bool mDrawsBehindWalls;

    SharedModel mModel;

    QByteArray mIdleAnimation;
    bool mIdling; // mCurrentAnimation is the idle animation
    bool mLooping; // Only relevant if not idling (idle is always looped)
    const Animation *mCurrentAnimation;

    float mPartialFrameTime;
    uint mCurrentFrame;
    bool mCurrentFrameChanged;

    Vector4 *mTransformedPositions;
    Vector4 *mTransformedNormals;

    QList<Vector4*> mTransformedPositionsAddMeshes;
    QList<Vector4*> mTransformedNormalsAddMeshes;

    QList<VertexBufferObject*> mPositionBufferAddMeshes;
    QList<VertexBufferObject*> mNormalBufferAddMeshes;

    float mTimeSinceLastRender;

    Skeleton *mSkeleton;

    // When an add-mesh is loaded, this maps the bone-ids from the addmesh to the bone ids in the
    // skeleton used by mModel
    QList< QVector<uint> > mAddMeshBoneMapping;

    VertexBufferObject mPositionBuffer;
    VertexBufferObject mNormalBuffer;

    QList<SharedModel> mAddMeshes;

    // These relate to mModel->placeholders()
    QVector<SharedMaterialState> mReplacementMaterials;

    QHash<QByteArray, Vector4> mMaterialPropertiesVec4;
    QHash<QByteArray, float> mMaterialPropertiesFloat;

    Q_DISABLE_COPY(ModelInstance);
};

inline void ModelInstance::setMaterialPropertyFloat(const QString &name, float value)
{
    mMaterialPropertiesFloat[name.toLatin1()] = value;
}

inline void ModelInstance::setMaterialPropertyVec4(const QString &name, const Vector4 &value)
{
    mMaterialPropertiesVec4[name.toLatin1()] = value;
}

inline const SharedModel &ModelInstance::model() const
{
    return mModel;
}

inline bool ModelInstance::isIdling() const
{
    return mIdling;
}

inline bool ModelInstance::drawsBehindWalls() const
{
    return mDrawsBehindWalls;
}

inline void ModelInstance::setDrawsBehindWalls(bool drawsBehindWalls)
{
    mDrawsBehindWalls = drawsBehindWalls;
}

inline bool ModelInstance::hasSkeleton() const
{
    return true;
}

inline const Skeleton *ModelInstance::skeleton() const
{
    return mSkeleton;
}

}

Q_DECLARE_METATYPE(EvilTemple::ModelInstance*)

#endif // MODELINSTANCE_H
