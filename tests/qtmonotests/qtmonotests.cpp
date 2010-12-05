
#include <QtTest>
#include <QtCore>

#include <QtMono/QMonoArgumentConverter>
#include <QtMono/QObjectMonoWrapper>

namespace mono {
    #include "mono/metadata/debug-helpers.h"
    #include "mono/metadata/assembly.h"
    #include "mono/metadata/threads.h"
}

using namespace mono;

class TestSender : public QObject
{
    Q_OBJECT
public:
    void trigger(int a, QString b, float c)
    {
        emit triggered(a, b, c);
    }
signals:
    void triggered(int,QString,float);
};

class QMonoArgumentConverterTest : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void testSomething();
    void testRoundtrip();
    void cleanupTestCase();
private:
    MonoDomain *mDomain;
    MonoAssembly *mAssembly;
    MonoImage *mImage;
};

void QMonoArgumentConverterTest::initTestCase()
{
    mDomain = mono_jit_init("qtmonotests.dll");
    
    mAssembly = mono_domain_assembly_open(mDomain, "qtmonotests.dll");

    mImage = mono_assembly_get_image(mAssembly);
    
    auto desc = mono_method_desc_new(":Main(string[])", FALSE);
    auto mainMethod = mono_method_desc_search_in_image(desc, mImage);
    QVERIFY(mainMethod != NULL);

    int argc = 1;
    char *argv[1] = {""};

    MonoObject *exception = NULL;
    mono_runtime_run_main(mainMethod, argc, argv, &exception);
}

void QMonoArgumentConverterTest::cleanupTestCase()
{
    mono_jit_cleanup(mDomain);
}

void QMonoArgumentConverterTest::testSomething()
{
    /*QMonoArgumentConverter<10> converter(mDomain);
    converter.add(QString("Yalla."));

    void** args = converter.args();
    mono::MonoString *arg1 = reinterpret_cast<mono::MonoString*>(args[0]);
    char *utf8 = mono::mono_string_to_utf8(arg1);
    qDebug("Text: %s", utf8);
    mono::mono_free(utf8);*/
}

void QMonoArgumentConverterTest::testRoundtrip()
{  
    TestSender testSender;

    MonoObject *exc = NULL;

    auto desc = mono_method_desc_new(":Test(object)", FALSE);
    auto startupMethod = mono_method_desc_search_in_image(desc, mImage);
    QVERIFY(startupMethod != NULL);

    QObjectMonoWrapperFactory wrapperFactory(mDomain);

    void *params[1] = { wrapperFactory.create(&testSender) };

    if (!params[0]) {
        QFAIL(qPrintable(wrapperFactory.error()));
    }
        
    mono_runtime_invoke(startupMethod, NULL, params, &exc);
    
    if (exc) {
        MonoString *str = mono_object_to_string(exc, &exc);
        qWarning("Exception raised while calling managed test method: %s", mono_string_to_utf8(str));
    }

    mono_method_desc_free(desc);
}

QTEST_MAIN(QMonoArgumentConverterTest);

#include "qtmonotests.moc"

