
#include <QScriptEngine>
#include <QSet>

#include <messagefile.h>
#include <virtualfilesystem.h>

#include "conversion/convertscriptstask.h"
#include "conversion/exclusions.h"
#include "conversion/prototypeconverter.h"
#include "conversion/pythonconverter.h"
#include "conversion/util.h"

using namespace Troika;

ConvertScriptsTask::ConvertScriptsTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{
}

uint ConvertScriptsTask::cost() const
{
    return 5;
}

QString ConvertScriptsTask::description() const
{
    return "Converting scripts";
}

struct SoundSchemeEntry {
    enum Type {
        BackgroundMusic,
        CombatIntro,
        CombatMusic,
        Ambience
    };

    QString filename;
    Type type;
    uint volume;
    uint frequency;
    uint fromTime;
    uint toTime;
    bool scatter;
    uint volumeFrom;
    uint volumeTo;
    uint balanceFrom;
    uint balanceTo;
};

static bool parseSoundSchemeEntry(const QString &line, SoundSchemeEntry &entry)
{
    static QRegExp timePattern("/time:(\\d+)-(\\d+)");

    QStringList parts = line.split(' ');

    if (parts.isEmpty())
        return false;

    entry.filename = parts.takeFirst();
    entry.filename.replace('\\', '/');
    entry.filename.prepend("sound/");

    // Set defaults
    entry.volume = 100;
    entry.scatter = false;
    entry.fromTime = 0;
    entry.toTime = 23;
    entry.frequency = 50;
    entry.type = SoundSchemeEntry::Ambience;

    foreach (QString command, parts) {
        command = command.toLower();

        if (command == "/loop")
            entry.type = SoundSchemeEntry::BackgroundMusic;
        else if (command == "/combatintro")
            entry.type = SoundSchemeEntry::CombatIntro;
        else if (command == "/combatmusic")
            entry.type = SoundSchemeEntry::CombatMusic;
        else if (command.startsWith("/scatter"))
            entry.scatter = true;
        else if (command.startsWith("/freq:"))
            entry.frequency = command.mid(6).toUInt();
        else if (command.startsWith("/vol:")) {
            int index = command.indexOf('-');
            if (index == -1) {
                entry.volume = command.mid(5).toUInt();
            } else {
                entry.volumeFrom = command.mid(5, index - 5).toUInt();
                entry.volumeTo = command.mid(index + 1).toUInt();
            }
        } else if (command.startsWith("/bal:"))
            continue; // Ignored
        else if (timePattern.exactMatch(command)) {
            entry.fromTime = timePattern.cap(1).toUInt();
            entry.toTime = timePattern.cap(2).toUInt();
        } else {
            qWarning("Unknown sound scheme command: %s.", qPrintable(command));
            return false;
        }
    }

    return true;
}

