#include "condFormatController.h"
#include "rdata.h"
#include "rmodel.h"
#include "CondFormatManager.h"
#include "condformatjson.h"

CondFormatController::CondFormatController(RModel*                model,
                                           Qtitan::GridTableView* view,
                                           QWidget*               parentWidget,
                                           QObject*               parent)
    : QObject(parent)
    , m_model(model)
    , m_view(view)
    , m_parentWidget(parentWidget)
{
    connect(this, &CondFormatController::formatsChanged,
            this, &CondFormatController::onFormatsChanged);
}

void CondFormatController::loadFromJson()
{
    RData* rdata = m_model->getRdata();
    if (!rdata) return;

    // Инициализируем пустыми векторами для каждой колонки
    for (const RCol& rcol : *rdata)
        m_formats.emplace(rcol.getIndex(), std::vector<CondFormat>());

    // Читаем JSON
    std::map<int, std::vector<CondFormat>> loaded;
    CondFormatJson cfj(rdata->t_name_, rdata->vCols_, loaded);
    cfj.from_json();

    for (auto& [key, val] : cfj.get_mcf()) {
        if (m_formats.count(key)) {
            m_formats[key] = val;
            m_model->setCondFormats(false, key, val);
        }
    }
}

void CondFormatController::editCondFormats(std::size_t column)
{
    const int col = static_cast<int>(column);
    auto it = m_formats.find(col);
    if (it == m_formats.end()) return;

    const QString title = m_model->headerData(
        col, Qt::Horizontal, Qt::DisplayRole).toString();

    CondFormatManager dlg(it->second, "UTF-8", m_parentWidget);
    dlg.setWindowTitle(tr("Conditional formats for \"%1\"").arg(title));

    if (dlg.exec() == QDialog::Accepted) {
        it->second = dlg.getCondFormats();
        m_model->setCondFormats(false, column, it->second);
        emit formatsChanged();
    }
}

void CondFormatController::onFormatsChanged()
{
    RData* rdata = m_model->getRdata();
    if (!rdata) return;
    CondFormatJson cfj(rdata->t_name_, rdata->vCols_, m_formats);
    cfj.save_json();
}
