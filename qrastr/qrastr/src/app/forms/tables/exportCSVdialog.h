#pragma once
#include <QDialog>

class RData;
class QLineEdit;
class QRadioButton;
class ITableRepository;

class ExportCSVdialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExportCSVdialog(ITableRepository* repo,
                             const std::string& tableName,
                             const std::string& defaultCols,
                             QWidget* parent = nullptr);
    ~ExportCSVdialog() = default;

private slots:
    void on_pushButton_clicked();
    void accept() override;

private:
    ITableRepository*              m_repo;
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
