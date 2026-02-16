#pragma once

#include <QObject>
#include <QString>
#include <QMenu>
#include <QAction>
#include <QMap>
#include <QList>
#include <list>
#include <memory>

class QAstra;
class RTablesDataManager;
class RtabWidget;
class CUIForm;

namespace ads {
    class CDockManager;
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
        RTablesDataManager* rtdm,
        ads::CDockManager* dockManager,
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
    
    /**
     * @brief Генерировать динамические формы из таблиц
     * @note Вызывается при каждом открытии меню свойств
     */
    void generateDynamicForms(QMenu* menu);
    RtabWidget* activeForm() const { return m_activeForm; }
signals:
    void formOpened(const QString& formName);
    void formClosed(const QString& formName);
    void activeFormChanged(RtabWidget* form);
    
public slots:
    /// @brief Обработка клика по меню форм
    void onFormMenuTriggered(QAction* action);
    void onFormClosed();  
    /**
     * @brief Начало расчёта - передаём сигнал всем открытым формам
     * @note Вызывает on_calc_begin() у всех RtabWidget
     */
    void onCalculationStarted();
    /**
     * @brief Конец расчёта - передаём сигнал всем открытым формам
     * @note Вызывает on_calc_end() у всех RtabWidget
     */
    void onCalculationFinished();
private:
    // ========== Зависимости ==========
    std::shared_ptr<QAstra> m_qastra;
    RTablesDataManager* m_rtdm;
    ads::CDockManager* m_dockManager;
    QWidget* m_parentWidget;
    
    // ========== Состояние ==========
    std::list<CUIForm> m_forms;                     // Статические формы
    QMap<QString, int> m_formNameToIndex;           // Быстрый поиск
    /** @brief
     * Список ВСЕХ открытых форм
     * Используется для передачи сигналов расчётов
     */
    QList<RtabWidget*> m_openForms;
    RtabWidget* m_activeForm = nullptr;
    // ========== Вспомогательные методы ==========
    CUIForm* findFormByName(const QString& name);
    CUIForm* findFormByIndex(int index);

    /// @brief Подключить сигналы расчётов к виджету формы
    void setupFormConnections(RtabWidget* widget);

    /// @brief Построить иерархию меню из MenuPath форм
    QMap<QString, QMenu*> buildMenuStructure(QMenu* rootMenu);
};
