
#if !defined(LAUNCHER_CONNECTIONMANAGER_H)
#define LAUNCHER_CONNECTIONMANAGER_H

struct QtMonoConnection
{
    int slotIndex;
    mono::MonoDelegate *delegate;
    // TODO: Pin the object or hold a GC handle to it
};

class QtMonoConnectionManager : public QObject {
public:
    QtMonoConnectionManager();
    ~QtMonoConnectionManager();

    static const QMetaObject staticMetaObject;
    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **argv);

    void execute(int slotIndex, void **argv);

    bool addSignalHandler(QObject *sender, int signalIndex,
        mono::MonoObject *receiver, Qt::ConnectionType type);

    bool removeSignalHandler(QObject *sender, int signalIndex,
        mono::MonoDelegate *receiver);

private:
    int mSlotCounter;
    QVector< QVector<QtMonoConnection> > mConnections;
};

#endif // LAUNCHER_CONNECTIONMANAGER_H