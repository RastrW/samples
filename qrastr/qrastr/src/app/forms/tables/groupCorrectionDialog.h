#pragma once

#include <QDialog>
class RData;
class RCol;
class QComboBox;
class QLabel;
class QRadioButton;
class QLineEdit;
class ITableRepository;

class GroupCorrectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GroupCorrectionDialog( std::shared_ptr<ITableRepository> tables,
                                    RData* prdata,
                                    RCol* prcol,
                                    QWidget *parent = nullptr);
    virtual ~GroupCorrectionDialog() = default;
private slots:
    void on_buttonBox_accepted();

private:
    std::shared_ptr<ITableRepository> m_tables;
    RData*            m_prdata;
    RCol*             m_prcol;

    std::string   m_selection;
    std::string   m_expression;

    QLineEdit*    m_leExpression;
    QLineEdit*    m_leSelection;
};
