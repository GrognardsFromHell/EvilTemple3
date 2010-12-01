
#define NOMINMAX // I *hate* windows headers

#include "engine/materialstate.h"

#include <QtCore/QFile>
#include <QtOpenGL/QGLBuffer>

#include <QtCore/QHash>
#include <QtCore/QPoint>
#include <QtGui/QImage>

#include "engine/backgroundmap.h"
#include "engine/renderstates.h"

#include <limits>

inline uint qHash(const QPoint &key)
{
    return qHash((key.x() << 16) & 0xFFFF0000 | (key.y() & 0xFFFF));
}

namespace EvilTemple {

/*
    LEGACY CENTER OF MAP: 19.9999352f, -13454.0
    LEGACY ORIGIN OF MAP: -8428,-4366
    LEGACY MAP WIDTH: 66 tiles
    LEGACY MAP HEIGHT: 71 tiles
*/

class BackgroundMapData : public AlignedAllocation
{
public:
    BackgroundMapData()
        : positionBuffer(QGLBuffer::VertexBuffer),
        texCoordBuffer(QGLBuffer::VertexBuffer),
        indexBuffer(QGLBuffer::IndexBuffer),
        mapOrigin(-8428, -4366),
        color(1, 1, 1, 1),
        initialized(false)
    {
        if (!positionBuffer.create()) {
            qWarning("Unableto create position buffer.");
        }
        if (!positionBuffer.bind()) {
            qWarning("Unable to bind position buffer.");
        }

        Vector4 positions[4];
        positions[0] = Vector4(mapOrigin.x(), mapOrigin.y() -256, -1, 1);
        positions[1] = Vector4(mapOrigin.x() + 256, mapOrigin.y() -256, -1, 1);
        positions[2] = Vector4(mapOrigin.x(), mapOrigin.y(), -1, 1);
        positions[3] = Vector4(mapOrigin.x() + 256, mapOrigin.y(), -1, 1);
        positionBuffer.allocate(positions, sizeof(positions));
        positionBuffer.release();

        texCoordBuffer.create();
        texCoordBuffer.bind();

        static const float bias = 0.5f / 256.0f;

        static const float texCoords[8] = {bias, bias, 1 - bias, bias, bias, 1 - bias, 1 - bias, 1 - bias};
        texCoordBuffer.allocate(texCoords, sizeof(texCoords));
        texCoordBuffer.release();

        static const short indices[6] = {0, 1, 3, 3, 2, 0};
        if (!indexBuffer.create()) {
            qWarning("Unable to create index buffer.");
        }
        if (!indexBuffer.bind()) {
            qWarning("Unable to bind index buffer.");
        }
        indexBuffer.allocate(indices, sizeof(indices));
        indexBuffer.release();

        boundingBox.setToInfinity();
    }

    void initialize(const RenderStates &renderStates)
    {
        if (initialized)
            return;

        initialized = true;

        QFile materialFile("materials/map_material.xml");

        if (!materialFile.open(QIODevice::ReadOnly)) {
            qWarning("Unable to open map material.");
            return;
        }

        Material material;

        if (!material.loadFromData(materialFile.readAll())) {
            qWarning("Unable to parse material file.");
            return;
        }

        materialFile.close();

        if (!materialState.createFrom(material, renderStates, NULL)) {
            qWarning("Unable to create material state for background map: %s", qPrintable(materialState.error()));
            return;
        }

        colorLocations.reserve(materialState.passCount);
        for (int i = 0; i < materialState.passCount; ++i) {
            int location = materialState.passes[i].program->uniformLocation("color");
            if (location != -1)
                colorLocations.append(location);
        }
    }

    bool setMapDirectory(const QString &mapDirectory)
    {
        this->mapDirectory = mapDirectory;

        // Clear all textures
        textures.clear();
        tilesPresent.clear();

        // Can also be null in case the map should be unloaded
        if (mapDirectory.isNull())
            return true;

        QFile indexFile(mapDirectory + "index.dat");
        indexFile.open(QIODevice::ReadOnly);
        QDataStream stream(&indexFile);
        stream.setByteOrder(QDataStream::LittleEndian);

        uint count;
        stream >> count;

        quint16 x, y;
        for (uint i = 0; i < count; ++i) {
            stream >> x >> y;
            tilesPresent.insert(QPoint(x,y), true);
        }

        return true;
    }

