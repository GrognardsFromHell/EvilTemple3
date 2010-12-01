
#include <QDir>
#include <QString>
#include <QHash>
#include <QFile>

#include "engine/resourcemanager.h"

#include "unzip.h"

namespace EvilTemple {
    
    typedef ResourceManager::Path Path;

    class ResourceManager::Private {
    public:

        QByteArray getFile(const Path &filename);

        QByteArray getArchiveFile(const Path &filename);

        bool addArchive(const QString &archiveFilename);

        bool addDirectory(const QString &directory);

        static QByteArray normalizePath(const Path &path);

    private:

        QList<QString> dataDirectories;

        QList<unzFile> archiveHandles; // Close archives when done

        struct ArchiveEntry {
            unzFile handle;
            unz64_file_pos position;
            size_t uncompressedSize;
            size_t compressedSize;
            size_t compressionMethod;
        };

        // Entries in this hashmap are normalized
        typedef QHash<QByteArray, ArchiveEntry> ArchiveEntries;

        ArchiveEntries archiveEntries; // Archive entries (lowercase filename -> archive pos)
    };

    QByteArray ResourceManager::Private::getFile(const Path &filename)
    {
        // Check for existence in any of the directories
        for (int i = 0; i < dataDirectories.length(); ++i) {
            QString dataDirFilename = dataDirectories[i] + QDir::separator() + filename;
#ifndef NDEBUG
            qDebug("Trying @ %s.", qPrintable(dataDirFilename));
#endif
            
            QFile f(dataDirFilename);
            if (f.open(QIODevice::ReadOnly)) {
                QByteArray data = f.readAll();
                f.close();
                return data;
            } else {
                qDebug("Unable to open %s.", qPrintable(dataDirFilename));
            }
        }

        return getArchiveFile(filename);
    }

    bool ResourceManager::Private::addDirectory(const QString &directory)
    {
        QDir d(directory);
        if (!d.exists())
            return false;

        dataDirectories.append(directory);
        return true;
    }

    QByteArray ResourceManager::Private::normalizePath(const Path &path)
    {
        Path result = path.toLower().replace('\\', '/');

        // Remove leading slashes
        if (result.startsWith('/')) {
            int removeAtStart = 0;
            for (int i = 0; i < result.length(); ++i) {
                if (result[i] == '/')
                    removeAtStart++;
                else
                    break;
            }

            result.remove(0, removeAtStart);
        }        

        // Compress slashes
        for (int i = 1; i < result.length(); ++i) {
            if (result[i] == '/' && result[i-1] == '/') {
                result.remove(i, 1);
                i--;
            }
        }

        return result;
    }

    QByteArray ResourceManager::Private::getArchiveFile(const QByteArray &filename)
    {
        ArchiveEntries::const_iterator entry = archiveEntries.find(filename);

        if (entry == archiveEntries.end()) {
            return 0;
        } else {
            int error = unzGoToFilePos64(entry->handle, &entry->position);

            if (error != UNZ_OK) {
                qWarning("Unable to seek to %s.", filename.constData());
                return 0;
            }

            error = unzOpenCurrentFile(entry->handle);

            if (error != UNZ_OK) {
                qWarning("Unable to open file %s for reading in archive.", filename.constData());
                return 0;
            }

            QByteArray result(entry->uncompressedSize, Qt::Uninitialized);

            error = unzReadCurrentFile(entry->handle, result.data(), result.size());

            unzCloseCurrentFile(entry->handle);

            if (error == UNZ_OK) {
                qWarning("Unable to read file from ZIP: %s.", filename.constData());
                return 0;
            }

            return result;
        }
    }


    bool ResourceManager::Private::addArchive(const QString &archiveFilename)
    {
        unzFile archiveHandle = unzOpen64(qPrintable(archiveFilename));

        if (!archiveHandle) {
            qWarning("Unable to open ZIP archive %s.", qPrintable(archiveFilename));
            return false;
        }

        archiveHandles.append(archiveHandle); // Ensures closing of archives

        int error;

        // Retrieve some global information about the archive
        unz_global_info archiveInfo;
        error = unzGetGlobalInfo(archiveHandle, &archiveInfo);

        if (error != UNZ_OK)  {
            qWarning("Unable to retrieve global information from zip archive %s.", qPrintable(archiveFilename));
            return false;
        }

        qDebug("Loading ZIP archive %s (%lu entries).", qPrintable(archiveFilename), archiveInfo.number_entry);

        error = unzGoToFirstFile(archiveHandle);

        if (error != UNZ_OK) {
            qWarning("Unable to seek to first file in ZIP archive %s.", qPrintable(archiveFilename));
            return false;
        }

        // Small performance optimization to resize the hashtable ahead of time
        archiveEntries.reserve(archiveEntries.size() + archiveInfo.number_entry);

        unz_file_info64 fileInfo;
        unz64_file_pos filePos; // Handle to the file in the archive
        QByteArray filename;

        do {
            error = unzGetCurrentFileInfo64(archiveHandle, &fileInfo, NULL, 0, NULL, 0, NULL, 0);

            if (error != UNZ_OK) {
                qWarning("Unable to read local file info from archive %s.", qPrintable(archiveFilename));
                continue;
            }

            filename.resize(fileInfo.size_filename);
            error = unzGetCurrentFileInfo64(archiveHandle, &fileInfo, filename.data(), fileInfo.size_filename,
                NULL, 0, NULL, 0);

            if (error != UNZ_OK) {
                qWarning("Unable to read filename from archive %s.", qPrintable(archiveFilename));
                continue;
            }

            error = unzGetFilePos64(archiveHandle, &filePos);

            if (error != UNZ_OK) {
                qWarning("Couldn't get position of %s in archive %s.", filename.constData(), 
                    qPrintable(archiveFilename));
                continue;
            }

            // An entry describes in which archive the file is stored and where.
            ArchiveEntry entry;
            entry.handle = archiveHandle;
            entry.position = filePos;
            entry.uncompressedSize = fileInfo.uncompressed_size;
            entry.compressedSize = fileInfo.compressed_size;
            entry.compressionMethod = fileInfo.compression_method;

            Path normalizedFilename = normalizePath(filename);

            archiveEntries[normalizedFilename] = entry;

            error = unzGoToNextFile(archiveHandle);
        } while (error == UNZ_OK);

        if (error != UNZ_END_OF_LIST_OF_FILE) {
            qWarning("Unable to seek to next file in ZIP archive %s.", qPrintable(archiveFilename));
            return false;
        }

        return true;
    }

    ResourceManager::Path ResourceManager::normalizePath(const ResourceManager::Path &path)
    {
        return Private::normalizePath(path);
    }   

    ResourceManager::ResourceManager() : d(new Private)
    {
    }

    ResourceManager::~ResourceManager()
    {
        delete d;
    }

    QByteArray ResourceManager::getResource(const ResourceManager::Path &path)
    {
        Path normalizedPath = normalizePath(path);

        return d->getFile(normalizedPath);
    }

    QList<ResourceManager::Path> ResourceManager::listResources(const ResourceManager::Path &path, bool recursive)
    {
        return QList<Path>();
    }

    QList<ResourceManager::Path> ResourceManager::findResources(const ResourceManager::Path &path, 
                                               const ResourceManager::Path &pattern, 
                                               bool recursive)
    {
        return QList<Path>();
    }  

    bool ResourceManager::addDirectory(const QString &path)
    {
        return d->addDirectory(path);
    }

    bool ResourceManager::addZipArchive(const QString &filename)
    {
        return d->addArchive(filename);
    }

}
