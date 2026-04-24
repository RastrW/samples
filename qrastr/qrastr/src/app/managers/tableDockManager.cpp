#include "tableDockManager.h"
using WrapperExceptionType = std::runtime_error;
#include "astra/IPlainRastrWrappers.h"
#include "qastra.h"
#include "table/rtabcontroller.h"
#include "table/rtabshell.h"
#include <QFileInfo>
#include <DockManager.h>
#include <QMenu>
#include "utils.h"

TableDockManager::TableDockManager(
    std::shared_ptr<QAstra> qastra,
    ads::CDockManager*      dockManager,
    std::shared_ptr<PyHlp>  pyHlp,
    QWidget*                parent)
    : QObject(parent)
    , m_qastra(qastra)
    , m_dockManager(dockManager)
    , m_pyHlp(pyHlp)
    , m_parentWidget(parent)
    , m_rtdm(std::make_unique<RTablesDataManager>(m_qastra))
{}

void TableDockManager::setForms(const std::list<CUIForm>& forms){
    m_forms = forms;
    m_rtdm->setForms(&m_forms);

    int index = 0;
    for (const auto& form : m_forms) {
        const QString qName = QString::fromStdString(
            stringutils::MkToUtf8(form.Name()));
        m_formNameToIndex[qName] = index++;
    }
}

void TableDockManager::openForm(const CUIForm& form)
{
    try {
        IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
        IRastrPayload   res    { tablesx->FindIndex(form.TableName()) };
        const int t_ind = res.Value();

        if (t_ind < 0) {
            spdlog::info("Таблица [{}] - [{}] не существует!",
                         form.Name(), form.TableName());
            return;
        }

        spdlog::info("Прочитана таблица [{}] - [{}]",
                     form.Name(), form.TableName());
        // ── Dock-виджет ───────────────────────────────────────────────────────
        const QString qName = QString::fromStdString(form.Name());

        auto* dw = new ads::CDockWidget(qName, m_parentWidget);
        dw->setObjectName(qName);
        dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
        // ── Виджет таблицы ────────────────────────────────────────────────────
        auto* ctrl = new RtabController(
            m_qastra.get(), form, m_rtdm.get(), m_dockManager, dw);

        // true (по умолчанию) = с тулбаром и шорткатами
        dw->setWidget(ctrl->createShell());

        m_dockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
        ctrl->setPyHlp(m_pyHlp);
        // Добавляем в список открытых форм
        // Сигналы будут передаваться через onCalculationStarted/Finished
        m_openForms.append(ctrl);
        m_activeForm = ctrl;
        // Уведомляем FormManager о новом dock-виджете
        emit windowOpened(dw);

        connect(dw, &ads::CDockWidget::closed,
                ctrl, &RtabController::slot_close);

        connect(dw, &ads::CDockWidget::closed,
                this, [this, ctrl, name = form.Name()]() {
                    m_openForms.removeOne(ctrl);
                    if (m_activeForm == ctrl)
                        m_activeForm = nullptr;
                    emit formClosed(QString::fromStdString(name));
                });

        emit formOpened(qName);
        emit activeFormChanged(ctrl);

    } catch (const std::exception& ex) {
        spdlog::error("TableDockManager::openForm('{}') threw: {}",
                      form.Name(), ex.what());
    } catch (...) {
        spdlog::error("TableDockManager::openForm('{}') threw unknown exception",
                      form.Name());
    }
}

void TableDockManager::openFormByIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(m_forms.size())) {
        spdlog::error("TableDockManager: invalid form index {}", index);
        return;
    }

    auto it = m_forms.begin();
    std::advance(it, index);

    CUIForm form = *it; // копия
    form.SetName(stringutils::MkToUtf8(form.Name()));
    openForm(form);
}

void TableDockManager::openFormByName(const QString& formName)
{
    const auto it = m_formNameToIndex.find(formName);
    if (it == m_formNameToIndex.end()) {
        spdlog::error("TableDockManager: form not found '{}'",
                      formName.toStdString());
        return;
    }
    openFormByIndex(it->second);
}

void TableDockManager::slot_calculationStarted(){}

void TableDockManager::slot_calculationFinished(){}

