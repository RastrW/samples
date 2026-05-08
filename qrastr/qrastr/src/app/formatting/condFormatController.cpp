#include "condFormatController.h"
#include "rdata.h"
#include "rmodel.h"
#include "condFormatDialog.h"
#include "condformatjson.h"
#include "condFormat.h"

CondFormatController::CondFormatController(RModel*                model,
                                           Qtitan::GridTableView* view,
                                           QWidget*               parentWidget)
    : QObject(parentWidget)
    , m_model(model)
    , m_view(view)
    , m_parentWidget(parentWidget){}

void CondFormatController::loadFromJson()
{
    const auto& rdata = m_model->getRdata();

    // Загружаем по имени колонки — без привязки к позиции
    auto loaded = CondFormatJson::load(rdata.t_name_);

    // Преобразуем имя → rdataPos через mCols_
    for (auto& [colName, vec] : loaded) {
        auto it = rdata.mCols_.find(colName);
        if (it == rdata.mCols_.end()) continue; // колонка удалена или переименована
        m_model->setCondFormats(static_cast<size_t>(it->second), std::move(vec));
    }
}

void CondFormatController::saveToJson()
{
    const auto& rdata = m_model->getRdata();

    // Снапшот: имя колонки → форматы
    std::unordered_map<std::string, std::vector<CondFormat>> snapshot;

    int rdataPos = 0;
    for (const RCol& rcol : rdata) {
        const auto& vec = m_model->getCondFormats(rdataPos);
        if (!vec.empty())
            snapshot[rcol.getColName()] = vec;
        ++rdataPos;
    }

    CondFormatJson::save(rdata.t_name_, snapshot);
}

void CondFormatController::editCondFormats(std::size_t column)
{
    const int col = static_cast<int>(column);

    // берём из модели — единственного источника истины.
    const std::vector<CondFormat>& current = m_model->getCondFormats(col);

    const QString title = m_model->headerData(
                                     col, Qt::Horizontal, Qt::DisplayRole).toString();

    CondFormatDialog dlg(current, "UTF-8", m_parentWidget);
    dlg.setWindowTitle(tr("Conditional formats for \"%1\"").arg(title));

    if (dlg.exec() == QDialog::Accepted) {
        // Пишем результат диалога сразу в модель — одно место, нет расхождения.
        m_model->setCondFormats(column, dlg.getCondFormats());

        // Обновляем отображение — layoutChanged не вызывается внутри setCondFormats,
        // чтобы не делать лишних перерисовок при пакетной загрузке из JSON.
        emit m_model->layoutChanged();

        saveToJson();
    }
}