#pragma once
#include <QString>
#include <QVariant>
#include <QRegularExpression>

/// @brief Распарсенное правило фильтрации одной ячейки автофильтра.
struct FilterRule {
    enum class Op {
        None,
        Eq, Neq, Lt, Le, Gt, Ge,
        // Строковые
        Contains,
        Like,       // wildcards: * ?
        NotLike,
        StartsWith,
        EndsWith
    };
    Op      op    = Op::None;
    QString value;          // строка для сравнения
    double  numValue = 0.0; // если значение числовое
    bool    isNum   = false;

    bool    isBool   = false;
    bool    boolValue = false;

    bool isActive() const
    {
        if (op == Op::None)
            return false;
        if (isBool)
            return true;
        return !value.trimmed().isEmpty();
    }

    static bool variantToBool(const QVariant& v, bool* ok = nullptr)
    {
        if (!v.isValid()) {
            if (ok) *ok = false;
            return false;
        }

        if (v.metaType().id() == QMetaType::Bool) {
            if (ok) *ok = true;
            return v.toBool();
        }

        const QString s = v.toString().trimmed().toLower();
        if (s == QStringLiteral("1") ||
            s == QStringLiteral("true") ||
            s == QStringLiteral("yes") ||
            s == QStringLiteral("on"))
        {
            if (ok) *ok = true;
            return true;
        }

        if (s == QStringLiteral("0") ||
            s == QStringLiteral("false") ||
            s == QStringLiteral("no") ||
            s == QStringLiteral("off"))
        {
            if (ok) *ok = true;
            return false;
        }

        if (ok) *ok = false;
        return false;
    }

    /// @brief Проверяет, удовлетворяет ли cellValue правилу.
    /// cellVal — Qt::EditRole из модели.
    bool matches(const QVariant& cellVal) const
    {
        if (!isActive())
            return true;

        if (isBool) {
            bool ok = false;
            const bool cellBool = variantToBool(cellVal, &ok);
            if (!ok)
                return false;

            switch (op) {
            case Op::Eq:  return cellBool == boolValue;
            case Op::Neq: return cellBool != boolValue;
            default:      return false;
            }
        }

        if (isNum) {
            bool ok = false;
            const double v = cellVal.toDouble(&ok);
            if (!ok)
                return false;

            switch (op) {
            case Op::Eq:  return qFuzzyCompare(v + 1.0, numValue + 1.0);
            case Op::Neq: return !qFuzzyCompare(v + 1.0, numValue + 1.0);
            case Op::Lt:  return v <  numValue;
            case Op::Le:  return v <= numValue;
            case Op::Gt:  return v >  numValue;
            case Op::Ge:  return v >= numValue;
            default:      return false;
            }
        }

        const QString cell = cellVal.toString();

        switch (op) {
        case Op::Eq:  return cell.compare(value, Qt::CaseInsensitive) == 0;
        case Op::Neq: return cell.compare(value, Qt::CaseInsensitive) != 0;

        // Для строк < / > / <= / >= — лексикографически
        case Op::Lt:  return cell < value;
        case Op::Le:  return cell <= value;
        case Op::Gt:  return cell > value;
        case Op::Ge:  return cell >= value;

        case Op::Contains:
            return cell.contains(value, Qt::CaseInsensitive);

        case Op::StartsWith:
            return cell.startsWith(value, Qt::CaseInsensitive);

        case Op::EndsWith:
            return cell.endsWith(value, Qt::CaseInsensitive);

        case Op::Like: {
            const QRegularExpression re(
                QRegularExpression::wildcardToRegularExpression(value),
                QRegularExpression::CaseInsensitiveOption);
            return re.match(cell).hasMatch();
        }

        case Op::NotLike: {
            const QRegularExpression re(
                QRegularExpression::wildcardToRegularExpression(value),
                QRegularExpression::CaseInsensitiveOption);
            return !re.match(cell).hasMatch();
        }

        default:
            return false;
        }
    }

    QString presentation() const
    {
        if (!isActive())
            return QStringLiteral("none");

        if (isBool)
            return boolValue ? QStringLiteral("true") : QStringLiteral("false");

        const QString rhs = isNum ? QString::number(numValue, 'g', 16) : value;

        switch (op) {
        case Op::Eq:        return QStringLiteral("= %1").arg(rhs);
        case Op::Neq:       return QStringLiteral("≠ %1").arg(rhs);
        case Op::Lt:        return QStringLiteral("< %1").arg(rhs);
        case Op::Le:        return QStringLiteral("≤ %1").arg(rhs);
        case Op::Gt:        return QStringLiteral("> %1").arg(rhs);
        case Op::Ge:        return QStringLiteral("≥ %1").arg(rhs);
        case Op::Contains:  return QStringLiteral("contains %1").arg(rhs);
        case Op::Like:      return QStringLiteral("mask %1").arg(rhs);
        case Op::NotLike:   return QStringLiteral("not mask %1").arg(rhs);
        case Op::StartsWith:return QStringLiteral("starts %1").arg(rhs);
        case Op::EndsWith:  return QStringLiteral("ends %1").arg(rhs);
        default:            return QStringLiteral("none");
        }
    }
};