#pragma once
#include "QtitanGrid.h"

class RGrid : public Qtitan::Grid
{
public:
    explicit RGrid(QWidget* parent = nullptr)
        : Qtitan::Grid(parent)
    {}
protected:
    Qtitan::GridViewBase* createView(GridViewType type) override;
};