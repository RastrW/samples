#pragma once

// Заголовочные файлы стандартной библиотеки C++
// Заголовочные файлы других библиотек.
#include <QMainWindow>
#include <QMap>

// Заголовочные файлы проекта.
#include <rtablesdatamanager.h>
#include "spdlog/common.h"
#include "cacheLog.h"

class QMdiArea;
class QSignalMapper;
class QHBoxLayout;

class QAstra;
class QTI;
class QBarsMDP;
class CUIForm;
class McrWnd;
struct _hint_data;
class rmodel;
namespace ads{ class CDockManager; }
class RtabWidget;
class FormProtocol;

enum MainTheme
{
    THEME_0 = -1,
    THEME_1 = Qt::darkRed,
    THEME_2 = Qt::darkMagenta,
    THEME_3 = Qt::darkGray,
    THEME_4 = Qt::darkGreen,
    THEME_5 = Qt::darkCyan,
};

enum StyleSetting
{
    DefaultStyleSetting = 0,
    Windows7ScenicStyleSetting,
    Office2016ColorfulStyleSetting,
    Office2016BDarkGrayStyleSetting,
    Office2016BlackStyleSetting,
    AdobePhotoshopLightGrayStyleSetting,
    AdobePhotoshopDarkGrayStyleSetting,
    VisualSudio2019BlueStyleSetting,
    VisualSudio2019DarkStyleSetting,
    FluentLightStyleSetting,
    FluentDarkStyleSetting
};

class PyHlp;

/**
 * @class MainWindow
 * @brief Главное окно приложения с MDI-интерфейсом
 *
 * Отвечает за:
 * - Создание меню, панелей инструментов, статусной строки
 * - Управление множественными документами (MDI - Multiple Document Interface)
 * - Связь GUI с расчётными модулями
 * - Докирование окон (протоколы, таблицы)
 */
class MainWindow : public QMainWindow{
    Q_OBJECT
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
private slots:
    ///< Графика
    void slot_openGraph();
    ///< Файлы
    // Создать новый файл
    void slot_newFile();
    // Открыть файл
    void slot_open();
    // Сохранить
    void slot_save();
    // Сохранить как
    void slot_saveAs();
    // Сохранить все
    void slot_saveAll();
    // Открыть недавний файл
    void slot_openRecentFile();
    ///< Настройки
    // Показать настройки форм
    void slot_showFormSettings();
    void slot_about();
    ///< Расчёты
    // Контроль исходных данных
    void slot_kddWrap();
    // Расчёт режима (установившийся режим)
    void slot_rgmWrap();
    // Оценка состояния
    void slot_ocWrap();
    // Расчёт МДП
    void slot_smzuTstWrap();
    // Расчёт токов короткого замыкания
    void slot_tkzWrap();
    // Расчёт допустимых токов от температуры
    void slot_idopWrap();
    // Подготовка данных для МДП
    void slot_barsMdpPrepareWrap();

    ///< Телеизмерения
    // Пересчёт дорасчётных измерений
    void slot_tiRecalcdorWrap();
    // Обновление таблиц по телеметрии
    void slot_tiUpdateTablesWrap();
    // Расчёт псевдоизмерений
    void slot_tiCalcptiWrap();
    // Фильтрация телеизмерений
    void slot_tiFiltrtiWrap();

    // Формы и макросы
    // Диалог макросов
    void slot_openMcrDialog();
    // Открыть форму из меню
    void slot_openForm(QAction* p_actn);

    // Обработка изменений данных
    void slot_dataChanged(std::string _t_name, QModelIndex index, QVariant value);
    void slot_dataChanged(std::string _t_name, std::string _col_name, int _row, QVariant _value);
    void slot_rowInserted(std::string _t_name, int _row);
    void slot_rowDeleted (std::string _t_name, int _row);

    void slot_itemPressed(const QModelIndex &index);
    // Настройки форм
    void setSettingsForms();
public slots:
    // Обновить меню
    void slot_updateMenu();
    // Активировать подокно
    void slot_setActiveSubWindow(QWidget *window);

    // Drag & Drop
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
public:
    MainWindow();
    virtual ~MainWindow();
    ///< Установка компонентов
    // Установить формы
    void setForms(const std::list<CUIForm>& forms);

    // Подключить Rastr
    void setQAstra(const std::shared_ptr<QAstra>& sp_qastra);
    // Подключить TI
    void setQTI(const std::shared_ptr<QTI>& sp_qti);
    // Подключить BarsMDP
    void setQBarsMDP(const std::shared_ptr<QBarsMDP>& sp_qbarsmdp);
private:
    QHBoxLayout* createStyleSetting();
    void setCurrentFile(const QString &fileName, const std::string Shablon = "");
    // Обновить список недавних файлов
    void updateRecentFileActions();
    // Чтение настроек окна
    int  readSettings();
    // Сохранение настроек окна
    int  writeSettings();
    // Обработка показа окна
    void showEvent( QShowEvent* event ) override;
    // Создать действия (меню, кнопки)
    void createActions();
    // Создать статусную строку
    void createStatusBar();
    // Сбросить кэш логов
    void logCacheFlush();
    // Диалог "Сохранить изменения?"
    bool maybeSave();
    // Обработка закрытия окна
    void closeEvent(QCloseEvent *event) override;
    // Открыть конкретную форму
    void openForm(CUIForm _uiform);

    QString strippedName(const QString &fullFileName);
    QString curFile;                             // текущий загруженный или сохраненный файл
    QString curDir;                              // директория текущего загруженного или сохраненного файла
    QMap<QString,QString> mFilesLoad;

    QMdiArea*          m_workspace     = nullptr;               // Область для множественных документов
    McrWnd*            m_pMcrWnd       = nullptr;               // Окно протокола/макросов
    FormProtocol*      m_pFormProtocol = nullptr;               // Главный протокол
    QSignalMapper*     m_windowMapper  = nullptr;               // Маппер для управления окнами

    QMenu*             m_menuOpen      = nullptr;               // Меню "Открыть"
    QMenu*             m_recentFilesMenu=nullptr;               // Недавние файлы
    QMenu*             m_menuCalcParameters = nullptr;          // Параметры расчётов
    QMenu*             m_menuCalcTI = nullptr;                  // Меню ТИ
    QMenu*             m_menuProgrammProperties = nullptr;      // Настройки программы
    QMenu*             m_menuProperties = nullptr;              // Свойства

    QAction*           separatorAct;                            // Разделитель в меню
    QHBoxLayout*       m_layoutActions = nullptr;               // actions: rgm,opf,...
    QToolBar*          m_toolbarCalc   = nullptr;               // Панель расчётов
    QToolBar*          m_toolbarTI     = nullptr;               // Панель ТИ


    QDockWidget*       m_dock        = nullptr;                 // Текущая дока
    RtabWidget*        m_prtw_current  = nullptr;               // Текущая активная таблица

    // The main container for Advanced Docking System
    ads::CDockManager* m_DockManager = nullptr;

    qrastr::CacheLogVector  m_v_cache_log;                           // Кэш логов

    std::shared_ptr<QAstra> m_sp_qastra;
    std::shared_ptr<QTI> m_sp_qti;
    std::shared_ptr<QBarsMDP> m_sp_qbarsmdp;

    std::unique_ptr<PyHlp> m_up_PyHlp;                          // Python helper (для выполнения макросов)

    RTablesDataManager m_RTDM;                                  // Менеджер данных таблиц

    std::list<CUIForm> m_lstUIForms;                            // Список форм для отображения
    // Недавние файлы
    enum { MaxRecentFiles = 10 };
    QAction *recentFileActs[MaxRecentFiles];
};
