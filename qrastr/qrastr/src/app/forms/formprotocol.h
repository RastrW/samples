#ifndef FORMPROTOCOL_H
#define FORMPROTOCOL_H
#pragma once

#include <QWidget>
#include <stack>


namespace Ui{
    class FormProtocol;
}
namespace Qtitan{ class TreeGrid;}
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
    void setIgnoreAppendProtocol(bool bl_ignore);
signals:
public slots:
    void onRastrLog(const _log_data&);
private slots:

    void onAppendProtocol(const QString& qstr);
private:
    Ui::FormProtocol*  ui = nullptr;
    ProtocolTreeModel* p_protocol_tree_model_;
    std::stack< std::shared_ptr< ProtocolTreeItem > > s_spti_stages_;
    Qtitan::TreeGrid* ptg_;
    bool ignore_append_protocol_ = false;
};

#endif // FORMPROTOCOL_H
