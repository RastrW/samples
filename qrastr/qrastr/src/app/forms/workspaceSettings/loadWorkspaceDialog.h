#pragma once

#include "workspacedialogbase.h"

/**
 * @brief Диалог "Загрузить рабочую область".
 * Позволяет пользователю выбрать одну рабочую область из сохраненного списка
 */
class LoadWorkspaceDialog : public WorkspaceDialogBase
{
    Q_OBJECT

public:
    /**
    * Входные параметры:
    * @param workspaces — список сохранённых названий рабочих областей.
    * @param parent     — родительский виджет.
    */
    explicit LoadWorkspaceDialog(const QStringList &workspaces,
                                 QWidget           *parent = nullptr);

    /// Название рабочей области, выбранной пользователем.
    /// Возвращает пустую строку, если ни одна строка не была выделена.
    [[nodiscard]] QString selectedWorkspace() const;

signals:
    /// Испускается при нажатии Ok с именем выбранной рабочей области.
    void workspaceSelected(const QString &name);

private slots:
    void onOkClicked();
};