static void convertSoundSchemes(IConversionService *service, IFileWriter *writer)
{
    QHash<uint, QString> soundSchemeIndex = service->openMessageFile("sound/schemeindex.mes");
    QHash<uint, QString> soundSchemeList = service->openMessageFile("sound/schemelist.mes");

    QVariantMap result;

    foreach (uint soundSchemeId, soundSchemeIndex.keys()) {
        if (soundSchemeId == 0)
            continue;

        QString name = soundSchemeIndex[soundSchemeId];

        QRegExp namePattern("(.*)\\s+#(\\d+)");
        if (!namePattern.exactMatch(name)) {
            qWarning("Unable to parse name of sound scheme: %s.", qPrintable(name));
            continue;
        }

        uint indexOffset = namePattern.cap(2).toUInt();

        QVariantMap scheme;
        scheme["name"] = namePattern.cap(1);
        QVariantList backgroundMusic;
        QVariantList ambientSounds;

        // Process 100 lines for the sound scheme.
        for (uint i = indexOffset; i < indexOffset + 100; ++i) {
            QString line = soundSchemeList.value(i, QString::null);

            if (line.isNull())
                continue;

            SoundSchemeEntry entry;
            if (!parseSoundSchemeEntry(line, entry)) {
                qWarning("Invalid sound scheme entry %d: %s.", i, qPrintable(line));
                continue;
            }

            QVariantMap newEntry;
            newEntry["filename"] = entry.filename;
            newEntry["volume"] = entry.volume;

            /**
              Presumably we could introduce a very generic script-guard system for ambience or
              background music sounds, instead of coding it like this.
              */
            if (entry.fromTime != 0 || entry.toTime != 23) {
                QVariantMap time;
                time["from"] = entry.fromTime;
                time["to"] = entry.toTime;
                newEntry["time"] = time;
            }

            switch (entry.type) {
            case SoundSchemeEntry::Ambience:
                if (entry.scatter) {
                    newEntry["scatter"] = true;
                }
                newEntry["frequency"] = entry.frequency;
                ambientSounds.append(newEntry);
                break;
            case SoundSchemeEntry::BackgroundMusic:
                backgroundMusic.append(newEntry);
                break;
            case SoundSchemeEntry::CombatIntro:
                scheme["combatIntro"] = newEntry;
                break;
            case SoundSchemeEntry::CombatMusic:
                scheme["combatMusic"] = newEntry;
                break;
            default:
                qWarning("Unknown type for scheme entry %d.", i);
                continue;
            }
        }

        scheme["ambientSounds"] = ambientSounds;
        scheme["backgroundMusic"] = backgroundMusic;

        result[QString("scheme-%1").arg(soundSchemeId)] = scheme;
    }

    QJson::Serializer serializer;

    writer->addFile("soundschemes.js", serializer.serialize(result));
}

static QVariantMap convertDialogScript(IConversionService *service, const QByteArray &rawScript, const QString &filename)
{
    PythonConverter converter(service);

    QVariantMap result;

    QString script = rawScript;

    QStringList lines = script.split("\n", QString::SkipEmptyParts);

    foreach (QString line, lines) {
        // Replace comment at end of line
        line.replace(QRegExp("//[^\\{\\}]*$"), "");
        line.replace(QRegExp("#[^\\{\\}]*$"), "");

        line = line.trimmed();

        QStringList parts = line.split(QRegExp("\\}\\s*\\{"), QString::KeepEmptyParts);
        for (int i = 0; i < parts.size(); ++i) {
            parts[i] = parts[i].trimmed(); // Trim spacing between parenthesis
        }

        if (parts.size() != 7)
            continue;

        parts[0] = parts[0].right(parts[0].length() - 1); // Skip opening bracket
        parts[parts.size() - 1] = parts[parts.size() - 1].left(parts[parts.size() - 1].length() - 1); // Skip closing bracket

        QString id = parts[0];
        QString text = parts[1];
        QString femaleText = parts[2];
        int intelligence = parts[3].toInt();
        QString guard = parts[4];
        QString nextId = parts[5];
        QString action = parts[6];

        // Fixes a broken line in the smithy's dialog
        if (guard.contains("game.areas[3] = 0"))
            continue;

        QVariantMap entry;
        entry["text"] = text;
        if (!femaleText.isEmpty() && femaleText != text)
            entry["femaleText"] = femaleText;
        if (intelligence != 0)
            entry["intelligence"] = intelligence;
        if (!guard.isEmpty()) {
            entry["guard"] = converter.convertDialogGuard(guard.toUtf8(), filename + ":" + id);
        }
        if (!nextId.isEmpty() && nextId != "0")
            entry["nextId"] = nextId.toUInt();
        if (!action.isEmpty())
            entry["action"] = converter.convertDialogAction(action.toUtf8(), filename + ":" + id);
        result[id] = entry;
    }

    return result;
}

static void convertLegacyMaps(IConversionService *service, IFileWriter *output)
{
    Exclusions exclusions;
    if (!exclusions.load(":/map_exclusions.txt")) {
        qWarning("Unable to load map exclusions in legacy mapping conversion.");
    }

    QHash<uint, QString> mapListMes = service->openMessageFile("rules/maplist.mes");

    QVariantMap legacyMaps;

    foreach (uint legacyId, mapListMes.keys()) {
        QString legacyIdString = QString("%1").arg(legacyId);

        // Only add it to the mapping if the map id wasn't excluded
        if (exclusions.isExcluded(legacyIdString))
            continue;

        QString entry = mapListMes[legacyId];
        QString mapId = entry.split(',')[0].toLower();
        legacyMaps[legacyIdString] = mapId;
    }

    QJson::Serializer serializer;
    output->addFile("legacy_maps.js", serializer.serialize(legacyMaps));
}

