#include "uiBuilder.h"

#include <QMenu>
#include <QMenuBar>
#include <QFileInfo>
#include <QApplication>
#include <QStyle>
#include <QToolBar>
#include <QStyleFactory>
#include <QStatusBar>
#include <QActionGroup>
#include <QSettings>
#include "params.h"

UIBuilder::UIBuilder(QMainWindow* mainWindow)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
{
    assert(m_mainWindow != nullptr);
}

void UIBuilder::buildAll() {
    // Создание всех действий
    createFileActions();
    createCalcActions();
    createTIActions();
    createBarsMDPActions();
    createGraphActions();
    createMacroActions();
    createWindowActions();
    createHelpActions();
    createStyleActions();
    // Построение меню и панелей
    buildMenuBar();
    buildToolBars();
    buildStatusBar();
}

void UIBuilder::createFileActions() {
    addAction("new",
              tr("&Новый"),
              ":/images/new_style/document.png",
              "Ctrl+N",
              tr("Create a new file"));
    addAction("load",
              tr("&Загрузить"),
              ":/images/new_style/open document.png",
              "Ctrl+O",
              tr("Load an existing file"));
    addAction("save",
              tr("&Сохранить"),
              ":/images/new_style/save.png",
              "Ctrl+S",
              tr("Save the document to disk"));
    addAction("saveAs",
              tr("&Сохранить как"),
              ":/images/new_style/save as.png",
              "",
              tr("Save the document under a new name"));
    addAction("saveAll",
              tr("&Сохранить все"),
              ":/images/new_style/save all.png",
              "",
              tr("Save all the document"));
    addAction("settings",
              tr("&Параметры"),
              ":/images/new_style/settings.png",
              "",
              tr("Open settings form."));
    addAction("exit",
              tr("E&xit"),
              ":/images/new_style/exit.png",
              "Ctrl+Q",
              tr("Exit the application"));

    // НЕДАВНИЕ ФАЙЛЫ
    // Создаём массив действий для недавних файлов
    QSettings s;
    int maxAllowed = s.value("maxRecentFiles", 10).toInt();
    for (int i = 0; i < maxAllowed; ++i) {
        QString actionName = QString("recentFile%1").arg(i);
        QAction* recentAction = new QAction(m_mainWindow);
        recentAction->setVisible(false);
        m_actions[actionName] = recentAction;
    }
}

void UIBuilder::createCalcActions() {
    addAction("kdd",
              tr("&Контроль"),
              "",
              "",
              tr("Контроль исходных данных"));
    addAction("rgm",
              tr("&Режим"),
              ":/images/Rastr3_rgm_16x16.png",
              "F5",
              tr("Расчет УР"));
    addAction("opf",
              tr("&ОС"),
              ":/images/Bee.png",
              "F6",
              tr("Оценка состояния"));
    addAction("smzu",
              tr("&МДП"),
              ":/images/mdp_16.png",
              "F7",
              tr("Расчет МДП"));
    addAction("kz",
              tr("&ТКЗ"),
              ":/images/TKZ_48.png",
              "F8",
              tr("Расчет ТКЗ"));
    addAction("idop",
              tr("&Доп. ток от Т"),
              "",
              "F9",
              tr("Расчет допустимых токов от температуры"));
}

void UIBuilder::createTIActions() {
    addAction("recalcDor",
              tr("&Пересчет дорасчетной ТМ"),
              ":/images/RecalcDor.png",
              "",
              tr("Пересчет дорасчетной ТМ"));
    addAction("updateTables",
              tr("&Обновить таблицы по ТМ"),
              ":/images/UpdateTablesTI.png",
              "",
              tr("Обновить таблицы по ТМ"));
    addAction("calcPTI",
              tr("&ПТИ"),
              ":/images/calc_PTI.png",
              "",
              tr("Расчет ПТИ"));
    addAction("filtrTI",
              tr("&Фильтр ТИ"),
              ":/images/filtr_1.png",
              "",
              tr("Расчет Фильтр ТИ"));
}

void UIBuilder::createBarsMDPActions() {
    addAction("prepareMDP",
              tr("&Подг. МДП"),
              "",
              "",
              tr("Подг. МДП"));
}

void UIBuilder::createGraphActions() {
    addAction("graphWeb",
              tr("&Графика Web"),
              ":/images/new_style/mental map.png",
              "F10",
              tr("Графика Web"));
    addAction("graphSDL",
              tr("&Графика SDL"),
              ":/images/new_style/animation model.png",
              "",
              tr("Графика SDL"));
}

