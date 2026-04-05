#pragma once
#include <QSortFilterProxyModel>

class ProtocolTreeItem;
enum class LogMessageTypes;

class ProtocolFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit ProtocolFilterProxyModel(QObject* parent = nullptr);

    // Пустой фильтр = показывать всё
    void setActiveFilter(const QSet<LogMessageTypes>& filter);

protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex& sourceParent) const override;

private:
    // Рекурсивная проверка дочерних строк в source-модели
    bool rowHasMatchingDescendant(const QModelIndex& parent) const;

    QSet<LogMessageTypes> m_filter;
};
