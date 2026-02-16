#include "calculationController.h"
#include "QAstra.h"
#include "QTI.h"
#include "QBarsMDP.h"
#include <QTimer>
#include <spdlog/spdlog.h>
#include <ctime>

KzParameters::KzParameters()
    : nonsym(eNonsym::KZ_1)
    , p1(1), p2(0), p3(0)
    , lengthFromP1InProc(0.0)
    , rd(0.0), z_re(0.0), z_im(0.0)
{}

CalculationController::CalculationController(
    std::shared_ptr<QAstra> qastra,
    std::shared_ptr<QTI> qti,
    std::shared_ptr<QBarsMDP> qbarsmdp,
    QObject* parent
)
    : QObject(parent)
    , m_qastra(qastra)
    , m_qti(qti)
    , m_qbarsmdp(qbarsmdp)
{
    assert(m_qastra != nullptr);
}

void CalculationController::executeRgm(const QString& parameters) {
    beginCalculation("RGM");
    
    eASTCode code = m_qastra->Rgm(parameters.toStdString());
    
    std::string str_msg;
    bool success = false;
    
    if (code == eASTCode::AST_OK) {
        str_msg = "Расчет режима выполнен успешно";
        spdlog::info("{}", str_msg);
        success = true;
    } else {
        str_msg = "Расчет режима завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::executeKdd(const QString& parameters) {
    beginCalculation("KDD");
    
    eASTCode code = m_qastra->Kdd(parameters.toStdString());
    
    std::string str_msg;
    bool success = false;
    
    if (code == eASTCode::AST_OK) {
        str_msg = "Контроль исходных данных выполнен успешно";
        spdlog::info("{}", str_msg);
        success = true;
    } else {
        str_msg = "Контроль исходных данных завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::executeOPF(const QString& parameters) {
    beginCalculation("OPF");
    
    eASTCode code = m_qastra->Opf(parameters.toStdString());
    
    std::string str_msg;
    bool success = false;
    
    if (code == eASTCode::AST_OK) {
        str_msg = "Оценка состояния выполнена успешно";
        spdlog::info("{}", str_msg);
        success = true;
    } else {
        str_msg = "Расчет оценки состояния завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::executeSMZUtst(const QString& parameters) {
    beginCalculation("SMZU");
    
    // Если параметры не заданы, используем текущий день месяца
    QString params = parameters;
    if (params.isEmpty()) {
        const std::time_t time_now = std::time(0);
        const std::tm* ptm_now = std::localtime(&time_now);
        params = QString::number(ptm_now->tm_mday);
    }
    
    eASTCode code = m_qastra->SMZU(params.toStdString());
    
    std::string str_msg;
    bool success = false;
    
    if (code == eASTCode::AST_OK) {
        str_msg = "Расчет МДП выполнен успешно";
        spdlog::info("{}", str_msg);
        success = true;
    } else {
        str_msg = "Расчет МДП завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    // Дополнительное логирование в Rastr
    _log_data log_data;
    log_data.lmt = success ? LogMessageTypes::Info : LogMessageTypes::Error;
    log_data.str_msg = str_msg;
    log_data.n_indx = -1;
    log_data.n_stage_id = -1;
    m_qastra->onRastrLog(log_data);
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::executeTkz(const KzParameters& params) {
    beginCalculation("KZ");
    
    const eASTCode code = m_qastra->Kz(
        params.parameters.toStdString(),
        params.nonsym,
        params.p1, params.p2, params.p3,
        params.lengthFromP1InProc,
        params.rd,
        params.z_re, params.z_im
    );
    
    std::string str_msg;
    bool success = false;
    
    if (code == eASTCode::AST_OK) {
        str_msg = "Расчет ТКЗ выполнен успешно";
        spdlog::info("{}", str_msg);
        success = true;
    } else {
        str_msg = "Расчет ТКЗ завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::showIdopDialog() {
    emit showDialogRequested("idop");
}

void CalculationController::recalcTiDor() {
    if (!checkTIAvailable()) {
        return;
    }
    
    beginCalculation("TI_RecalcDor");
    QElapsedTimer t_timer;
    t_timer.start();

    long code = m_qti->RecalcDor();
    
    std::string str_msg;
    bool success = false;
    
    if (code == 1) {
        str_msg = fmt::format("Пересчет дорасчетных измерений выполнен за {} мс.",
                              t_timer.nsecsElapsed() / 1000000.0);
        spdlog::info("{}", str_msg);
        success = true;
    } else {
        str_msg = "Пересчет дорасчетных измерений завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::updateTiTables() {
    if (!checkTIAvailable()) {
        return;
    }
    
    beginCalculation("TI_UpdateTables");
    QElapsedTimer t_timer;
    t_timer.start();

    long code = m_qti->UpdateTables();
    
    std::string str_msg;
    bool success = false;
    
    if (code == 1) {
        str_msg = fmt::format("Обновление данных по ТМ (Привязка->T) выполнено за {} мс.",
                              t_timer.nsecsElapsed() / 1000000.0);
        spdlog::info("{}", str_msg);
        success = true;
    } else {
        str_msg = "Пересчет дорасчетных измерений завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::calcPTI() {
    if (!checkTIAvailable()) {
        return;
    }
    
    beginCalculation("TI_CalcPTI");
    
    QElapsedTimer t_calc_pti;
    t_calc_pti.start();

    long code = m_qti->CalcPTI();
    
    std::string str_msg;
    bool success = false;
    
    if (code == 1) {
        str_msg = fmt::format("Расчет ПТИ выполнен за {} мс.", t_calc_pti.nsecsElapsed() / 1000000.0);
        spdlog::info("{}", str_msg);
        
        // Добавление ПТИ в таблицу
        QElapsedTimer t_add_pti;
        t_add_pti.start();

        code = m_qti->DobavPTI();
        
        if (code == 1) {
            str_msg += fmt::format("\nПТИ записаны в ТИ:Каналы за {} мс.",
                                   t_add_pti.nsecsElapsed() / 1000000.0);
            spdlog::info("PTI added to channels");
            success = true;
        } else {
            str_msg = "Ошибка записи ПТИ в ТИ:Каналы!";
            spdlog::error("{} : {}", static_cast<int>(code), str_msg);
            success = false;
        }
    } else {
        str_msg = "Расчет ПТИ завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::filtrTI() {
    if (!checkTIAvailable()) {
        return;
    }
    
    beginCalculation("TI_FiltrTI");
    QElapsedTimer t_filtr_ti;
    t_filtr_ti.start();

    long code = m_qti->FiltrTI();
    
    std::string str_msg;
    bool success = false;
    
    if (code == 1) {
        str_msg = fmt::format("Расчет Фильтра ТИ выполнен за {} мс.",
                              t_filtr_ti.nsecsElapsed() / 1000000.0);
        spdlog::info("{}", str_msg);
        success = true;
    } else {
        str_msg = "Расчет Фильтра ТИ завершился аварийно!";
        spdlog::error("{} : {}", static_cast<int>(code), str_msg);
        success = false;
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::prepareBarsMDP(const QString& sections) {
    if (!checkBarsMDPAvailable()) {
        return;
    }
    
    // Если секции не заданы - запрашиваем через диалог
    if (sections.isEmpty()) {
        // Запрашиваем показ диалога у MainWindow
        emit showDialogRequested("mdp_prepare");
        return;
    }
    
    beginCalculation("BarsMDP_Prepare");
    QElapsedTimer t_barsmdp;
    t_barsmdp.start();

    std::string str_msg;
    bool success = true;
    
    try {
        m_qbarsmdp->Init(sections.toStdString().c_str());
        m_qbarsmdp->UpdateMDPFields();
        m_qbarsmdp->UpdateAUTOFields();
        
        str_msg = fmt::format("Подготовка для расчета МДП для сечений {} за {} мс.",
                              sections.toStdString(), t_barsmdp.nsecsElapsed() / 1000000.0);
        spdlog::info("{}", str_msg);
    }
    catch (const std::exception& ex) {
        success = false;
        str_msg = fmt::format("Ошибка в ходе подготовки для расчета МДП для сечений {}: {}",
                              sections.toStdString(), ex.what());
        spdlog::error("{}", str_msg);
    }
    catch (...) {
        success = false;
        str_msg = fmt::format("Неизвестная ошибка в ходе подготовки для расчета МДП для сечений {}",
                              sections.toStdString());
        spdlog::error("{}", str_msg);
    }
    
    endCalculation(success, QString::fromStdString(str_msg));
}

void CalculationController::beginCalculation(const QString& name) {
    m_isCalculating = true;
    m_currentCalculation = name;
    
    spdlog::info("Starting calculation: {}", name.toStdString());
    emit calculationStarted(name);
}

void CalculationController::endCalculation(bool success, const QString& message) {
    m_isCalculating = false;
    QString calcName = m_currentCalculation;
    m_currentCalculation.clear();
    
    if (success) {
        spdlog::info("{}", message.toStdString());
    } else {
        spdlog::error("{}", message.toStdString());
    }
    
    emit statusMessage(message, 0);
    emit calculationFinished(calcName, success);
}

QString CalculationController::formatMessage(eASTCode code, const QString& calcName) const {
    bool success = (code == eASTCode::AST_OK);
    
    QString message;
    if (success) {
        message = QString("%1 выполнен успешно").arg(calcName);
    } else {
        message = QString("%1 завершился аварийно! (код: %2)")
            .arg(calcName)
            .arg(static_cast<int>(code));
    }
    
    return message;
}

void CalculationController::logResult(eASTCode code, const QString& calcName) {
    if (code == eASTCode::AST_OK) {
        spdlog::info("{} completed successfully", calcName.toStdString());
    } else {
        spdlog::error("{} failed with code: {}", 
                      calcName.toStdString(), 
                      static_cast<int>(code));
    }
}

bool CalculationController::checkTIAvailable() {
    if (m_qti == nullptr) {
        QString error = "Plugin TI not initialized! Function is unavailable.";
        spdlog::warn("{}", error.toStdString());
        emit calculationError(error);
        emit statusMessage(error, 5000);
        return false;
    }
    return true;
}

bool CalculationController::checkBarsMDPAvailable() {
    if (m_qbarsmdp == nullptr) {
        QString error = "Plugin BarsMDP not initialized! Function is unavailable.";
        spdlog::warn("{}", error.toStdString());
        emit calculationError(error);
        emit statusMessage(error, 5000);
        return false;
    }
    return true;
}
