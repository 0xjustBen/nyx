#include "core/roster_store.hpp"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QUuid>

namespace nyx {

RosterStore::RosterStore(const QString &dbPath) : m_path(dbPath) {
    m_connName = "nyx-roster-" + QUuid::createUuid().toString(QUuid::Id128);
}

RosterStore::~RosterStore() { close(); }

bool RosterStore::open()
{
    QFileInfo fi(m_path);
    QDir().mkpath(fi.absolutePath());
    auto db = QSqlDatabase::addDatabase("QSQLITE", m_connName);
    db.setDatabaseName(m_path);
    if (!db.open()) return false;
    m_open = true;
    QSqlQuery q(db);
    return q.exec(
        "CREATE TABLE IF NOT EXISTS friends ("
        " jid TEXT PRIMARY KEY,"
        " name TEXT,"
        " tag TEXT,"
        " presence TEXT,"
        " game TEXT,"
        " activity TEXT,"
        " last_seen INTEGER"
        ")");
}

void RosterStore::close()
{
    if (m_open) {
        QSqlDatabase::removeDatabase(m_connName);
        m_open = false;
    }
}

QList<Friend> RosterStore::loadAll()
{
    QList<Friend> out;
    if (!m_open) return out;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    if (!q.exec("SELECT jid,name,tag,presence,game,activity FROM friends ORDER BY last_seen DESC"))
        return out;
    while (q.next()) {
        Friend f;
        f.jid      = q.value(0).toString();
        f.name     = q.value(1).toString();
        f.tag      = q.value(2).toString();
        f.presence = q.value(3).toString();
        f.game     = q.value(4).toString();
        f.activity = q.value(5).toString();
        out.append(f);
    }
    return out;
}

void RosterStore::upsert(const Friend &f)
{
    if (!m_open) return;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare(
        "INSERT INTO friends(jid,name,tag,presence,game,activity,last_seen)"
        " VALUES(?,?,?,?,?,?,?)"
        " ON CONFLICT(jid) DO UPDATE SET"
        " name=excluded.name, tag=excluded.tag,"
        " presence=excluded.presence, game=excluded.game,"
        " activity=excluded.activity, last_seen=excluded.last_seen");
    q.addBindValue(f.jid);
    q.addBindValue(f.name);
    q.addBindValue(f.tag);
    q.addBindValue(f.presence);
    q.addBindValue(f.game);
    q.addBindValue(f.activity);
    q.addBindValue((qint64)QDateTime::currentSecsSinceEpoch());
    q.exec();
}

void RosterStore::remove(const QString &jid)
{
    if (!m_open) return;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("DELETE FROM friends WHERE jid=?");
    q.addBindValue(jid);
    q.exec();
}

void RosterStore::touchLastSeen(const QString &jid)
{
    if (!m_open) return;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("UPDATE friends SET last_seen=? WHERE jid=?");
    q.addBindValue((qint64)QDateTime::currentSecsSinceEpoch());
    q.addBindValue(jid);
    q.exec();
}

void RosterStore::clear()
{
    if (!m_open) return;
    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.exec("DELETE FROM friends");
}

} // namespace nyx
