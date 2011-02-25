
#include <QIcon>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QAbstractButton>

#include <conversion/converter.h>

#include "converterwizard.h"
#include "choosedirectorypage.h"
#include "conversionpage.h"

#include <iostream>

ConverterWizard::ConverterWizard(QWidget *parent) :
    QWizard(parent)
{
    setWindowTitle(tr("EvilTemple - Resource Converter"));
    setWindowIcon(QIcon(":/application.ico"));
    setPixmap(LogoPixmap, QPixmap(":/64x64.png"));

    addPage(new ChooseDirectoryPage);
    addPage(new ConversionPage);
}

int main(int argc, char **argv) {

    QApplication app(argc, argv);

    ConverterWizard wizard;
    wizard.show();

    int result = app.exec();

    // If there's an error code already, return that
    if (result < 0)
        return result;

    // Otherwise, return -1 if the user cancelled.
    if (wizard.result() == QWizard::Rejected)
        return -1;

    return result;

    /*
    std::cout << "Conversion utility for Temple of Elemental Evil." << std::endl;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " " << "<install-dir>" << std::endl;
        return -1;
    }

    Converter converter(QString::fromLocal8Bit(argv[1]), QString("data/"));

    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "-external")) {
            converter.setExternal(true);
        }
    }

    if (!converter.convert()) {
        std::cout << "ERROR: Conversion failed." << std::endl;
        return -1;
    }*/

    return 0;
}
