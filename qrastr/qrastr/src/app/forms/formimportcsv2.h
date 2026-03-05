#pragma once

#include <QDialog>

class RData;
class QLineEdit;
class QRadioButton;

class formimportcsv2 : public QDialog
{
    Q_OBJECT
public:
    explicit formimportcsv2(RData* prdata,
                            QWidget *parent = nullptr);
    virtual ~formimportcsv2() = default;
private slots:
    void on_pushButton_clicked();
    void on_buttonBox_accepted();

private:
    RData*        m_prdata;
    QLineEdit*    m_leFile;
    QLineEdit*    m_leTname;
    QLineEdit*    m_leParams;
    QLineEdit*    m_leDivider;
    QLineEdit*    m_leSelection;
    QLineEdit*    m_leByDefault;
    QRadioButton* m_rbAdd;
    QRadioButton* m_rbLoad;
    QRadioButton* m_rbLoad2;
    QRadioButton* m_rbUnion;
    QRadioButton* m_rbUpdate;
};
