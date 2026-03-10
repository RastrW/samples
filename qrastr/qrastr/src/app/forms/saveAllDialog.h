#pragma once

#include <QDialog>
#include <QDir>

class QAstra;
class QTableWidget;

class SaveAllDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SaveAllDialog(QAstra* pqastra, QMap<QString,QString> _mFilesLoad,
                         QWidget *parent = nullptr);

    void showEvent(QShowEvent* event) override;

private slots:
    void on_buttonBox_accepted();

private:
    QAstra*              m_pqastra;
    QDir                 m_dirShabl;
    QMap<QString,QString> m_FilesLoad;
    QTableWidget*        m_twSaveFiles;

    enum class _cols : int { save, templ, file, path };
};
