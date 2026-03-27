#pragma once

#include <QObject>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QSet>
#include <list>
#include <memory>
#include <rtablesdatamanager.h>
#include "logManager.h"

class QAstra;
class RtabWidget;
class CUIForm;
class PyHlp;
class GraphServer;
class IGraphManager;

namespace ads {
    class CDockManager;
    class CDockWidget;
}

/**
 * @class Менеджер форм и таблиц
 * - Поддержка статических форм (из конфигурационных файлов)
 * - Поддержка динамических форм (генерируемых из таблиц с .form шаблонами)
 * - Передача сигналов sig_calcBegin/End всем открытым формам
 * - Хранение списка открытых форм для передачи сигналов
 */
class FormManager : public QObject {
    Q_OBJECT
    
public:
    explicit FormManager(
        std::shared_ptr<QAstra> qastra,
        ads::CDockManager* dockManager,
        std::shared_ptr<PyHlp> pPyHlp,
        LogManager* logManager,
        QWidget* parent = nullptr
    );
    ~FormManager() = default;
    
    // ========== Управление формами ==========
    /// @brief Установить список статических форм
    void setForms(const std::list<CUIForm>& forms);
    
    const std::list<CUIForm>& forms() const { return m_forms; }

    ///@brief Открыть форму
    void openForm(const CUIForm& form);
    
    void openFormByName(const QString& formName);
    void openFormByIndex(int index);

    // ========== Построение меню ==========
    /// @brief Построить меню "Открыть" из статических форм
    void buildFormsMenu(QMenu* parentMenu, QMenu* calcParametersMenu = nullptr);
    /**
     * @brief Построить меню "Свойства" из динамических форм
     * @note Генерирует формы из таблиц с .form шаблонами!
     */
    void buildPropertiesMenu(QMenu* propertiesMenu);
    
    RtabWidget* activeForm() const { return m_activeForm; }
    void closeGraphWebServer();
    // ========== Логирование ==========
    LogManager* logManager() const { return m_logManager; }
    /// Инициализировать виджеты лога (до ADS restoreState)
    void createLogWidgets();
    /// Добавить dock-виджеты лога (после ADS restoreState)
    void setupLogDockWidgets();

    /// @brief Имена всех незакрытых dock-виджетов для сохранения в рабочей области.
    QStringList openWidgetNames() const;

    /**
        * @brief Открыть виджет по objectName.
        *
        * Диспетчер: протоколы → графика → таблицы.
        * Все ветки обёрнуты в try-catch, чтобы ошибка создания одного виджета
        * не прерывала восстановление остальных.
    */
    void openWidgetByName(const QString& name);

    /**
     * @brief Закрыть все виджеты, кроме протоколов.
     *
     * Протоколы ("Полный протокол", "Протокол Astra") никогда не закрываются —
     * они продолжают собирать события логов даже когда невидимы.
     * WorkspaceManager управляет их видимостью через toggleView().
    */
    void closeAllWidgets();

    const QSet<QString>& protocolDockNames();
signals:
    void formOpened(const QString& formName);
    void formClosed(const QString& formName);
    void activeFormChanged(RtabWidget* form);
    
public slots:
    ///@brief Автоматическое размещение форм
    void slot_cascadeForms();
    void slot_tileForms();

    /// @brief Обработка клика по меню форм
    void slot_formMenuTriggered(QAction* action);
    /**
     * @brief Начало расчёта - передаём сигнал всем открытым формам
     * @note Вызывает on_calc_begin() у всех RtabWidget
     */
    void slot_calculationStarted();
    /**
     * @brief Конец расчёта - передаём сигнал всем открытым формам
     * @note Вызывает on_calc_end() у всех RtabWidget
     */
    void slot_calculationFinished();
    void slot_openProtocol();
    // ========== Графика ==========
    void slot_openSDLGraph();
    void slot_openWebGraph();
private:
    // ========== Зависимости ==========
    std::shared_ptr<QAstra> m_qastra;
    ads::CDockManager* m_dockManager;
    QWidget* m_parentWidget;
    std::shared_ptr<PyHlp> m_pPyHlp;
    RTablesDataManager m_rtdm;                  // Менеджер данных таблиц
    // ========== Состояние ==========
    std::list<CUIForm> m_forms;                     // Статические формы
    std::map<QString, int> m_formNameToIndex;           // Быстрый поиск

    //sdl
    IGraphManager* m_graphSDLManager = nullptr;
    IGraphManager* m_graphWebManager = nullptr;

    LogManager* m_logManager = nullptr;

    const QSet<QString> m_protocolNames = {
        QStringLiteral("Полный протокол"),
        QStringLiteral("Протокол Astra")
    };
    /** @brief
     * Список ВСЕХ открытых форм
     * Используется для передачи сигналов расчётов
     */
    QList<RtabWidget*> m_openForms;
    QList<ads::CDockWidget*>  m_openDockWidgets;
    RtabWidget* m_activeForm = nullptr;
    /// @brief Общий метод регистрации dock-виджета в m_openDockWidgets
    void registerDockWidget(ads::CDockWidget* dw);
    /**
     * @brief Генерировать динамические формы из таблиц
     * @note Вызывается при каждом открытии меню свойств
     */
    void generateDynamicForms(QMenu* menu);
};
