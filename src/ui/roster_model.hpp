#pragma once
#include <QAbstractListModel>
#include <QString>
#include <vector>

namespace nyx {

struct Friend {
    QString jid;
    QString name;
    QString tag;
    QString presence;
    QString game;
    QString activity;
};

class RosterModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        JidRole = Qt::UserRole + 1,
        NameRole,
        TagRole,
        PresenceRole,
        GameRole,
        ActivityRole,
    };

    explicit RosterModel(QObject *parent = nullptr);

    Q_INVOKABLE int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void upsert(const Friend &f);
    void updatePresence(const QString &jid, const QString &presence,
                        const QString &game, const QString &activity);
    void remove(const QString &jid);
    void clear();

private:
    std::vector<Friend> m_items;
};

} // namespace nyx
