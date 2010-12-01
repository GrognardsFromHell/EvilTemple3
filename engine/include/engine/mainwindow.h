#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "engine/global.h"

#include <QMainWindow>
#include <QDeclarativeView>
#include <QVariant>

class QGraphicsView;
class QDeclarativeItem;

namespace EvilTemple {

class MainWindowData;

class ENGINE_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void readSettings();
    void writeSettings();
    void showFromSettings();

    void updateTitle();
    void viewStatusChanged(QDeclarativeView::Status status);

protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);

private:
    QScopedPointer<MainWindowData> d_ptr;
};

}

#endif // MAINWINDOW_H
