#pragma once

#include <QDialog>


class QLineEdit;
class QLabel;
class QDateTimeEdit;
class QAstra;

class CalcIacceptableDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CalcIacceptableDialog(QAstra* pqastra, QWidget *parent = nullptr);
    virtual ~CalcIacceptableDialog() = default;
private slots:
    void on_buttonBox_accepted();
    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    QAstra* pqastra;
    QLineEdit*    m_leTemperature;
    QLineEdit*    m_leEmergencyLoading;
    QLineEdit*    m_leSelection;
    QLabel*       m_lData;
    QDateTimeEdit* m_dateTimeEdit;
};
