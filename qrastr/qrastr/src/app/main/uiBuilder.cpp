#include "uiBuilder.h"

#include <QMenu>
#include <QMenuBar>
#include <QFileInfo>
#include <QApplication>
#include <QStyle>
#include <QToolBar>
#include <QStatusBar>

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

    // Построение меню и панелей
    buildMenuBar();
    buildToolBars();
    buildStatusBar();
}

void UIBuilder::createFileActions() {
    addAction("new",
              tr("&Новый"),
              ":/images/document_new.png",
              "Ctrl+N",
              tr("Create a new file"));
    addAction("open",
              tr("&Загрузить"),
              ":/images/folder_out.png",
              "Ctrl+O",
              tr("Open an existing file"));
    addAction("save",
              tr("&Сохранить"),
              ":/images/disk_blue.png",
              "Ctrl+S",
              tr("Save the document to disk"));
    addAction("saveAs",
              tr("&Сохранить как"),
              "",
              "",
              tr("Save the document under a new name"));
    addAction("saveAll",
              tr("&Сохранить все"),
              ":/images/Save_all.png",
              "",
              tr("Save all the document"));
    addAction("settings",
              tr("&Параметры"),
              "",
              "",
              tr("Open settings form."));
    addAction("exit",
              tr("E&xit"),
              "",
              "Ctrl+Q",
              tr("Exit the application"));

    // НЕДАВНИЕ ФАЙЛЫ (10 действий)
    // Создаём массив действий для недавних файлов
    for (int i = 0; i < kMaxRecentFiles; ++i) {
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
    addAction("graph",
              tr("&graph"),
              "",
              "F10",
              tr("Графика"));
}

void UIBuilder::createMacroActions() {
    addAction("macro",
              tr("&macro"),
              "",
              "F11",
              tr("Run macro"));
}

void UIBuilder::createWindowActions() {
    // Закрыть активное окно
    addAction("close",
              tr("&Close"),
              "",
              "Ctrl+F4",
              tr("Close the active window"));

    // Закрыть все окна
    addAction("closeAll",
              tr("Close &All"),
              "",
              "",
              tr("Close all the windows"));

    // Расположить окна плиткой
    addAction("tile",
              tr("&Tile"),
              "",
              "",
              tr("Tile the windows"));

    // Расположить окна каскадом
    addAction("cascade",
              tr("&Cascade"),
              "",
              "",
              tr("Cascade the windows"));

    // Следующее окно
    addAction("next",
              tr("Ne&xt"),
              "",
              "Ctrl+F6",
              tr("Move the focus to the next window"));

    // Предыдущее окно (Ctrl+Shift+F6)
    addAction("previous",
              tr("Pre&vious"),
              "",
              "Ctrl+Shift+F6",
              tr("Move the focus to the previous window"));
}

void UIBuilder::createHelpActions() {
    addAction("about",
              tr("&О программе"),
              "",
              "",
              tr("Show the application's About box"));
}

void UIBuilder::buildMenuBar() {
    QMenuBar* menuBar = m_mainWindow->menuBar();

    // МЕНЮ "ФАЙЛЫ"
    m_menus["file"] = menuBar->addMenu(tr("&Файлы"));
    m_menus["file"]->addAction(m_actions["new"]);
    m_menus["file"]->addAction(m_actions["open"]);
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
    for (int i = 0; i < kMaxRecentFiles; ++i) {
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
    macroAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    m_menus["macro"]->addAction(macroAction);

    // МЕНЮ "ГРАФИКА"
    m_menus["graph"] = menuBar->addMenu(tr("&Графика"));
    // Используем стандартную иконку Qt для Network
    QAction* graphAction = m_actions["graph"];
    graphAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon));
    m_menus["graph"]->addAction(graphAction);

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

    // МЕНЮ "ОТКРЫТЬ" (для статических форм)
    // Заполняется через FormManager::buildFormsMenu()
    m_menus["open"] = menuBar->addMenu(tr("&Открыть"));

    menuBar->addSeparator();

    // МЕНЮ "ОКНА"
    m_menus["window"] = menuBar->addMenu(tr("Окна"));
    m_menus["window"]->addAction(m_actions["close"]);
    m_menus["window"]->addAction(m_actions["closeAll"]);
    m_menus["window"]->addSeparator();
    m_menus["window"]->addAction(m_actions["tile"]);
    m_menus["window"]->addAction(m_actions["cascade"]);
    m_menus["window"]->addSeparator();
    m_menus["window"]->addAction(m_actions["next"]);
    m_menus["window"]->addAction(m_actions["previous"]);

    // МЕНЮ "ПОМОЩЬ"
    m_menus["help"] = menuBar->addMenu(tr("&Помощь"));
    m_menus["help"]->addAction(m_actions["about"]);
}

void UIBuilder::buildToolBars() {
    // ПАНЕЛЬ "ФАЙЛ"
    m_toolBars["file"] = m_mainWindow->addToolBar(tr("Файл"));
    m_toolBars["file"]->addAction(m_actions["new"]);
    m_toolBars["file"]->addAction(m_actions["open"]);
    m_toolBars["file"]->addAction(m_actions["save"]);

    // ПАНЕЛЬ "РАСЧЁТЫ"
    m_toolBars["calc"] = m_mainWindow->addToolBar(tr("Расчеты"));
    m_toolBars["calc"]->addAction(m_actions["rgm"]);
    m_toolBars["calc"]->addAction(m_actions["smzu"]);
    m_toolBars["calc"]->addAction(m_actions["prepareMDP"]);
    m_toolBars["calc"]->addAction(m_actions["kz"]);
    m_toolBars["calc"]->addAction(m_actions["macro"]);

    // ПАНЕЛЬ "ТЕЛЕИЗМЕРЕНИЯ"
    m_toolBars["ti"] = m_mainWindow->addToolBar(tr("Телеизмерения"));
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
    return m_actions.value(name, nullptr);
}

QMenu* UIBuilder::menuByName(const QString& name) const {
    return m_menus.value(name, nullptr);
}

QToolBar* UIBuilder::toolBarByName(const QString& name) const {
    return m_toolBars.value(name, nullptr);
}

void UIBuilder::updateRecentFileActions(const QStringList& recentFiles) {
    int numRecentFiles = qMin(recentFiles.size(), kMaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
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

    // Скрываем неиспользуемые действия
    for (int j = numRecentFiles; j < kMaxRecentFiles; ++j) {
        QString actionName = QString("recentFile%1").arg(j);
        QAction* action = m_actions[actionName];
        if (action) {
            action->setVisible(false);
        }
    }
}
