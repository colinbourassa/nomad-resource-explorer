#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QStringList>
#include <QPalette>
#include <QColor>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    a.setPalette(darkPalette);
    a.setStyleSheet("QToolTip { color: #ffffff; background-color: #13365c; border: 1px solid white; }");


    a.setApplicationVersion(QString("%1.%2.%3")
                            .arg(NRE_VER_MAJOR)
                            .arg(NRE_VER_MINOR)
                            .arg(NRE_VER_PATCH));
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
