#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QStringList>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion("1.0.0");
    a.setApplicationName("Nomad Resource Explorer");

    QCommandLineParser parser;
    parser.setApplicationDescription("Graphical browser for the resource files from the 1993 DOS game 'Nomad'");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("gamedir", "Directory containing game data files");

    parser.process(a);
    const QStringList args = parser.positionalArguments();
    const QString gameDir = (args.size() > 0) ? args[0] : QString("");

    MainWindow w(gameDir);
    w.show();

    return a.exec();
}
