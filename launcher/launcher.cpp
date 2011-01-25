// Launcher.cpp : Defines the entry point for the console application.
//

#include "launcher_stable.h"

#include "mainwindow.h"

#include <QtMono/QtMono>
#include <QtMono/QMonoConnectionManager>
#include <QtMono/QMonoQObjectWrapper>

#include "datafileengine.h"

#include <engine/mainwindow.h>
#include <engine/paths.h>
#include <engine/gameview.h>

#include <QElapsedTimer>
#include <QGLWidget>

MainWindow *mainWindow;

using namespace monopp;

static void GameWindow_SetWindowTitle(mono::MonoString *str) {
    mainWindow->setWindowTitle(fromMonoString(str));
}

static mono::MonoString *GameWindow_GetWindowTitle() {
    return toMonoString(mainWindow->windowTitle());
}

static quint32 gcHandle = 0;

static mono::MonoDelegate *delegateObj = NULL;

static void GameWindow_AddDrawFrameEventHandler(mono::MonoDelegate *delegate) {
    delegateObj = delegate;
    gcHandle = mono::mono_gchandle_new((mono::MonoObject*)delegate, TRUE);
}

static void GameWindow_RemoveDrawFrameEventHandler(mono::MonoDelegate *delegate) {
    qDebug("Removed an event handler to the main window draw routine.");
}

typedef void (__stdcall *DrawFrameEvent)(mono::MonoDelegate*, mono::MonoObject *mainMenu, mono::MonoObject **);

void loadFonts();
void loadFont(const QString &filename);

void handleMonoException(mono::MonoObject *ex);

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    EvilTemple::Paths paths;
    
    DataFileEngineHandler dataFileEngine(paths.installationDir().absoluteFilePath("data"));
    dataFileEngine.addArchives(paths.generatedDataPath());

    loadFonts();

    QString filename = a.applicationDirPath().append(QDir::separator()).append("Bootstrap.dll");

    qDebug("Expecting Bootstrap.dll at %s", qPrintable(filename));

    mainWindow = new MainWindow;
    
    MonoDomain domain("yada", MonoDomain::DotNet4);
            
    MonoAssembly assembly = domain.openAssembly(qPrintable(filename));

    if (!assembly.isValid()) {
        qWarning("Unable to load assembly Bootstrap.dll");
        return -1;
    }

    mono::mono_add_internal_call("Bootstrap.GameWindow::GetWindowTitle", GameWindow_GetWindowTitle);
    mono::mono_add_internal_call("Bootstrap.GameWindow::SetWindowTitle", GameWindow_SetWindowTitle);
    mono::mono_add_internal_call("Bootstrap.GameWindow::AddDrawFrameEventHandler", GameWindow_AddDrawFrameEventHandler);
    mono::mono_add_internal_call("Bootstrap.GameWindow::RemoveDrawFrameEventHandler", GameWindow_RemoveDrawFrameEventHandler);

    MonoMethodDesc startMethodDesc("Bootstrap.Bootstrapper:Main(string[])");
    
    MonoImage image = assembly.image();
    MonoMethod method = image.findMethod(startMethodDesc);

    if (method) {
        qWarning("Found corresponding Main(string[]) method.\n");
    }
    
    int retval = method.runAsMain(argc, argv);

    // Initialize QtMono Services
    auto connectionManager = new QMonoConnectionManager(domain);

    auto monoQObjectWrapper = QMonoQObjectWrapper::getInstance();

    if (!monoQObjectWrapper->initialize(connectionManager, domain)) {
        qWarning("Unable to initialize QMonoQObjectWrapper services: %s", qPrintable(monoQObjectWrapper->error()));
        return -1;
    }

    qDebug("Main() returned %d", retval);

    MonoDelegate<DrawFrameEvent> paintDelegate(delegateObj);

    DrawFrameEvent drawFrameEvent = paintDelegate.functionPointer();
    
    QGLContext *context = const_cast<QGLContext*>(QGLContext::currentContext());

    mainWindow->makeCurrent();

    glewInit();

    EvilTemple::GameView gameView;    
    gameView.setViewport(mainWindow);
    QObject *obj = gameView.addGuiItem("interface/MainMenu.qml");
        
    mono::MonoObject *mainMenuWrapper = QMonoQObjectWrapper::getInstance()->create(obj);

    gameView.show();
    
    while (mainWindow->isVisible()) {
        mono::MonoObject *ex = NULL;

        if (mainWindow->isActiveWindow())
            drawFrameEvent(paintDelegate, mainMenuWrapper, &ex);

        if (ex) {
            monopp::handleMonoException(ex);
        }
        
        QPainter painter(mainWindow);
        gameView.render(&painter);
        painter.end();

        // Swap front/back buffer
        mainWindow->swapBuffers();

        // Process user input events
        a.processEvents();
    }

    mainWindow->doneCurrent();

    return 0;
}

static void loadFonts()
{
    loadFont("fonts/5inq_-_Handserif.ttf");
    loadFont("fonts/ArtNoveauDecadente.ttf");
    loadFont("fonts/Fontin-Bold.ttf");
    loadFont("fonts/Fontin-Italic.ttf");
    loadFont("fonts/Fontin-Regular.ttf");
    loadFont("fonts/Fontin-SmallCaps.ttf");
}

static void loadFont(const QString &filename)
{
    int result = QFontDatabase::addApplicationFont(filename);
    if (result == -1) {
        qWarning("Unable to load font %s.", qPrintable(filename));
    }
}
