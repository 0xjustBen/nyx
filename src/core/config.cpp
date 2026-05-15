#include "core/config.hpp"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSettings>

namespace nyx {

std::string Config::path()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return (dir + "/nyx.ini").toStdString();
}

bool Config::load()
{
    QSettings s(QString::fromStdString(path()), QSettings::IniFormat);
    mode = s.value("mode", QString::fromStdString(mode)).toString().toStdString();
    region = s.value("region", QString::fromStdString(region)).toString().toStdString();
    startMinimized = s.value("startMinimized", startMinimized).toBool();
    autoPatch = s.value("autoPatch", autoPatch).toBool();
    return true;
}

bool Config::save() const
{
    QSettings s(QString::fromStdString(path()), QSettings::IniFormat);
    s.setValue("mode", QString::fromStdString(mode));
    s.setValue("region", QString::fromStdString(region));
    s.setValue("startMinimized", startMinimized);
    s.setValue("autoPatch", autoPatch);
    return true;
}

} // namespace nyx
