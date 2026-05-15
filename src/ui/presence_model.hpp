#pragma once
#include <QObject>
#include <QString>

namespace nyx {

class PresenceModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString show READ show NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
public:
    explicit PresenceModel(QObject *parent = nullptr);

    QString show() const { return m_show; }
    QString status() const { return m_status; }

    void set(const QString &show, const QString &status);

signals:
    void changed();

private:
    QString m_show = "chat";
    QString m_status;
};

} // namespace nyx
