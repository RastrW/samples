#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QStringList>

/// @brief Базовый класс для диалогов работы с рабочими областями.
class WorkspaceDialogBase : public QDialog
{
    Q_OBJECT

public:
    explicit WorkspaceDialogBase(const QStringList &workspaces,
                                 QWidget *parent = nullptr);

protected:
    /// Вставить виджет над списком (до вызова setupUi).
    void insertWidgetAboveList(QWidget *widget);

    /// Вставить виджет под списком (до вызова setupUi).
    void insertWidgetBelowList(QWidget *widget);

    /// Завершить построение компоновки. Вызывается из конструктора
    /// производного класса после добавления всех дополнительных виджетов.
    void finalizeLayout();
    ///список рабочих областей
    QListWidget        *m_listWidget  = nullptr;
    QDialogButtonBox   *m_buttonBox   = nullptr;

private:
    QVBoxLayout        *m_mainLayout  = nullptr;
    QList<QWidget *>    m_aboveWidgets;
    QList<QWidget *>    m_belowWidgets;
};
