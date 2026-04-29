#pragma once
#include "QtitanGrid.h"

/// @brief Repository для вещественных ячеек.
class DoubleEditorRepository : public Qtitan::GridStringEditorRepository
{
    Q_OBJECT
public:
    explicit DoubleEditorRepository(int    decimals = 2,
                                    double minVal   = -1e15,
                                    double maxVal   =  1e15);

    int decimals() const { return m_decimals; }

private:
    int m_decimals;
};