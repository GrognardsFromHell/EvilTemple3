
#include "engine/geometryrenderables.h"
#include "engine/materials.h"
#include "engine/drawhelper.h"
#include "engine/gamemath_streams.h"

#include <QVector2D>

namespace EvilTemple {

struct MovementIndicatorDrawStrategy : public DrawStrategy {

    MovementIndicatorDrawStrategy(const MovementIndicator *data)
        : mData(data)
    {
    }

    void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        int location;

        float extent = mData->radius() + mData->circleWidth();

        location = state.program->uniformLocation("circleColor");
        if (location != -1) {
            bindUniform(location, mData->circleColor());
        }

        location = state.program->uniformLocation("fillColor");
        if (location != -1) {
            if (mData->isPulsating()) {
                Vector4 color = mData->fillColor();
                color.setW(color.w() * (0.5f + sin(mData->pulseScaling() * Pi) * 0.5f));
                bindUniform(location, color);
            } else {
                bindUniform(location, mData->fillColor());
            }
        }

        location = state.program->uniformLocation("radius");
        if (location != -1)
            glUniform1f(location, mData->radius() / extent);

        location = state.program->uniformLocation("circleWidth");
        if (location != -1)
            glUniform1f(location, mData->circleWidth() / extent);

        SAFE_GL(glDrawArrays(GL_QUADS, 0, 4));
    }

    const MovementIndicator *mData;
};

MovementIndicator::MovementIndicator(Materials *materials)
    :
    mDrawLine(false),
    mSource(0, 0, 0, 1),
    mRadius(10),
    mCircleWidth(2),
    mLineWidth(10),
    mSecondaryLineLength(0.5),
    mLineColor(1, 1, 1, 1),
    mSecondaryLineColor(1, 0, 0, 1),
    mCircleColor(0, 0, 1, 1),
    mFillColor(0, 0, 1, 0.5f),
    mPulsating(true),
    mPulseScaling(0),
    mBoundingBoxInvalid(true),
    mBuffersInvalid(true),
    mCircleMaterial(materials->load("materials/movement_indicator_circle.xml"))
{
    setRenderCategory(Default);
}

void MovementIndicator::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{
    Q_UNUSED(overrideMaterial)

    if (!mCircleMaterial)
        return;

    if (mBuffersInvalid)
        updateBuffers();

    ModelBufferSource bufferSource(mVertices.bufferId(), 0, mTexCoords.bufferId());

    MovementIndicatorDrawStrategy drawer(this);

    DrawHelper<MovementIndicatorDrawStrategy, ModelBufferSource> drawHelper;
    drawHelper.draw(renderStates, mCircleMaterial.data(), drawer, bufferSource);
}

void MovementIndicator::elapseTime(float secondsElapsed)
{
    mPulseScaling += secondsElapsed;
    mPulseScaling -= floor(mPulseScaling);
}

void MovementIndicator::updateBuffers()
{
    float extent = mRadius + mCircleWidth;

    Vector4 vertices[4] = {
        Vector4(extent, 0, -extent, 1),
        Vector4(extent, 0, extent, 1),
        Vector4(-extent, 0, extent, 1),
        Vector4(-extent, 0, -extent, 1),
    };

    mVertices.upload(vertices, sizeof(vertices));

    float texCoords[8] = {
        1, 0,
        1, 1,
        0, 1,
        0, 0,
    };

    mTexCoords.upload(texCoords, sizeof(texCoords));

    mBuffersInvalid = false;
}

void MovementIndicator::updateBoundingBox()
{
    Vector4 extent(mRadius, mRadius, mRadius, 0);
    mBoundingBox.setMinimum(- extent);
    mBoundingBox.setMaximum(extent);
    mBoundingBoxInvalid = false;
}

LineRenderable::LineRenderable() : mColor(1, 1, 1, 1)
{
}

void LineRenderable::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{
    glDisable(GL_DEPTH_TEST);
    glColor4fv(mColor.data());

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(renderStates.projectionMatrix().data());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(renderStates.worldViewMatrix().data());

    glBegin(GL_LINES);
    foreach (const Line &line, mLines) {
        glVertex4fv(line.first.data());
        glVertex4fv(line.second.data());
    }

    glEnd();

    glEnable(GL_DEPTH_TEST);
}

void LineRenderable::addLine(const Vector4 &start, const Vector4 &end)
{
    mBoundingBox.merge(start);
    mBoundingBox.merge(end);
    mLines.append(Line(start, end));
}

const Box3d &LineRenderable::boundingBox()
{
    return mBoundingBox;
}

struct DecoratedLineDrawStrategy : public DrawStrategy {

    DecoratedLineDrawStrategy(int count)
        : mCount(count)
    {
    }

    void draw(const RenderStates &renderStates, MaterialPassState &state) const
    {
        SAFE_GL(glDrawArrays(GL_TRIANGLES, 0, mCount));
    }

    int mCount;

};

struct DecoratedLineBufferSource : public BufferSource {
    inline DecoratedLineBufferSource(GLint positionBuffer, GLint colorBuffer, GLint texCoordBuffer)
        : mPositionBuffer(positionBuffer), mColorBuffer(colorBuffer), mTexCoordBuffer(texCoordBuffer)
    {
    }

    inline GLint buffer(const MaterialPassAttributeState &attribute) const
    {
        switch (attribute.bufferType)
        {
        case -1:
            if (attribute.binding.bufferName() == "colors")
                return mColorBuffer;
        case 0:
            return mPositionBuffer;
        case 2:
            return mTexCoordBuffer;
        default:
            break;
        }

        qWarning("Unknown buffer id requested: %d.", attribute.bufferType);
        return 0;
    }

