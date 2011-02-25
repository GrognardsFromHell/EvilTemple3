
#include <QtGlobal>
#include <QtXml>
#include <QDataStream>
#include <QScopedPointer>
#include <QFileInfo>
#include <QTextStream>
#include <QBuffer>
#include <QScriptEngine>
#include <QSet>
#include <QXmlStreamWriter>

#include <iostream>

#include <zipwriter.h>
#include <virtualfilesystem.h>
#include <messagefile.h>
#include <troikaarchive.h>
#include <prototypes.h>
#include <troika_materials.h>
#include <zonetemplates.h>

#include "conversion/util.h"
#include "conversion/converter.h"
#include "conversion/conversiontask.h"
#include "conversion/converthairtask.h"
#include "conversion/convertmapstask.h"
#include "conversion/convertscriptstask.h"
#include "conversion/convertparticlesystemstask.h"
#include "conversion/converttranslationstask.h"
#include "conversion/convertinterfacetask.h"
#include "conversion/convertsoundstask.h"
#include "conversion/convertmoviestask.h"
#include "conversion/convertmodelstask.h"
#include "conversion/version.h"
#include "conversion/qdirvfshandler.h"

class ZipWriterHolder : public IFileWriter
{
public:
    ZipWriterHolder(const QString &filename, ZipWriter *writer) : mWriter(writer), mClosed(false), mFilename(filename)
    {
    }

    ~ZipWriterHolder()
    {
        delete mWriter;
        if (!mClosed) {
            QFile::remove(mFilename);
        }
    }

    void addFile(const QString &filename, const QByteArray &content, bool compressable);

    void close();

private:
    ZipWriter *mWriter;
    bool mClosed;
    QString mFilename;
};

void ZipWriterHolder::addFile(const QString &filename, const QByteArray &content, bool compressable)
{
    mWriter->addFile(filename, content, compressable ? 9 : 0);
}

void ZipWriterHolder::close()
{
    mWriter->close();
    mClosed = true;

    QString newName = mFilename.left(mFilename.length() - 3) + "zip";

    QFile::remove(newName);
    QFile::rename(mFilename, newName);
}

class ConverterData : public QObject, public IConversionService {
Q_OBJECT
public:
    Converter *converter;

    QString mInputPath, mOutputPath;

    QScopedPointer<Troika::VirtualFileSystem> vfs;

    QScopedPointer<Troika::Prototypes> mPrototypes;

    QScopedPointer<Troika::ZoneTemplates> mZoneTemplates;

    QScopedPointer<Troika::Materials> mMaterials;

    QHash<uint,QString> mParticleSystemHashes;

    QHash<uint, QString> mInternalDescription;

    QHash<uint, QString> mMapIds;

    bool external;

    // Maps lower-case mesh filenames (normalized separators) to an information structure
    QSet<QString> meshReferences;

    int sectionsDone;
    int lastProgressUpdate;

    ConversionTask *mCurrentTask;

    uint mWorkDone;

    uint mTotalWork;

    ConverterData(Converter *_converter, const QString &inputPath, const QString &outputPath) :
            converter(_converter), mInputPath(inputPath), mOutputPath(outputPath), external(false),
            sectionsDone(0), lastProgressUpdate(0), mCurrentTask(NULL), mWorkDone(0), mTotalWork(0)
    {
        // Force both paths to be in the system specific format and end with a separator.
        mInputPath = QDir::toNativeSeparators(mInputPath);
        if (!mInputPath.endsWith(QDir::separator()))
            mInputPath.append(QDir::separator());

        mOutputPath = QDir::toNativeSeparators(mOutputPath);
        if (!mOutputPath.endsWith(QDir::separator()))
            mOutputPath.append(QDir::separator());
    }

    IFileWriter *createOutput(const QString &groupName)
    {
        return new ZipWriterHolder(mOutputPath + groupName + ".new", new ZipWriter(mOutputPath + groupName + ".new"));
    }

    Troika::VirtualFileSystem *virtualFileSystem()
    {
        return vfs.data();
    }

    Troika::Materials *materials()
    {
        return mMaterials.data();
    }

    QHash<uint, QString> openMessageFile(const QString &filename)
    {
        return Troika::MessageFile::parse(vfs->openFile(filename));
    }

    /**
     Services related to getting access to already parsed data.
     */
    Troika::ZoneTemplates *zoneTemplates()
    {
        return mZoneTemplates.data();
    }

    Troika::Prototypes *prototypes()
    {
        return mPrototypes.data();
    }

    /**
      Registers a mesh filename, so it will be converted by the mesh conversion process.
      */
    void addMeshReference(const QString &meshFilename)
    {
        meshReferences.insert(meshFilename);
    }

    const QSet<QString> &getMeshReferences() const
    {
        return meshReferences;
    }

    void addParticleSystem(const QString &particleSystemName)
    {
        mParticleSystemHashes[ELFHash(particleSystemName.toLatin1())] = particleSystemName;
    }

    QString getParticleSystemFromHash(uint hash)
    {
        return mParticleSystemHashes[hash];
    }

    QString getInternalName(uint id)
    {
        return mInternalDescription.value(id, QString::null);
    }