static void convertQuests(IConversionService *service, IFileWriter *output)
{
    QHash<uint, QString> questMes = service->openMessageFile("mes/gamequestlog.mes");

    QVariantMap quests;

    // At most 200 quests
    for (int i = 0; i < 200; ++i) {
        if (questMes.contains(i)) {
            QVariantMap quest;
            quest["name"] = questMes[i];
            quest["description"] = questMes[200 + i];
            quests[QString("quest-%1").arg(i)] = quest;
        }
    }

    QJson::Serializer serializer;
    output->addFile("quests.js", serializer.serialize(quests));
}

static void convertDialogScripts(IConversionService *service, IFileWriter *output)
{
    Troika::VirtualFileSystem *vfs = service->virtualFileSystem();

    QVariantMap dialogFiles;

    QJson::Serializer serializer;

    QSet<QString> filesWritten;

    QStringList allDialogFiles = vfs->listAllFiles("*.dlg");
    foreach (const QString &filename, allDialogFiles) {
        if (filesWritten.contains(filename))
            continue;

        if (!normalizePath(filename).startsWith("dlg/"))
            continue;

        QVariantMap dialogScript = convertDialogScript(service, vfs->openFile(filename), filename);

        if (dialogScript.isEmpty()) {
            qWarning("Dialog script %s is empty.", qPrintable(filename));
            continue;
        }

        filesWritten.insert(filename);

        QString shortFilename = filename.right(filename.length() - 4);
        uint id = shortFilename.left(5).toUInt();

        shortFilename.replace(".dlg", ".js");
        shortFilename.prepend("legacy/dialog/");

        dialogFiles[QString("%1").arg(id)] = shortFilename;
        output->addFile(shortFilename, serializer.serialize(dialogScript));
    }

    output->addFile("dialogs.js", serializer.serialize(dialogFiles));
}

static QByteArray convertScript(IConversionService *service, const QByteArray &pythonScript, const QString &filename)
{
    PythonConverter converter(service);

    return converter.convert(pythonScript, filename).toLocal8Bit();
}

static void convertScripts(IConversionService *service, IFileWriter *output)
{
    Troika::VirtualFileSystem *vfs = service->virtualFileSystem();

    QVariantMap scriptFiles;

    QSet<QString> filesWritten;

    QStringList allScriptFiles = vfs->listAllFiles("*.py");

    foreach (const QString &filename, allScriptFiles) {
        if (filesWritten.contains(filename))
            continue;

        if (!normalizePath(filename).startsWith("scr/spell")
            && !normalizePath(filename).startsWith("scr/py"))
            continue;

        QByteArray script = convertScript(service, vfs->openFile(filename), filename);

        if (script.isEmpty()) {
            qWarning("Sript %s is empty.", qPrintable(filename));
            continue;
        }

        filesWritten.insert(filename);

        QString shortFilename = filename.right(filename.length() - 4); // Skips scr/ at front of filename

        if (shortFilename.startsWith("py")) {
            uint id = shortFilename.left(7).right(5).toUInt();
            shortFilename.replace(".py", ".js");
            shortFilename.prepend("legacy/scripts/");
            scriptFiles[QString("%1").arg(id)] = shortFilename;
        } else if (shortFilename.startsWith("Spell")) {
            QString spellId = shortFilename.left(8).toLower();
            shortFilename.replace(".py", ".js");
            shortFilename.prepend("legacy/scripts/");
            scriptFiles[spellId] = shortFilename;
        } else {
            continue;
        }

        output->addFile(shortFilename, script);
    }

    // Process some scripts separately
    output->addFile("legacy/scripts/utilities.js",
                    convertScript(service, vfs->openFile("scr/utilities.py"), "scr/utilities.py"));
    output->addFile("legacy/scripts/random_encounter.js",
                    convertScript(service, vfs->openFile("scr/random_encounter.py"), "scr/random_encounter.py"));
    output->addFile("legacy/scripts/rumor_control.js",
                    convertScript(service, vfs->openFile("scr/rumor_control.py"), "scr/rumor_control.py"));

    QJson::Serializer serializer;
    output->addFile("scripts.js", serializer.serialize(scriptFiles));
}

