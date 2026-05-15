#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

#include "ui/app_controller.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("nyx");
    app.setOrganizationDomain("nyx.io");
    app.setApplicationName("Nyx");
    app.setWindowIcon(QIcon(":/qt/qml/Nyx/resources/icons/tray.svg"));

    QQuickStyle::setStyle("Basic");

    nyx::AppController controller;
    controller.start();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("App", &controller);
    engine.loadFromModule("Nyx", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
