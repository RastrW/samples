#pragma once

#include <QDialog>
class RData;
class RCol;
class QComboBox;
class QLabel;
class QRadioButton;
class QLineEdit;

class formgroupcorrection : public QDialog
{
    Q_OBJECT
public:
    explicit formgroupcorrection(RData* _prdata, RCol* prcol,
                                 QWidget *parent = nullptr);
    virtual ~formgroupcorrection() = default;
private slots:
    void on_buttonBox_accepted();

private:
    RData*        m_prdata;
    RCol*         m_prcol;
    std::string   m_selection;
    std::string   m_expression;

    QLineEdit*    m_leExpression;
    QLineEdit*    m_leSelection;
};