static void convertWorldmapPaths(IConversionService *service, IFileWriter *writer)
{
    const QString worldmapPathsFilename = "art/interface/worldmap_ui/worldmap_ui_paths.bin";

    QByteArray worldmapPathsData = service->virtualFileSystem()->openFile(worldmapPathsFilename);

    if (worldmapPathsData.isNull()) {
        qWarning("Unable to open worldmap paths: %s.", qPrintable(worldmapPathsFilename));
        return;
    }

    QDataStream stream(worldmapPathsData);
    stream.setByteOrder(QDataStream::LittleEndian);

    int count;
    stream >> count;
    Q_ASSERT(count >= 0);

    QVariantList paths;

    for (int i = 0; i < count; ++i) {
        QVariantMap path;

        uint fromX, fromY;
        uint toX, toY;
        stream >> fromX >> fromY >> toX >> toY;

        QVariantList from;
        from << fromX << fromY;
        path["from"] = from;

        QVariantList to;
        to << toX << toY;
        path["to"] = to;

        int elementCount;
        stream >> elementCount;
        Q_ASSERT(elementCount >= 0);

        QVariantList pathElements;

        for (int j = 0; j < elementCount; ++j) {
            uchar element;
            stream >> element;

            switch (element) {
            case 5:
                pathElements << "up";
                break;
            case 6:
                pathElements << "down";
                break;
            case 7:
                pathElements << "left";
                break;
            case 8:
                pathElements << "right";
                break;
            case 9:
                pathElements << "upleft";
                break;
            case 10:
                pathElements << "upright";
                break;
            case 11:
                pathElements << "downleft";
                break;
            case 12:
                pathElements << "downright";
                break;
            case 13:
                pathElements << "stay";
                break;
            default:
                qWarning("Unknown opcode for element %d in worldmap path %d: %02x", j, i, element);
                break;
            }
        }

        // ToEE aligns the paths on 4 byte boundaries
        if (elementCount % 4 != 0) {
            stream.skipRawData(4 - (elementCount % 4));
        }

        path["path"] = pathElements;

        paths << QVariant(path);

        Q_ASSERT(stream.status() == QDataStream::Ok);
    }

    QJson::Serializer serializer;
    writer->addFile("worldmapPaths.js", serializer.serialize(paths));
}

static QScriptValue readMesFile(QScriptContext *ctx, QScriptEngine *engine, Troika::VirtualFileSystem *vfs)
{
    if (ctx->argumentCount() != 1) {
        return ctx->throwError("readMesFile takes one string argument.");
    }

    QString filename = ctx->argument(0).toString();

    QHash<uint,QString> mapping = MessageFile::parse(vfs->openFile(filename));

    QScriptValue result = engine->newObject();

    foreach (uint key, mapping.keys()) {
        result.setProperty(QString("%1").arg(key), QScriptValue(mapping[key]));
    }

    return result;
}

static QScriptValue addFile(QScriptContext *ctx, QScriptEngine *engine, IFileWriter *writer)
{
    if (ctx->argumentCount() != 3) {
        return ctx->throwError("addFile takes three arguments: filename, content, compression.");
    }

    QString filename = ctx->argument(0).toString();
    QString content = ctx->argument(1).toString();
    int compression = ctx->argument(2).toInt32();

    writer->addFile(filename, content.toUtf8(), compression > 0);

    return QScriptValue(true);
}

