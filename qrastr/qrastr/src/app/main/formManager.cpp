#include "formManager.h"
#include "qastra.h"
#include "rtablesdatamanager.h"
#include "rtabwidget.h"
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <DockWidget.h>
#include <DockManager.h>
#include <spdlog/spdlog.h>
#include <QFileInfo>

#include <astra/stringutils.h>
#include "utils.h"
using WrapperExceptionType = std::runtime_error;
#include "astra/IPlainRastrWrappers.h"
#include "web/graphWebManager.h"
#include "sdl/GraphSDLManager.h"

FormManager::FormManager
    (std::shared_ptr<QAstra> qastra,
     ads::CDockManager* dockManager,
     std::shared_ptr<PyHlp> pPyHlp,
     QWidget* parent)
    : QObject(parent)
    , m_qastra(qastra)
    , m_dockManager(dockManager)
    , m_pPyHlp(pPyHlp)
    , m_parentWidget(parent)
{
    assert(m_qastra != nullptr);
    assert(m_dockManager != nullptr);
    assert(m_pPyHlp != nullptr);

    m_rtdm.setQAstra(qastra.get());

    m_graphSDLManager = new GraphSDLManager(m_dockManager, m_parentWidget,
                                            m_qastra->getRastr().get(), this);
    m_graphWebManager = new GraphWebManager(m_dockManager, m_parentWidget,
                                            m_qastra->getRastr().get(), this);

    for (IGraphManager* mgr : {m_graphSDLManager, m_graphWebManager}) {
        connect(mgr, &IGraphManager::windowOpened,
                this, &FormManager::registerDockWidget);
    }
}

void FormManager::setForms(const std::list<CUIForm>& forms) {
    m_forms = forms;
    m_rtdm.setForms(&m_forms);
    
    // Создаём индекс для быстрого поиска
    int index = 0;
    for (const auto& form : forms) {
        QString formName = QString::fromStdString(form.Name());
        m_formNameToIndex[formName] = index++;
    }
}

void FormManager::openForm(const CUIForm& form) {
    // Проверка существования таблицы
    IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
    IRastrPayload res{ tablesx->FindIndex(form.TableName()) };
    int t_ind = res.Value();
    
    if (t_ind < 0) {
        spdlog::info("Таблица [{}] - [{}] не существует!", form.Name(), form.TableName());
        return;
    }
    
    spdlog::info("Прочитана таблица [{}] - [{}]", form.Name(), form.TableName());

    // Докирование
    auto dw = new ads::CDockWidget(QString::fromStdString(form.Name()),
                                    m_parentWidget);
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);

    // Создание виджета формы
    RtabWidget* prtw = new RtabWidget(
        m_qastra.get(),
        form,
        &m_rtdm,
        m_dockManager,
        dw);

    if (prtw != nullptr){
        prtw->setPyHlp(m_pPyHlp);
        // Выравнивание данных по шаблону, выравнивание текста по левому краю
        prtw->slot_widthByTemplate();
        // Добавляем в список открытых форм
        // Сигналы будут передаваться через onCalculationStarted/Finished
        m_openForms.append(prtw);
        registerDockWidget(dw);

        dw->setWidget(prtw->createDockContent());

        m_dockManager->addDockWidgetTab(ads::TopDockWidgetArea, dw);
        // Обработка закрытия
        connect(dw, &ads::CDockWidget::closed, prtw, &RtabWidget::slot_close);
        connect(dw, &ads::CDockWidget::closed,
                this, [this, prtw, dw, formName = form.Name()]() {
                    m_openForms.removeOne(prtw);
                    emit formClosed(QString::fromStdString(formName));
                });

        m_activeForm = prtw;
    }

    emit formOpened(QString::fromStdString(form.Name()));
    emit activeFormChanged(prtw);
}

void FormManager::slot_openSDLGraph() {
    m_graphSDLManager->openWindow();
    emit formOpened("Графика SDL");
}

void FormManager::slot_openWebGraph() {  
    m_graphWebManager->openWindow();
    emit formOpened("Графика Web");
}

void FormManager::closeGraphWebServer(){
    m_graphWebManager->closeAll();
}

void FormManager::openFormByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_forms.size())) {
        spdlog::error("Invalid form index: {}", index);
        return;
    }
    
    auto it = m_forms.begin();
    std::advance(it, index);
    openForm(*it);
}

void FormManager::openFormByName(const QString& formName) {
    if (!m_formNameToIndex.contains(formName)) {
        spdlog::error("Form not found: {}", formName.toStdString());
        return;
    }
    
    int index = m_formNameToIndex[formName];
    openFormByIndex(index);
}

