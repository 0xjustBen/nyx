#pragma once
#include <QString>
#include <QList>
#include "ui/roster_model.hpp"

namespace nyx {

// SQLite-backed roster cache. Friends, last known presence/game, last seen
// timestamp. Survives across launches so the UI can populate immediately
// instead of waiting for a fresh XMPP roster IQ.
class RosterStore {
public:
    explicit RosterStore(const QString &dbPath);
    ~RosterStore();

    bool open();
    void close();

    QList<Friend> loadAll();
    void upsert(const Friend &f);
    void remove(const QString &jid);
    void touchLastSeen(const QString &jid);
    void clear();

private:
    QString m_path;
    bool m_open = false;
    QString m_connName;
};

} // namespace nyx