static QScriptValue readTabFile(QScriptContext *ctx, QScriptEngine *engine, Troika::VirtualFileSystem *vfs)
{
    if (ctx->argumentCount() != 1) {
        return ctx->throwError("readTabFile takes one string argument.");
    }

    QString filename = ctx->argument(0).toString();

    QByteArray content = vfs->openFile(filename);
    QList<QByteArray> lines = content.split('\n');
    QList< QList<QByteArray> > tabFileContent;

    for (int i = 0; i < lines.length(); ++i) {
        QByteArray line = lines[i];
        if (line.endsWith('\r')) {
            line = line.left(line.length() - 1);
        }

        if (line.isEmpty())
            continue;

        tabFileContent.append(line.split('\t'));
    }

    QScriptValue result = engine->newArray(tabFileContent.length());

    for (int i = 0; i < tabFileContent.length(); ++i) {
        QList<QByteArray> line = tabFileContent[i];
        QScriptValue record = engine->newArray(line.length());
        for (int j = 0; j < line.length(); ++j) {
            record.setProperty(j, QScriptValue(QString(line[j])));
        }
        result.setProperty(i, record);
    }

    return result;
}

void ConvertScriptsTask::convertPrototypes(IFileWriter *writer, QScriptEngine *engine)
{
    PrototypeConverter converter(service()->virtualFileSystem());

    QVariantMap result = converter.convertPrototypes(service()->prototypes());

    QScriptValue postprocess = engine->globalObject().property("postprocess");

    QScriptValueList args;
    args.append(engine->toScriptValue<QVariantMap>(result));

    postprocess.call(QScriptValue(), args);

    if (engine->hasUncaughtException()) {
        qWarning("JS Error: %s", qPrintable(engine->uncaughtException().toString()));
        return;
    }
}

static QHash<uint,QString> voiceLineMapping;
static QHash<uint,QString> voiceAreaMapping;
static QHash<uint,QString> voiceSceneryMapping;

static QVariant getVoiceLine(IConversionService *service, const QString &basePath, uint id, const QString &text) {
    QString filename = QString("%1%2.mp3").arg(basePath).arg(id);

    QVariantMap result;

    result["text"] = text;

    if (service->virtualFileSystem()->exists(filename)) {
        result["sound"] = QString::number(id) + ".mp3";
    }

    return result;
}

