#include "engine/modelinstanceattachedscenenode.h"

namespace EvilTemple {

ModelInstanceAttachedSceneNode::ModelInstanceAttachedSceneNode(Scene *scene)
    : SceneNode(scene)
{
    setAnimated(true);
}

void ModelInstanceAttachedSceneNode::updateWorldMatrix() const
{
    SceneNode::updateWorldMatrix();

    if (!mModelInstance)
        return;

    const Skeleton *skeleton = mModelInstance->skeleton();

    if (!skeleton)
        return;

    const Bone *bone = skeleton->bone(mBoneName);

    if (!bone)
        return;

    Matrix4 flipZ;
    flipZ.setToIdentity();
    flipZ(2,2) = -1;

    Vector4 xUnit(1, 0, 0, 0);
    xUnit = bone->fullWorld() * xUnit;

    Vector4 yUnit(0, 1, 0, 0);
    yUnit = bone->fullWorld() * yUnit;

    Vector4 zUnit(0, 0, 1, 0);
    zUnit = bone->fullWorld() * zUnit;

    mWorldMatrix = flipZ * bone->fullWorld() * Matrix4::scaling(1 / xUnit.length(), 1 / yUnit.length(), 1 / zUnit.length())
            * flipZ;
}

void ModelInstanceAttachedSceneNode::elapseTime(float elapsedSeconds)
{
    Q_UNUSED(elapsedSeconds);

    if (!mModelInstance)
        return;

    const Skeleton *skeleton = mModelInstance->skeleton();

    if (!skeleton)
        return;

    const Bone *bone = skeleton->bone(mBoneName);

    if (!bone)
        return;

    Matrix4 flipZ;
    flipZ.setToIdentity();
    flipZ(2,2) *= -1;

    Matrix4 boneSpace = flipZ * bone->fullWorld() * flipZ;
    Vector4 trans = boneSpace.column(3);

    setPosition(trans);
}

}
