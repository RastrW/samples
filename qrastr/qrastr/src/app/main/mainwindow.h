#pragma once

#include <QMainWindow>
#include <memory>
#include <list>
#include <rtablesdatamanager.h>

class FileManager;
class CalculationController;
class FormManager;
class SettingsManager;
class UIBuilder;
class QAstra;
class QTI;
class QBarsMDP;
class QMdiArea;
class McrWnd;
class FormProtocol;

class PyHlp;
class CUIForm;
class QAction;

namespace ads {
    class CDockManager;
    class CDockWidget;
}

/// @class Главное окно приложения
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow();
    ~MainWindow() override;
    
    /**
     * @brief Инициализировать главное окно
     * @param qastra плагин Rastr
     * @param qti плагин TI
     * @param qbarsmdp плагин BarsMDP
     * @param forms список форм
     */
    void initialize(
        std::shared_ptr<QAstra> qastra,
        std::shared_ptr<QTI> qti,
        std::shared_ptr<QBarsMDP> qbarsmdp,
        const std::list<CUIForm>& forms
    );
    
signals:
    // Файл загружен
    void sig_fileLoaded();
    // Строка добавлена в таблицу
    void sig_rowInserted(std::string _t_name, int _row);
    // Строка удалена
    void sig_rowDeleted(std::string _t_name, int _row);
    // Таблица обновлена
    void sig_update(std::string _t_name);
    // Начало расчёта
    void sig_calcBegin();
    // Конец расчёта
    void sig_calcEnd();
protected:
    // Обработка закрытия окна
    void closeEvent(QCloseEvent* event) override;
    // Обработка показа окна
    void showEvent(QShowEvent* event) override;
    // Drag & Drop
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
private slots:
    // Активировать подокно
    void slot_subWindowActivated();
    // Обновить список недавних файлов
    void slot_updateRecentFiles();
    // Обновить меню
    void slot_updateMenu();
    
    // Диалоговые расчёты
    // Расчёт допустимых токов от температуры
    void showIdopDialog();
    // Подготовка данных для МДП
    void showMDPPrepareDialog();
    // Диалог макросов
    void slot_openMcrDialog();
    void slot_openGraph();

    void slot_about();
private:
    // ========== КОМПОНЕНТЫ-ДЕЛЕГАТЫ ==========
    std::unique_ptr<FileManager>
        m_fileManager;
    std::unique_ptr<CalculationController>
        m_calcController;
    std::unique_ptr<FormManager>
        m_formManager;
    std::unique_ptr<SettingsManager>
        m_settingsManager;
    std::unique_ptr<UIBuilder>
        m_uiBuilder;
    
    // ========== ПЛАГИНЫ ==========
    std::shared_ptr<QAstra> m_qastra;
    std::shared_ptr<QTI> m_qti;
    std::shared_ptr<QBarsMDP> m_qbarsmdp;
    
    // ========== GUI ЭЛЕМЕНТЫ ==========
    QMdiArea* m_workspace = nullptr;            // Область для множественных документов
    ads::CDockManager* m_dockManager = nullptr; // The main container for Advanced Docking System
    McrWnd* m_globalProtocol = nullptr;                // Глобавльный протокол
    FormProtocol* m_mainProtocol = nullptr;     // Главный протокол
    
    // ========== ВСПОМОГАТЕЛЬНЫЕ КОМПОНЕНТЫ ==========
    std::unique_ptr<PyHlp> m_pyHelper;          // Python helper (для выполнения макросов)
    RTablesDataManager m_rtdm;                  // Менеджер данных таблиц
    
    // ========== ИНИЦИАЛИЗАЦИЯ ==========
    void setupDockWidgets();
    void setupLogging();
    void setupConnections();
};
