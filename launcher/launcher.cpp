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

using namespace monopp;

void addSystemObject(MonoImage &image, const QString &name, mono::MonoObject *object);
void loadFonts();
void loadFont(const QString &filename);

void handleMonoException(mono::MonoObject *ex);

typedef void (__stdcall *NoArgCallbackMethod)(mono::MonoObject **exc);

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    EvilTemple::Paths paths;
    
    DataFileEngineHandler dataFileEngine(paths.installationDir().absoluteFilePath("data"));
    dataFileEngine.addArchives(paths.generatedDataPath());

    loadFonts();

    QString filename = a.applicationDirPath().append(QDir::separator()).append("Bootstrap.dll");

    qDebug("Expecting Bootstrap.dll at %s", qPrintable(filename));

    MainWindow *mainWindow = new MainWindow;
    
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
    
    mainWindow->makeCurrent();

    glewInit();

    EvilTemple::GameView gameView;    
    gameView.setViewport(mainWindow);
	
    gameView.show();

	mono::MonoObject *wrappedGameView = QMonoQObjectWrapper::getInstance()->create(&gameView);

	addSystemObject(image, "GameView", wrappedGameView);

	qDebug("Finished registering system objects.");

	
    MonoMethodDesc startupMethodDesc("Bootstrap.Bootstrapper:Startup()");
	
    MonoMethodDesc drawFrameMethodDesc("Bootstrap.Bootstrapper:DrawFrame()");

	MonoMethod startupMethod = image.findMethod(startupMethodDesc);

	MonoMethod drawFrameMethod = image.findMethod(drawFrameMethodDesc);

	Q_ASSERT(startupMethod && drawFrameMethod);

	NoArgCallbackMethod startupMethodFn = (NoArgCallbackMethod)startupMethod.unmanagedThunk();
	NoArgCallbackMethod drawFrameMethodFn = (NoArgCallbackMethod)drawFrameMethod.unmanagedThunk();
    
	mono::MonoObject *ex = NULL;
	startupMethodFn(&ex);
	if (ex)
		monopp::handleMonoException(ex);

    while (mainWindow->isVisible()) {
        if (mainWindow->isActiveWindow()) {
			ex = NULL;
            drawFrameMethodFn(&ex);

			if (ex)
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

static void addSystemObject(MonoImage &image, const QString &name, mono::MonoObject *object)
{
    MonoMethodDesc addMethodDesc("Bootstrap.SystemObjects:Add(string,object)");
    
    MonoMethod method = image.findMethod(addMethodDesc);

	Q_ASSERT(method.isValid());

	mono::MonoString *monoName = toMonoString(name);

	void* params[2] = {monoName, object};
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
