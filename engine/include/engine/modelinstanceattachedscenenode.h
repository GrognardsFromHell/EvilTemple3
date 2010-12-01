#ifndef MODELINSTANCEATTACHEDSCENENODE_H
#define MODELINSTANCEATTACHEDSCENENODE_H

#include <QMetaType>

#include "scenenode.h"

namespace EvilTemple {

class ModelInstance;

class ModelInstanceAttachedSceneNode : public SceneNode {
Q_OBJECT
Q_PROPERTY(EvilTemple::ModelInstance *modelInstance READ modelInstance WRITE setModelInstance)
Q_PROPERTY(QByteArray boneName READ boneName WRITE setBoneName)
public:
    ModelInstanceAttachedSceneNode(Scene *scene);

    ModelInstance *modelInstance() const;
    void setModelInstance(ModelInstance *modelInstance);

    const QByteArray &boneName() const;
    void setBoneName(const QByteArray &boneName);

    void elapseTime(float elapsedSeconds);

protected:
    void updateWorldMatrix() const;

private:
    QPointer<ModelInstance> mModelInstance;
    QByteArray mBoneName;
};

inline ModelInstance *ModelInstanceAttachedSceneNode::modelInstance() const
{
    return mModelInstance;
}

inline void ModelInstanceAttachedSceneNode::setModelInstance(ModelInstance *modelInstance)
{
    mModelInstance = modelInstance;
}

inline const QByteArray &ModelInstanceAttachedSceneNode::boneName() const
{
    return mBoneName;
}

inline void ModelInstanceAttachedSceneNode::setBoneName(const QByteArray &boneName)
{
    mBoneName = boneName;
}

}

Q_DECLARE_METATYPE(EvilTemple::ModelInstanceAttachedSceneNode*)

#endif // MODELINSTANCEATTACHEDSCENENODE_H
