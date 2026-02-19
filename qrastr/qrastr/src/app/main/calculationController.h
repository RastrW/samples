#pragma once

#include <QObject>
#include <QString>
#include <memory>

class QAstra;
class QTI;
class QBarsMDP;
enum class eASTCode;
enum class eNonsym;

/// @struct Параметры для расчёта токов короткого замыкания
struct KzParameters {
    QString parameters;
    eNonsym nonsym;
    long p1;
    long p2;
    long p3;
    double lengthFromP1InProc;
    double rd;
    double z_re;
    double z_im;
    
    KzParameters();
};

/// @class Контроллер расчётных операций
class CalculationController : public QObject {
    Q_OBJECT
    
public:
    explicit CalculationController(
        std::shared_ptr<QAstra> qastra,
        std::shared_ptr<QTI> qti,
        std::shared_ptr<QBarsMDP> qbarsmdp,
        QObject* parent = nullptr
    );
    ~CalculationController() = default;
    
    // ========== ПРОСТЫЕ РАСЧЁТЫ ==========
    /// @brief Расчёт установившегося режима
    void executeRgm(const QString& parameters = ""); 
    /// @brief Контроль исходных данных
    void executeKdd(const QString& parameters = "");
    /// @brief Оценка состояния
    void executeOPF(const QString& parameters = "s");
    /// @brief Расчёт МДП (СМЗУ)
    void executeSMZUtst(const QString& parameters = "");
    /// @brief Расчёт токов короткого замыкания
    void executeTkz(const KzParameters& params);
    
    // ========== ТЕЛЕИЗМЕРЕНИЯ ==========
    /// @brief Пересчёт дорасчётных измерений
    void recalcTiDor();
    /// @brief Обновление таблиц по телеметрии
    void updateTiTables();
    /// @brief Расчёт псевдотелеизмерений
    void calcPTI();
    /// @brief Фильтрация телеизмерений
    void filtrTI();
    
    // ========== МДП ==========   
    /**
     * @brief Подготовка данных для расчёта МДП
     * @param sections номера сечений (если пустое - запрос у пользователя)
     */
    void prepareBarsMDP(const QString& sections = "");
    
    // ========== ДИАЛОГОВЫЕ РАСЧЁТЫ ==========
    
    /// @brief Запрос показа диалога расчёта допустимых токов
    void showIdopDialog();
    
    // ========== СОСТОЯНИЕ ==========
    bool isCalculating() const { return m_isCalculating; }
    QString currentCalculation() const { return m_currentCalculation; }
signals:
    void calculationStarted(const QString& calcType);
    void calculationFinished(const QString& calcType, bool success);
    void calculationError(const QString& error);
    
    /**
     * @brief Сообщение для статусной строки
     * @param message текст сообщения
     * @param timeout время отображения (0 = постоянно)
     */
    void statusMessage(const QString& message, int timeout = 0);
    
    /**
     * @brief Запрос показа диалога (для GUI-логики)
     * @param dialogType тип диалога ("idop", "mdp_prepare")
     */
    void showDialogRequested(const QString& dialogType,
                             const QString& params = "");  
private:
    // ========== Зависимости ==========
    std::shared_ptr<QAstra> m_qastra;
    std::shared_ptr<QTI> m_qti;
    std::shared_ptr<QBarsMDP> m_qbarsmdp;
    
    // ========== Состояние ==========
    bool m_isCalculating = false;
    QString m_currentCalculation;
    
    // ========== Вспомогательные методы ==========
    void beginCalculation(const QString& name);
    void endCalculation(bool success, const QString& message);
    
    QString formatMessage(eASTCode code, const QString& calcName) const;
    void logResult(eASTCode code, const QString& calcName);
    
    bool checkTIAvailable();
    bool checkBarsMDPAvailable();
};
