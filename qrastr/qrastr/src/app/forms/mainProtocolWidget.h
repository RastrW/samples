#pragma once

#include <QWidget>
#include <stack>

namespace Ui{
    class FormProtocol;
}
struct _log_data;
class QStringListModel;
class ProtocolTreeItem;
class ProtocolTreeModel;
class QTreeView;
enum class LogMessageTypes;
class QVBoxLayout;
class QPushButton;
class ProtocolFilterProxyModel;

class MainProtocolWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainProtocolWidget(QWidget* parent = nullptr);
    ~MainProtocolWidget() = default;

    void setIgnoreAppendProtocol(bool bl_ignore);

public slots:
    void onRastrLog(const _log_data&);

private slots:
    void onAppendProtocol(const QString& qstr);
    void applyFilter(const QSet<LogMessageTypes>& filter);
    void onContextMenu(const QPoint& pos);

private:
    void setupFilterPanel(QVBoxLayout* layout);
    void setupTreeView(QVBoxLayout* layout);
    QPixmap iconByIndex(int idx) const;
    int iconIndexForMessage(LogMessageTypes lmt) const;
    int iconIndexForStage(const ProtocolTreeItem* item) const;

    ProtocolTreeModel*      m_model        = nullptr;
    ProtocolFilterProxyModel* m_proxy      = nullptr;
    QTreeView*              m_treeView     = nullptr;
    std::stack<std::shared_ptr<ProtocolTreeItem>> m_stages;

    // Кнопки фильтрации
    QPushButton* m_btnAll     = nullptr;
    QPushButton* m_btnError   = nullptr;
    QPushButton* m_btnWarning = nullptr;
    QPushButton* m_btnMessage = nullptr;
    QPushButton* m_btnInfo    = nullptr;

    bool m_ignoreAppendProtocol = false;
};