void UIBuilder::createMacroActions() {
    addAction("macro",
              tr("&Макросы"),
              ":/images/new_style/python.png",
              "F11",
              tr("Открыть макросы"));
}

void UIBuilder::createWindowActions() {
    // Закрыть активное окно
    addAction("close",
              tr("&Закрыть"),
              "",
              "Ctrl+F4",
              tr("Закрыть активное окно"));

    // Закрыть все окна
    addAction("closeAll",
              tr("&Close All"),
              "",
              "",
              tr("Закрыть все окна"));

    // Расположить окна плиткой
    addAction("tile",
              tr("&Плитка"),
              "",
              "",
              tr("Расположить окна плиткой"));

    // Расположить окна каскадом
    addAction("cascade",
              tr("&Каскад"),
              "",
              "",
              tr("Расположить окна каскадом"));

    addAction("protocol",
              tr("&Протокол"),
              "",
              "Ctrl+L",
              tr("Открыть окно протокола"));

    // Рабочая область
    addAction("saveWorkSpace",
              tr("&Сохранить область"),
              "",
              "",
              tr("Сохранить рабочую область"));

    addAction("loadWorkSpace",
              tr("&Загрузить область"),
              "",
              "",
              tr("Загрузить рабочую область"));
    // Следующее окно
    addAction("next",
              tr("&Следующее"),
              "",
              "Ctrl+F6",
              tr("Перейти к следующему окну"));

    // Предыдущее окно (Ctrl+Shift+F6)
    addAction("previous",
              tr("&Предыдущее"),
              "",
              "Ctrl+Shift+F6",
              tr("Перейти к предыдущему окну"));
}

void UIBuilder::createHelpActions() {
    addAction("about",
              tr("&О программе"),
              "",
              "",
              tr("Show the application's About box"));
}

void UIBuilder::createStyleActions() {
    // Создаём действие для каждого доступного стиля Qt
    for (const QString& style : QStyleFactory::keys()) {
        QString actionName = "style_" + style;
        QAction* action = new QAction(style, m_mainWindow);
        action->setCheckable(true);
        action->setData(style);
        m_actions[actionName] = action;
    }
}

