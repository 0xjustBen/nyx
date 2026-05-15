#include "ui/presence_model.hpp"

namespace nyx {

PresenceModel::PresenceModel(QObject *parent) : QObject(parent) {}

void PresenceModel::set(const QString &show, const QString &status)
{
    if (show == m_show && status == m_status) return;
    m_show = show;
    m_status = status;
    emit changed();
}

} // namespace nyx
