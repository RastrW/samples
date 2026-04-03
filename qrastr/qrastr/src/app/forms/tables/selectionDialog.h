#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;

class SelectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectionDialog(std::string selection,
                             std::string colName,
                             QWidget*    parent = nullptr);
    virtual ~SelectionDialog() = default;

signals:
    void sig_selectionAccepted(std::string selection);

private:
    void insertAtCursor(const QString& text);

    QLineEdit* m_edit    = nullptr;
};
