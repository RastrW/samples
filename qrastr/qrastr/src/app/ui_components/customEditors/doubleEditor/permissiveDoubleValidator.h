#pragma once
#include <QDoubleValidator>

/// @brief Валидатор: разрешает пустое поле и запятую вместо точки.
class PermissiveDoubleValidator : public QDoubleValidator
{
public:
    PermissiveDoubleValidator(double min, double max, int decimals, QObject* parent)
        : QDoubleValidator(min, max, decimals, parent)
    {
        setNotation(QDoubleValidator::StandardNotation);
        // Locale с точкой как разделителем — стандарт для технических данных
        QLocale loc(QLocale::C);
        loc.setNumberOptions(QLocale::RejectGroupSeparator);
        setLocale(loc);
    }

    State validate(QString& input, int& pos) const override
    {
        // Нормализуем запятую → точка ДО любых проверок
        // Делаем это прямо в input — QLineEdit в Qt6 применяет изменения
        // при любом статусе (Intermediate тоже)
        input.replace(',', '.');

        const QString trimmed = input.trimmed();

        if (trimmed.isEmpty())     return Intermediate;
        if (trimmed == u"-")       return Intermediate;
        if (trimmed == u".")       return Intermediate;
        if (trimmed.endsWith('.')) return Intermediate;
        if (trimmed.endsWith("e") || trimmed.endsWith("E") ||
            trimmed.endsWith("e-") || trimmed.endsWith("e+"))
            return Intermediate;

        return QDoubleValidator::validate(input, pos);
    }

    // fixup вызывается при потере фокуса если состояние Intermediate
    void fixup(QString& input) const override
    {
        input.replace(',', '.');
        const QString trimmed = input.trimmed();
        if (trimmed.isEmpty() || trimmed == u"-" || trimmed == u".") {
            //input = u"0";
            return;
        }
        // Убираем висящую точку: "3." → "3"
        if (trimmed.endsWith('.'))
            input = trimmed.chopped(1);
    }
};