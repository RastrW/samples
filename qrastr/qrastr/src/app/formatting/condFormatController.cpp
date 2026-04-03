#include "condFormatController.h"
#include "rdata.h"
#include "rmodel.h"
#include "condFormatDialog.h"
#include "condformatjson.h"

CondFormatController::CondFormatController(RModel*                model,
                                           Qtitan::GridTableView* view,
                                           QWidget*               parentWidget)
    : QObject(parentWidget)
    , m_model(model)
    , m_view(view)
    , m_parentWidget(parentWidget){}

void CondFormatController::loadFromJson()
{
    RData* rdata = m_model->getRdata();
    if (!rdata) return;

    // грузим прямо в модель
    auto loaded = CondFormatJson::load(rdata->t_name_, rdata->vCols_);

    for (auto& [col, vec] : loaded)
        m_model->setCondFormats(static_cast<size_t>(col), vec);
}

void CondFormatController::saveToJson()
{
    RData* rdata = m_model->getRdata();
    if (!rdata) return;

    // Собираем актуальное состояние из модели (не из локальной копии).
    std::unordered_map<int, std::vector<CondFormat>> snapshot;
    for (const RCol& rcol : *rdata) {
        int idx = rcol.getIndex();
        const auto& vec = m_model->getCondFormats(idx);
        if (!vec.empty())
            snapshot[idx] = vec;
    }

    // статический вызов, данные передаются явно.
    CondFormatJson::save(rdata->t_name_, rdata->vCols_, snapshot);
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