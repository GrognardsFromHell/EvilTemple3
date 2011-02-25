#ifndef CONVERTMAPSTASK_H
#define CONVERTMAPSTASK_H

#include "global.h"

#include "conversiontask.h"
#include "exclusions.h"

class CONVERSIONSHARED_EXPORT ConvertMapsTask : public ConversionTask
{
Q_OBJECT
public:
    ConvertMapsTask(IConversionService *service, QObject *parent = NULL);

    void run();

    uint cost() const;

    QString description() const;

private:
    Exclusions mMapExclusions;

    void convertMapObject(Troika::ZoneTemplate *zoneTemplate, IFileWriter *writer);

};

#endif // CONVERTMAPSTASK_H
