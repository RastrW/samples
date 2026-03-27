#pragma once
#include <QDialog>

class QTreeWidget;
class QDialogButtonBox;
class QVBoxLayout;

/// @brief Базовый класс для диалогов работы с рабочими областями.
class WorkSpaceDialogBase : public QDialog
{
    Q_OBJECT

public:
    explicit WorkSpaceDialogBase(const QStringList &workspaces,
                                 QWidget           *parent = nullptr);

protected:
    void insertWidgetAboveList(QWidget *widget);
    void insertWidgetBelowList(QWidget *widget);
    void finalizeLayout();

    QTreeWidget      *m_tree       = nullptr;
    QDialogButtonBox *m_buttonBox  = nullptr;

private:
    QVBoxLayout      *m_mainLayout = nullptr;
    QList<QWidget *>  m_aboveWidgets;
    QList<QWidget *>  m_belowWidgets;
};
