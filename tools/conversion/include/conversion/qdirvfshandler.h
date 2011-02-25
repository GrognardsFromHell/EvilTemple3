#ifndef QDIRVFSHANDLER_H
#define QDIRVFSHANDLER_H

#include "global.h"

#include <QDir>

#include <virtualfilesystem.h>

class CONVERSIONSHARED_EXPORT QDirVfsHandler : public Troika::VirtualFileSystemHandler
{
public:
    QDirVfsHandler(const QDir &baseDir);

    QByteArray openFile(const QString &filename);
    bool exists(const QString &filename);
    QStringList listFiles(const QString &path, const QString &filter = "*");
    QStringList listAllFiles(const QString &filenameFilter);
private:
    QDir mBaseDir;
};


#endif // QDIRVFSHANDLER_H
