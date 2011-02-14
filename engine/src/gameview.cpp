
#include <GL/glew.h>

#include <QDesktopServices>
#include <QtCore/QElapsedTimer>
#include <QtDeclarative/QtDeclarative>
#include <QtGui/QResizeEvent>
#include <QtOpenGL/QGLContext>
#include <QtGui/QImage>

#include "engine/paths.h"

#include "engine/gameview.h"

#include "engine/renderstates.h"
#include "engine/modelfile.h"
#include "engine/modelinstance.h"

#include "engine/clippinggeometry.h"
#include "engine/particlesystem.h"
#include "engine/lighting.h"
#include "engine/lighting_debug.h"

#include "engine/scene.h"
#include "engine/boxrenderable.h"
#include "engine/profiler.h"
#include "engine/materials.h"
#include "engine/audioengine.h"
#include "engine/models.h"
#include "engine/binkplayer.h"
#include "engine/modelviewer.h"

#include <QPointer>

#include <gamemath.h>
using namespace GameMath;

#ifdef Q_OS_WIN32
#include <windows.h>
static bool initialized = false;
LARGE_INTEGER frequency;

static qint64 getTicks() {
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = true;
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return (now.QuadPart * 1000) / frequency.QuadPart;
}
#endif

namespace EvilTemple {

    static const int MaxFrameTimes = 100;
    
    class VideoPlayerThread : public QThread
    {
    Q_OBJECT
    public:
        VideoPlayerThread(BinkPlayer *player) : mPlayer(player) {}
    protected:
        void run();
        BinkPlayer *mPlayer;
    };

    class GameViewData : public QObject, public AlignedAllocation
    {
    Q_OBJECT
    public:
        GameViewData(GameView *view)
            : q(view),
            clippingGeometry(renderStates), dragging(false), lightDebugger(renderStates),
            materials(renderStates), models(&materials, renderStates),
            particleSystems(&models, &materials), scene(&materials), lastAudioEnginePosition(0, 0, 0, 1),
            scrollingDisabled(false),
            mPlayingVideo(false),
            mVideoPlayerThread(&mVideoPlayer),
            wasScrolling(false)
        {
            Paths paths;
            QUrl baseUrl = QUrl::fromLocalFile(paths.installationDir().absoluteFilePath("data/"));
            qDebug("Using QML base url: %s", qPrintable(baseUrl.toString()));
            uiEngine.setBaseUrl(baseUrl);

            connect(&uiEngine, SIGNAL(warnings(QList<QDeclarativeError>)), SLOT(uiWarnings(QList<QDeclarativeError>)));

            sceneTimer.invalidate();

            GlobalTextureCache::start();

            if (!particleSystems.loadTemplates()) {
                qWarning("Unable to load particle system templates: %s.", qPrintable(particleSystems.error()));
            }

            if (!audioEngine.open()) {
                qWarning("Unable to initialize audio engine.");
            }

            audioEngine.setVolume(0.25f);

            // Set some properties on the audio engine relating to positional audio
            audioEngine.setListenerOrientation(Vector4(-1, 0, -1, 0).normalized(),
                                               Vector4(0, 1, 0, 0));

            // Old: -44
            Quaternion rot1 = Quaternion::fromAxisAndAngle(1, 0, 0, deg2rad(-44.42700648682643f));
            Matrix4 rotate1matrix = Matrix4::transformation(Vector4(1,1,1,0), rot1, Vector4(0,0,0,0));

            // Old: 90-135
            Quaternion rot2 = Quaternion::fromAxisAndAngle(0, 1, 0, deg2rad(135.0000005619373f));
            Matrix4 rotate2matrix = Matrix4::transformation(Vector4(1,1,1,0), rot2, Vector4(0,0,0,0));

            Matrix4 flipZMatrix;
            flipZMatrix.setToIdentity();
            flipZMatrix(2, 2) = -1;

            Matrix4 id;
            id.setToIdentity();
            id(2,3) = -3000;

            baseViewMatrix = id * flipZMatrix * rotate1matrix * rotate2matrix;

            renderStates.setViewMatrix(baseViewMatrix);

            lightDebugger.loadMaterial();
            Light::setDebugRenderer(&lightDebugger);

            connect(&mVideoPlayer, SIGNAL(videoFrame(QImage)), SLOT(updateVideoFrame(QImage)));
            connect(&mVideoPlayerThread, SIGNAL(finished()), SLOT(stoppedPlayingVideo()));
        }

