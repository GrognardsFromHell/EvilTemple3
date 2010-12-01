
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QGLWidget(parent)
{
}

MainWindow::~MainWindow()
{
}

bool MainWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Paint:
        qDebug("Paint event filtered.");
        return true;
    }

    return QGLWidget::event(e);
}
