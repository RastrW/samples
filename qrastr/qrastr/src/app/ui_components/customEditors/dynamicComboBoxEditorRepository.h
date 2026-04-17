#pragma once
#include "QtitanGrid.h"
#include "rmodel.h"
#include <spdlog/spdlog.h>

class DynamicComboBoxEditorRepository : public Qtitan::GridComboBoxEditorRepository
{
    Q_OBJECT
public:
    explicit DynamicComboBoxEditorRepository(
        RModel* model, 
        int columnIndex)
        : GridComboBoxEditorRepository()
        , m_model(model)
        , m_columnIndex(columnIndex)
    {
    }

    QVariant defaultValue(Qt::ItemDataRole role) const
    {
        // переопределяем defaultValue, чтобы всегда возвращать свежий список
        if (role == static_cast<Qt::ItemDataRole>(Qtitan::ComboBoxRole))
        {
            // Берём список прямо из модели
            const auto info = m_model->getColumnEditorInfo(m_columnIndex);
            if (info.editorType == RModel::ColumnEditorInfo::Type::ComboBox)
            {
                spdlog::debug("DynamicComboBoxEditorRepository col={} returning list size={}",
                              m_columnIndex, info.comboItems.size());
                return info.comboItems;
            }
        }
        // Для остальных ролей — используем базовую реализацию
        return GridComboBoxEditorRepository::defaultValue(role);
    }

private:
    RModel* m_model;
    int m_columnIndex;
};