#include "mainwindow.h"
#include "qsurfaceformat.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QSurfaceFormat format;

#ifdef Q_OS_MAC
    format.setVersion(4,1);
#else
    format.setVersion(4,0);
#endif
    format.setProfile(QSurfaceFormat::CoreProfile);

    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
