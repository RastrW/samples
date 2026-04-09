#include "autoFilterCondition.h"
#include "filterRule.h"

AutoFilterCondition::AutoFilterCondition(Qtitan::GridFilter* filter)
    : GridFilterCondition(filter)
{}

bool AutoFilterCondition::isTrue(const QModelIndex& index)
{
    if (m_rules.isEmpty()) return true;
    const QAbstractItemModel* model = index.model();
    if (!model) return true;

    for (auto it = m_rules.constBegin(); it != m_rules.constEnd(); ++it) {
        const FilterRule& rule = it.value();
        if (!rule.isActive()) continue;
        QModelIndex cellIdx = model->index(index.row(), it.key());
        QVariant cellVal = model->data(cellIdx, Qt::EditRole);
        if (!rule.matches(cellVal)) return false;
    }
    return true;
}

Qtitan::GridFilterCondition*
AutoFilterCondition::clone() const
{
    auto* copy = new AutoFilterCondition(m_filter);
    copy->m_rules = m_rules;
    return copy;
}

QString AutoFilterCondition::createPresentation() const
{
    QStringList parts;
    for (auto it = m_rules.constBegin(); it != m_rules.constEnd(); ++it) {
        if (it.value().isActive())
            parts << QString("col%1: %2").arg(it.key()).arg(it.value().value);
    }
    return parts.isEmpty() ? QString("autofilter (none)") : parts.join("; ");
}

int AutoFilterCondition::conditionCount() const
{
    int n = 0;
    for (const auto& r : m_rules) if (r.isActive()) ++n;
    return n;
}

void AutoFilterCondition::setRule(int colIndex, const FilterRule& rule)
{
    m_rules[colIndex] = rule;
}

void AutoFilterCondition::clearRule(int colIndex)
{
    m_rules.remove(colIndex);
}

void AutoFilterCondition::clearAll()
{
    m_rules.clear();
}

bool AutoFilterCondition::hasActiveRules() const
{
    for (const auto& r : m_rules) if (r.isActive()) return true;
    return false;
}