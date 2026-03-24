#pragma once

#include "workspacedialogbase.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

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
    /**
    * Входные параметры:
    * @param workspaces       — текущий список названий рабочих областей.
    * @param loadOnStartup    — текущее значение флага «Загрузка при старте».
    * @param parent           — родительский виджет.
    */
    explicit SaveWorkspaceDialog(const QStringList &workspaces,
                                 bool               loadOnStartup,
                                 QWidget           *parent = nullptr);

    /// Название новой рабочей области, введённой пользователем.
    /// Возвращает пустую строку, если ввод не был выполнен.
    [[nodiscard]] QString newWorkspaceName() const;

    /// Состояние флага «Загрузка при старте» на момент закрытия диалога.
    [[nodiscard]] bool loadOnStartup() const;

private slots:
    void onAddClicked();
    void onOkClicked();

private:
    void setupAddWidget();

    // --- Виджеты ---
    QCheckBox  *m_loadOnStartupCheck = nullptr;

    // Строка «Добавить»
    QWidget    *m_addRow             = nullptr; ///< контейнер поля ввода + кнопки
    QLineEdit  *m_nameEdit           = nullptr;
    QPushButton *m_addButton         = nullptr;

    QString     m_newWorkspaceName;
};
