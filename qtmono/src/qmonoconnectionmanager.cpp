
#include "qmonoargumentconverter.h"

#include "monopp.h"

#include "qmonoconnectionmanager.h"

static const uint qt_meta_data_QtObjectConnectionManager[] = {

    // content:
    1,       // revision
    0,       // classname
    0,    0, // classinfo
    1,   10, // methods
    0,    0, // properties
    0,    0, // enums/sets

    // slots: signature, parameters, type, tag, flags
    35,   34,   34,   34, 0x0a,

    0        // eod
};

static const char qt_meta_stringdata_QMonoConnectionManager[] = {
    "QMonoConnectionManager\0\0execute()\0"
};

const QMetaObject QMonoConnectionManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QMonoConnectionManager,
    qt_meta_data_QtObjectConnectionManager, 0 }
};

const QMetaObject *QMonoConnectionManager::metaObject() const
{
    return &staticMetaObject;
}

QMonoConnectionManager::QMonoConnectionManager(mono::MonoDomain *domain)
    : mSlotCounter(0), mDomain(domain)
{
}

QMonoConnectionManager::~QMonoConnectionManager()
{
}

bool QMonoConnectionManager::addSignalHandler(QObject *sender, int signalIndex, mono::MonoObject *receiver, 
    Qt::ConnectionType connectionType)
{
    QtMonoConnection newConnection;
    newConnection.delegate = receiver;
    newConnection.signalIndex = signalIndex;

    mono::MonoClass *handlerClass = mono::mono_object_get_class(receiver);
    monopp::MonoMethod handlerMethod = mono::mono_get_delegate_invoke(handlerClass);

    // Parse signature and compare with slot
    monopp::MonoMethodSignature handlerSignature = handlerMethod.signature();
    
    auto signalMethod = sender->metaObject()->method(signalIndex);
    auto signalParamCount = signalMethod.parameterTypes().count();

    /*
    It's possible to "omit" some of the signal's parameters. But the signal handler musn't have more parameters
    than the signal.
     */
    if (handlerSignature.parameterCount() > signalParamCount) {
        qWarning("Cannot connect a mono method with %d parameters to a signal with %d.", 
            handlerSignature.parameterCount(), 
            signalParamCount);
        return false;
    }

    qDebug("Signal signature: %s", signalMethod.signature());

    QVector<mono::MonoType*> handlerParameterTypes = handlerSignature.parameterTypes();
    
    for (int i = 0; i < handlerParameterTypes.size(); ++i) {
        // MonoType must conform to the signal's parameter type.
        auto signalTypeName = signalMethod.parameterTypes().at(i);
        auto signalType = QMetaType::type(signalTypeName);
        monopp::MonoType handlerType = handlerParameterTypes[i];

        if (!QMonoArgumentConverter::isAssignableFrom(handlerType, signalType)) {
            qWarning("Cannot attach receiver to signal %s, because parameter types @ position %i are not compatible: "
                "%s cannot be assigned to type %s", signalMethod.signature(), i, signalTypeName.constData(),
                handlerType.name());
            return false;
        }
    }
    
    int slotIndex = ++mSlotCounter;

    Q_ASSERT(!mConnections.contains(slotIndex));

    mConnections.insert(slotIndex, newConnection);

    slotIndex += this->metaObject()->methodOffset();

    return sender->metaObject()->connect(sender, signalIndex, this, slotIndex);
}

void *QMonoConnectionManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QMonoConnectionManager))
        return static_cast<void*>(const_cast<QMonoConnectionManager*>(this));
    return QObject::qt_metacast(_clname);
}

int QMonoConnectionManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        execute(_id, _a);
        _id -= mSlotCounter;
    }
    return _id;
}

