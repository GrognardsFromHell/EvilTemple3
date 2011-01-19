
#include "connectionmanager.h"

static const uint qt_meta_data_QObjectConnectionManager[] = {

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

static const char qt_meta_stringdata_QtMonoConnectionManager[] = {
    "QtMonoConnectionManager\0\0execute()\0"
};

const QMetaObject QtMonoConnectionManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QtMonoConnectionManager,
    qt_meta_data_QObjectConnectionManager, 0 }
};

const QMetaObject *QtMonoConnectionManager::metaObject() const
{
    return &staticMetaObject;
}


QtMonoConnectionManager::QtMonoConnectionManager()
    : mSlotCounter(0)
{
}

QtMonoConnectionManager::~QtMonoConnectionManager()
{
}

void *QtMonoConnectionManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QtMonoConnectionManager))
        return static_cast<void*>(const_cast<QtMonoConnectionManager*>(this));
    return QObject::qt_metacast(_clname);
}

int QtMonoConnectionManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// Convert to C++ types.
template<typename T>
T *copyValueArg(void *arg, qint64 *valueArgs, int &valueArgsCount) {
    static_assert(sizeof(T) <= sizeof(qint64), 
        "The type you are trying to convert doesn't fit into the value argument array.");
    T *valueArg = reinterpret_cast<T*>(valueArgs + valueArgsCount++);
    *valueArg = *reinterpret_cast<T*>(arg);
    return valueArg;
}

bool QtMonoConnectionManager::addSignalHandler(QObject *sender, int signalIndex, mono::MonoDelegate *receiver, Qt::ConnectionType type)
{
	QtMonoConnection newConnection;
	newConnection.delegate = receiver;
	newConnection.signalIndex = signalIndex;

	int slotIndex = ++mSlotCounter;

	Q_ASSERT(!mConnections.contains(slotIndex));

	mConnections.insert(slotIndex, newConnection);

	slotIndex += this->metaObject()->methodOffset();

	return sender->metaObject()->connect(sender, signalIndex, this, slotIndex);
}

void QtMonoConnectionManager::execute(int slotIndex, void **argv)
{
    int signalIndex = -1;

    mono::MonoDelegate *delegate = NULL;
    
	Connections::const_iterator it = mConnections.find(slotIndex);
	
	Q_ASSERT(it != mConnections.end());

	delegate = it->delegate;
	signalIndex = it->signalIndex;

    Q_ASSERT(delegate);
	Q_ASSERT(signalIndex != -1);

    const QMetaObject *meta = sender()->metaObject();
    const QMetaMethod method = meta->method(signalIndex);
	
	qDebug("EXEC, ID: %d, DELEGATE: %x, SIGNAL: %s", slotIndex, delegate, method.signature());

    QList<QByteArray> parameterTypes = method.parameterTypes();
    int argc = parameterTypes.count();

    if (argc > 10) {
        qWarning("Can only handle signals with 10 or fewer arguments.");
        // TODO: Check this upon connecting to the signal
        return;
    }

    static_assert(sizeof(double) <= sizeof(qint64), "The size of the largest floating point type must be smaller than or equal to 64-bit.");
    qint64 valueArguments[10];
    int valueArgsCount = 0;
    void *argumentPointers[10]; // The actual argument pointers
    mono::MonoString *stringArguments[10];
    int stringArgsCount = 0;

    for (int i = 0; i < argc; ++i) {
        void *arg = argv[i + 1];

        QByteArray typeName = parameterTypes.at(i);
        int argType = QMetaType::type(parameterTypes.at(i));

        /*Void = 0, Bool = 1, Int = 2, UInt = 3, LongLong = 4, ULongLong = 5,
            Double = 6, QChar = 7, QVariantMap = 8, QVariantList = 9,
            QString = 10, QStringList = 11, QByteArray = 12,
            QBitArray = 13, QDate = 14, QTime = 15, QDateTime = 16, QUrl = 17,
            QLocale = 18, QRect = 19, QRectF = 20, QSize = 21, QSizeF = 22,
            QLine = 23, QLineF = 24, QPoint = 25, QPointF = 26, QRegExp = 27,
            QVariantHash = 28, QEasingCurve = 29, LastCoreType = QEasingCurve,*/

        void *actualArg = NULL;

        switch (argType) {
        case QMetaType::Bool:
            actualArg = copyValueArg<bool>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::UChar:
            actualArg = copyValueArg<uchar>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::Char:
            actualArg = copyValueArg<char>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::UShort:
            actualArg = copyValueArg<ushort>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::Short:
            actualArg = copyValueArg<short>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::Int:
            actualArg = copyValueArg<int>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::UInt:
            actualArg = copyValueArg<uint>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::ULongLong:
            actualArg = copyValueArg<quint64>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::LongLong:
            actualArg = copyValueArg<qint64>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::Float:
            actualArg = copyValueArg<float>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::Double:
            actualArg = copyValueArg<double>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::QChar:
            actualArg = copyValueArg<QChar>(arg, valueArguments, valueArgsCount);
            break;
        case QMetaType::QVariantMap:
            // Convert to Dict<string,object>
            break;
        case QMetaType::QVariantList:
            // Convert to array of object
            break;
        case QMetaType::QString:
            {
            auto qstr = reinterpret_cast<QString*>(arg);
            mono::MonoString *str = mono::mono_string_new_utf16(mono::mono_domain_get(), qstr->utf16(), qstr->length());
            stringArguments[stringArgsCount++] = str;
            actualArg = str;
            }
            break;
        case 0:
        default:
            qWarning("QScriptEngine: Unable to handle unregistered datatype '%s' "
                "when invoking handler of signal %s::%s",
                typeName.constData(), meta->className(), method.signature());
            // TODO: Clean up and return. No signals should be triggered for which parameters are unknown.
            // TODO: Couldn't this be performed when connecting to the signal? Signals won't actually change during runtime, would they?
            return; // TODO: ENSURE that previously allocated arguments are cleaned up
        }

        argumentPointers[i] = actualArg;
    }

	mono::MonoObject *exc = NULL;
	mono::mono_runtime_delegate_invoke(reinterpret_cast<mono::MonoObject*>(delegate), argumentPointers, &exc);

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
