#pragma once
#include <QString>
#include <QWidget>
#include "filterRule.h"

class QLineEdit;
inline constexpr int kRowHeight = 25;
// смещение от левого края виджета до первой колонки
inline constexpr int kBorderX   = 57;

/// @brief Небольшая ячейка автофильтра: кнопка оператора + поле значения.
/// Для bool — только кнопка с popup.
class FilterCell : public QWidget
{
    Q_OBJECT
public:
    explicit FilterCell(bool isNumeric, bool isBool, QWidget* parent = nullptr);

    FilterRule rule() const;
    void setRule(const FilterRule& rule);
    void clear();
    void setPlaceholderText(const QString& text);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void sig_filterChanged(const FilterRule& rule);

private slots:
    void slotTextChanged(const QString& text);
    void showOpMenu();

private:
    QString opText(FilterRule::Op op) const;
    void updateUi();
    void applyRule(const FilterRule& rule, bool emitSignal);

private:
    QToolButton* m_opBtn = nullptr;
    QLineEdit*   m_edit  = nullptr;
    FilterRule   m_rule;
    bool         m_isNumeric = false;
    bool         m_isBool = false;
    QString      m_placeholder;
};