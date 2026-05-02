#include "tableDockManager.h"
#include "table/rtabcontroller.h"
#include "table/rtabshell.h"
#include <QFileInfo>
#include <DockManager.h>
#include <QMenu>
#include "utils.h"
#include "table/linkedForm/linkedFormBond.h"


TableDockManager::TableDockManager(
    std::shared_ptr<ITableRepository> tables,
    std::shared_ptr<ITableEvents>     tableEvents,
    ads::CDockManager*                dockManager,
    std::shared_ptr<PyHlp>            pyHlp,
    QWidget*                          parent)
    : QObject(parent)
    , m_tables(tables)
    , m_tableEvents(tableEvents)
    , m_dockManager(dockManager)
    , m_pyHlp(pyHlp)
    , m_parentWidget(parent)
{}

void TableDockManager::setForms(const std::vector<CUIForm>& forms) {
    m_pForms = &forms;
    m_tables->setForms(forms);

    int index = 0;
    for (const auto& form : forms) {
        const QString qName = QString::fromStdString(form.DisplayName());
        m_formNameToIndex[qName] = index++;
    }
}

std::pair<ads::CDockWidget*, RtabController*>
TableDockManager::openForm(const CUIForm& form,
                           std::optional<LinkedForm> linkedFilter)
{
    try {
        TableProperties tabProp;
        tabProp.formQName   = QString::fromStdString(form.DisplayName());
        tabProp.isLinked    = linkedFilter.has_value();
        tabProp.isVertical  = form.Vertical();
        tabProp.withToolbar = !tabProp.isLinked && !tabProp.isVertical;

        if (!m_tables->tableExists(form.TableName())) {
            spdlog::info("Таблица [{}] не существует!", form.TableName());
            return {nullptr, nullptr};
        }
        // ── Dock-виджет ───────────────────────────────────────────────────────
        auto* dw = new ads::CDockWidget(tabProp.formQName, m_parentWidget);
        dw->setObjectName(tabProp.formQName);
        dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
        // ── Виджет таблицы ────────────────────────────────────────────────────
        auto* ctrl = new RtabController(
            m_tables, m_tableEvents, form, m_dockManager, this, dw);

        auto* shell = ctrl->createShell(tabProp);
        if (!shell) { dw->deleteLater(); return {nullptr, nullptr}; }
        // true (по умолчанию) = с тулбаром и шорткатами
        dw->setWidget(shell);

        if (tabProp.isLinked) {
            ctrl->applyLinkedFormFromController(*linkedFilter);
            m_dockManager->addDockWidgetTab(ads::BottomAutoHideArea, dw);
        } else {
            m_dockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
        }

        ctrl->setPyHlp(m_pyHlp);
        // Добавляем в список открытых форм
        m_openForms.append(ctrl);
        m_activeForm = ctrl;
        // Уведомляем FormManager о новом dock-виджете
        emit windowOpened(dw);

        connect(dw, &ads::CDockWidget::closed,
                this, [this, ctrl, formQName = tabProp.formQName]() {
                    m_openForms.removeOne(ctrl);
                    if (m_activeForm == ctrl) m_activeForm = nullptr;
                    emit formClosed(formQName);
                });

        emit formOpened(tabProp.formQName);
        emit activeFormChanged(ctrl);
        return {dw, ctrl};
    } catch (const std::exception& ex) {
        spdlog::error("TableDockManager::openForm('{}') threw: {}",
                      form.DisplayName(), ex.what());
        return {nullptr, nullptr};
    } catch (...) {
        spdlog::error("TableDockManager::openForm('{}') threw unknown exception",
                      form.DisplayName());
        return {nullptr, nullptr};
    }
}

void TableDockManager::openFormByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_pForms->size())) {
        spdlog::error("TableDockManager: invalid form index {}", index);
        return;
    }
    openForm((*m_pForms)[index]);
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

RtabController* TableDockManager::openLinkedForm(
    const LinkedForm&      lf,
    Qtitan::GridTableView* parentView,
    RtabController*        parentCtrl)
{
    const CUIForm* pUIForm = m_tables->getForm(lf.linkedform);
    if (!pUIForm) {
        spdlog::error("TableDockManager::openLinkedForm: form '{}' not found",
                      lf.linkedform);
        return nullptr;
    }

    // Создаём dock и контроллер через openForm с фильтром
    // openForm регистрирует dock в FormManager через windowOpened
    auto [childDock, childCtrl] = openForm(*pUIForm, lf);
    if (!childCtrl) return nullptr;

    // Создаём bond — он сам управляет временем жизни связи
    // parent = parentCtrl: bond удалится вместе с родительским контроллером
    new LinkedFormBond(parentCtrl, parentView, childCtrl, childDock);
    return childCtrl;
}

void TableDockManager::buildFormsMenu(QMenu* parentMenu,
                                      QMenu* calcParametersMenu)
{
    if (!parentMenu) return;
    // Кэш меню: ключ = категория ("" → "Остальное")
    std::unordered_map<QString, QMenu*> menuMap;
    int index = 0;

    for (const auto& form : *m_pForms) {
         // --- Подготовка данных ---
        const std::string pathUtf8 = stringutils::MkToUtf8(form.MenuPath());
        const QString     qName    = QString::fromStdString(form.DisplayName());

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

void TableDockManager::slot_formMenuTriggered(QAction* action) {
    const int index = action->data().toInt();
    if (index < 0 || index >= static_cast<int>(m_pForms->size())) return;
    openForm((*m_pForms)[index]);
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

    // Вся COM-логика скрыта за интерфейсом
    std::vector<CUIForm> dynamicForms;
    m_tables->getDynamicForms(dynamicForms);

    for (const CUIForm& form : dynamicForms) {
        // Добавляем действие в меню
        QAction* action = menu->addAction(
            QString::fromStdString(form.DisplayName()));
        // При клике - открываем динамическую форму
        connect(action, &QAction::triggered,
                this,   [this, form]() { openForm(form); });
    }
}