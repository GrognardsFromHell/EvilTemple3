
#if !defined(ENGINE_RESOURCEMANAGER_H)
#define ENGINE_RESOURCEMANAGER_H

#include <QList>
#include <QByteArray>

#include "global.h"
#include "iresources.h"

namespace EvilTemple {

/**
    Provides facilities to access static game resources.

    Paths are considered to case-insensitive and will be converted to lower-case first. Backslashes will be
    replaced with slashes and the path will be normalized in general.

    Normalization entails: Compressing redundant slashes to a single slash and resolving "/../" or "/./".
 */
class ENGINE_EXPORT ResourceManager : public IResources {
public:

    ResourceManager();
    ~ResourceManager();

    typedef QByteArray Path;

    /**
        Reads an entire resource into memory, given the resource's path.
     */
    QByteArray getResource(const Path &path);

    /**
        Lists all resources within a given directory.
     */
    QList<Path> listResources(const Path &path, bool recursive = false);

    /**
        Searches for resources, given a pattern that will be matched against the path of the resources.
     */
    QList<Path> findResources(const Path &path,
                              const Path &pattern, 
                              bool recursive = false);

    /**
        This will add resources from a directory to the resource manager.
      */
    bool addDirectory(const QString &path);

    /**
        Adds a ZIP Archive to the resource manager. It will be indexed when this function is called.
      */
    bool addZipArchive(const QString &filename);

    /**
      Normalizes a path. This method is used internally by the resource accessors.
      */
    static Path normalizePath(const Path &path);

private:
    class Private;

    Private *d;

};

}

#endif // ENGINE_RESOURCEMANAGER_H
