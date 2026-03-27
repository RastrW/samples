#include "protocolFilterProxyModel.h"
#include <astra/IPlainRastr.h>
#include "protocoltreemodel.h"

inline uint qHash(LogMessageTypes key, uint seed = 0) noexcept
{
    return ::qHash(static_cast<int>(key), seed);
}

ProtocolFilterProxyModel::ProtocolFilterProxyModel
    (QObject* parent)
	: QSortFilterProxyModel(parent)
{}

void ProtocolFilterProxyModel::setActiveFilter(const QSet<LogMessageTypes>& filter) {
	m_filter = filter;
	invalidateFilter();
}

bool ProtocolFilterProxyModel::filterAcceptsRow(
    int sourceRow, const QModelIndex& sourceParent) const
{
    // Фильтр не задан — показываем всё
    if (m_filter.isEmpty()) return true;

    const QModelIndex idx =
        sourceModel()->index(sourceRow, 0, sourceParent);
    if (!idx.isValid()) return false;

    //Получаем int, затем приводим к enum
    const auto lmt = static_cast<LogMessageTypes>(
        sourceModel()->data(idx,  static_cast<int>(ProtocolTreeModel::CustomRoles::MessageTypeRole)).toInt());
    // Стадии (OpenStage) показываем, только если у них есть нужные дети
    if (lmt == LogMessageTypes::OpenStage)
        return rowHasMatchingDescendant(idx);

    return m_filter.contains(lmt);
}

bool ProtocolFilterProxyModel::rowHasMatchingDescendant(
    const QModelIndex& parent) const
{
    const int rows = sourceModel()->rowCount(parent);
    for (int r = 0; r < rows; ++r) {
        const QModelIndex child = sourceModel()->index(r, 0, parent);

        const auto lmt = static_cast<LogMessageTypes>(
            sourceModel()->data(child, static_cast<int>(ProtocolTreeModel::CustomRoles::MessageTypeRole)).toInt());

        if (lmt == LogMessageTypes::OpenStage) {
            if (rowHasMatchingDescendant(child)) return true;
        } else {
            if (m_filter.contains(lmt)) return true;
        }
    }
    return false;
}
