#pragma once
#include "QtitanGrid.h"

class RGridTableView : public Qtitan::GridTableView
{
    Q_OBJECT
public:
    RGridTableView(Qtitan::GridBase* grid);

signals:
    void sig_columnWidthChanged(Qtitan::GridColumnBase* column);

protected:
    void columnWidthChanged(Qtitan::GridColumnBase* column) override;
};