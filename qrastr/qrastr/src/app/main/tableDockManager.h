#pragma once
#include <QObject>
#include "RTablesDataManager.h"

namespace ads { class CDockManager; class CDockWidget; }

class RtabWidget;
class QAstra;
class PyHlp;
class CUIForm;
class QAction;
class QMenu;

/**
 * @class TableDockManager
 * @brief Управляет dock-виджетами таблиц (CUIForm).
 *
 * Отвечает за:
 *  - хранение списка статических форм и быстрый поиск по имени
 *  - создание dock-виджетов таблиц (openForm / openFormByName / openFormByIndex)
 *  - построение меню «Открыть» и «Свойства»
 *  - трансляцию сигналов расчёта всем открытым RtabWidget
 *
 * Аналог GraphSDLManager / GraphWebManager / MacroDockManager —
 * испускает windowOpened() для регистрации в FormManager.
*/
class TableDockManager : public QObject {
    Q_OBJECT

public:
    explicit TableDockManager(
        std::shared_ptr<QAstra> qastra,
        ads::CDockManager*      dockManager,
        std::shared_ptr<PyHlp>  pyHlp,
        QWidget*                parent = nullptr);

    // ── Формы ────────────────────────────────────────────────────────────────

    /// Установить список статических форм.
    void setForms(const std::list<CUIForm>& forms);

    const std::list<CUIForm>& forms() const { return m_forms; }

    void openForm        (const CUIForm& form);
    void openFormByName  (const QString& formName);
    void openFormByIndex (int index);

    /// Все открытые RtabWidget (для трансляции calc-сигналов).
    const QList<RtabWidget*>& openForms() const { return m_openForms; }

    RtabWidget* activeForm() const { return m_activeForm; }

    // ── Меню ─────────────────────────────────────────────────────────────────

    /**
     * @brief Построить меню «Открыть» из статических форм.
     * @param parentMenu       — основное меню таблиц
     * @param calcParametersMenu — необязательный подраздел параметров расчёта
     */
    void buildFormsMenu    (QMenu* parentMenu,
                        QMenu* calcParametersMenu = nullptr);

    /**
     * @brief Подключить динамическую генерацию меню «Свойства».
     * @note Меню перестраивается при каждом открытии (aboutToShow).
     */
    void buildPropertiesMenu(QMenu* propertiesMenu);

signals:
    /// Новый dock-виджет создан → FormManager::registerDockWidget.
    void windowOpened (ads::CDockWidget* dw);

    void formOpened   (const QString& formName);
    void formClosed   (const QString& formName);
    void activeFormChanged(RtabWidget* form);

public slots:
    /// Транслировать начало расчёта всем открытым RtabWidget.
    void slot_calculationStarted();
    /// Транслировать конец расчёта всем открытым RtabWidget.
    void slot_calculationFinished();

private slots:
    void slot_formMenuTriggered(QAction* action);

private:
    // ── Зависимости ──────────────────────────────────────────────────────────
    std::shared_ptr<QAstra> m_qastra;
    ads::CDockManager*      m_dockManager;
    std::shared_ptr<PyHlp>  m_pyHlp;
    QWidget*                m_parentWidget;

    RTablesDataManager      m_rtdm;

    // ── Состояние ────────────────────────────────────────────────────────────
    std::list<CUIForm>         m_forms;
    std::map<QString, int>     m_formNameToIndex;  ///< Быстрый поиск по имени
    QList<RtabWidget*>         m_openForms;
    RtabWidget*                m_activeForm {nullptr};

    // ── Вспомогательные методы ───────────────────────────────────────────────

    /// Генерировать пункты меню «Свойства» из таблиц с .form-шаблонами.
    void generateDynamicForms(QMenu* menu);
};