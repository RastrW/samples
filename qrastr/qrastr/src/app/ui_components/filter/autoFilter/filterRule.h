#pragma once
#include <QString>
#include <QVariant>

/// @brief Распарсенное правило фильтрации одной ячейки автофильтра.
struct FilterRule {
    enum class Op { None, Eq, Neq, Lt, Le, Gt, Ge };
    Op      op    = Op::None;
    QString value;          // строка для сравнения
    double  numValue = 0.0; // если значение числовое
    bool    isNum   = false;

    bool isActive() const { return op != Op::None; }

    /// @brief Проверяет, удовлетворяет ли cellValue правилу.
    /// cellVal — Qt::EditRole из модели
    /// (double для числовых колонок, QString для строк).
    bool matches(const QVariant& cellVal) const {
        if (op == Op::None) return true;
        if (isNum) {
            bool ok = false;
            double v = cellVal.toDouble(&ok);
            if (!ok) return false;
            switch (op) {
            case Op::Eq:  return qFuzzyCompare(v, numValue);
            case Op::Neq: return !qFuzzyCompare(v, numValue);
            case Op::Lt:  return v <  numValue;
            case Op::Le:  return v <= numValue;
            case Op::Gt:  return v >  numValue;
            case Op::Ge:  return v >= numValue;
            default:      return false;
            }
        } else {
            QString cell = cellVal.toString();
            switch (op) {
            case Op::Eq:  return cell.compare(value, Qt::CaseInsensitive) == 0;
            case Op::Neq: return cell.compare(value, Qt::CaseInsensitive) != 0;
            // Для строк < / > / <= / >= — лексикографически
            case Op::Lt:  return cell < value;
            case Op::Le:  return cell <= value;
            case Op::Gt:  return cell > value;
            case Op::Ge:  return cell >= value;
            default:      return false;
            }
        }
    }
};