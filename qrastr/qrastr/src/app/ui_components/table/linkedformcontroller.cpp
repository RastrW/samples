#include "linkedformcontroller.h"

#include "rtabController.h"
#include "rtabshell.h"

#include "qastra.h"
#include "rtablesdatamanager.h"
#include "rmodel.h"
#include "rdata.h"
#include "qmcr/pyhlp.h"
#include "filter/customFilterCondition.h"
#include "utils.h"

#include <DockManager.h>
#include <DockWidget.h>
#include <QtitanGrid.h>

#include <QDir>

#include <filesystem>
#include <fstream>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include "QDataBlocks.h"

namespace fs = std::filesystem;

LinkedFormController::LinkedFormController(
    QAstra*                  qastra,
    RTablesDataManager*      rtdm,
    ads::CDockManager*       dockManager,
    Qtitan::GridTableView*   view,
    RModel*                  model,
    const CUIForm&           form,
    RtabController*          parentController)
    : QObject(parentController)
    , m_qastra(qastra)
    , m_rtdm(rtdm)
    , m_dockManager(dockManager)
    , m_view(view)
    , m_model(model)
    , m_form(form)
    , m_parentController(parentController)
{}

void LinkedFormController::setPyHlp(std::shared_ptr<PyHlp> pyHlp)
{
    m_pyHlp = pyHlp;
}

void LinkedFormController::disconnectAll()
{
    for (auto& conn : m_lf.vconn)
        QObject::disconnect(conn);
    m_lf.vconn.clear();
}

void LinkedFormController::buildLinkedFormsMenu(int contextRow, QMenu* menu)
{
    auto table = m_rtdm->get("formcontext", "");

    for (int irow = 0; irow < table->RowsCount(); ++irow) {
        const std::string formName = std::get<std::string>(table->Get(irow, 0));
        if (m_form.Name() != formName) continue;

        LinkedForm lf;
        lf.linkedform = std::visit(ToString(), table->Get(irow, 1));
        lf.linkedname = std::visit(ToString(), table->Get(irow, 2));
        lf.selection  = std::visit(ToString(), table->Get(irow, 3));
        lf.bind       = std::visit(ToString(), table->Get(irow, 4));
        lf.pbaseform  = m_parentController;
        // Заполняем bind-значения из текущей строки контекстного меню
        for (const auto& key : split(lf.bind, ','))
            lf.vbindvals.push_back(getLongValue(key, contextRow));

        QAction* action = new QAction(
            QString::fromStdString(lf.linkedname), menu);
        // lf захватывается по значению — каждое действие имеет свой снимок
        connect(action, &QAction::triggered,
                this, [this, lf]() { openLinkedForm(lf); });
        menu->addAction(action);
    }
}

void LinkedFormController::buildLinkedMacroMenu(int contextRow, QMenu* menu)
{
    auto table = m_rtdm->get("macrocontext", "");

    for (int irow = 0; irow < table->RowsCount(); ++irow) {
        const std::string formName = std::visit(ToString(), table->Get(irow, 0));
        if (m_form.Name() != formName) continue;

        const long formType    = std::visit(ToLong(), table->Get(irow, 6));
        const long defAppendix = std::visit(ToLong(), table->Get(irow, 4));
        if (formType != 0 || !defAppendix) continue;

        LinkedMacro lm;
        lm.col       = std::visit(ToString(), table->Get(irow, 1));
        lm.macrofile = std::visit(ToString(), table->Get(irow, 2));
        lm.macrodesc = std::visit(ToString(), table->Get(irow, 3));
        lm.addstr    = std::visit(ToString(), table->Get(irow, 5));
        lm.pbaseform = m_parentController;

        QAction* action = new QAction(
            QString::fromStdString(lm.macrodesc), menu);
        connect(action, &QAction::triggered,
                this, [this, lm, contextRow]() {
                    openLinkedMacro(lm, contextRow);
                });
        menu->addAction(action);
    }
}

void LinkedFormController::applyLinkedForm(LinkedForm lf)
{
    m_lf = lf;

    const std::string selection = lf.get_selection_result();

    // Передаём строку выборки в плагин
    IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
    IRastrTablePtr  table{ tablesx->Item(m_model->getRdata()->t_name_) };
    IRastrResultVerify(table->SetSelection(selection));

    // Получаем индексы строк, прошедших выборку
    DataBlock<FieldVariantData> variantBlock;
    const IRastrPayload keys = table->Key();
    IRastrResultVerify(table->DataBlock(keys.Value(), variantBlock));
    const auto indices = variantBlock.IndexesVector();

	//Создаём CustomFilterCondition для QTitan Grid с этими индексами
    auto* groupCondition = new GridFilterGroupCondition(m_view->filter());
    auto* condition      = new CustomFilterCondition(m_view->filter());
    groupCondition->addCondition(condition);

    for (long idx : indices)
        condition->addRow(idx);
	
	//Активируем фильтр — Grid показывает только нужные строки.
    m_view->filter()->setCondition(groupCondition, true);
    m_view->filter()->setActive(true);
    m_view->showFilterPanel();
}

