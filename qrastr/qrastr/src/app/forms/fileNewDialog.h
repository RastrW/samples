#pragma once
#include <QDialog>
#include <set>

class QTableWidget;
class QTableWidgetItem;
class QCheckBox;


class FileNewDialog : public QDialog
{
    Q_OBJECT
public:
    using _s_checked_templatenames = std::set<std::string>;
    explicit FileNewDialog(QWidget *parent = nullptr);
    virtual ~FileNewDialog() = default;

    _s_checked_templatenames getCheckedTemplateNames() const;

    static constexpr const int n_colnum_checked_      = 0;
    static constexpr const int n_colnum_templatename_ = 1;

private slots:

    void slot_itemChanged(QTableWidgetItem* item);

private:
    void selectAllToggled(Qt::CheckState state);

    QTableWidget*     m_twTemplates;
    QCheckBox*        m_cbSelectAll;
    bool              m_bUpdating = false; // предотвращает рекурсию
};