void FormManager::slot_calculationStarted() {
    // Передаём сигнал ВСЕМ открытым формам
    for (RtabWidget* widget : m_openForms) {
        if (widget) {
            widget->on_calc_begin();
        }
    }
}

void FormManager::slot_calculationFinished() {
    // Передаём сигнал ВСЕМ открытым формам
    for (RtabWidget* widget : m_openForms) {
        if (widget) {
            widget->on_calc_end();
        }
    }
}

void FormManager::buildFormsMenu
    (QMenu* parentMenu, QMenu* calcParametersMenu) {

    QMap<QString, QMenu*> map_menu;  
    // Первый проход - создание подменю
    for (const auto& j_form : m_forms) {
        std::string str_MenuPath = stringutils::MkToUtf8(j_form.MenuPath());
        auto vmenu = split(str_MenuPath, '\\');
        
        std::string str_Name = stringutils::MkToUtf8(j_form.Name());
        
        if (str_MenuPath.empty()) {
            continue;
        }
        
        if (j_form.AddToMenuIndex() >= vmenu.size()) {
            continue;
        }
        
        QString qstr_MenuPath = QString::fromStdString(vmenu[j_form.AddToMenuIndex()]);
        
        if (!map_menu.contains(qstr_MenuPath)) {
            map_menu.insert(
                qstr_MenuPath,
                parentMenu->addMenu(qstr_MenuPath.isEmpty() ? "Остальное" : qstr_MenuPath)
            );
        }
    }
    
    // Второй проход - добавление действий
    int i = 0;
    for (const auto& j_form : m_forms) {
        std::string str_Name = stringutils::MkToUtf8(j_form.Name());
        std::string str_MenuPath = stringutils::MkToUtf8(j_form.MenuPath());
        auto vmenu = split(str_MenuPath, '\\');
        
        if (!str_MenuPath.empty() && j_form.AddToMenuIndex() >= vmenu.size()) {
            i++;
            continue;
        }
        
        QString qstr_MenuPath;
        if (str_MenuPath.empty()) {
            qstr_MenuPath = "";
        } else {
            qstr_MenuPath = QString::fromStdString(vmenu[j_form.AddToMenuIndex()]);
        }
        
        QMenu* cur_menu = parentMenu;
        if (map_menu.contains(qstr_MenuPath)) {
            cur_menu = map_menu[qstr_MenuPath];
        }
        
        // Параметры расчётов
        if (calcParametersMenu && j_form.AddToMenuIndex() == 2) {
            cur_menu = calcParametersMenu;
        }
        
        // Добавляем действие (если имя не начинается с '_')
        if (!str_Name.empty() && str_Name.at(0) != '_') {
            QAction* p_actn = cur_menu->addAction(QString::fromStdString(str_Name));
            p_actn->setData(i);
        }
        
        i++;
    }
    
    // Подключаем обработчик
    connect(parentMenu, &QMenu::triggered,
            this, &FormManager::slot_formMenuTriggered,
            Qt::UniqueConnection);
    
    if (calcParametersMenu) {
        connect(calcParametersMenu, &QMenu::triggered,
                this, &FormManager::slot_formMenuTriggered,
                Qt::UniqueConnection);
    }
}

void FormManager::slot_formMenuTriggered(QAction* p_actn) {
    const int n_indx = p_actn->data().toInt();
    
    auto it = m_forms.begin();
    std::advance(it, n_indx);
    
    auto form = *it;
    form.SetName(stringutils::MkToUtf8(form.Name()));
    
    openForm(form);
}

void FormManager::buildPropertiesMenu(QMenu* propertiesMenu) {
    // Подключаемся к сигналу aboutToShow для динамической генерации
    connect(propertiesMenu, &QMenu::aboutToShow,
            this, [this, propertiesMenu]() {
                generateDynamicForms(propertiesMenu);
            });
}

