
#if !defined(__QTMONO_QMONOQOBJECTWRAPPER_H__)
#define __QTMONO_QMONOQOBJECTWRAPPER_H__

#include "monopp.h"

#include <QObject>
#include <QPointer>

class QMonoQObjectWrapper {
public:

    static QMonoQObjectWrapper *getInstance();

    bool initialize(QMonoConnectionManager *connectionManager, monopp::MonoDomain &domain);

    bool isInitialized() const;

    const QString &error() const;
    
    mono::MonoObject *create(QObject *object);

	template<typename T>
	void registerQObjectSubtype(T dummy = NULL)
	{
		QObject *ptrTest = dummy; // This shouldn't compile if T is not assignable to QObject*
		Q_UNUSED(ptrTest);
		registerQObjectSubtype(qRegisterMetaType<T>());
	}

private:
	void registerQObjectSubtype(int metaTypeId);

    QMonoQObjectWrapper();
    ~QMonoQObjectWrapper();

    class Data;
    Data *d;

    mono::MonoObject *convertVariantToObject(const QVariant &variant);
    QVariant convertObjectToVariant(mono::MonoObject *object);
    
    static bool __stdcall InvokeMember(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoArray *args, mono::MonoObject **result);
    static void __stdcall FreeHandle(QPointer<QObject> *handle);
    static bool __stdcall GetProperty(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoObject **result);
    static bool __stdcall SetProperty(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoObject *value);

    static void __stdcall ConnectToSignal(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoObject *handler);
    static void __stdcall DisconnectFromSignal(QPointer<QObject> *handle, mono::MonoString *name, mono::MonoObject *handler);

    static void __stdcall AddVariantListItem(QVariantList *list, mono::MonoObject *obj);
    static void __stdcall AddVariantMapItem(QVariantMap *map, mono::MonoString *key, mono::MonoObject *obj);

    Q_DISABLE_COPY(QMonoQObjectWrapper)
};

#endif // __QTMONO_QMONOQOBJECTWRAPPER_H__