        ~GameViewData()
        {
            GlobalTextureCache::stop();
        }

        void updateListenerPosition()
        {
            Vector4 worldCenter = q->worldCenter();
            if (!(worldCenter == lastAudioEnginePosition)) {
                lastAudioEnginePosition = worldCenter;
                audioEngine.setListenerPosition(worldCenter);
            }
        }

        void centerOnWorld(float worldX, float worldY)
        {
            Matrix4 matrix = Matrix4::translation(-worldX, 0, -worldY);
            renderStates.setViewMatrix(baseViewMatrix * matrix);

            audioEngine.setListenerPosition(Vector4(worldX, 0, worldY, 1));
        }

        Vector4 worldPositionFromScreen(const QPoint &point)
        {
            float width = viewportSize.width();
            float height = viewportSize.height();

            // Construct a picking ray
            Vector4 nearPlanePoint(2 * point.x() / width - 1,
                                   2 * (height - point.y()) / height - 1,
                                   0,
                                   1);

            Vector4 farPlanePoint = nearPlanePoint;
            farPlanePoint.setZ(1);

            Matrix4 matrix = renderStates.viewProjectionMatrix().inverted();

            Vector4 pickingRayOrigin = matrix.mapPosition(nearPlanePoint);
            Vector4 pickingRayDirection = (matrix.mapPosition(farPlanePoint) - pickingRayOrigin).normalized();

            // Using the picking ray direction, project the picking ray's origin onto the x,z plane.

            Q_ASSERT(pickingRayDirection.y() < 0); // The assumption is that the picking ray goes *into* the scene

            float t = pickingRayOrigin.y() / pickingRayDirection.y();

            return pickingRayOrigin - t * pickingRayDirection;
        }

        Ray3d getPickingRay(float x, float y)
        {
            float width = viewportSize.width();
            float height = viewportSize.height();

            // Construct a picking ray
            Vector4 nearPlanePoint(2 * x / width - 1,
                                   2 * (height - y) / height - 1,
                                   0,
                                   1);

            Vector4 farPlanePoint = nearPlanePoint;
            farPlanePoint.setZ(1);

            Matrix4 matrix = renderStates.viewProjectionMatrix().inverted();

            Vector4 pickingRayOrigin = matrix.mapPosition(nearPlanePoint);
            Vector4 pickingRayDirection = (matrix.mapPosition(farPlanePoint) - pickingRayOrigin).normalized();

            return Ray3d(pickingRayOrigin, pickingRayDirection);
        }

        Vector4 getWorldCenter()
        {
            Ray3d pickingRay = getPickingRay(viewportSize.width() * 0.5f, viewportSize.height() * 0.5f);

            // Get intersection with x,z plane
            if (qFuzzyIsNull(pickingRay.direction().y())) {
                // In this case, the ray is parallel to the x,z axis
                return Vector4(0, 0, 0, 0);
            }

            float d = pickingRay.origin().y() / pickingRay.direction().y();

            return pickingRay.origin() - d * pickingRay.direction();
        }

        Renderable *pickObject(const QPoint &point)
        {
            return scene.pickRenderable(getPickingRay(point.x(), point.y()));
        }

        bool scrollingDisabled;

        QImage mCurrentVideoFrame;
        bool mPlayingVideo;
        BinkPlayer mVideoPlayer;
        VideoPlayerThread mVideoPlayerThread;
        QGraphicsScene movieEmptyScene; // We switch to this scene for playback
        QString cursorFilename;

        QDeclarativeEngine uiEngine;
        QGraphicsScene uiScene;
        QPointer<QDeclarativeItem> rootItem;

        QSize viewportSize;

        RenderStates renderStates;

        Materials materials;

        bool dragging;
        QPoint lastPoint;
        bool mouseMovedDuringDrag;
        Matrix4 baseViewMatrix; // Without translations

        QPointer<Renderable> lastMouseOverRenderable;

        LightDebugRenderer lightDebugger;

        ClippingGeometry clippingGeometry;

        AudioEngine audioEngine;

