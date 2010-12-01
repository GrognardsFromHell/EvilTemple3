
#include <QDesktopServices>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QFontDatabase>

#include <common/paths.h>

#include "game.h"
#include "mainwindow.h"
#include "translations.h"

namespace EvilTemple {

    class GameData {
    public:
        ScriptEngine *scriptEngine;

        SaveGames *saveGames;
        CharacterVault *characterVault;
        Translations translations;
        const Paths *paths;

        QDir dataPath;
    };

    Game::Game(const Paths *paths, QObject *parent) :
        QObject(parent), d(new GameData)
    {
        d->paths = paths;

        // Deduce the data path
        d->dataPath = d->paths->installationDir();
        if (!d->dataPath.cd("data")) {
            qWarning("Data directory in installation directory not found.");
        }
    }

    Game::~Game()
    {
    }

    bool Game::start()
    {
        qDebug("Using data path: %s.", qPrintable(d->dataPath.absolutePath()));

        qDebug("Initializing script engine.");
        d->scriptEngine = new ScriptEngine(this);
        d->scriptEngine->setObjectName("scriptEngine");

        if (!d->translations.load("translation.dat")) {
            qFatal("Unable to load translations.");
        }
        d->scriptEngine->exposeQObject("translations", &d->translations);
        
        return true;
    }

    ScriptEngine *Game::scriptEngine() const
    {
        return d->scriptEngine;
    }

    const Translations *Game::translations() const
    {
        return &d->translations;
    }

    const Paths *Game::paths() const
    {
        return d->paths;
    }

}
