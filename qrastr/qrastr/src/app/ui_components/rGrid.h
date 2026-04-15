#pragma once
#include "QtitanGrid.h"

class RGrid : public Qtitan::Grid
{
public:
    explicit RGrid(QWidget* parent = nullptr);

protected:
    Qtitan::GridViewBase* createView(Qtitan::Grid::GridViewType type) override;
};