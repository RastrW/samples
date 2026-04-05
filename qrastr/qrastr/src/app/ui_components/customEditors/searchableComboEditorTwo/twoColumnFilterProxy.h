#pragma once
#include <QSortFilterProxyModel>

// TwoColumnFilterProxy — независимая фильтрация по двум столбцам
class TwoColumnFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TwoColumnFilterProxy(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent) {}

public slots:
    void setIndexFilter(const QString& text) { m_filter0 = text; invalidateFilter(); }
    void setNameFilter (const QString& text) { m_filter1 = text; invalidateFilter(); }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        auto match = [&](int col, const QString& filter) -> bool {
            if (filter.isEmpty()) return true;
            const QModelIndex idx = sourceModel()->index(source_row, col, source_parent);
            return sourceModel()->data(idx).toString()
                       .contains(filter, Qt::CaseInsensitive);
        };
        return match(0, m_filter0) && match(1, m_filter1);
    }

private:
    QString m_filter0;
    QString m_filter1;
};