void UIBuilder::buildMenuBar() {
    QMenuBar* menuBar = m_mainWindow->menuBar();

    // МЕНЮ "ФАЙЛЫ"
    m_menus["file"] = menuBar->addMenu(tr("&Файлы"));
    m_menus["file"]->addAction(m_actions["new"]);
    m_menus["file"]->addAction(m_actions["load"]);
    m_menus["file"]->addAction(m_actions["save"]);
    m_menus["file"]->addAction(m_actions["saveAs"]);
    m_menus["file"]->addAction(m_actions["saveAll"]);

    // Подменю "Настройки программы"
    m_menus["programmProperties"] =
        m_menus["file"]->addMenu(tr("&Настройки программы"));
    m_menus["programmProperties"]->addAction(m_actions["settings"]);

    // Подменю "Свойства" (для динамических форм из таблиц)
    // Заполняется через FormManager::buildPropertiesMenu()
    m_menus["properties"] = m_menus["programmProperties"]->addMenu(tr("&Настройки"));

    m_menus["file"]->addSeparator();

    // Подменю "Последние" (недавние файлы)
    m_menus["recentFiles"] = m_menus["file"]->addMenu(tr("&Последние"));
    QSettings s;
    int maxAllowed = s.value("maxRecentFiles", 10).toInt();
    for (int i = 0; i < maxAllowed; ++i) {
        QString actionName = QString("recentFile%1").arg(i);
        m_menus["recentFiles"]->addAction(m_actions[actionName]);
    }

    m_menus["file"]->addSeparator();
    m_menus["file"]->addAction(m_actions["exit"]);

    menuBar->addSeparator();

    // МЕНЮ "МАКРО"
    m_menus["macro"] = menuBar->addMenu(tr("&Макро"));
    // Используем стандартную иконку Qt для Play
    QAction* macroAction = m_actions["macro"];
    m_menus["macro"]->addAction(macroAction);

    // МЕНЮ "ГРАФИКА"
    m_menus["graph"] = menuBar->addMenu(tr("&Графика"));
    // Используем стандартную иконку Qt для Network
    QAction* graphWebAction = m_actions["graphWeb"];
    m_menus["graph"]->addAction(graphWebAction);
    QAction* graphSDLAction = m_actions["graphSDL"];
    m_menus["graph"]->addAction(graphSDLAction);

    // МЕНЮ "РАСЧЁТЫ"
    m_menus["calc"] = menuBar->addMenu(tr("&Расчеты"));
    m_menus["calc"]->addAction(m_actions["kdd"]);
    m_menus["calc"]->addAction(m_actions["rgm"]);
    m_menus["calc"]->addAction(m_actions["opf"]);
    m_menus["calc"]->addAction(m_actions["smzu"]);
    m_menus["calc"]->addAction(m_actions["prepareMDP"]);
    m_menus["calc"]->addAction(m_actions["kz"]);
    m_menus["calc"]->addAction(m_actions["idop"]);

    // Подменю "Параметры" (для форм параметров расчётов)
    // Заполняется через FormManager::buildFormsMenu()
    m_menus["calcParameters"] = m_menus["calc"]->addMenu(tr("&Параметры"));

    // Подменю "ТИ" (телеизмерения)
    m_menus["calcTI"] = m_menus["calc"]->addMenu(tr("&ТИ"));
    m_menus["calcTI"]->addAction(m_actions["calcPTI"]);
    m_menus["calcTI"]->addAction(m_actions["recalcDor"]);
    m_menus["calcTI"]->addAction(m_actions["updateTables"]);

    // МЕНЮ "ТАБЛИЦЫ" (для статических форм)
    // Заполняется через FormManager::buildFormsMenu()
    m_menus["tables"] = menuBar->addMenu(tr("&Таблицы"));

    menuBar->addSeparator();

    // МЕНЮ "ОКНА"
    m_menus["window"] = menuBar->addMenu(tr("Окна"));
    m_menus["window"]->addAction(m_actions["close"]);
    m_menus["window"]->addAction(m_actions["closeAll"]);
    m_menus["window"]->addSeparator();
    m_menus["window"]->addAction(m_actions["tile"]);
    m_menus["window"]->addAction(m_actions["cascade"]);
    m_menus["window"]->addSeparator();
    m_menus["window"]->addAction(m_actions["protocol"]);
    m_menus["window"]->addSeparator();
    m_menus["window"]->addAction(m_actions["saveWorkSpace"]);
    m_menus["window"]->addAction(m_actions["loadWorkSpace"]);
    m_menus["window"]->addSeparator();
    m_menus["window"]->addAction(m_actions["next"]);
    m_menus["window"]->addAction(m_actions["previous"]);

    // МЕНЮ "ПОМОЩЬ"
    m_menus["help"] = menuBar->addMenu(tr("&Помощь"));
    m_menus["help"]->addAction(m_actions["about"]);

    // МЕНЮ "СТИЛЬ"
    m_menus["style"] = menuBar->addMenu(tr("Стиль"));
    QActionGroup* styleGroup = new QActionGroup(m_mainWindow);
    styleGroup->setExclusive(true);

    QString currentStyle = QApplication::style()->objectName();

    for (const QString& style : QStyleFactory::keys()) {
        QString actionName = "style_" + style;
        QAction* action;
        auto it = m_actions.find(actionName);
        if (it != m_actions.end()) {
            action = it->second;
        }
        if (action) {
            styleGroup->addAction(action);
            m_menus["style"]->addAction(action);
            // Отмечаем текущий стиль
            if (style.toLower() == currentStyle.toLower()) {
                action->setChecked(true);
            }
        }
    }

    // Подключаем смену стиля прямо здесь
    connect(styleGroup, &QActionGroup::triggered,
            [this](QAction* action) {
                const QString styleName = action->data().toString();
                QApplication::setStyle(QStyleFactory::create(styleName));
                // Сохраняем немедленно:
                QSettings settings;
                settings.setValue("appStyle", styleName);
            });
}

void UIBuilder::buildToolBars() {
    // ПАНЕЛЬ "ФАЙЛ"
    m_toolBars["file"] = m_mainWindow->addToolBar(tr("Файл"));
    m_toolBars["file"]->setObjectName("fileToolBar");
    m_toolBars["file"]->addAction(m_actions["new"]);
    m_toolBars["file"]->addAction(m_actions["load"]);
    m_toolBars["file"]->addAction(m_actions["save"]);

    // ПАНЕЛЬ "РАСЧЁТЫ"
    m_toolBars["calc"] = m_mainWindow->addToolBar(tr("Расчеты"));
    m_toolBars["calc"]->setObjectName("calcToolBar");
    m_toolBars["calc"]->addAction(m_actions["rgm"]);
    m_toolBars["calc"]->addAction(m_actions["smzu"]);
    m_toolBars["calc"]->addAction(m_actions["prepareMDP"]);
    m_toolBars["calc"]->addAction(m_actions["kz"]);
    m_toolBars["calc"]->addAction(m_actions["macro"]);

    // ПАНЕЛЬ "ТЕЛЕИЗМЕРЕНИЯ"
    m_toolBars["ti"] = m_mainWindow->addToolBar(tr("Телеизмерения"));
    m_toolBars["ti"]->setObjectName("tiToolBar");
    m_toolBars["ti"]->addAction(m_actions["calcPTI"]);
    m_toolBars["ti"]->addAction(m_actions["filtrTI"]);
    m_toolBars["ti"]->addAction(m_actions["opf"]);
    m_toolBars["ti"]->addAction(m_actions["recalcDor"]);
    m_toolBars["ti"]->addAction(m_actions["updateTables"]);
}

