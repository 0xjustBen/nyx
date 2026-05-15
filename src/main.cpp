#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QObject>
#include <QDebug>
#include <QQuickWindow>
#include <QImage>
#include <QTimer>
#include <QSslSocket>

#include "ui/app_controller.hpp"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("nyx");
    app.setOrganizationDomain("nyx.io");
    app.setApplicationName("Nyx");
    app.setWindowIcon(QIcon(":/qt/qml/Nyx/resources/icons/tray.svg"));

    // Force OpenSSL backend on Windows (default is SChannel which doesn't
    // play nicely with our EC P-256 leaf served via QSslServer).
#if defined(Q_OS_WIN)
    qputenv("QT_TLS_BACKEND", "openssl");
#endif
    qInfo().noquote() << "tls backend:" << QSslSocket::activeBackend()
                      << "lib:" << QSslSocket::sslLibraryVersionString();

    QQuickStyle::setStyle("Basic");

    nyx::AppController controller;
    QObject::connect(&controller, &nyx::AppController::logLine,
                     [](const QString &l){ qInfo().noquote() << "nyx:" << l; });
    controller.start();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("App", &controller);
    engine.loadFromModule("Nyx", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    // If NYX_SCREENSHOT=<path> is set, grab the window after first paint
    // (and optionally each subsequent screen swap) then quit.
    if (qEnvironmentVariableIsSet("NYX_SCREENSHOT")) {
        QString path = qEnvironmentVariable("NYX_SCREENSHOT");
        QString screen = qEnvironmentVariable("NYX_SCREEN", "main");
        QObject *root = engine.rootObjects().first();
        QQuickWindow *win = qobject_cast<QQuickWindow *>(root);
        if (win) {
            QTimer::singleShot(800, &app, [win, path, screen, root]{
                root->setProperty("currentScreen", screen);
                QTimer::singleShot(500, win, [win, path]{
                    QImage img = win->grabWindow();
                    img.save(path);
                    qInfo() << "saved" << path << img.size();
                    QGuiApplication::quit();
                });
            });
        }
    }

    return app.exec();
}
