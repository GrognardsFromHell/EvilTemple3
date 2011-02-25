#ifndef MATERIALS_H
#define MATERIALS_H

#include "troikaformatsglobal.h"

#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QHash>
#include <QImage>

namespace Troika {

    class Material;
    class MaterialsData;
    class VirtualFileSystem;

    class TROIKAFORMATS_EXPORT Materials : public QObject
    {
        Q_OBJECT
    public:
        explicit Materials(VirtualFileSystem *vfs, QObject *parent = 0);
        ~Materials();

        QSharedPointer<Material> loadFromFile(const QString &filename);
    private:
        QScopedPointer<MaterialsData> d_ptr;

        Q_DISABLE_COPY(Materials)
    };

}

#endif // MATERIALS_H
