#include "linkedformcontroller.h"

#include "rtabController.h"
#include "rtabshell.h"
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

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include "ITableRepository.h"

LinkedFormController::LinkedFormController(
    QAstra*                  qastra,
    RTablesDataManager*      pRTDM,
    ads::CDockManager*       dockManager,
    Qtitan::GridTableView*   view,
    RModel*                  model,
    const CUIForm&           form,
    RtabController*          parentController)
    : QObject(parentController)
    , m_qastra(qastra)
    , m_rtdm(pRTDM)
    , m_dockManager(dockManager)
    , m_view(view)
    , m_model(model)
    , m_form(form)
    , m_parentController(parentController)
{}
void LinkedFormController::disconnectAll()
{
    // Отключаем соединения focusRowChanged → child
    for (auto& conn : m_lf.vconn)
        QObject::disconnect(conn);
    m_lf.vconn.clear();

    // Дочерний dock закроется сам при удалении CDockManager.
    m_childDockWidget = nullptr;
}

void LinkedFormController::openLinkedForm(LinkedForm lf)
{
    CUIForm* pUIForm = m_rtdm->getForm(lf.linkedform);
    if (!pUIForm) return;
    // Создаём дочерний виджет — он заведёт собственный LinkedFormController
    auto* child = new RtabController(
        m_qastra,
        *pUIForm,
        m_rtdm,
        m_dockManager,
        nullptr); //parent для QObject, dock управляет временем жизни

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

    child->applyLinkedFormFromController(lf);

    m_childDockWidget = new ads::CDockWidget(
        stringutils::MkToUtf8(pUIForm->Name()).c_str(), nullptr);

    m_childDockWidget->setWidget(child->createShell(false)); // false = без тулбара
    m_childDockWidget->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);

    // Когда дочерний dock закрывается пользователем — зануляем указатель
    connect(m_childDockWidget, &ads::CDockWidget::closed,
            this, [this]() { m_childDockWidget = nullptr; });

    // Когда дочерний dock закрывается — вызываем slot_close у child
    connect(m_childDockWidget, &ads::CDockWidget::closed,
            child, &RtabController::slot_close);

    // Когда РОДИТЕЛЬСКИЙ контроллер уничтожается — закрываем дочерний dock.
    // Используем destroyed (не closed) — это безопасно: сигнал из деструктора,
    // не из цепочки закрытия dock.
    connect(m_parentController, &QObject::destroyed,
            m_childDockWidget, [dw = m_childDockWidget]() {
                dw->closeDockWidget();
            });

    m_dockManager->addDockWidgetTab(ads::BottomAutoHideArea, m_childDockWidget);
}

void LinkedFormController::openLinkedMacro(LinkedMacro lm, int contextRow)
{
    spdlog::info("LinkedFormController: run macro {}", lm.macrofile.c_str());

    // ── Путь к макросу ────────────────────────────────────────────
    const QString macroPath =
        QDir::current().filePath(
            "contextmacro/" +
            QFileInfo(QString::fromStdString(lm.macrofile)).completeBaseName() +
            ".py");

    if (!QFileInfo::exists(macroPath)) {
        spdlog::warn("context macro not found: {}", macroPath.toStdString());
        return;
    }

    // Чтобы макросы работали в Data должен лежать astra_py. модуль
#ifdef _WIN32
    const QString astraPyFile = "astra_py.cp312-win_amd64.pyd";
#else
    const QString astraPyFile = "astra_py.cpython-311-x86_64-linux-gnu.so";
#endif
    // QDir::cleanPath убирает лишние /../ из пути
    const QString astraPyPath =
        QDir::cleanPath(
            QDir::current().filePath("../Data/astr_py/Release/" + astraPyFile));

    if (!QFileInfo::exists(astraPyPath)) {
        spdlog::warn("astra_py module not found: {}", astraPyPath.toStdString());
        return;
    }

    // ── Чтение файла макроса ──────────────────────────────────────
    QFile file(macroPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        spdlog::error("Cannot open macro file: {}", macroPath.toStdString());
        return;
    }
    std::string content = file.readAll().toStdString();

    // ── Вставляем отладочный заголовок ───────────────────────────
    const std::string debugLine = fmt::format(
        "rastr.print(f\"run contextmacro: {}[row={}]\")\n",
        lm.macrofile, contextRow);
    const std::string aRowLine =
        "aRow=" + std::to_string(contextRow) + "\n";

    content.insert(0, aRowLine);
    content.insert(0, debugLine);

    // ── Запуск ───────────────────────────────────────────────────
    if (!m_pyHlp) {
        spdlog::warn("LinkedFormController: PyHlp не установлен, макрос не выполнен!");
        return;
    }

    switch (m_pyHlp->run(content.data())) {
    case PyHlp::Result::Error:
        spdlog::warn("LinkedFormController: m_pyHlp->run() return Error!");        break;
    case PyHlp::Result::RuntimeError:
        spdlog::warn("LinkedFormController: m_pyHlp->run() return RuntimeError!"); break;
    case PyHlp::Result::SyntaxError:
        spdlog::warn("LinkedFormController: m_pyHlp->run() return SyntaxError!");  break;
    default:
        break;
    }
}

void LinkedFormController::buildLinkedFormsMenu(int contextRow, QMenu* menu)
{
    auto table = m_rtdm->getBlock("formcontext", "");

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
    auto table = m_rtdm->getBlock("macrocontext", "");

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

    const std::string tname     = m_model->getRdata()->t_name_;
    const std::string selection = lf.get_selection_result();

    const std::vector<long> indices = m_rtdm->rowsBySelection(tname, selection);

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

void LinkedFormController::setPyHlp(std::shared_ptr<PyHlp> pyHlp)
{
    m_pyHlp = pyHlp;
}

