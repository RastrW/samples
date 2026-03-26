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
#include "sdl/graphSDLManager.h"

FormManager::FormManager
    (std::shared_ptr<QAstra> qastra,
     ads::CDockManager* dockManager,
     std::shared_ptr<PyHlp> pPyHlp,
     LogManager* logManager,
     QWidget* parent)
    : QObject(parent)
    , m_qastra(qastra)
    , m_dockManager(dockManager)
    , m_pPyHlp(pPyHlp)
    , m_parentWidget(parent)
    , m_logManager(logManager)
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

    connect(m_logManager, &LogManager::dockWidgetCreated,
            this, &FormManager::registerDockWidget);
}

const QSet<QString>& FormManager::protocolDockNames() {
    return m_protocolNames;
}

void FormManager::setForms(const std::list<CUIForm>& forms) {
    m_forms = forms;
    m_rtdm.setForms(&m_forms);

    int index = 0;
    for (const auto& form : forms) {
        // Используем UTF-8
        QString formName = QString::fromStdString(
            stringutils::MkToUtf8(form.Name()));
        m_formNameToIndex[formName] = index++;
    }
}

void FormManager::openForm(const CUIForm& form) {
    try {
        // Проверка существования таблицы
        IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
        IRastrPayload res{ tablesx->FindIndex(form.TableName()) };
        int t_ind = res.Value();

        if (t_ind < 0) {
            spdlog::info("Таблица [{}] - [{}] не существует!",
                         form.Name(), form.TableName());
            return;
        }

        spdlog::info("Прочитана таблица [{}] - [{}]",
                     form.Name(), form.TableName());

        // Докирование
        auto dw = new ads::CDockWidget(QString::fromStdString(form.Name()),
                                       m_parentWidget);
        dw->setObjectName(QString::fromStdString(form.Name()));
        dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);

        // Создание виджета формы
        RtabWidget* prtw = new RtabWidget(
            m_qastra.get(),
            form,
            &m_rtdm,
            m_dockManager,
            dw);

        if (prtw != nullptr) {
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
            connect(dw, &ads::CDockWidget::closed,
                    prtw, &RtabWidget::slot_close);
            connect(dw, &ads::CDockWidget::closed,
                    this, [this, prtw, formName = form.Name()]() {
                        m_openForms.removeOne(prtw);
                        emit formClosed(QString::fromStdString(formName));
                    });

            m_activeForm = prtw;
        }

        emit formOpened(QString::fromStdString(form.Name()));
        emit activeFormChanged(prtw);

    } catch (const std::exception& ex) {
        spdlog::error("openForm('{}') threw: {}", form.Name(), ex.what());
    } catch (...) {
        spdlog::error("openForm('{}') threw unknown exception", form.Name());
    }
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

void FormManager::createLogWidgets() {
    m_logManager->createWidgets();
}

void FormManager::setupLogDockWidgets() {
    m_logManager->setupDockWidgets();
}

void FormManager::slot_openProtocol() {
    m_logManager->openProtocol();
}

const QSet<QString>& protocolDockNames() {
    static const QSet<QString> kSet = {
        QStringLiteral("Полный протокол"),
        QStringLiteral("Протокол Astra")
    };
    return kSet;
}

void FormManager::openFormByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(m_forms.size())) {
        spdlog::error("Invalid form index: {}", index);
        return;
    }

    auto it = m_forms.begin();
    std::advance(it, index);
    CUIForm form = *it; // копия
    form.SetName(stringutils::MkToUtf8(form.Name()));
    openForm(form);
}

