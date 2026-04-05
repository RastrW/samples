#pragma once

#include <QObject>
#include <QSet>

class QAstra;
class RtabWidget;
class CUIForm;
class PyHlp;
class GraphServer;
class IGraphManager;
class MacroDockManager;
class TableDockManager;
class LogManager;
class QMenu;

namespace ads {
    class CDockManager;
    class CDockWidget;
}

/**
 * @class Координатор всех dock-виджетов рабочей области.
 *
 * Делегирует предметную логику специализированным менеджерам:
 *  - TableDockManager  — таблицы (CUIForm)
 *  - MacroDockManager  — редактор макросов
 *  - GraphSDLManager / GraphWebManager — графика
 *  - LogManager        — протоколы
 */
class FormManager : public QObject {
    Q_OBJECT

public:
    explicit FormManager(
        std::shared_ptr<QAstra> qastra,
        ads::CDockManager*      dockManager,
        LogManager*             logManager,
        QWidget*                parent = nullptr);

    ~FormManager() = default;

    // ── Делегаты ─────────────────────────────────────────────────────────────

    TableDockManager* tableDockManager() const { return m_tableDockManager; }
    MacroDockManager* macroDockManager() const { return m_macroDockManager; }
    LogManager*       logManager()       const { return m_logManager; }

    // ── Таблицы (проброс в TableDockManager) ─────────────────────────────────

    void setForms(const std::list<CUIForm>& forms);

    void buildFormsMenu    (QMenu* parentMenu,
                        QMenu* calcParametersMenu = nullptr);
    void buildPropertiesMenu(QMenu* propertiesMenu);

    RtabWidget* activeForm() const;

    // ── Графика ──────────────────────────────────────────────────────────────
    void closeGraphWebServer();

    // ── Логирование ──────────────────────────────────────────────────────────
    /// Инициализировать виджеты лога (до ADS restoreState).
    void createLogWidgets();
    /// Добавить dock-виджеты лога (после ADS restoreState).
    void setupLogDockWidgets();

    // ── Рабочая область ──────────────────────────────────────────────────────
    /// Имена всех незакрытых dock-виджетов для сохранения.
    QStringList openWidgetNames() const;
    /**
    * @brief Открыть виджет по objectName.
    * Диспетчер: протоколы → макрос → графика → таблицы.
    */
    void openWidgetByName(const QString& name);
    /**
     * @brief Закрыть все виджеты, кроме протоколов.
     */
    void closeAllWidgets();
    const QSet<QString>& protocolDockNames() const;
    // ── Макросы ──────────────────────────────────────────────────────────────

    void openMacroWindow();
    /**
     * @brief Подготовить к закрытию приложения:
     *  закрыть все виджеты и floating-контейнеры ADS
     *  ДО того как будут уничтожены дочерние менеджеры.
     *  Иначе будут возникает проблемы с плавающими окнами, отделёныыми отд
     *  mainwindow.
     */
    void prepareForClose();
signals:
    void formOpened       (const QString& formName);
    void formClosed       (const QString& formName);
    void activeFormChanged(RtabWidget* form);

public slots:
    void slot_cascadeForms();
    void slot_tileForms();

    void slot_calculationStarted();
    void slot_calculationFinished();

    void slot_openProtocol();
    void slot_openSDLGraph();
    void slot_openWebGraph();

private:
    // ── Зависимости ──────────────────────────────────────────────────────────
    std::shared_ptr<QAstra> m_qastra;
    ads::CDockManager*      m_dockManager;
    QWidget*                m_parentWidget;

    // ── Специализированные менеджеры ─────────────────────────────────────────
    LogManager*        m_logManager        = nullptr;
    TableDockManager*  m_tableDockManager  = nullptr;
    MacroDockManager*  m_macroDockManager  = nullptr;
    IGraphManager*     m_graphSDLManager   = nullptr;
    IGraphManager*     m_graphWebManager   = nullptr;

    // ── Реестр всех открытых dock-виджетов ───────────────────────────────────
    QList<ads::CDockWidget*> m_openDockWidgets;

    const QSet<QString> m_protocolNames = {
        QStringLiteral("Полный протокол"),
        QStringLiteral("Протокол Astra")
    };
    std::shared_ptr<PyHlp> m_pPyHlp;
    /// Зарегистрировать dock-виджет и подписаться на его удаление.
    void registerDockWidget(ads::CDockWidget* dw);

};