void QMonoConnectionManager::execute(int slotIndex, void **argv)
{
    int signalIndex = -1;

    mono::MonoObject *delegate = NULL;

    Connections::const_iterator it = mConnections.find(slotIndex);

    Q_ASSERT(it != mConnections.end());

    delegate = it->delegate;
    signalIndex = it->signalIndex;

    Q_ASSERT(delegate);
    Q_ASSERT(signalIndex != -1);

    auto meta = sender()->metaObject();
    auto method = meta->method(signalIndex);

    qDebug("EXEC, ID: %d, DELEGATE: %x, SIGNAL: %s", slotIndex, delegate, method.signature());

    auto parameterTypes = method.parameterTypes();
    int argc = parameterTypes.count();

    QMonoArgumentConverter argConverter(argc, mDomain);
        
    for (int i = 0; i < argc; ++i) {
        void *arg = argv[i + 1];

        const auto &argType = parameterTypes.at(i);

        argConverter.add(arg, argType);
    }

    mono::MonoObject *exc = NULL;
    mono::mono_runtime_delegate_invoke(delegate, argConverter.args(), &exc);

    if (exc) {
        qWarning("Exception thrown when invoking Mono signal Handler.");
    }

    /*
    for (int i = 0; i < argc; ++i) {
        void *arg = argv[i + 1];
        QByteArray typeName = parameterTypes.at(i);
        int argType = QMetaType::type(parameterTypes.at(i));

        switch (argType) {
        case QMetaType::Int:


        case 0:
        default:
            qWarning("QScriptEngine: Unable to handle unregistered datatype '%s' "
                "when invoking handler of signal %s::%s",
                typeName.constData(), meta->className(), method.signature());
            // TODO: Clean up and return. No signals should be triggered for which parameters are unknown.
            // TODO: Couldn't this be performed when connecting to the signal? Signals won't actually change during runtime, would they?
            return; // TODO: ENSURE that previously allocated arguments are cleaned up

        
        }
        if (!argType) {
            
        } else if (argType == QMetaType::QVariant) {
            actual = QScriptEnginePrivate::jscValueFromVariant(exec, *reinterpret_cast<QVariant*>(arg));
        } else {
            actual = QScriptEnginePrivate::create(exec, argType, arg);
        }
        argsVector[i] = actual;
    }

    JSC::ExecState *exec = engine->currentFrame;
    QVarLengthArray<JSC::JSValue, 8> argsVector(argc);
    for (int i = 0; i < argc; ++i) {
        JSC::JSValue actual;
        void *arg = argv[i + 1];
        QByteArray typeName = parameterTypes.at(i);
        int argType = QMetaType::type(parameterTypes.at(i));
        if (!argType) {
            qWarning("QScriptEngine: Unable to handle unregistered datatype '%s' "
                "when invoking handler of signal %s::%s",
                typeName.constData(), meta->className(), method.signature());
            actual = JSC::jsUndefined();
        } else if (argType == QMetaType::QVariant) {
            actual = QScriptEnginePrivate::jscValueFromVariant(exec, *reinterpret_cast<QVariant*>(arg));
        } else {
            actual = QScriptEnginePrivate::create(exec, argType, arg);
        }
        argsVector[i] = actual;
    }
    JSC::ArgList jscArgs(argsVector.data(), argsVector.size());

    JSC::JSValue senderObject;
    if (senderWrapper && senderWrapper.inherits(&QScriptObject::info)) // ### check if it's actually a QObject wrapper
        senderObject = senderWrapper;
    else {
        QScriptEngine::QObjectWrapOptions opt = QScriptEngine::PreferExistingWrapperObject;
        senderObject = engine->newQObject(sender(), QScriptEngine::QtOwnership, opt);
    }

    JSC::JSValue thisObject;
    if (receiver && receiver.isObject())
        thisObject = receiver;
    else
        thisObject = engine->globalObject();

    JSC::CallData callData;
    JSC::CallType callType = slot.getCallData(callData);
    if (exec->hadException())
        exec->clearException(); // ### otherwise JSC asserts
    JSC::call(exec, slot, callType, callData, thisObject, jscArgs);

    if (exec->hadException()) {
        if (slot.inherits(&QtFunction::info) && !static_cast<QtFunction*>(JSC::asObject(slot))->qobject()) {
            // The function threw an error because the target QObject has been deleted.
            // The connections list is stale; remove the signal handler and ignore the exception.
            removeSignalHandler(sender(), signalIndex, receiver, slot);
            exec->clearException();
        } else {
            engine->emitSignalHandlerException();
        }
    }*/
}
