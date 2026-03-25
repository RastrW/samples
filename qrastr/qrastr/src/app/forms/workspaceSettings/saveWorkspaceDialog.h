#pragma once

#include "workspacedialogbase.h"


class QTreeWidget;
class QButtonGroup;
class QLineEdit;
class QRadioButton;
/**
 * @brief Диалог "Сохранить рабочую область".
 *
 * Позволяет пользователю:
 *  - просматривать список существующих рабочих областей;
 *  - добавлять новую рабочую область (кнопка «Добавить» раскрывает поле ввода);
 *  - устанавливать флаг «Загрузка при старте»;
 */
class SaveWorkspaceDialog : public WorkspaceDialogBase
{
    Q_OBJECT

public:
    explicit SaveWorkspaceDialog(const QStringList &workspaces,
                                 const QString     &startupName,
                                 QWidget           *parent = nullptr);

    [[nodiscard]] QString     newWorkspaceName()      const;
    [[nodiscard]] QString     startupWorkspaceName()  const;
    [[nodiscard]] QStringList deletedWorkspaceNames() const;

private slots:
    void onAddOkClicked();
    void onAddCancelClicked();
    void onAddButtonClicked();
    void onDeleteClicked();
    void onOkClicked();

private:
    void setupAddRow();
    void updateStartupControls();   ///< снять/включить кнопки если список пуст
    void setupTree(const QString &startupName);

    QButtonGroup *m_radioGroup    = nullptr;  ///< группа радиокнопок в колонке 2
    QRadioButton *m_noStartupRadio = nullptr;  ///< «не загружать при старте»

    // Строка добавления
    QWidget      *m_addRow        = nullptr;
    QLineEdit    *m_nameEdit      = nullptr;
    QPushButton  *m_addButton     = nullptr;  ///< «Добавить» — раскрывает строку
    QPushButton  *m_addOkBtn      = nullptr;
    QPushButton  *m_addCancelBtn  = nullptr;

    QPushButton  *m_deleteButton  = nullptr;

    QString       m_newWorkspaceName;
    QStringList   m_deletedNames;
};