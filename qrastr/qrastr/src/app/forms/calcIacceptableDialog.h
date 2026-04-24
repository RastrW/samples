#pragma once

#include <QDialog>


class QLineEdit;
class QLabel;
class QDateTimeEdit;
class ICalculationEngine;

class CalcIacceptableDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CalcIacceptableDialog(std::shared_ptr<ICalculationEngine> calcEngine,
                                   QWidget *parent = nullptr);
    virtual ~CalcIacceptableDialog() = default;
signals:
    /// Испускается когда расчёт внутри диалога завершён
    void sig_calculationFinished();
private slots:
    void on_buttonBox_accepted();
    void on_checkBox_checkStateChanged(int state);
protected:
    // Переопределите done() чтобы испустить сигнал перед закрытием
    void done(int result) override;
private:
    std::shared_ptr<ICalculationEngine> m_calcEngine;
    QLineEdit*    m_leTemperature;
    QLineEdit*    m_leEmergencyLoading;
    QLineEdit*    m_leSelection;
    QLabel*       m_lData;
    QDateTimeEdit* m_dateTimeEdit;
};
