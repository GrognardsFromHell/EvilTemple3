
#ifndef SCENE_H
#define SCENE_H

#include "engine/global.h"

#include <QObject>
#include <QtCore/QScopedPointer>

#include "scenenode.h"

namespace EvilTemple {

class SceneData;
class Materials;

/**
  Models a scene that can be drawn by the engine.
  */
class ENGINE_EXPORT Scene : public QObject {
Q_OBJECT
public:
    Scene(Materials *materials);
    ~Scene();

    void elapseTime(float elapsedSeconds);

    void render(RenderStates &renderStates);

public slots:
    /**
      Creates a new scene node and adds it as a top-level node to the scene.
      */
    SceneNode *createNode();

    /**
      Adds a node to the list of nodes that will be cleared when the scene is cleared.
      */
    void addNode(SceneNode *node);

    /**
      Removes the given scene node from this scene. Please note that this will also
      delete the scene node.
      */
    void removeNode(SceneNode *node);

    void clear();

    void addTextOverlay(const Vector4 &position, const QString &text, const Vector4 &color, float lifetime = 2.5f);

    /**
    Returns the number of objects drawn by the last call to render.
    */
    int objectsDrawn() const;

    SceneNode *pickNode(const Ray3d &ray) const;

    Renderable *pickRenderable(const Ray3d &ray) const;

private:
    QScopedPointer<SceneData> d;
    Q_DISABLE_COPY(Scene)
};

}

Q_DECLARE_METATYPE(EvilTemple::Scene*)

#endif // SCENE_H
