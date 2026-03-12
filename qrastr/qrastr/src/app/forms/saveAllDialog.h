#pragma once

#include <QDialog>
#include <QDir>
#include <QMap>

class QAstra;
class QTableWidget;
class QCheckBox;
class QTableWidgetItem;

class SaveAllDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SaveAllDialog(QAstra* pqastra, QMap<QString,QString> _mFilesLoad,
                         QWidget *parent = nullptr);

    void showEvent(QShowEvent* event) override;

private slots:
    void slot_buttonBoxAccepted();
    void slot_itemChanged(QTableWidgetItem* item);

private:
    void selectAllToggled(Qt::CheckState state);

    QAstra*               m_pqastra;
    QDir                  m_dirShabl;
    QMap<QString,QString> m_FilesLoad;
    QTableWidget*         m_twSaveFiles;
    QCheckBox*            m_cbSelectAll;
    bool                  m_bUpdating = false; // предотвращает рекурсию

    enum class _cols : int { save, templ, file, path };
};
