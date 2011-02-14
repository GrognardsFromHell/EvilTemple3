// Launcher.cpp : Defines the entry point for the console application.
//

#include "launcher_stable.h"

#include "mainwindow.h"

#include "engine/scene.h"

#include <QtMono/QtMono>
#include <QtMono/QMonoConnectionManager>
#include <QtMono/QMonoQObjectWrapper>
#include <QMessageBox>

#include "datafileengine.h"

#include <engine/mainwindow.h>
#include <engine/paths.h>
#include <engine/gameview.h>
#include <engine/datafileengine.h>

#include <QElapsedTimer>
#include <QGLWidget>

#include <iostream>

#include "framecounter.h"

using namespace monopp;

void addSystemObject(MonoImage &image, const QString &name, QObject *object);
void loadFonts();
void loadFont(const QString &filename);

void handleFatalMonoException(mono::MonoObject *ex);

typedef void (__stdcall *NoArgCallbackMethod)(mono::MonoObject **exc);

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    EvilTemple::Paths paths;
    
    EvilTemple::DataFileEngineHandler dataFileEngine;
    dataFileEngine.setDataPath(paths.installationDir().absoluteFilePath("data"));

    QStringList archives = paths.generatedDataDir().entryList(QStringList() << "*.zip", QDir::Files);
    foreach (const QString &archive, archives) {
        dataFileEngine.addArchive(paths.generatedDataDir().absoluteFilePath(archive));
    }

    loadFonts();

    QString filename = a.applicationDirPath().append(QDir::separator()).append("Bootstrap.dll");

    qDebug("Expecting Bootstrap.dll at %s", qPrintable(filename));

    MonoDomain domain("yada", MonoDomain::DotNet4);
            
    MonoAssembly assembly = domain.openAssembly(qPrintable(filename));

    if (!assembly.isValid()) {
        qWarning("Unable to load assembly Bootstrap.dll");
        return -1;
    }
    
    MonoMethodDesc startMethodDesc("Bootstrap.MainHelper:Main(string[])");
    
    MonoImage image = assembly.image();
    MonoMethod method = image.findMethod(startMethodDesc);

    Q_ASSERT(method);

    int retval = method.runAsMain(argc, argv);

    if (retval != 0) {
        qWarning("Main method returned error code: %d", retval);
        return retval;
    }

    // Initialize QtMono Services
    auto connectionManager = new QMonoConnectionManager(domain);

    auto monoQObjectWrapper = QMonoQObjectWrapper::getInstance();

	monoQObjectWrapper->registerQObjectSubtype<EvilTemple::Scene*>();

    if (!monoQObjectWrapper->initialize(connectionManager, domain)) {
        qWarning("Unable to initialize QMonoQObjectWrapper services: %s", qPrintable(monoQObjectWrapper->error()));
        return -1;
    }

    MainWindow *mainWindow = new MainWindow;
    mainWindow->makeCurrent();

    glewInit();

    EvilTemple::GameView gameView;    
    gameView.setViewport(mainWindow);

    gameView.resize(800, 600);
	
    addSystemObject(image, "GameView", &gameView);
    addSystemObject(image, "GameWindow", mainWindow);
    addSystemObject(image, "Paths", &paths);
    
    MonoMethodDesc startupMethodDesc("Bootstrap.Bootstrapper:Startup()");
	
    MonoMethodDesc drawFrameMethodDesc("Bootstrap.Bootstrapper:DrawFrame()");

	MonoMethod startupMethod = image.findMethod(startupMethodDesc);

	MonoMethod drawFrameMethod = image.findMethod(drawFrameMethodDesc);

	Q_ASSERT(startupMethod && drawFrameMethod);

	NoArgCallbackMethod startupMethodFn = (NoArgCallbackMethod)startupMethod.unmanagedThunk();
	NoArgCallbackMethod drawFrameMethodFn = (NoArgCallbackMethod)drawFrameMethod.unmanagedThunk();
    
	mono::MonoObject *ex = NULL;
	startupMethodFn(&ex);
	if (ex) {
		handleFatalMonoException(ex);
        return -1;
    }

    gameView.show();

    FrameCounter fpsCounter;
        
    while (mainWindow->isVisible()) {
        if (fpsCounter.frameDrawn()) {
            gameView.setWindowTitle(QString("EvilTemple - (FPS: %1)").arg(fpsCounter.fps()));
        }

        if (mainWindow->isActiveWindow()) {
			ex = NULL;
            drawFrameMethodFn(&ex);

			if (ex)
	            monopp::handleMonoException(ex);
        }
        
        gameView.paint();

        // Process user input events
        a.processEvents();
    }

    mainWindow->doneCurrent();

    gameView.setViewport(NULL); // This deletes the old viewport!

    return 0;
}

static void addSystemObject(MonoImage &image, const QString &name, QObject *object)
{
    mono::MonoObject *wrappedObject = QMonoQObjectWrapper::getInstance()->create(object);

    MonoMethodDesc addMethodDesc("Bootstrap.SystemObjects:Add(string,object)");
    
    MonoMethod method = image.findMethod(addMethodDesc);

	Q_ASSERT(method.isValid());

	mono::MonoString *monoName = toMonoString(name);

	void* params[2] = {monoName, wrappedObject};
	mono::MonoObject *exc = NULL;
	mono::mono_runtime_invoke(method, NULL, params, &exc);
	if (exc) {
		monopp::handleMonoException(exc);
	}
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

static void handleFatalMonoException(mono::MonoObject *ex)
{
    mono::MonoObject *toStringEx = NULL;
    mono::MonoString *exStr = mono::mono_object_to_string(ex, &toStringEx);

    QString errorMessage;

    if (toStringEx) {
        errorMessage = "Unable to convert exception to string, because an inner exception occurred.";
    } else {
        errorMessage = fromMonoString(exStr);
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Uncaught Mono Exception");
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.setText("An uncaught Exception occurred in the scripts.\nThe application will shut down now.");
    msgBox.setDetailedText(errorMessage);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.exec();
}