    /**
      For every texture we have in our cache, a metric is computed, stating how far away
      the texture is from the screen viewport. This metric is used to determine whether
      the texture should be culled from the cache.

      @param xMin The first visible tile.
      @param yMin The first visible tile.
      @param xMax The last visible tile.
      @param yMax The last visible tile.
      */
    void cleanCache(int xMin, int yMin, int xMax, int yMax) {
        TextureCache::iterator it = textures.begin();

        while (it != textures.end()) {
            // Compute the manhatten distance metric for the texture
            int xDist = xMin - it.key().x();
            if (xDist < 0) {
                xDist = qMax<int>(0, it.key().x() - xMax);
            }

            int yDist = yMin - it.key().y();
            if (yDist < 0) {
                yDist = qMax<int>(0, it.key().y() - yMax);
            }

            if (qMax<int>(xDist, yDist) > 3) {
                //qDebug("Removing %d,%d from texture cache.", it.key().x(), it.key().y());
                it = textures.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool isTilePresent(const QPoint &point)
    {
        return tilesPresent.contains(point);
    }

    SharedTexture loadTexture(const QPoint &point)
    {
        TextureCache::iterator it = textures.find(point);

        if (it != textures.end()) {
            return *it;
        }

        QString filename = QString("%1%3-%2.jpg").arg(mapDirectory).arg(point.x()).arg(point.y());

        QFile backgroundTexture(filename);

        SharedTexture result(new Texture);
        result->setMagFilter(GL_NEAREST);
        result->setMinFilter(GL_NEAREST);
        result->setWrapModeS(GL_CLAMP);
        result->setWrapModeT(GL_CLAMP);
        textures.insert(point, result);

        if (backgroundTexture.open(QIODevice::ReadOnly)) {
            if (!result->loadJpeg(backgroundTexture.readAll())) {
                qWarning("Unable to load background image %s.", qPrintable(filename));
            }
        } else {
            qWarning("Unable to find background image %s.", qPrintable(filename));
        }

        return result;
    }

    QHash<QPoint, bool> tilesPresent;
    typedef QHash<QPoint, SharedTexture> TextureCache;

    Vector4 color;

    bool initialized;
    TextureCache textures;
    QString mapDirectory;
    MaterialState materialState;
    QGLBuffer positionBuffer;
    QGLBuffer texCoordBuffer;
    QGLBuffer indexBuffer;
    QVector<int> colorLocations;

    Box3d boundingBox;

    QPointF mapOrigin;
};

BackgroundMap::BackgroundMap() : d(new BackgroundMapData)
{
    setRenderCategory(Background);
}

BackgroundMap::~BackgroundMap()
{

}

const QString &BackgroundMap::directory()
{
    return d->mapDirectory;
}

bool BackgroundMap::setDirectory(const QString &directory)
{
    return d->setMapDirectory(directory);
}

void BackgroundMap::setColor(const Vector4 &color)
{
    d->color = color;
}

const Vector4 &BackgroundMap::color() const
{
    return d->color;
}

void BackgroundMap::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{
    Q_UNUSED(overrideMaterial);

    d->initialize(renderStates);

    MaterialState *material = &d->materialState;

    glDepthMask(GL_FALSE);

    if (d->color.w() < 1) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    for (int i = 0; i < material->passCount; ++i) {
        MaterialPassState &pass = material->passes[i];

        pass.program->bind();

        glActiveTexture(GL_TEXTURE0);

        // Bind uniforms
        for (int j = 0; j < pass.uniforms.size(); ++j) {
            pass.uniforms[j]->bind();
        }

        int colorLocation = d->colorLocations[i];
        if (colorLocation != -1) {
            SAFE_GL(glUniform4fv(colorLocation, 1, d->color.data()));
        }

        // Bind attributes
        for (int j = 0; j < pass.attributes.size(); ++j) {
            MaterialPassAttributeState &attribute = pass.attributes[j];

            // Bind the correct buffer
            switch (attribute.bufferType) {
            case 0:
                d->positionBuffer.bind();
                break;
            case 2:
                d->texCoordBuffer.bind();
                break;
            }

            // Assign the attribute
            glEnableVertexAttribArray(attribute.location);
            glVertexAttribPointer(attribute.location, attribute.binding.components(), attribute.binding.type(),
                                  attribute.binding.normalized(), attribute.binding.stride(), (GLvoid*)attribute.binding.offset());
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind any previously bound buffers

        d->indexBuffer.bind();

        int tileLocation = pass.program->uniformLocation("tilePos");

        // Draw the actual model
        int tilePosition[2] = {0, 0};

        /*
            To deduce which map-tiles need to be painted, we retrieve the screen-space viewport
            (without translation, meaning -> absolute coordinates), and relate it to the origin
            of the current map in the same coordinate space.
         */
        const Box2d &screenViewport = renderStates.screenViewport();

        int left = screenViewport.left() - d->mapOrigin.x();
        int top = screenViewport.top() + d->mapOrigin.y();
        int right = screenViewport.right() - d->mapOrigin.x();
        int bottom = screenViewport.bottom() + d->mapOrigin.y();

        // Negative map tile coordinates are not considered
        int firstVisibleX = qMax<int>(0, left / 256);
        int firstVisibleY = qMax<int>(0, top / 256);
        int lastVisibleX = qMax<int>(0, right / 256);
        int lastVisibleY = qMax<int>(0, bottom / 256);

        for (int x = firstVisibleX; x <= lastVisibleX; ++x) {
            for (int y = firstVisibleY; y <= lastVisibleY; ++y) {
                if (!d->isTilePresent(QPoint(x, y)))
                    continue;

                tilePosition[0] = x;
                tilePosition[1] = y;

                const SharedTexture &texture = d->loadTexture(QPoint(x,y));
                texture->bind();
                glUniform1iv(tileLocation, 2, tilePosition);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            }
        }

        d->indexBuffer.release();

        // Unbind attributes
        for (int j = 0; j < pass.attributes.size(); ++j) {
            MaterialPassAttributeState &attribute = pass.attributes[j];
            glDisableVertexAttribArray(attribute.location);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        pass.program->unbind();

        // Clean the cache
        d->cleanCache(firstVisibleX, firstVisibleY, lastVisibleX, lastVisibleY);
    }

    if (d->color.w() < 1) {
        glDisable(GL_BLEND);
    }

    glDepthMask(GL_TRUE);
}

const Box3d &BackgroundMap::boundingBox()
{
    return d->boundingBox;
}

/**
  The background map intersects everything at the farthest possible distance.
  */
IntersectionResult BackgroundMap::intersect(const Ray3d &ray) const
{
    IntersectionResult result;
    result.distance = std::numeric_limits<float>::max();
    result.intersects = true;
    return result;
}

}

