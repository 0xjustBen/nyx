#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <cstdint>

namespace nyx {

class Launcher : public QObject {
    Q_OBJECT
public:
    explicit Launcher(QObject *parent = nullptr);

    // Locate the RiotClientServices executable for current OS.
    static QString riotClientPath();

    // Launch RiotClientServices with --client-config-url and optional product flags.
    // Returns true if Process::startDetached succeeded.
    bool launch(uint16_t configProxyPort,
                const QString &launchProduct = "league_of_legends",
                const QString &patchline = "live");

signals:
    void log(const QString &line);
};

} // namespace nyx
