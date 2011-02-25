#ifndef MATERIALS_H
#define MATERIALS_H

#include "engine/global.h"

#include <QObject>
#include <QScopedPointer>

#include "materialstate.h"

namespace EvilTemple {

class MaterialsData;

class ENGINE_EXPORT Materials : public QObject
{
    Q_OBJECT
public:
    explicit Materials(RenderStates &renderStates, QObject *parent = 0);
    ~Materials();

    /**
      Loads predefined materials
      */
    bool load();

    /**
      Use this to retrieve errors if loading the materials failed.
      */
    const QString &error() const;

signals:

public slots:

    /**
      Returns a material that can be used to represent a missing material.
      */
    EvilTemple::SharedMaterialState &missingMaterial();

    /**
      Retrieve a material.
      */
    EvilTemple::SharedMaterialState load(const QString &filename);

private:
    QScopedPointer<MaterialsData> d;

};

}

Q_DECLARE_METATYPE(EvilTemple::Materials*)

#endif // MATERIALS_H
