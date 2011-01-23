
#if !defined(QTMONO_CONNECTIONMANAGER_H)
#define QTMONO_CONNECTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMetaObject>
#include <QMetaMethod>

#include "monopp.h"

struct QtMonoConnection
{
    int signalIndex;
    mono::MonoObject *delegate;
    // TODO: Pin the object or hold a GC handle to it
};

class QMonoConnectionManager : public QObject {
public:
    QMonoConnectionManager(mono::MonoDomain *domain);
    ~QMonoConnectionManager();

    static const QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **argv);

    void execute(int slotIndex, void **argv);

    bool addSignalHandler(QObject *sender, int signalIndex,
        mono::MonoObject *receiver, Qt::ConnectionType type);

    bool removeSignalHandler(QObject *sender, int signalIndex,
        mono::MonoObject *receiver);

private:
    mono::MonoDomain *mDomain;

    int mSlotCounter; // Will wrap after roughly 2 billion connections
    typedef QHash<int, QtMonoConnection> Connections;
    Connections mConnections;
};

#endif // QTMONO_CONNECTIONMANAGER_H