        Models models;

        ParticleSystems particleSystems;

        QElapsedTimer sceneTimer;
        Scene scene;

        Vector4 lastAudioEnginePosition;

        QList<float> lastFrameTimes;

        void playVideo(const QString &video);

        void resize(int width, int height) {
            float halfWidth = width * 0.5f;
            float halfHeight = height * 0.5f;
            glViewport(0, 0, width, height);

            viewportSize.setWidth(width);
            viewportSize.setHeight(height);

            uiScene.setSceneRect(0, 0, width, height);
            if (rootItem) {
                rootItem->setWidth(width);
                rootItem->setHeight(height);
            }

            const float zoom = 1;

            Matrix4 projectionMatrix = Matrix4::ortho(-halfWidth / zoom, halfWidth / zoom, -halfHeight / zoom, halfHeight / zoom, 1, 5000);
            renderStates.setProjectionMatrix(projectionMatrix);
        }

        void handleMouseScrolling();

    public slots:

        void uiWarnings(const QList<QDeclarativeError> &warnings)
        {
            foreach (const QDeclarativeError &error, warnings) {
                qWarning("%s", qPrintable(error.toString()));
            }
        }

        void updateVideoFrame(const QImage &image)
        {
            mCurrentVideoFrame = image;
        }

        void stoppedPlayingVideo()
        {
            qDebug("Finished playing video. Thread was stopped.");

            q->setScene(&uiScene);
            mPlayingVideo = false;
            mVideoPlayer.close();
            mCurrentVideoFrame = QImage();
        }

    private:
        GameView *q;
        bool wasScrolling;
        QElapsedTimer lastScrollEvent;
    };

    void VideoPlayerThread::run()
    {
        mPlayer->play();
    }

    void GameViewData::handleMouseScrolling()
    {
        if (dragging || scrollingDisabled || !q->underMouse() || uiScene.itemAt(lastPoint)) {
            wasScrolling = false;
            return;
        }

        float threshold = 0.05f; // 5% of the visible area should be sufficient for scrolling
        float xThreshold = threshold * q->width();
        float yThreshold = threshold * q->height();

        QPoint direction;

        if (lastPoint.x() <= xThreshold) {
            direction += (1 - lastPoint.x() / xThreshold) * 50 * QPoint(-1, 0);
        }
        if (lastPoint.y() <= yThreshold) {
            direction += (1 - lastPoint.y() / yThreshold) * 50 * QPoint(0, -1);
        }
        if (lastPoint.x() >= q->width() - xThreshold) {
            float f = 1 - (q->width() - lastPoint.x()) / xThreshold;
            direction += f * 50 * QPoint(1, 0);
        }
        if (lastPoint.y() >= q->height() - yThreshold) {
            float f = 1 - (q->height() - lastPoint.y()) / yThreshold;
            direction += f * 50 * QPoint(0, 1);
        }

        if (direction.isNull()) {
            wasScrolling = false;
            return;
        }

        QElapsedTimer now;
        now.start();

        if (wasScrolling && !direction.isNull()) {
            qint64 msecsElapsed = lastScrollEvent.msecsTo(now);

            // The timer resolution may be too coarse to scroll in very short increments, so we assume the speed
            // in relation to 25ms
            if (msecsElapsed >= 25) {
                float factor = msecsElapsed / 25.0f;

                Vector4 worldPos1 = worldPositionFromScreen(QPoint());
                Vector4 worldPos2 = worldPositionFromScreen(factor * direction);

                q->centerOnWorld(q->worldCenter() + (worldPos2 - worldPos1));

                lastScrollEvent = now;
            }
        }

        if (!wasScrolling) {
            lastScrollEvent = now;
            wasScrolling = true;
        }
    }

