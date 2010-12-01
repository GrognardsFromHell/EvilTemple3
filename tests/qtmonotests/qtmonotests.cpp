
#include <QtTest>
#include <QtCore>

#include <QtMono/QMonoArgumentConverter>

class QMonoArgumentConverterTest : public QObject
{
Q_OBJECT
private slots:
    void testSomething();
};

void QMonoArgumentConverterTest::testSomething()
{
    mono::MonoDomain *domain = mono::mono_jit_init("qtmonotests.exe");

    QMonoArgumentConverter<10> converter(domain);
    converter.add(QString("Yalla."));

    void** args = converter.args();
    mono::MonoString *arg1 = reinterpret_cast<mono::MonoString*>(args[0]);
    char *utf8 = mono::mono_string_to_utf8(arg1);
    qDebug("Text: %s", utf8);
    mono::mono_free(utf8);
}

QTEST_MAIN(QMonoArgumentConverterTest);

#include "qtmonotests.moc"
