// Launcher.cpp : Defines the entry point for the console application.
//

#include "launcher_stable.h"

#include "mainwindow.h"

#include <QtMono/QtMono>
#include <QtMono/QMonoConnectionManager>

#include "datafileengine.h"

#include <engine/mainwindow.h>
#include <engine/paths.h>
#include <engine/gameview.h>

#include <QElapsedTimer>
#include <QGLWidget>

MainWindow *mainWindow;

using namespace monopp;

static QMonoConnectionManager *connectionManager = NULL;

inline QString fromMonoString(mono::MonoString *str) {
    mono::mono_unichar2 *rawStr = mono_string_chars(str);
    QChar *ch = (QChar*)rawStr;
    int size = mono_string_length(str);
    return QString::fromRawData(ch, size);
}

inline mono::MonoString *toMonoString(const QString &str) {
    mono::MonoDomain *domain = mono::mono_domain_get();
    return mono_string_new_utf16(domain, str.utf16(), str.length());
}

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

static bool __stdcall QObjectWrapper_InvokeMember(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoArray *args, mono::MonoObject **result)
{
    bool success = false;
    char *nameUtf8 = mono::mono_string_to_utf8(name);
	int nameUtf8Length = strlen(nameUtf8);

    QObject *obj = handle->data();

    if (!obj) {
        qWarning("Invoking %s on NULL Pointer for handle %x", nameUtf8, handle);
    } else {             
        const QMetaObject *metaObj = obj->metaObject();

        for (int i = 0; i < metaObj->methodCount(); ++i) {
            QMetaMethod method = metaObj->method(i);

			const char *startOfParamList = strchr(method.signature(), '(');
			if (!startOfParamList) {
				qWarning("Cannot find parameter list in method signature: %s", method.signature());
				continue;
			}

			int nameLength = startOfParamList - method.signature();

			if (nameUtf8Length != nameLength)
				continue;

			if (memcmp(method.signature(), nameUtf8, nameLength))
				continue;
            
            if (method.methodType() == QMetaMethod::Signal) {
				int argCount = mono_array_length(args);

				if (argCount != 1) {
					qWarning("Subscribing to signal %s failed, since not exactly one argument was given. %d arguments were given instead.", method.signature(), argCount);
					continue;
				}

				// Get the one argument to this call
				mono::MonoObject *argObj = mono_array_get(args, mono::MonoObject*, 0);

				// TODO Check that it's really a delegate
				// TODO Check that it has the right signature.
                
                qDebug("Subscribing to signal. Expecting a single delegate with the correct signature as an argument: %s.", method.signature());
				
				if (!connectionManager->addSignalHandler(obj, i, argObj, Qt::DirectConnection)) {
					continue;
				}

                success = true;
				break;
			} else if (method.methodType() == QMetaMethod::Slot) {
				method.invoke(obj);
				success = true;
				break;
			}
                
            break;
        }
    }

    mono::mono_free(nameUtf8);
    
    return success;
}

static void __stdcall QObjectWrapper_FreeHandle(QPointer<QObject> *handle)
{
    qDebug("Freeing QObjectWrapper handle %x", handle);
    delete handle;
}

typedef void (__stdcall *DrawFrameEvent)(mono::MonoDelegate*, mono::MonoObject *mainMenu, mono::MonoException**);

void loadFonts();
void loadFont(const QString &filename);
mono::MonoObject *createQObjectWrapper(MonoImage image, QObject *obj);

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

    connectionManager = new QMonoConnectionManager(domain);

    MonoAssembly assembly = domain.openAssembly(qPrintable(filename));

    if (!assembly.isValid()) {
        qWarning("Unable to load assembly Bootstrap.dll");
        return -1;
    }

    mono::mono_add_internal_call("Bootstrap.GameWindow::GetWindowTitle", GameWindow_GetWindowTitle);
    mono::mono_add_internal_call("Bootstrap.GameWindow::SetWindowTitle", GameWindow_SetWindowTitle);
    mono::mono_add_internal_call("Bootstrap.GameWindow::AddDrawFrameEventHandler", GameWindow_AddDrawFrameEventHandler);
    mono::mono_add_internal_call("Bootstrap.GameWindow::RemoveDrawFrameEventHandler", GameWindow_RemoveDrawFrameEventHandler);

    mono::mono_add_internal_call("Bootstrap.QObjectWrapper::FreeHandle", QObjectWrapper_FreeHandle);
    mono::mono_add_internal_call("Bootstrap.QObjectWrapper::InvokeMember", QObjectWrapper_InvokeMember);

    MonoMethodDesc startMethodDesc("Bootstrap.Bootstrapper:Main(string[])");
    
    MonoImage image = assembly.image();
    MonoMethod method = image.findMethod(startMethodDesc);

    if (method) {
        qWarning("Found corresponding Main(string[]) method.\n");
    }
    
    int retval = method.runAsMain(argc, argv);

    qDebug("Main() returned %d", retval);

    MonoDelegate<DrawFrameEvent> paintDelegate(delegateObj);

    DrawFrameEvent drawFrameEvent = paintDelegate.functionPointer();
    
    QGLContext *context = const_cast<QGLContext*>(QGLContext::currentContext());

    mainWindow->makeCurrent();

    glewInit();

    EvilTemple::GameView gameView;    
    gameView.setViewport(mainWindow);
    QObject *obj = gameView.addGuiItem("interface/MainMenu.qml");
        
    mono::MonoObject *mainMenuWrapper = createQObjectWrapper(image, obj);

    gameView.show();
    
    while (mainWindow->isVisible()) {
        mono::MonoException *ex;

        if (mainWindow->isActiveWindow())
            drawFrameEvent(paintDelegate, mainMenuWrapper, &ex);
        
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

mono::MonoObject *createQObjectWrapper(MonoImage image, QObject *obj)
{
    // Get the C# wrapper class
    MonoClass wrapperClass = image.findClass("Bootstrap", "QObjectWrapper");

    if (!wrapperClass)
        qFatal("Unable to load Bootstrap.QObjectWrapper");

    // Retrieve the constructor
    MonoMethod ctor = wrapperClass.findMethod(".ctor", 1);

    if (!ctor.isValid())
        qFatal("Unable to retrieve constructor for wrapper class.");
    
    QPointer<QObject> *handle = new QPointer<QObject>(obj);

    mono::MonoObject *result = mono::mono_object_new(mono::mono_domain_get(), wrapperClass);

    qDebug("Initializing wrapper class for QObject %x", handle);

    void *params[1] = {&handle};
    mono::MonoObject *exc = NULL;
    mono::mono_runtime_invoke(ctor, result, params, &exc);

    return result;
}
