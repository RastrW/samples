#pragma once

#include <QMainWindow>
#include <spdlog/sinks/qt_sinks.h>

class FileManager;
class CalculationController;
class FormManager;
class AppSettingsManager;
class UIBuilder;
class QAstra;
class QTI;
class QBarsMDP;
class QMdiArea;
class ProtocolLogWidget;
class ProtocolWidget;
class LogManager;
class WorkspaceManager;

class CUIForm;
class QAction;
class McrWnd;

namespace ads {
    class CDockManager;
    class CDockWidget;
}
struct EngineContext;

/// @class Главное окно приложения
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow();
    ~MainWindow() override;
    
    /**
     * @brief Инициализировать главное окно
     * @param forms список форм
     */
    void initialize(
        const EngineContext& engCtxt,
        const std::list<CUIForm>& forms);

    std::shared_ptr<spdlog::sinks::sink> getProtocolLogSink() const;
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
    // Обновить меню
    void slot_updateMenu();
    
    // Диалоговые расчёты
    // Расчёт допустимых токов от температуры
    void showIdopDialog();
    // Подготовка данных для МДП
    void showMDPPrepareDialog();

    void slot_about();
private:
    // ========== КОМПОНЕНТЫ-ДЕЛЕГАТЫ ==========
    std::unique_ptr<FileManager>
        m_fileManager;
    std::unique_ptr<CalculationController>
        m_calcController;
    std::unique_ptr<FormManager>
        m_formManager;
    std::unique_ptr<AppSettingsManager>
        m_appSettingsManager;
    std::unique_ptr<UIBuilder>
        m_uiBuilder;
    LogManager* m_logManager = nullptr;
    std::unique_ptr<WorkspaceManager>
        m_workspaceManager;
    
    // ========== GUI ЭЛЕМЕНТЫ ==========
    ads::CDockManager* m_dockManager = nullptr;   // The main container for Advanced Docking System

    // ========== ВСПОМОГАТЕЛЬНЫЕ КОМПОНЕНТЫ ==========
    std::shared_ptr<spdlog::sinks::sink> m_qtLogSink; // сохраняем в setupLogSinks
    // ========== ИНИЦИАЛИЗАЦИЯ ==========
    void setupConnections();
};
