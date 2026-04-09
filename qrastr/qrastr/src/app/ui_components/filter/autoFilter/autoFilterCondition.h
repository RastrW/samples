#pragma once
#include <QMap>
#include <QtitanGrid.h>

class FilterRule;

/// @brief GridFilterCondition, хранящий правила автофильтра по колонкам.
/// Условия объединяются через AND: строка видима, только если все активные
/// правила возвращают true.
class AutoFilterCondition : public Qtitan::GridFilterCondition
{
public:
    explicit AutoFilterCondition(Qtitan::GridFilter* filter = nullptr);

    // ── GridFilterCondition interface ────────────────────────────────────────
    bool isTrue(const QModelIndex& index) override;
    Qtitan::GridFilterCondition* clone() const override;
    QString createPresentation() const override;
    int conditionCount() const override;
#ifndef QTN_NOUSE_XML_MODULE
    bool saveToXML(IXmlStreamWriter*) override  { return true; }
    bool loadFromXML(IXmlStreamReader*) override { return true; }
#endif

    // ── API ──────────────────────────────────────────────────────────────────
    void setRule(int colIndex, const FilterRule& rule);
    void clearRule(int colIndex);
    void clearAll();
    bool hasActiveRules() const;
    const QMap<int, FilterRule>& rules() const { return m_rules; }

private:
    QMap<int, FilterRule> m_rules; // colIndex → правило
};