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
