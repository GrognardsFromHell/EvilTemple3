/********************************************************************************
** Form generated from reading UI file 'choosedirectorypage.ui'
**
** Created: Wed 23. Feb 11:40:04 2011
**      by: Qt User Interface Compiler version 4.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHOOSEDIRECTORYPAGE_H
#define UI_CHOOSEDIRECTORYPAGE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWizardPage>

QT_BEGIN_NAMESPACE

class Ui_ChooseDirectoryPage
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLineEdit *path;
    QPushButton *choosePath;
    QHBoxLayout *statusLayout;
    QLabel *statusIcon;
    QLabel *statusText;

    void setupUi(QWizardPage *ChooseDirectoryPage)
    {
        if (ChooseDirectoryPage->objectName().isEmpty())
            ChooseDirectoryPage->setObjectName(QString::fromUtf8("ChooseDirectoryPage"));
        ChooseDirectoryPage->resize(400, 149);
        verticalLayout_2 = new QVBoxLayout(ChooseDirectoryPage);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        path = new QLineEdit(ChooseDirectoryPage);
        path->setObjectName(QString::fromUtf8("path"));

        horizontalLayout->addWidget(path);

        choosePath = new QPushButton(ChooseDirectoryPage);
        choosePath->setObjectName(QString::fromUtf8("choosePath"));

        horizontalLayout->addWidget(choosePath);


        verticalLayout_2->addLayout(horizontalLayout);

        statusLayout = new QHBoxLayout();
        statusLayout->setObjectName(QString::fromUtf8("statusLayout"));
        statusIcon = new QLabel(ChooseDirectoryPage);
        statusIcon->setObjectName(QString::fromUtf8("statusIcon"));
        statusIcon->setMaximumSize(QSize(24, 32));
        statusIcon->setPixmap(QPixmap(QString::fromUtf8(":/24-em-cross.png")));

        statusLayout->addWidget(statusIcon);

        statusText = new QLabel(ChooseDirectoryPage);
        statusText->setObjectName(QString::fromUtf8("statusText"));
        statusText->setWordWrap(true);

        statusLayout->addWidget(statusText);


        verticalLayout_2->addLayout(statusLayout);


        retranslateUi(ChooseDirectoryPage);

        QMetaObject::connectSlotsByName(ChooseDirectoryPage);
    } // setupUi

    void retranslateUi(QWizardPage *ChooseDirectoryPage)
    {
        ChooseDirectoryPage->setWindowTitle(QApplication::translate("ChooseDirectoryPage", "WizardPage", 0, QApplication::UnicodeUTF8));
        choosePath->setText(QApplication::translate("ChooseDirectoryPage", "Browse", 0, QApplication::UnicodeUTF8));
        statusIcon->setText(QString());
        statusText->setText(QApplication::translate("ChooseDirectoryPage", "TextLabel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ChooseDirectoryPage: public Ui_ChooseDirectoryPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHOOSEDIRECTORYPAGE_H
