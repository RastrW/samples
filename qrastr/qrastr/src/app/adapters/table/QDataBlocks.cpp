#include "QDataBlocks.h"
#include "IDataBlocksWrappers.h"

void MyDataSet::Dump()
{
    std::cout << "DataSet " << RowsCount() << "x" << ColumnsCount() << std::endl;
    std::cout << "#;";
    for (const auto& Column : Columns_)
        std::cout << Column.Name_ << "(" <<
            //MapFieldVariantName(Column.Column_->Type()) << ",[" <<
            Column.Column_->DataSize() << "]" <<
            ");";

    std::cout << std::endl;
    for (IndexT row{ 0 }; row < RowsCount(); row++)
    {
        bool RowStart{ true };
        std::cout << Indexes_[row] << ";";

        for (const auto& Column : Columns_)
        {
            if (RowStart)
                RowStart = false;
            else
                std::cout << ";";

            const auto& Col{ Column.Column_ };

            // здесь, со спецификой датасета:
            // если у столбца есть индексы, значит он сжат
            // и мы должны запрашивать по индексу выборки
            // если индексов нет или они совпадают с индексами
            // датасета - запрашиваем по индексу строки

            // это выдуманная для теста логика, ей нет необходимости
            // пользоваться в других проектах

            if(Col->IndexesSize() > 0 && Col->IndexesSize() < RowsCount())
                std::cout << std::visit(ToString(), Column.Column_->Get(Indexes_[row], 0));
            else
                std::cout << std::visit(ToString(), Column.Column_->Get(row, 0));
        }
        std::cout << std::endl;
    }
}
