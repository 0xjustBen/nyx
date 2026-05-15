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

#if defined(Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

// Returns true if the current process is running as Administrator.
static bool isElevated()
{
    BOOL elevated = FALSE;
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION te{};
        DWORD len = 0;
        if (GetTokenInformation(token, TokenElevation, &te, sizeof(te), &len))
            elevated = te.TokenIsElevated;
        CloseHandle(token);
    }
    return elevated == TRUE;
}

// If not elevated AND not already a re-launch attempt, relaunch via UAC.
// Pass "--nyx-elevated" sentinel so we don't loop if elevation is denied.
static bool maybeRelaunchAsAdmin(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--nyx-elevated") return false;
        if (std::string(argv[i]) == "--no-elevate")   return false;
    }
    if (isElevated()) return false;

    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);

    // Build arg string with original args + sentinel.
    std::wstring args = L"--nyx-elevated";
    for (int i = 1; i < argc; ++i) {
        args += L" ";
        // Quote each arg.
        args += L"\"";
        for (char *p = argv[i]; *p; ++p) args += (wchar_t)*p;
        args += L"\"";
    }

    SHELLEXECUTEINFOW info{};
    info.cbSize = sizeof(info);
    info.fMask  = SEE_MASK_NOCLOSEPROCESS;
    info.lpVerb = L"runas";
    info.lpFile = exe;
    info.lpParameters = args.c_str();
    info.nShow  = SW_SHOWNORMAL;
    if (ShellExecuteExW(&info)) {
        // Elevated copy launched — exit current process.
        if (info.hProcess) CloseHandle(info.hProcess);
        return true;
    }
    // User declined UAC — continue unelevated. Kill-existing won't work for
    // processes from other sessions but app still functions.
    return false;
}
#endif

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    if (maybeRelaunchAsAdmin(argc, argv)) return 0;
#endif
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
