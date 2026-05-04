#include "autoFilterCondition.h"
#include "filterRule.h"

bool AutoFilterCondition::isTrue(const QModelIndex& index) {
    if (m_rules.empty()) return true;
    const QAbstractItemModel* model = index.model();
    if (!model) return true;

    for (const auto& [colIdx, rule] : m_rules) {
        if (!rule.isActive()) continue;
        const QVariant cellVal = model->data(
            model->index(index.row(), colIdx), Qt::EditRole);
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
    for (const auto& [key, rule] : m_rules) {
        if (rule.isActive())
            parts << QStringLiteral("col%1: %2")
                         .arg(key)
                         .arg(rule.presentation());
    }
    return parts.isEmpty() ? QStringLiteral("autofilter (none)")
                           : parts.join(QStringLiteral("; "));
}

int AutoFilterCondition::conditionCount() const {
    int n = 0;
    for (const auto& [_, r] : m_rules)
        if (r.isActive()) ++n;
    return n;
}

void AutoFilterCondition::setRule(int colIndex, const FilterRule& rule) {
    if (!rule.isActive()) m_rules.erase(colIndex);
    else m_rules[colIndex] = rule;
}

void AutoFilterCondition::clearRule(int colIndex) { m_rules.erase(colIndex); }
void AutoFilterCondition::clearAll()              { m_rules.clear(); }

bool AutoFilterCondition::hasActiveRules() const {
    for (const auto& [_, r] : m_rules)
        if (r.isActive()) return true;
    return false;
}