    void loadArchives() {
        // VFS searches front-to-back, since overrides reside in the data/ dir it has to come first
        QDir dataDir(mInputPath);
        if (dataDir.cd("data")) {
            qDebug("Adding override directory: %s", qPrintable(dataDir.absolutePath()));

            vfs->add(new QDirVfsHandler(dataDir));
        }

        QDir moduleDir(mInputPath);
        if (moduleDir.cd("modules/toee")) {
            qDebug("Adding override directory: %s", qPrintable(moduleDir.absolutePath()));

            vfs->add(new QDirVfsHandler(moduleDir));
        }

        // Add base archives (ToEE1.dat to ToEE4.dat)
        for (int i = 1; i <= 4; ++i) {
            QString archivePath = QString("%1ToEE%2.dat").arg(mInputPath).arg(i);

            if (QFile::exists(archivePath)) {
                qDebug("Adding archive %s", qPrintable(archivePath));
                vfs->add(new Troika::TroikaArchive(archivePath, vfs.data()));
            }
        }

        QString modulePath = QString("%1modules%2ToEE.dat").arg(mInputPath).arg(QDir::separator());
        qDebug("Adding archive %s", qPrintable(modulePath));
        vfs->add(new Troika::TroikaArchive(modulePath, vfs.data()));
    }

    bool openInput() {
        vfs.reset(new Troika::VirtualFileSystem());
        loadArchives();

        mPrototypes.reset(new Troika::Prototypes(vfs.data()));
        mZoneTemplates.reset(new Troika::ZoneTemplates(vfs.data(), mPrototypes.data()));
        mMaterials.reset(new Troika::Materials(vfs.data()));

        // Validate archives here?

        QHash<uint,QString> maplist = openMessageFile("rules/maplist.mes");
        foreach (uint mapId, maplist.keys()) {
            mMapIds[mapId] = maplist[mapId].split(',')[0].toLower();
        }

        return true;
    }

    bool createOutputDirectory() {
        QDir outputDir(mOutputPath);
        if (!outputDir.exists()) {
            qDebug("Creating output directory %s.", qPrintable(mOutputPath));
            if (QDir::isAbsolutePath(mOutputPath)) {
                return QDir::root().mkpath(mOutputPath);
            } else {
                return QDir::current().mkpath(mOutputPath);
            }
        } else {
            return true;
        }
    }

    bool convert()
    {
        if (!openInput()) {
            qFatal("Unable to open Temple of Elemental Evil data files in %s.", qPrintable(mInputPath));
        }

        if (!createOutputDirectory()) {
            qFatal("Unable to create output directory %s.", qPrintable(mOutputPath));
        }

        mInternalDescription = openMessageFile("oemes/oname.mes");

        QList<ConversionTask*> tasks;
        tasks << new ConvertHairTask(this)
              << new ConvertScriptsTask(this)
              << new ConvertParticleSystemsTask(this)
              << new ConvertTranslationsTask(this)
              << new ConvertMapsTask(this)
              << new ConvertSoundsTask(this)
              << new ConvertMoviesTask(this)
              << new ConvertInterfaceTask(this)
              << new ConvertModelsTask(this)
              ;

        // Sum up total work over all tasks
        mTotalWork = 0;
        for (int i = 0; i < tasks.size(); ++i) {
            mTotalWork += tasks[i]->cost();
        }

        mWorkDone = 0;
        for (int i = 0; i < tasks.size(); ++i) {
            mCurrentTask = tasks[i];

            emit converter->progressUpdate(mWorkDone, mTotalWork, mCurrentTask->description());

            try {
                connect(mCurrentTask, SIGNAL(progress(int,int)), SLOT(updateProgress(int,int)));
                mCurrentTask->run();
                mCurrentTask->disconnect(SIGNAL(progress(int,int)));
            } catch (ConversionTaskAborted) {
                qDebug("Cancelled conversion at user request.");
                mCurrentTask->disconnect(SIGNAL(progress(int,int)));
                mCurrentTask = NULL;
                return false;
            }

            mWorkDone += mCurrentTask->cost();
            mCurrentTask = NULL;
        }

        // Write a conversion report
        writeConversionReport();

        return true;
    }

    QString convertMapId(uint mapId) const
    {
        return mMapIds[mapId];
    }

    void writeConversionReport()
    {
        QFile output(mOutputPath + "conversion.xml");

        if (!output.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
            qWarning("Unable to write conversion report file: %s", qPrintable(output.errorString()));
            return;
        }

        QXmlStreamWriter writer(&output);

        writer.writeStartDocument();

        writer.writeStartElement("conversion");
        writer.writeAttribute("version", QString::number(DataFormatVersion));

        writer.writeEndDocument();
    }

public slots:
    void updateProgress(int done, int total)
    {
        QString message = QString("%1 (%2/%3)").arg(mCurrentTask->description()).arg(done).arg(total);

        float factor = done / (float)total;

        int overallDone = mWorkDone + factor * mCurrentTask->cost();

        emit converter->progressUpdate(overallDone, mTotalWork, message);
    }

};

Converter::Converter(const QString &inputPath, const QString &outputPath) :
        d_ptr(new ConverterData(this, inputPath, outputPath))
{
}

Converter::~Converter()
{
}

bool Converter::convert()
{
    return d_ptr->convert();
}

void Converter::setExternal(bool ext)
{
    d_ptr->external = ext;
}

void Converter::cancel()
{
    if (d_ptr->mCurrentTask)
        d_ptr->mCurrentTask->abort();
}

int Converter::version() const
{
    return DataFormatVersion;
}

#include "converter.moc"
