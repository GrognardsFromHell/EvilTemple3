
#if !defined(LAUNCHER_CONNECTIONMANAGER_H)
#define LAUNCHER_CONNECTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMetaObject>
#include <QMetaMethod>

namespace mono {
#include <mono/metadata/object.h>
#include <mono/jit/jit.h>
}

struct QtMonoConnection
{
    int signalIndex;
    mono::MonoObject *delegate;
    // TODO: Pin the object or hold a GC handle to it
};

class QtMonoConnectionManager : public QObject {
public:
    QtMonoConnectionManager(mono::MonoDomain *domain);
    ~QtMonoConnectionManager();

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

#endif // LAUNCHER_CONNECTIONMANAGER_H