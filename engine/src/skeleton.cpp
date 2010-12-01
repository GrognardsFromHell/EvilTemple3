
#include "engine/skeleton.h"
#include "engine/gamemath_streams.h"

namespace EvilTemple {

QDataStream &operator >>(QDataStream &stream, Skeleton &skeleton)
{
    int bonesCount;
    stream >> bonesCount;

    Q_ASSERT(bonesCount >= 0);

    skeleton.mBones = new Bone[bonesCount];
    skeleton.mBonePointers.resize(bonesCount);

    Matrix4 relativeWorld;

    for (int j = 0; j < bonesCount; ++j) {
        Bone &bone = skeleton.mBones[j];
        QByteArray boneName;
        int parentId;

        stream >> boneName >> parentId >> relativeWorld;

        if (skeleton.mBoneMap.contains(boneName)) {
            qWarning("Duplicate bone %s in skeleton %s", boneName.constData(), qPrintable(skeleton.mName));
        }

        // Insert the bone into the name->bone map
        skeleton.mBonePointers[j] = skeleton.mBones + j;
        skeleton.mBoneMap.insert(boneName, skeleton.mBonePointers[j]);

        Q_ASSERT(parentId >= -1 && parentId < j);

        bone.setBoneId(j);
        bone.setRelativeWorld(relativeWorld);
        bone.setName(boneName);

        if (parentId == -1) {
            bone.setFullWorld(relativeWorld);
            continue;
        }

        Bone *parent = skeleton.mBonePointers[parentId];

        bone.setParent(parent);
        bone.setFullWorld(parent->fullWorld() * relativeWorld);
    }

    return stream;
}

Skeleton::~Skeleton()
{
    delete [] mBones;
}

Skeleton::Skeleton(const Skeleton &other)
    : mName(other.mName + " (Copy)")
{
    mBones = new Bone[other.mBonePointers.size()];
    for (int i = 0; i < other.mBonePointers.size(); ++i) {
        mBones[i] = other.mBones[i];
    }

    mBonePointers.resize(other.mBonePointers.size());
    mBoneMap.reserve(other.mBoneMap.capacity());

    // Re-create the pointer maps
    for (int i = 0; i < mBonePointers.size(); ++i) {
        Bone *bonePointer = mBones + i;

        // Rewire the parent
        if (bonePointer->parent()) {
            bonePointer->setParent(mBonePointers[bonePointer->parent()->boneId()]);
        }

        mBoneMap.insert(mBones[i].name(), bonePointer);
        mBonePointers[i] = bonePointer;
    }
}

}
