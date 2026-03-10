#pragma once

#include <QDialog>

class QLineEdit;

class SelectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectionDialog(std::string selection,
                           std::string colName, QWidget *parent = nullptr);
    virtual ~SelectionDialog() = default;
signals:
    void sig_selectionAccepted(std::string selection);

private slots:
    void on_buttonBox_accepted();

private:
    std::string  m_colName;
    std::string  m_selection;
    QLineEdit*   m_textEdit;
};
