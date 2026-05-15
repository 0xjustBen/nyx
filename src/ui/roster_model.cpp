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
        case PresenceRole: return f.presence;
        case GameRole:     return f.game;
    }
    return {};
}

QHash<int, QByteArray> RosterModel::roleNames() const
{
    return {
        {JidRole, "jid"},
        {NameRole, "name"},
        {PresenceRole, "presence"},
        {GameRole, "game"},
    };
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
