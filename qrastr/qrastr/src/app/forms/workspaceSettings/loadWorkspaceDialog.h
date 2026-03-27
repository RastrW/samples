#pragma once

#include "workSpaceDialogBase.h"

/**
 * @brief Диалог "Загрузить рабочую область".
 *
 * Отображает список сохранённых рабочих областей.
 * Вторая колонка показывает, какая область будет загружена при старте,
 * но доступна только для чтения — изменить её через этот диалог нельзя.
 */
class LoadWorkspaceDialog : public WorkSpaceDialogBase
{
    Q_OBJECT

public:
    /**
     * @param workspaces   — список сохранённых рабочих областей.
     * @param startupName  — имя области, загружаемой при старте (может быть пустым).
     * @param parent       — родительский виджет.
     */
    explicit LoadWorkspaceDialog(const QStringList &workspaces,
                                 const QString     &startupName,
                                 QWidget           *parent = nullptr);

    /// Имя выбранной пользователем области; пусто, если выбор не сделан.
    [[nodiscard]] QString selectedWorkspace() const;

signals:
    void workspaceSelected(const QString &name);

private slots:
    void onOkClicked();

private:
    void setupColumns(const QString &startupName);
};