static void convertVoices(IConversionService *service, IFileWriter *writer)
{
    voiceLineMapping[0] = "acknowledge";
    voiceLineMapping[1] = "deny";
    voiceLineMapping[2] = "encumbered";
    voiceLineMapping[3] = "death";
    voiceLineMapping[4] = "criticalHp";
    voiceLineMapping[5] = "seesDeath";
    voiceLineMapping[6] = "combat";
    voiceLineMapping[7] = "criticalHitByParty";
    voiceLineMapping[8] = "criticalHitOnParty";
    voiceLineMapping[9] = "criticalMissByParty";
    voiceLineMapping[10] = "friendlyFire";
    voiceLineMapping[11] = "valuableLoot";
    // voiceLineMapping[12] = "areas"; // Receives special handling
    voiceLineMapping[13] = "bossMonster";
    // voiceLineMapping[14] = "taggedScenery"; // Special handling
    voiceLineMapping[15] = "bored";

    // Area key translations
    voiceAreaMapping[1200] = "default";
    voiceAreaMapping[1201] = "hommlet";
    voiceAreaMapping[1202] = "moathouse";
    voiceAreaMapping[1203] = "nulb";
    voiceAreaMapping[1204] = "temple";
    voiceAreaMapping[1205] = "emridy_meadows";
    voiceAreaMapping[1206] = "imeryds_run";
    voiceAreaMapping[1207] = "temple_secret_exit";
    voiceAreaMapping[1208] = "moathouse_secret_exit";
    voiceAreaMapping[1209] = "ogre_cave";
    voiceAreaMapping[1210] = "deklo_grove";
    voiceAreaMapping[1211] = "fire_node";
    voiceAreaMapping[1212] = "water_node";
    voiceAreaMapping[1213] = "air_node";
    voiceAreaMapping[1214] = "earth_node";

    // Secenery key translations
    voiceSceneryMapping[1400] = "generic";
    voiceSceneryMapping[1401] = "bronze_doors";
    voiceSceneryMapping[1402] = "minotaur_statue";
    voiceSceneryMapping[1403] = "hall_of_statues";
    voiceSceneryMapping[1404] = "hall_of_verdigris";
    voiceSceneryMapping[1405] = "lubash_stairs";
    voiceSceneryMapping[1406] = "thrommels_room";
    voiceSceneryMapping[1407] = "earth_temple";
    voiceSceneryMapping[1408] = "fire_temple";
    voiceSceneryMapping[1409] = "water_temple";
    voiceSceneryMapping[1410] = "air_temple";
    voiceSceneryMapping[1411] = "throne";
    voiceSceneryMapping[1412] = "greater_temple";
    voiceSceneryMapping[1413] = "zuggtmoys_level";

    QVariantList voicesResult;

    // Open rules/pcvoice.mes and read all registered voice types
    QHash<uint,QString> voices = service->openMessageFile("rules/pcvoice.mes");
    QHash<uint,QString> voiceNames = service->openMessageFile("mes/pcvoice.mes");

    foreach (uint voiceId, voices.keys()) {
        QHash<uint,QString> messages = service->openMessageFile(QString("mes/pcvoice/%1").arg(voices[voiceId]));

        QVariantMap voiceObj;

        QString voiceIdStr = voices[voiceId];
        voiceIdStr.replace(".mes", "");

        voiceObj["id"] = voiceIdStr;
        voiceObj["name"] = voiceNames[voiceId];
        voiceObj["gender"] = voices[voiceId].toLower().startsWith('f') ? "female" : "male";

        QString basePath = QString("sound/speech/pcvoice/%1/").arg(voiceId, 2, 10, QChar('0'));
        voiceObj["basePath"] = basePath;

        QVariantMap genericTopics;
        foreach (uint lineKey, voiceLineMapping.keys()) {

            // Gather all lines
            QVariantList lines;
            for (uint i = lineKey * 100; i < (lineKey + 1) * 100; ++i) {
                if (messages.contains(i)) {
                    lines << getVoiceLine(service, basePath, i, messages[i]);
                }
            }

            genericTopics[voiceLineMapping[lineKey]] = lines;
        }

        voiceObj["generic"] = genericTopics;

        // Reactions to areas
        QVariantMap areas;
        foreach (uint areaKey, voiceAreaMapping.keys()) {
            if (!messages.contains(areaKey))
                continue;
            areas[voiceAreaMapping[areaKey]] = getVoiceLine(service, basePath, areaKey, messages[areaKey]);
        }
        voiceObj["areas"] = areas;

        QVariantMap sceneryComments;
        foreach (uint sceneryKey, voiceSceneryMapping.keys()) {
            if (!messages.contains(sceneryKey))
                continue;
            sceneryComments[voiceSceneryMapping[sceneryKey]] = getVoiceLine(service, basePath,
                                                                            sceneryKey, messages[sceneryKey]);
        }
        voiceObj["scenery"] = sceneryComments;

        voicesResult << voiceObj;
    }

    QJson::Serializer serializer;
    writer->addFile("voices.js", serializer.serialize(voicesResult));
}

void ConvertScriptsTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("scripts"));

    QScriptEngine engine;

    VirtualFileSystem *vfs = service()->virtualFileSystem();

    QScriptValue readMesFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)readMesFile, vfs);
    engine.globalObject().setProperty("readMes", readMesFn);

    QScriptValue readTabFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)readTabFile, vfs);
    engine.globalObject().setProperty("readTab", readTabFn);

    QScriptValue addFileFn = engine.newFunction((QScriptEngine::FunctionWithArgSignature)addFile, writer.data());
    engine.globalObject().setProperty("addFile", addFileFn);

    QFile scriptFile(":/converter.js");

    if (!scriptFile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        qFatal("Unable to open converter script: scripts/converter.js");
    }

    QString scriptCode = scriptFile.readAll();

    engine.evaluate(scriptCode, "scripts/converter.js");

    convertPrototypes(writer.data(), &engine);

    convertDialogScripts(service(), writer.data());

    convertQuests(service(), writer.data());

    convertScripts(service(), writer.data());

    convertLegacyMaps(service(), writer.data());

    convertSoundSchemes(service(), writer.data());

    convertWorldmapPaths(service(), writer.data());

    convertVoices(service(), writer.data());

    writer->close();
}