void FormManager::generateDynamicForms(QMenu* menu) {
    // Очищаем меню перед обновлением
    menu->clear();
    
    IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
    IRastrPayload cnt{ tablesx->Count() };
    
    for (int i = 0; i < cnt.Value(); i++) {
        IRastrTablePtr table{ tablesx->Item(i) };
        IRastrPayload tab_name{ table->Name() };
        IRastrPayload templ_name{ table->TemplateName() };
        IRastrPayload tab_desc{ table->Description() };
        
        std::string str_tab_name = tab_name.Value();
        std::string str_templ_name = templ_name.Value();
        
        // Проверяем, что шаблон таблицы - это .form файл
        if (QFileInfo(QString::fromStdString(str_templ_name)).suffix() == "form") {
            // Создаём динамическую форму
            CUIForm _form;
            _form.SetName(tab_desc.Value());
            _form.SetTableName(tab_name.Value());
            
            // Определяем вертикальность (если 1 строка)
            IRastrColumnsPtr columns{ table->Columns() };
            size_t nrows = IRastrPayload{ table->Size() }.Value();
            size_t ncols = IRastrPayload{ columns->Count() }.Value();
            
            if (nrows == 1) {
                _form.SetVertical(true);
            }
            
            // Добавляем поля из колонок
            for (size_t j = 0; j < ncols; j++) {
                IRastrColumnPtr column{ columns->Item(j) };
                IRastrPayload col_name{ column->Name() };
                _form.Fields().emplace_back(col_name.Value());
            }
            
            // Добавляем действие в меню
            QAction* p_actn = menu->addAction(QString::fromStdString(_form.Name()));
            
            // При клике - открываем динамическую форму
            connect(p_actn, &QAction::triggered,
                    this, [this, _form]() {
                        openForm(_form);
                    });
        }
    }
}

void FormManager::slot_formClosed() {
    // Обработка закрытия формы
    // (уже обрабатывается в лямбде при создании DockWidget)
}

CUIForm* FormManager::findFormByName(const QString& name) {
    for (auto& form : m_forms) {
        if (QString::fromStdString(form.Name()) == name) {
            return &form;
        }
    }
    return nullptr;
}

CUIForm* FormManager::findFormByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_forms.size())) {
        return nullptr;
    }
    
    auto it = m_forms.begin();
    std::advance(it, index);
    return &(*it);
}

QMap<QString, QMenu*> FormManager::buildMenuStructure(QMenu* rootMenu) {
    QMap<QString, QMenu*> menuMap;
    
    for (const auto& form : m_forms) {
        std::string menuPath = stringutils::MkToUtf8(form.MenuPath());
        if (menuPath.empty()) {
            continue;
        }
        
        QStringList pathParts = QString::fromStdString(menuPath).split('\\');
        
        if (form.AddToMenuIndex() >= pathParts.size()) {
            continue;
        }
        
        QString menuName = pathParts[form.AddToMenuIndex()];
        if (menuName.isEmpty()) {
            menuName = "Остальное";
        }
        
        if (!menuMap.contains(menuName)) {
            menuMap[menuName] = rootMenu->addMenu(menuName);
        }
    }
    
    return menuMap;
}

void FormManager::registerDockWidget(ads::CDockWidget* dw) {
    m_openDockWidgets.append(dw);
    connect(dw, &ads::CDockWidget::closed,
            this, [this, dw]() {
                m_openDockWidgets.removeOne(dw);
            });
}

void FormManager::slot_cascadeForms() {
    if (m_openDockWidgets.isEmpty()) return;

    // Берём геометрию области dock manager как опорную
    QRect available = m_dockManager->rect();
    QPoint origin   = m_dockManager->mapToGlobal(available.topLeft())
                    + QPoint(20, 20);

    constexpr int kOffset     = 30;  // шаг каскада в пикселях
    constexpr int kWidth      = 600;
    constexpr int kHeight     = 400;

    int step = 0;
    for (ads::CDockWidget* dw : m_openDockWidgets) {
        if (!dw) continue;

        // Открепляем виджет — делаем floating
        dw->setFloating();

        QPoint pos = origin + QPoint(step * kOffset, step * kOffset);

        // Позиционируем и задаём размер
        if (auto* floatContainer = dw->dockContainer()) {
            if (floatContainer->isFloating()) {
                floatContainer->window()->setGeometry(
                    pos.x(), pos.y(), kWidth, kHeight);
            }
        }

        step++;
    }
}

void FormManager::slot_tileForms() {
    if (m_openDockWidgets.isEmpty()) return;

    QRect available = m_dockManager->rect();
    int count = m_openDockWidgets.size();

    // Вычисляем сетку: cols x rows
    int cols = qMax(1, static_cast<int>(std::ceil(std::sqrt(count))));
    int rows = (count + cols - 1) / cols;

    int w = available.width()  / cols;
    int h = available.height() / rows;

    QPoint origin = m_dockManager->mapToGlobal(available.topLeft());

    int i = 0;
    for (ads::CDockWidget* dw : m_openDockWidgets) {
        if (!dw) continue;

        dw->setFloating();

        int col = i % cols;
        int row = i / cols;
        QPoint pos = origin + QPoint(col * w, row * h);

        if (auto* c = dw->dockContainer(); c && c->isFloating()) {
            c->window()->setGeometry(pos.x(), pos.y(), w, h);
        }
        i++;
    }
}
