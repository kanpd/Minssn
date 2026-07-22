#include "mainwindow.h"

#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("MinecraftLauncher");
    a.setApplicationVersion("1.0.0");
    
    QString dataDir = QDir::homePath() + "/.minecraft_launcher";
    QDir().mkpath(dataDir);
    
    MainWindow w;
    w.show();
    
    return a.exec();
}