void TableDockManager::buildFormsMenu(QMenu* parentMenu,
                                      QMenu* calcParametersMenu)
{
    if (!parentMenu) return;
    // Кэш меню: ключ = категория ("" → "Остальное")
    std::unordered_map<QString, QMenu*> menuMap;
    int index = 0;

    for (const auto& form : m_forms) {
         // --- Подготовка данных ---
        const std::string nameUtf8 = stringutils::MkToUtf8(form.Name());
        const std::string pathUtf8 = stringutils::MkToUtf8(form.MenuPath());
        const QString     qName    = QString::fromStdString(nameUtf8);

        QString menuKey; // "" = Остальное

        if (!pathUtf8.empty()) {
            const auto parts = split(pathUtf8, '\\');
            if (form.AddToMenuIndex() < parts.size()){
                menuKey = QString::fromStdString(parts[form.AddToMenuIndex()]);
            }
            // иначе остаётся "", т.е. "Остальное"
        }

        // Получаем или создаём подменю
        auto it = menuMap.find(menuKey);
        QMenu* targetMenu = (it == menuMap.end())
                                ? menuMap.emplace(menuKey,
                                                  parentMenu->addMenu(
                                                      menuKey.isEmpty() ? QStringLiteral("Остальное") : menuKey)
                                                  ).first->second
                                : it->second;

        // Спец-случай: параметры расчёта
        if (calcParametersMenu && form.AddToMenuIndex() == 2)
            targetMenu = calcParametersMenu;

        if (!qName.isEmpty() && qName[0] != '_') {
            QAction* action = targetMenu->addAction(qName);
            action->setData(index);
        }

        ++index;
    }

    // Один connect на всё дерево: triggered всплывает до parentMenu
    auto connectMenu = [this](QMenu* menu) {
        if (!menu) return;
        connect(menu, &QMenu::triggered,
                this, &TableDockManager::slot_formMenuTriggered,
                Qt::UniqueConnection);
    };

    connectMenu(parentMenu);
    connectMenu(calcParametersMenu);
}

void TableDockManager::slot_formMenuTriggered(QAction* action)
{
    const int index = action->data().toInt();

    auto it = m_forms.begin();
    std::advance(it, index);

    CUIForm form = *it;
    form.SetName(stringutils::MkToUtf8(form.Name()));
    openForm(form);
}

void TableDockManager::buildPropertiesMenu(QMenu* propertiesMenu)
{
    // Меню перестраивается при каждом открытии
    connect(propertiesMenu, &QMenu::aboutToShow,
            this, [this, propertiesMenu]() {
                generateDynamicForms(propertiesMenu);
            });
}

void TableDockManager::generateDynamicForms(QMenu* menu)
{
    // Очищаем меню перед обновлением
    menu->clear();

    IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
    IRastrPayload   cnt    { tablesx->Count() };

    for (int i = 0; i < cnt.Value(); ++i) {
        IRastrTablePtr  table     { tablesx->Item(i) };
        IRastrPayload   tab_name  { table->Name() };
        IRastrPayload   templ_name{ table->TemplateName() };
        IRastrPayload   tab_desc  { table->Description() };

        const std::string str_templ = templ_name.Value();
        // Проверяем, что шаблон таблицы - это .form файл
        if (QFileInfo(QString::fromStdString(str_templ)).suffix() != "form")
            continue;
        // Создаём динамическую форму
        CUIForm form;
        form.SetName(tab_desc.Value());
        form.SetTableName(tab_name.Value());
        // Определяем вертикальность (если 1 строка)
        IRastrColumnsPtr columns{ table->Columns() };
        const size_t nrows = IRastrPayload{ table->Size()          }.Value();
        const size_t ncols = IRastrPayload{ columns->Count()       }.Value();

        if (nrows == 1)
            form.SetVertical(true);
        // Добавляем поля из колонок
        for (size_t j = 0; j < ncols; ++j) {
            IRastrColumnPtr col     { columns->Item(j) };
            IRastrPayload   col_name{ col->Name()       };
            form.Fields().emplace_back(col_name.Value());
        }
        // Добавляем действие в меню
        QAction* action = menu->addAction(QString::fromStdString(form.Name()));
        // При клике - открываем динамическую форму
        connect(action, &QAction::triggered,
                this,   [this, form]() { openForm(form); });
    }
}