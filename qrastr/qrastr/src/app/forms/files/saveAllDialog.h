#pragma once
#include "checkboxListDialog.h"
#include <QDir>

class IFileOperations;
class QTableWidget;
class QCheckBox;
class QTableWidgetItem;

class SaveAllDialog : public CheckboxListDialog
{
    Q_OBJECT
public:
    explicit SaveAllDialog(std::shared_ptr<IFileOperations> fileOps,
                           std::map<QString,QString> _mFilesLoad,
                            QWidget *parent = nullptr);

    void showEvent(QShowEvent* event) override;

private slots:
    void slot_buttonBoxAccepted();
private:
    std::shared_ptr<IFileOperations> m_fileOps;
    QDir                  m_dirShabl;
    std::map<QString,QString> m_FilesLoad;

    enum class _cols : int { save, templ, file, path };
};
