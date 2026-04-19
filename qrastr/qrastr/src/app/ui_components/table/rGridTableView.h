#pragma once
#include "QtitanGrid.h"

class RGridTableView : public Qtitan::GridTableView
{
    Q_OBJECT
public:
    // Фабричный метод — вызывает initialize()
    static RGridTableView* create(Qtitan::GridBase* grid);

signals:
    void sig_columnWidthChanged(Qtitan::GridColumnBase* column);

protected:
    explicit RGridTableView(Qtitan::GridBase* grid);

    void columnWidthChanged(Qtitan::GridColumnBase* column) override;
};