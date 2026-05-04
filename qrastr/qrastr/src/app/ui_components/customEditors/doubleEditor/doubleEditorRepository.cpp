#include "doubleEditorRepository.h"
#include "permissiveDoubleValidator.h"

DoubleEditorRepository::DoubleEditorRepository(int    decimals,
                                double minVal,
                                double maxVal)
    : GridStringEditorRepository()
    , m_decimals(decimals)
{
    auto* val = new PermissiveDoubleValidator(minVal, maxVal, decimals, this);
    setValidator(val);
    // Выравнивание по правому краю — стандарт для чисел
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

QString DoubleEditorRepository::convertToText(const QVariant& value)
{
    if (!value.isValid() || value.isNull())
        return {};
    bool ok = false;
    const double d = value.toDouble(&ok);
    if (!ok) return value.toString();
    return QString::number(d, 'f', m_decimals);
}