    GameView::GameView(QWidget *parent) :
            QGraphicsView(parent), d(new GameViewData(this)), mScrollingBorder(0)
    {
        QPixmap defaultCursor("art/interface/cursors/MainCursor.png");
        setCursor(QCursor(defaultCursor, 2, 2));

        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setOptimizationFlags(QGraphicsView::DontSavePainterState);
        setCacheMode(CacheNone);
        setFrameStyle(0);
        setScene(&d->uiScene);

        // Sets several Qt Quick related options
        setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        d->uiScene.setItemIndexMethod(QGraphicsScene::NoIndex);
        viewport()->setFocusPolicy(Qt::NoFocus);
        setFocusPolicy(Qt::StrongFocus);

        d->uiScene.setStickyFocus(true);

        connect(&d->uiEngine, SIGNAL(warnings(QList<QDeclarativeError>)), SLOT(warnings(QList<QDeclarativeError>)));

        d->uiEngine.rootContext()->setContextProperty("gameView", this);
        // d->uiEngine.rootContext()->setContextProperty("translations", game->translations());

        QObject *xchange = new QObject(this);
        xchange->setProperty("accepted", false);
        d->uiEngine.rootContext()->setContextProperty("exchangeObject", xchange);

        qmlRegisterType<EvilTemple::ModelViewer>("EvilTemple", 1, 0, "ModelViewer");

        setMouseTracking(true);
    }

    void GameView::warnings(const QList<QDeclarativeError> &warnings)
    {
        foreach (const QDeclarativeError &error, warnings) {
            qWarning("%s", qPrintable(error.toString()));
        }
    }

    GameView::~GameView()
    {
    }

    void GameView::drawBackground(QPainter *painter, const QRectF &rect)
    {
        Q_UNUSED(painter);
        Q_UNUSED(rect);

        Profiler::newFrame();

        ProfileScope<Profiler::FrameRender> profiler;

#ifdef Q_OS_WIN
        qint64 startTime = getTicks();
#endif

        while (glGetError() != GL_NO_ERROR);

        if (d->mPlayingVideo) {

            if (d->mCurrentVideoFrame.isNull())
                return;

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            QGLContext *context = const_cast<QGLContext*>(QGLContext::currentContext());
            Q_ASSERT(context);

            glEnable(GL_TEXTURE_2D);
            GLuint texture = context->bindTexture(d->mCurrentVideoFrame, GL_TEXTURE_2D, GL_RGBA, QGLContext::NoBindOption);

            glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex2i(-1, -1);
            glTexCoord2f(0, 0);
            glVertex2i(-1, 1);
            glTexCoord2f(1, 0);
            glVertex2i(1, 1);
            glTexCoord2f(1, 1);
            glVertex2i(1, -1);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            context->deleteTexture(texture);

            return;
        }

        d->handleMouseScrolling();

        // Update the audio engine state if necessary
        d->updateListenerPosition();
        
        SAFE_GL(glEnable(GL_MULTISAMPLE));

        SAFE_GL(glEnable(GL_DEPTH_TEST));
        SAFE_GL(glEnable(GL_CULL_FACE));
        SAFE_GL(glEnable(GL_BLEND));
        SAFE_GL(glDisable(GL_STENCIL_TEST));

        glClearColor(0, 0, 0, 0);
        glClearStencil(1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

        if (!d->sceneTimer.isValid()) {
            d->sceneTimer.start();
        } else {
            qint64 elapsedMs = d->sceneTimer.restart();
            double elapsedSeconds = elapsedMs / 1000.0;

            // A single frame may never advance time more than 1/10th of a second
            if (elapsedSeconds > 0.1)
                elapsedSeconds = 0.1;

            float texAnimTime = d->renderStates.textureAnimationTime() + elapsedSeconds;

            d->renderStates.setTextureAnimationTime(texAnimTime);

            d->scene.elapseTime(elapsedSeconds);
        }

        d->scene.render(d->renderStates);

        SAFE_GL(glDisable(GL_CULL_FACE));
        SAFE_GL(glDisable(GL_DEPTH_TEST));
        SAFE_GL(glDisable(GL_TEXTURE_2D));
        SAFE_GL(glDisable(GL_LIGHTING));

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(d->renderStates.projectionMatrix().data());

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(d->renderStates.viewMatrix()(0, 3), d->renderStates.viewMatrix()(1, 3), 0);

        // Draw a line-stippled version of the bounding box
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_LOOP);
        glVertex3f(mScrollBoxMinX, mScrollBoxMinY, -1);
        glVertex3f(mScrollBoxMaxX, mScrollBoxMinY, -1);
        glVertex3f(mScrollBoxMaxX, mScrollBoxMaxY, -1);
        glVertex3f(mScrollBoxMinX, mScrollBoxMaxY, -1);
        glEnd();

        glClear(GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        SAFE_GL(glDisable(GL_MULTISAMPLE));

#ifdef Q_OS_WIN
        qint64 elapsed = getTicks() - startTime;
        d->lastFrameTimes.append(elapsed);
        if (d->lastFrameTimes.size() > MaxFrameTimes)
            d->lastFrameTimes.removeAt(0);
#endif
    }

    QObject *GameView::showView(const QString &url)
    {
        QDeclarativeItem *widget = qobject_cast<QDeclarativeItem*>(addGuiItem(url));

        if (!widget)
            return NULL;

        d->rootItem = widget;
        widget->setWidth(width());
        widget->setHeight(height());

        return widget;
    }

    QObject *GameView::addGuiItem(const QString &path)
    {
        QUrl url = QUrl::fromLocalFile(path);
        qDebug("Adding gui item from %s.", qPrintable(url.toString()));

        // Create the console
        QDeclarativeComponent *component = new QDeclarativeComponent(&d->uiEngine, this);
        component->loadUrl(url);

        QEventLoop eventLoop;
        while (!component->isReady()) {
            if (component->isError()) {
                qWarning("Error creating widget: %s.", qPrintable(component->errorString()));
                return NULL;
            }
            eventLoop.processEvents();
        }

        if (component->isError()) {
            qWarning("Error creating widget: %s.", qPrintable(component->errorString()));
            return NULL;
        }

        QDeclarativeItem *widget = qobject_cast<QDeclarativeItem*>(component->create());

        d->uiScene.addItem(widget);

        return widget;
    }

    QDeclarativeEngine *GameView::uiEngine()
    {
        return &d->uiEngine;
    }

    void GameView::resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);