void UIBuilder::buildStatusBar() {
    m_mainWindow->statusBar()->showMessage(tr("Ready"));
}

QAction* UIBuilder::addAction(
    const QString& name,
    const QString& text,
    const QString& iconPath,
    const QString& shortcut,
    const QString& statusTip) {
    QAction* action;

    // Создание действия с иконкой или без
    if (iconPath.isEmpty()) {
        action = new QAction(text, m_mainWindow);
    } else {
        action = new QAction(QIcon(iconPath), text, m_mainWindow);
    }

    // Установка горячей клавиши
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence(shortcut));
        action->setShortcutContext(Qt::ApplicationShortcut);
    }

    // Установка подсказки в статусной строке
    if (!statusTip.isEmpty()) {
        action->setStatusTip(statusTip);
    }

    // Сохранение в карте
    m_actions[name] = action;

    return action;
}

QAction* UIBuilder::actionByName(const QString& name) const {
    auto it = m_actions.find(name);
    return (it != m_actions.end()) ? it->second : nullptr;
}

QMenu* UIBuilder::menuByName(const QString& name) const {
    auto it = m_menus.find(name);
    return (it != m_menus.end()) ? it->second : nullptr;
}

QToolBar* UIBuilder::toolBarByName(const QString& name) const {
    auto it = m_toolBars.find(name);
    return (it != m_toolBars.end()) ? it->second : nullptr;
}

void UIBuilder::updateRecentFileActions(const QStringList& recentFiles) {
    QSettings s;
    int maxAllowed = s.value("maxRecentFiles", 10).toInt();
    int numVisible = qMin(recentFiles.size(), maxAllowed);

    for (int i = 0; i < numVisible; ++i) {
        QString actionName = QString("recentFile%1").arg(i);
        QAction* action = m_actions[actionName];

        if (action) {
            // Разбор строки "file <template>"
            QStringList qslist = recentFiles[i].split(" ");
            QString file = qslist[0];
            QString shabl;

            if (qslist.size() > 1) {
                shabl = qslist[1];
                if (!shabl.isEmpty()) {
                    shabl.remove(0, 1);  // Удаляем '<'
                    shabl.chop(1);       // Удаляем '>'

                    // Извлекаем имя файла из полного пути шаблона
                    QFileInfo fileInfo(shabl);
                    shabl = fileInfo.fileName();
                }
            }

            // Формируем текст действия
            QString text;
            if (shabl.isEmpty()) {
                text = tr("&%1 %2").arg(i + 1).arg(file);
            } else {
                text = tr("&%1 %2 %3").arg(i + 1).arg(file).arg("<" + shabl + ">");
            }

            action->setText(text);
            action->setData(recentFiles[i]);
            action->setVisible(true);
        }
    }

    // Скрываем все action-ы начиная с numVisible, сколько бы их ни было
    int existing = 0;
    while (m_actions.count(QString("recentFile%1").arg(existing)))
        ++existing;

    for (int j = numVisible; j < existing; ++j) {
        if (auto* a = m_actions[QString("recentFile%1").arg(j)])
            a->setVisible(false);
    }
}

std::pair<int,int> UIBuilder::ensureRecentFileCapacity(int count) {
    // Считаем, сколько уже есть
    int existing = 0;
    while (m_actions.count(QString("recentFile%1").arg(existing)))
        ++existing;

    if (count <= existing)
        return {existing, existing};   // ничего не нужно

    for (int i = existing; i < count; ++i) {
        QString name = QString("recentFile%1").arg(i);
        auto* act = new QAction(m_mainWindow);
        act->setVisible(false);
        m_actions[name] = act;
        m_menus["recentFiles"]->addAction(act);   // добавляем в меню
    }
    return {existing, count};
}