void FormManager::openFormByName(const QString& formName) {
    if (m_formNameToIndex.find(formName) == m_formNameToIndex.end()) {
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

void FormManager::registerDockWidget(ads::CDockWidget* dw) {
    m_openDockWidgets.append(dw);

    // Удаляем из списка только если виджет реально уничтожается при закрытии.
    // Лог-виджеты имеют CustomCloseHandling (только скрываются) —
    // они остаются в списке навсегда и всегда участвуют в cascade/tile.
    if (dw->features().testFlag(ads::CDockWidget::DockWidgetDeleteOnClose)) {
        connect(dw, &ads::CDockWidget::closed,
                this, [this, dw]() {
                    m_openDockWidgets.removeOne(dw);
                });
    }
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

void FormManager::buildFormsMenu(QMenu* parentMenu, QMenu* calcParametersMenu)
{
    if (!parentMenu) {
        return;
    }

    // Кэш меню: ключ = категория ("" → "Остальное")
    std::unordered_map<QString, QMenu*> menuMap;

    int index = 0;

    for (const auto& form : m_forms) {

        // --- Подготовка данных ---
        const std::string nameUtf8 = stringutils::MkToUtf8(form.Name());
        const std::string pathUtf8 = stringutils::MkToUtf8(form.MenuPath());

        const QString qName = QString::fromStdString(nameUtf8);

        QString menuKey; // "" = Остальное

        if (!pathUtf8.empty()) {
            auto parts = split(pathUtf8, '\\');

            if (form.AddToMenuIndex() < parts.size()) {
                menuKey = QString::fromStdString(parts[form.AddToMenuIndex()]);
            }
            // иначе остаётся "", т.е. "Остальное"
        }

        // --- Получаем или создаём меню ---
        QMenu* targetMenu = nullptr;

        auto it = menuMap.find(menuKey);
        if (it == menuMap.end()) {
            targetMenu = parentMenu->addMenu(
                menuKey.isEmpty() ? QStringLiteral("Остальное") : menuKey
                );
            menuMap.emplace(menuKey, targetMenu);
        } else {
            targetMenu = it->second;
        }

        // --- Спец-логика: параметры расчёта ---
        if (calcParametersMenu && form.AddToMenuIndex() == 2) {
            targetMenu = calcParametersMenu;
        }

        // --- Добавление action ---
        if (!qName.isEmpty() && qName[0] != '_') {
            QAction* action = targetMenu->addAction(qName);
            action->setData(index);
        }

        ++index;
    }

    // --- Подключение сигналов ---
    auto connectMenu = [this](QMenu* menu) {
        if (!menu) return;

        connect(menu, &QMenu::triggered,
                this, &FormManager::slot_formMenuTriggered,
                Qt::UniqueConnection);
    };

    connectMenu(parentMenu);
    connectMenu(calcParametersMenu);
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

QStringList FormManager::openWidgetNames() const {
    QStringList result;
    const auto& allDocks = m_dockManager->dockWidgetsMap();
    for (auto it = allDocks.cbegin(); it != allDocks.cend(); ++it) {
        const ads::CDockWidget* dw = it.value();
        if (dw && !dw->isClosed() && !dw->objectName().isEmpty())
            result.append(dw->objectName());
    }
    return result;
}

void FormManager::openWidgetByName(const QString& name) {
    try {
        // ── Протоколы: только показать (не создавать заново) ──────────────
        if (protocolDockNames().contains(name)) {
            const auto allDocks = m_dockManager->dockWidgetsMap();
            auto it = allDocks.find(name);
            if (it != allDocks.end()) {
                it.value()->toggleView(true);
            } else {
                // setupDockWidgets ещё не был вызван — безопасно пропускаем,
                // restoreState расставит их позже
                spdlog::warn("Log dock '{}' not in ADS yet, skipping",
                             name.toStdString());
            }
            return;
        }

        // ── Графика ───────────────────────────────────────────────────────
        if (name == QStringLiteral("Графика SDL")) { slot_openSDLGraph(); return; }
        if (name == QStringLiteral("Графика Web")) { slot_openWebGraph(); return; }

        // ── Таблицы ───────────────────────────────────────────────────────
        openFormByName(name);

    } catch (const std::exception& ex) {
        spdlog::error("openWidgetByName('{}') threw: {}",
                      name.toStdString(), ex.what());
    } catch (...) {
        spdlog::error("openWidgetByName('{}') threw unknown exception",
                      name.toStdString());
    }
}

void FormManager::closeAllWidgets() {
    // Итерируем по копии карты: closeDockWidget() модифицирует состояние ADS
    const auto allDocks = m_dockManager->dockWidgetsMap();
    for (auto it = allDocks.cbegin(); it != allDocks.cend(); ++it) {
        ads::CDockWidget* dw = it.value();
        if (!dw) continue;
        if (protocolDockNames().contains(dw->objectName())) continue; // протокол — skip
        if (!dw->isClosed())
            dw->closeDockWidget();
    }

    m_openForms.clear();
    m_openDockWidgets.clear();
}