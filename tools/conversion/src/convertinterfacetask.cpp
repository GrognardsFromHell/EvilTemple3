
#include "tga.h"

#include <QImage>
#include <QBuffer>
#include <virtualfilesystem.h>

#include "conversion/convertinterfacetask.h"
#include "conversion/util.h"

ConvertInterfaceTask::ConvertInterfaceTask(IConversionService *service, QObject *parent)
    : ConversionTask(service, parent)
{

}

void ConvertInterfaceTask::run()
{
    QScopedPointer<IFileWriter> writer(service()->createOutput("interface"));

    convertImages(writer.data());
    convertTextures(writer.data());

    writer->close();
}

uint ConvertInterfaceTask::cost() const
{
    return 5;
}

QString ConvertInterfaceTask::description() const
{
    return "Converting interface";
}


static const int tileWidth = 256;
static const int tileHeight = tileWidth;

void ConvertInterfaceTask::convertImage(IFileWriter *writer, const QString &baseName, int width, int height)
{
    QImage image(width, height, QImage::Format_ARGB32);

    int xTiles = (width + 255) / 256; // Round up
    int yTiles = (height + 255) / 256; // Also round up

    int destY = height; // We're counting down here.

    for (int y = 0; y < yTiles; ++y) {

        int destX = 0;
        int lastTileHeight;
        for (int x = 0; x < xTiles; ++x) {
            QString imagePath = QString("%1_%2_%3.tga").arg(baseName).arg(x).arg(y);
            QByteArray tgaData = service()->virtualFileSystem()->openFile(imagePath);

            EvilTemple::TargaImage tgaImage(tgaData);

            if (!tgaImage.load()) {
                qWarning("Unable to load tile %s (TGA) of combined image.", qPrintable(imagePath));
                continue;
            }

            mExcludePaths.append(imagePath); // Make sure it doesnt get converted twice

            uint pixelSize;
            if (tgaImage.format() == GL_BGRA) {
                pixelSize = 4;
            } else {
                pixelSize = 3;
            }

            for (uint sy = 0; sy < tgaImage.height(); ++sy) {
                uchar *destScanline = image.scanLine(destY - tgaImage.height() + sy);
                destScanline += destX * sizeof(QRgb);
                uchar *srcScanline = (uchar*)tgaImage.data() + (tgaImage.height() - 1 - sy) * tgaImage.width() * pixelSize;

                if (pixelSize == sizeof(QRgb)) {
                    qMemCopy(destScanline, srcScanline, pixelSize * tgaImage.width());
                } else {
                    Q_ASSERT(pixelSize == 3);

                    for (uint i = 0; i < pixelSize * tgaImage.width(); i += pixelSize) {
                        *(destScanline++) = *(srcScanline++);
                        *(destScanline++) = *(srcScanline++);
                        *(destScanline++) = *(srcScanline++);
                        *(destScanline++) = 0xFF;
                    }
                }
            }

            destX += tileWidth;
            lastTileHeight = tgaImage.height();
        }

        destY -= tileHeight; // We're counting down
    }

    QByteArray pngData;
    QBuffer pngBuffer(&pngData);

    if (!image.save(&pngBuffer, "png")) {
        qWarning("Unable to save image %s.img (PNG).", qPrintable(baseName));
    } else {
        pngBuffer.close();

        writer->addFile(baseName + ".png", pngData, 0);
    }
}

void ConvertInterfaceTask::convertImages(IFileWriter *writer)
{
    QStringList images = service()->virtualFileSystem()->listAllFiles("*.img");

    foreach (const QString &imageFile, images) {

        QDataStream imgFile(service()->virtualFileSystem()->openFile(imageFile));
        imgFile.setByteOrder(QDataStream::LittleEndian);

        quint16 width, height;
        imgFile >> width >> height;

        // Strip the .img suffix for the base name
        QString baseName = imageFile.left(imageFile.length() - 4);

        convertImage(writer, baseName, width, height);
    }
}

void ConvertInterfaceTask::convertTextures(IFileWriter *writer)
{
    QStringList images = service()->virtualFileSystem()->listAllFiles("*.tga");

    uint workDone = 0;

    foreach (const QString &imagePath, images) {

        ++workDone;

        if (workDone % 10 == 0) {
            emit progress(workDone, images.size());
        }

        if (normalizePath(imagePath).startsWith("art/meshes/")) {
            continue;
        }

        if (mExcludePaths.contains(imagePath))
            continue;

        QByteArray tgaData = service()->virtualFileSystem()->openFile(imagePath);

        EvilTemple::TargaImage tgaImage(tgaData);

        if (!tgaImage.load()) {
            qWarning("Unable to load image %s (TGA).", qPrintable(imagePath));
            continue;
        }

        QString pngPath = imagePath;
        pngPath.replace(QRegExp("\\.tga$", Qt::CaseInsensitive), ".png");

        QByteArray pngData;
        QBuffer pngBuffer(&pngData);

        QImage image(tgaImage.width(), tgaImage.height(), QImage::Format_ARGB32);

        uint pixelSize;

        if (tgaImage.format() == GL_BGRA) {
            pixelSize = 4;
        } else {
            pixelSize = 3;
        }

        for (uint sy = 0; sy < tgaImage.height(); ++sy) {
            uchar *destScanline = image.scanLine(sy);
            uchar *srcScanline = (uchar*)tgaImage.data() + (tgaImage.height() - 1 - sy) * tgaImage.width() * pixelSize;

            if (pixelSize == sizeof(QRgb)) {
                qMemCopy(destScanline, srcScanline, pixelSize * tgaImage.width());
            } else {
                Q_ASSERT(pixelSize == 3);

                for (uint i = 0; i < pixelSize * tgaImage.width(); i += pixelSize) {
                    *(destScanline++) = 0xFF;
                    *(destScanline++) = *(srcScanline++);
                    *(destScanline++) = *(srcScanline++);
                    *(destScanline++) = *(srcScanline++);
                }
            }
        }

        if (!image.save(&pngBuffer, "png")) {
            qWarning("Unable to save image %s (PNG).", qPrintable(imagePath));
            continue;
        }

        pngBuffer.close();

        writer->addFile(pngPath, pngData, false);
    }
}
