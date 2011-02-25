/********************************************************************************
** Form generated from reading UI file 'conversionpage.ui'
**
** Created: Wed 23. Feb 11:40:04 2011
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONVERSIONPAGE_H
#define UI_CONVERSIONPAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QProgressBar>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWizardPage>

QT_BEGIN_NAMESPACE

class Ui_ConversionPage
{
public:
    QVBoxLayout *verticalLayout_2;
    QProgressBar *progressBar;
    QTextBrowser *textBrowser;

    void setupUi(QWizardPage *ConversionPage)
    {
        if (ConversionPage->objectName().isEmpty())
            ConversionPage->setObjectName(QString::fromUtf8("ConversionPage"));
        ConversionPage->resize(402, 300);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ConversionPage->sizePolicy().hasHeightForWidth());
        ConversionPage->setSizePolicy(sizePolicy);
        verticalLayout_2 = new QVBoxLayout(ConversionPage);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        progressBar = new QProgressBar(ConversionPage);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setValue(24);

        verticalLayout_2->addWidget(progressBar);

        textBrowser = new QTextBrowser(ConversionPage);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));

        verticalLayout_2->addWidget(textBrowser);


        retranslateUi(ConversionPage);

        QMetaObject::connectSlotsByName(ConversionPage);
    } // setupUi

    void retranslateUi(QWizardPage *ConversionPage)
    {
        ConversionPage->setWindowTitle(QApplication::translate("ConversionPage", "WizardPage", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ConversionPage: public Ui_ConversionPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONVERSIONPAGE_H
