#ifndef FORMPROTOCOL_H
#define FORMPROTOCOL_H

#include <QWidget>
#include <stack>

namespace Ui{
    class FormProtocol;
}
struct _log_data;
class QStringListModel;
class ProtocolTreeItem;
class ProtocolTreeModel;
class FormProtocol
    : public QWidget{
    Q_OBJECT
public:
    explicit FormProtocol(QWidget *parent = nullptr);
    ~FormProtocol();
signals:
private slots:
    void onRastrLog(const _log_data&);
    void onAppendProtocol(const QString& qstr);
private:
    Ui::FormProtocol*  ui = nullptr;
    QStringListModel*  pmodel_ = nullptr;
    ProtocolTreeModel* p_protocol_tree_model_;
    std::int64_t n_row_counter_ = 0;
    std::shared_ptr<ProtocolTreeItem> spti_stage_previos_;
    std::shared_ptr<ProtocolTreeItem> spti_stage_;
    std::stack< std::shared_ptr<ProtocolTreeItem> > s_spti_stages_;
};

#endif // FORMPROTOCOL_H
