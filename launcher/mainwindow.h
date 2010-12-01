
#ifndef LAUNCHER_MAINWINDOW_H
#define LAUNCHER_MAINWINDOW_H

#include <QGLWidget>

class MainWindow : public QGLWidget {
Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = NULL);
    ~MainWindow();

    bool event(QEvent *e);
};

#endif // LAUNCHER_MAINWINDOW_H
