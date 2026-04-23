#pragma once

#include <QDialog>

class QLineEdit;
class QRadioButton;
class ITableRepository;

class ImportCSV2dialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImportCSV2dialog(ITableRepository*  repo,
                              const std::string& tableName,
                              const std::string& defaultCols,
                              QWidget*           parent = nullptr);
    virtual ~ImportCSV2dialog() = default;
private slots:
    void on_pushButton_clicked();
    void on_buttonBox_accepted();

private:
    ITableRepository* m_repo;

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
