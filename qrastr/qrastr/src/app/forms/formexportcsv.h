#pragma once
#include <QDialog>

class RData;
class QLineEdit;
class QRadioButton;

class formexportcsv : public QDialog
{
    Q_OBJECT
public:
    explicit formexportcsv(RData* _prdata, QWidget *parent = nullptr);
    ~formexportcsv() = default;

private slots:
    void on_pushButton_clicked();
    void accept() override;

private:
    RData*        prdata_;
    std::string   m_path;
    QLineEdit*    m_lePath;
    QLineEdit*    m_leTable;
    QLineEdit*    m_leParam;
    QLineEdit*    m_leSeparator;
    QLineEdit*    m_leSelection;
    QRadioButton* m_rbOverwrite;
    QRadioButton* m_rbExtend;
    QRadioButton* m_rbOverwriteHeaders;
};
