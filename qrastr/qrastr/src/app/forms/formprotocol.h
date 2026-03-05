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
class QTreeView;

class FormProtocol : public QWidget
{
    Q_OBJECT
public:
    explicit FormProtocol(QWidget *parent = nullptr);
    ~FormProtocol() = default;

    void setIgnoreAppendProtocol(bool bl_ignore);

public slots:
    void onRastrLog(const _log_data&);

private slots:
    void onAppendProtocol(const QString& qstr);

private:
    ProtocolTreeModel*
        m_protocolTreeModel;
    std::stack< std::shared_ptr<ProtocolTreeItem> >
        m_sptiSstages;
    Qtitan::TreeGrid*
        m_ptg;
    QTreeView*
        twProtocol;
    bool
        m_ignoreAppendProtocol {false};
};
