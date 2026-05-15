#include "ui/roster_model.hpp"

namespace nyx {

RosterModel::RosterModel(QObject *parent) : QAbstractListModel(parent) {}

int RosterModel::rowCount(const QModelIndex &) const
{
    return static_cast<int>(m_items.size());
}

QVariant RosterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= (int)m_items.size())
        return {};
    const auto &f = m_items[index.row()];
    switch (role) {
        case JidRole:      return f.jid;
        case NameRole:     return f.name;
        case TagRole:      return f.tag;
        case PresenceRole: return f.presence;
        case GameRole:     return f.game;
        case ActivityRole: return f.activity;
    }
    return {};
}

QHash<int, QByteArray> RosterModel::roleNames() const
{
    return {
        {JidRole,      "jid"},
        {NameRole,     "name"},
        {TagRole,      "tag"},
        {PresenceRole, "presence"},
        {GameRole,     "game"},
        {ActivityRole, "activity"},
    };
}

void RosterModel::updatePresence(const QString &jid, const QString &presence,
                                 const QString &game, const QString &activity)
{
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].jid == jid) {
            m_items[i].presence = presence;
            if (!game.isEmpty())     m_items[i].game = game;
            if (!activity.isEmpty()) m_items[i].activity = activity;
            auto ix = index((int)i);
            emit dataChanged(ix, ix);
            return;
        }
    }
    // Presence for unknown JID: create a stub entry.
    Friend f;
    f.jid = jid;
    f.name = jid.section('@', 0, 0);
    f.presence = presence;
    f.game = game;
    f.activity = activity;
    beginInsertRows({}, (int)m_items.size(), (int)m_items.size());
    m_items.push_back(f);
    endInsertRows();
}

void RosterModel::upsert(const Friend &f)
{
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].jid == f.jid) {
            m_items[i] = f;
            auto ix = index((int)i);
            emit dataChanged(ix, ix);
            return;
        }
    }
    beginInsertRows({}, (int)m_items.size(), (int)m_items.size());
    m_items.push_back(f);
    endInsertRows();
}

void RosterModel::remove(const QString &jid)
{
    for (size_t i = 0; i < m_items.size(); ++i) {
        if (m_items[i].jid == jid) {
            beginRemoveRows({}, (int)i, (int)i);
            m_items.erase(m_items.begin() + i);
            endRemoveRows();
            return;
        }
    }
}

void RosterModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

} // namespace nyx