        if (event->size() == d->viewportSize)
            return;

        // Update projection matrix
        d->resize(event->size().width(), event->size().height());

        emit viewportChanged();
    }

    void GameView::mouseMoveEvent(QMouseEvent *evt)
    {
        if (d->mPlayingVideo)
            return;

        QGraphicsView::mouseMoveEvent(evt);

        if (d->dragging) {
            d->mouseMovedDuringDrag = true;

            if (!isScrollingDisabled()) {
                int diffX = evt->pos().x() - d->lastPoint.x();
                int diffY = evt->pos().y() - d->lastPoint.y();

                Vector4 diff(diffX, -diffY, 0, 0);

                Matrix4 transform = d->renderStates.viewMatrix().transposed();
                // Clear last column
                transform(3, 0) = 0;
                transform(3, 1) = 0;
                transform(3, 2) = 0;
                transform(3, 3) = 1;

                // This should now be the inverse of the funky view matrix.
                diff = transform * diff;

                transform.setToIdentity();
                transform(0,3) = diff.x() - diff.y();
                transform(2,3) = diff.z() - diff.y();
                // TODO: We should instead project it to the x,z plane.

                Matrix4 viewMatrix = d->renderStates.viewMatrix();
                d->renderStates.setViewMatrix(viewMatrix * transform);
                evt->accept();
            }
        } else {
            // It's possible that we've hovered a GUI element, then we have to
            // trigger the mouseLeft event
            if (d->uiScene.itemAt(evt->posF())) {
                Renderable *lastMouseOver = d->lastMouseOverRenderable;
                d->lastMouseOverRenderable = NULL;
                if (lastMouseOver)
                    lastMouseOver->mouseLeaveEvent(evt);
            } else {
                Renderable *renderable = d->pickObject(evt->pos());
                Renderable *lastMouseOver = d->lastMouseOverRenderable;

                if (renderable != lastMouseOver) {
                    if (lastMouseOver)
                        lastMouseOver->mouseLeaveEvent(evt);
                    if (renderable)
                        renderable->mouseEnterEvent(evt);
                    d->lastMouseOverRenderable = renderable;
                }

                if (renderable)
                    renderable->mouseMoveEvent(evt);
            }
        }

        d->lastPoint = evt->pos();
    }

    void GameView::mousePressEvent(QMouseEvent *evt)
    {
        if (d->mPlayingVideo) {
            d->mVideoPlayer.stop();
            return;
        }

        if (d->uiScene.itemAt(evt->posF())) {
            QGraphicsView::mousePressEvent(evt);
        } else {
            d->dragging = true;
            d->mouseMovedDuringDrag = false;
            evt->accept();
            d->lastPoint = evt->pos();
        }
    }

    void GameView::mouseReleaseEvent(QMouseEvent *evt)
    {
        if (d->mPlayingVideo) {
            d->mVideoPlayer.stop();
            return;
        }

        if (d->dragging) {
            if (!d->mouseMovedDuringDrag) {
                Renderable *renderable = d->pickObject(evt->pos());

                if (renderable) {
                    renderable->mouseReleaseEvent(evt);
                }
            }
            d->dragging = false;
        } else {
            QGraphicsView::mouseReleaseEvent(evt);
        }
    }

    void GameView::mouseDoubleClickEvent(QMouseEvent *evt)
    {
        if (d->mPlayingVideo)
            return;

        // Sadly it's not really possible to detect, whether the double click was
        // really accepted by the scene. Instead we will simply check if there
        // is a QGraphicsItem at the clicked position.
        if (d->uiScene.itemAt(evt->posF())) {
            QGraphicsView::mouseDoubleClickEvent(evt);
        } else {
            Renderable *renderable = d->pickObject(evt->pos());

            if (renderable) {
                renderable->mouseDoubleClickEvent(evt);
            }
        }
    }

    void GameView::centerOnWorld(const Vector4 &center)
    {
        d->centerOnWorld(center.x(), center.z());
    }

    Vector4 GameView::worldCenter() const
    {
        return d->getWorldCenter();
    }

    int GameView::objectsDrawn() const
    {
        return d->scene.objectsDrawn();
    }

    Scene *GameView::scene() const
    {
        return &d->scene;
    }

    ClippingGeometry *GameView::clippingGeometry() const
    {
        return &d->clippingGeometry;
    }

    Materials *GameView::materials() const
    {
        return &d->materials;
    }

    ParticleSystems* GameView::particleSystems() const
    {
        return &d->particleSystems;
    }

    const QSize &GameView::viewportSize() const
    {
        return d->viewportSize;
    }

    AudioEngine *GameView::audioEngine() const
    {
        return &d->audioEngine;
    }

    Models *GameView::models() const
    {
        return &d->models;
    }

    QUrl GameView::takeScreenshot()
    {
        QGLWidget *widget = qobject_cast<QGLWidget*>(viewport());
        Q_ASSERT(widget);

        QImage screenshot = widget->grabFrameBuffer();

        QDir currentDir = QDir::current();
        if (!currentDir.exists("screenshots"))
            currentDir.mkdir("screenshots");

        QDateTime now = QDateTime::currentDateTime();

        QString currentDateTime = "screenshot-" + now.toString("yyyy-MM-dd-hh-mm-ss");
        uint suffix = 0;
        QString currentFilename = "screenshots/" + currentDateTime + ".jpg";

        while (currentDir.exists(currentFilename)) {
            suffix++;
            currentFilename = QString("screenshots/%1-%2.jpg").arg(currentDateTime).arg(suffix);
        }

        screenshot.save(currentFilename, "jpg", 85);

        return QUrl::fromLocalFile(currentDir.absoluteFilePath(currentFilename));
    }

    QString GameView::readBase64(const QUrl &filename)
    {
        QFile file(filename.toLocalFile());

        if (!file.open(QIODevice::ReadOnly)) {
            return QByteArray();
        }

        QByteArray content = file.readAll();

        file.close();

        QString result = QString::fromLatin1(content.toBase64());

        return result;
    }

    void GameView::deleteScreenshot(const QUrl &url)
    {
        QString localFile = url.toLocalFile();

        QDir currentDir = QDir::current();
        if (!currentDir.cd("screenshots"))
            return;

        QFileInfo localFileInfo(localFile);
        currentDir.remove(localFileInfo.fileName());
    }

    void GameView::openBrowser(const QUrl &url)
    {
        qDebug("Opening browser with URL %s.", qPrintable(url.toString()));
        QDesktopServices::openUrl(url);
    }

    void GameView::setScrollingDisabled(bool disabled)
    {
        d->scrollingDisabled = disabled;
    }

    bool GameView::isScrollingDisabled() const
    {
        return d->scrollingDisabled;
    }

    bool GameView::playMovie(const QString &filename)
    {
        if (d->mPlayingVideo) {
            qDebug("Already playing video. Movie queue is still todo...");
            return false;
        }

        if (!d->mVideoPlayer.open(filename)) {
            qDebug("Unable to open video: %s: %s", qPrintable(filename), qPrintable(d->mVideoPlayer.errorString()));
            return false;
        }

        d->movieEmptyScene.setBackgroundBrush(QBrush(Qt::NoBrush));
        setScene(&d->movieEmptyScene);

        d->mPlayingVideo = true;
        d->mVideoPlayerThread.start();
        return true;
    }

    bool GameView::playUiSound(const QString &filename)
    {
        SharedSoundHandle handle = d->audioEngine.playSoundOnce(filename, SoundCategory_Interface);
        return !handle.isNull();
    }

    const QString &GameView::currentCursor() const
    {
        return d->cursorFilename;
    }

    void GameView::setCurrentCursor(const QString &filename)
    {
        d->cursorFilename = filename;

        QPixmap cursor(filename);
        setCursor(QCursor(cursor, 2, 2));
    }

    void GameView::keyPressEvent(QKeyEvent *event)
    {
        // If nothing in the scene has focus, process the event here
        if (!QGraphicsView::scene() || !QGraphicsView::scene()->focusItem()) {
            QPoint direction;

            switch (event->key()) {
            case Qt::Key_Left:
                direction += QPoint(-1, 0);
                break;
            case Qt::Key_Right:
                direction += QPoint(1, 0);
                break;
            case Qt::Key_Up:
                direction += QPoint(0, -1);
                break;
            case Qt::Key_Down:
                direction += QPoint(0, 1);
                break;
            }

            if (!direction.isNull()) {
                direction *= 50;

                Vector4 worldPos1 = d->worldPositionFromScreen(QPoint());
                Vector4 worldPos2 = d->worldPositionFromScreen(direction);

                centerOnWorld(worldCenter() + (worldPos2 - worldPos1));
                return;
            }
        }

        QGraphicsView::keyPressEvent(event);
    }

    QImage GameView::getFrameTimeHistogram()
    {
        QImage result(MaxFrameTimes, 50, QImage::Format_ARGB32);
        result.fill(0);

        int sx = result.width() - d->lastFrameTimes.size();

        float maxTime = 50;
        for (int i = 0; i < d->lastFrameTimes.size(); ++i) {
            if (d->lastFrameTimes[i] > maxTime) {
                maxTime = d->lastFrameTimes[i];
            }
        }

        for (int i = 0; i < d->lastFrameTimes.size(); ++i) {
            int height = d->lastFrameTimes[i] / maxTime * result.height();

            for (int y = result.height() - 1; y >= result.height() - height; --y) {
                result.setPixel(sx + i, y, qRgb(1, 0, 0));
            }
        }

        return result;
    }

    float GameView::getFrameTimeAverage()
    {
        if (d->lastFrameTimes.isEmpty())
            return 0;

        float result = d->lastFrameTimes.at(0);

        for (int i = 0; i < d->lastFrameTimes.size(); ++i) {
            result += d->lastFrameTimes.at(i);
        }

        return result / d->lastFrameTimes.size();
    }

    Vector4 GameView::screenFromWorld(const Vector4 &position) const
    {
        Vector4 projPos = d->renderStates.viewProjectionMatrix().mapPosition(position);

        return Vector4((projPos.x() / 2 + 0.5f) * d->viewportSize.width(),
                       (1 - (projPos.y() / 2 + 0.5f)) * d->viewportSize.height(),
                       0,
                       1);
    }

    Vector4 GameView::worldFromScreen(uint x, uint y) const
    {
        return d->worldPositionFromScreen(QPoint(x, y));
    }

    void GameView::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        /*
        Since we want total control over how often the game view gets drawn within the main loop,
        we disable processing of paint events here and call the super-class's paintEvent method manually.
        */
    }

    void GameView::paint()
    {
        QPaintEvent e(QRect(QPoint(0, 0), size()));
        QGraphicsView::paintEvent(&e);
    }

}

#include "gameview.moc"