void LinkedFormController::openLinkedForm(LinkedForm lf)
{
    CUIForm* pUIForm = m_rtdm->getForm(lf.linkedform);
    if (!pUIForm) {
        spdlog::warn("LinkedFormController: форма [{}] не найдена", lf.linkedform);
        return;
    }
    // Создаём дочерний виджет — он заведёт собственный LinkedFormController
    auto* child = new RtabController(
        m_qastra, *pUIForm, m_rtdm, m_dockManager,
        nullptr);   //parent для QObject, dock управляет временем жизни

    if (m_pyHlp)
        child->setPyHlp(m_pyHlp);

    // смена фокусной строки в НАШЕМ view => обновление дочерней формы.
    // Соединение сохраняем в lf.vconn, который потом уйдёт в child->m_lf
    // и будет отключён при закрытии дочернего виджета.
    lf.vconn.push_back(
        connect(m_view, &Qtitan::GridTableView::focusRowChanged,
                child, [parentView = m_view, child](int, int) {
                    GridRow row = parentView->focusedRow();
                    if (!row.isValid()) return;
                    const int modelRow = row.modelIndex().row();
                    if (modelRow >= 0)
                        child->notifyParentRowChanged(modelRow);
                }));
    // Передаём LinkedForm в дочерний контроллер — он сохранит vconn
    child->applyLinkedFormFromController(lf);

    auto* dw = new ads::CDockWidget(
        stringutils::MkToUtf8(pUIForm->Name()).c_str(),
        nullptr); // CDockWidget без явного QWidget* parent — dock сам управляет

    dw->setWidget(child->createShell(false));   // false = без тулбара
    // Добавляем в нижнюю авто-скрытую панель Dock
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    connect(dw, &ads::CDockWidget::closed, child, &RtabController::slot_close);

    m_dockManager->addDockWidgetTab(ads::BottomAutoHideArea, dw);
}

void LinkedFormController::openLinkedMacro(LinkedMacro lm, int contextRow)
{
    spdlog::info("LinkedFormController: run macro {}", lm.macrofile.c_str());

    // Макросы лежат в <рабочая директория>/contextmacro/  с расширением .py
    fs::path macroPath = QDir::currentPath().toStdString();

    macroPath /= "contextmacro";
    macroPath /= lm.macrofile;
    macroPath.replace_extension(".py");
    if (!fs::exists(macroPath))
    {
        spdlog::warn("context macro not found: {}", macroPath.string());
        return;
    }

    // Чтобы макросы работали в Data должен лежать astra_py. модуль
    fs::path astra_py_Path = QDir::currentPath().toStdString() + "/../Data/astr_py/Release/";
#ifdef _WIN32
    astra_py_Path /= "astra_py.cp312-win_amd64.pyd";
#else
    astra_py_Path /= "astra_py.cpython-311-x86_64-linux-gnu.so";
#endif
    if (!fs::exists(astra_py_Path))
    {
        spdlog::warn("astra_py. module not found: {}", astra_py_Path.string());
        return;
    }

    std::ifstream file(macroPath);
    std::string content(
        (std::istreambuf_iterator<char>(file)),
         std::istreambuf_iterator<char>());

    // Вставляем в начало макроса отладочный вывод и переменную aRow
    const std::string debugLine = fmt::format(
        "rastr.print(f\"run contextmacro: {}[row={}]\")\n",
        lm.macrofile, contextRow);
    const std::string aRowLine =
        "aRow=" + std::to_string(contextRow) + "\n";

    content.insert(0, aRowLine);
    content.insert(0, debugLine);

    if (m_pyHlp)
    {
        PyHlp::Result py_res = m_pyHlp->run(content.data());
        switch(py_res)
        {
            case PyHlp::Result::Error:
                spdlog::warn("LinkedFormController:  m_pyHlp->run() return Error!");
                break;
            case PyHlp::Result::RuntimeError:
                spdlog::warn("LinkedFormController:  m_pyHlp->run() return RuntimeError!");
                break;
            case PyHlp::Result::SyntaxError:
                spdlog::warn("LinkedFormController:  m_pyHlp->run() return SyntaxError!");
                break;
            default:
                break;
        }
    }
    else
        spdlog::warn("LinkedFormController: PyHlp не установлен, макрос не выполнен!");
}

void LinkedFormController::onParentRowChanged(int newRow)
{
    spdlog::debug("LinkedFormController::onParentRowChanged: row = {}", newRow);

    m_lf.row = newRow;

    // fillBindVals обращается к pbaseform->getLongValue
    m_lf.fillBindVals();

    applyLinkedForm(m_lf);
}

int LinkedFormController::getLongValue(const std::string& col, long row)
{
    // Читаем значение напрямую из кешированного DataBlock модели.
    // Используется только при построении меню (начальное заполнение vbindvals).
    const int colIdx = m_model->getRdata()->mCols_.at(col);
    return std::visit(ToLong(),
                      m_model->getRdata()->pnparray_->Get(row, colIdx));
}