    GLint mPositionBuffer;
    GLint mColorBuffer;
    GLint mTexCoordBuffer;
};

DecoratedLineRenderable::DecoratedLineRenderable(Materials *materials)
    : mBuffersInvalid(true),
    mMaterial(materials->load("materials/decorated_line.xml")),
    mLineWidth(16),
    mVertexCount(0)
{
}

void DecoratedLineRenderable::render(RenderStates &renderStates, MaterialState *overrideMaterial)
{
    if (!mMaterial)
        return;

    if (mBuffersInvalid)
        updateBuffers();

    DecoratedLineBufferSource bufferSource(mVertices.bufferId(), mVertexColors.bufferId(), mTexCoords.bufferId());

    DecoratedLineDrawStrategy drawer(mVertexCount);

    DrawHelper<DecoratedLineDrawStrategy, DecoratedLineBufferSource> drawHelper;
    drawHelper.draw(renderStates, mMaterial.data(), drawer, bufferSource);
}

void DecoratedLineRenderable::add(const Vector4 &point, const Vector4 &color)
{
    if (mBoundingBox.isNull()) {
        mBoundingBox.setMinimum(point);
        mBoundingBox.setMaximum(point);
    } else {
        mBoundingBox.merge(point);
    }

    mPoints.append(point);
    mColors.append(color);
}

void DecoratedLineRenderable::clear()
{
    mBoundingBox.setMinimum(Vector4(0,0,0,0));
    mBoundingBox.setMaximum(Vector4(0,0,0,0));
    mPoints.clear();
    mColors.clear();
    mVertexCount = 0;
}

void DecoratedLineRenderable::updateBuffers()
{
    if (mPoints.size() < 2) {
        mBuffersInvalid = false;
        return;
    }

    int countEstimate = 4 * (mPoints.count() + 1);

    QByteArray vertexData(sizeof(Vector4) * countEstimate, Qt::Uninitialized);

    QByteArray colorData(2 * sizeof(float) * countEstimate, Qt::Uninitialized);

    QByteArray texCoordData(sizeof(Vector4) * countEstimate, Qt::Uninitialized);

    QDataStream vertexStream(&vertexData, QIODevice::WriteOnly);
    vertexStream.setByteOrder(QDataStream::LittleEndian);
    vertexStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    QDataStream colorStream(&colorData, QIODevice::WriteOnly);
    colorStream.setByteOrder(QDataStream::LittleEndian);
    colorStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    QDataStream texCoordStream(&texCoordData, QIODevice::WriteOnly);
    texCoordStream.setByteOrder(QDataStream::LittleEndian);
    texCoordStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    mVertexCount = 0;

    Vector4 planeNormal(0, 1, 0, 0);

    QVector2D texCoords[4] = {
        QVector2D(1, 1),
        QVector2D(0, 1),
        QVector2D(0, 0),
        QVector2D(1, 0)
    };

    Vector4 previous = mPoints[0];
    Vector4 oldPerpenNormal = (mPoints[1] - previous).cross(planeNormal).normalized();
    Vector4 bottomLeft = previous - oldPerpenNormal * mLineWidth * 0.5f;
    Vector4 bottomRight = previous + oldPerpenNormal * mLineWidth * 0.5f;

    /*
      Now we process every junction and build triangles.
      */
    for (int i = 1; i < mPoints.size() - 1; ++i) {
        Vector4 point = mPoints[i];

        Vector4 dir = point - previous;

        Vector4 perpenNormal = dir.cross(planeNormal).normalized();

        float d = perpenNormal.dot(oldPerpenNormal);

        /**
          This line segment is superflous. We can skip it safely if the color matches the
          color of the next segment.
          */
        if (qFuzzyCompare(d, 1) && mColors[i] == mColors[i+1])
            continue;

        bool right = d > 1; // In which direction does the line turn?

        printf("%05d %f\n", i, d);

        oldPerpenNormal = perpenNormal;

        perpenNormal *= mLineWidth * 0.5f;

        Vector4 topLeft = point - perpenNormal;
        Vector4 topRight = point + perpenNormal;

        vertexStream << topLeft << bottomLeft << bottomRight
                        << bottomRight << topRight << topLeft;
        texCoordStream << texCoords[0] << texCoords[1] << texCoords[2]
                        << texCoords[2] << texCoords[3] << texCoords[0];

        bottomLeft = topLeft;
        bottomRight = topRight;

        colorStream << mColors[i] << mColors[i] << mColors[i]
                    << mColors[i] << mColors[i] << mColors[i];

        mVertexCount += 6;

        previous = point;
    }

    // Now the last two triangles need to be built
    Vector4 point = mPoints.last();

    Vector4 perpenNormal = (point - previous).cross(planeNormal).normalized() * mLineWidth * 0.5f;

    Vector4 topLeft = point - perpenNormal;
    Vector4 topRight = point + perpenNormal;

    vertexStream << topLeft << bottomLeft << bottomRight
                    << bottomRight << topRight << topLeft;
    texCoordStream << texCoords[0] << texCoords[1] << texCoords[2]
                    << texCoords[2] << texCoords[3] << texCoords[0];

    Vector4 lastColor = mColors.last();
    colorStream << lastColor << lastColor << lastColor
                << lastColor << lastColor << lastColor;

    mVertexCount += 6;

    mVertices.upload(vertexData.constData(), vertexData.size());
    mVertexColors.upload(colorData.constData(), colorData.size());
    mTexCoords.upload(texCoordData.constData(), texCoordData.size());

    mBuffersInvalid = false;
}

const Box3d &DecoratedLineRenderable::boundingBox()
{
    return mBoundingBox;
}

}


