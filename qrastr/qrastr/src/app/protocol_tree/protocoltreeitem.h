#pragma once

#include <memory>
#include <QVariant>
#include <QList>

enum class LogMessageTypes;

class ProtocolTreeItem{
public:
    using _vsptis = std::vector<std::shared_ptr<ProtocolTreeItem>>;

    explicit ProtocolTreeItem(QVariantList data,
                              LogMessageTypes lmt,
                              ProtocolTreeItem* parentItem = nullptr);

    void appendChild( _vsptis::value_type& child);
    ProtocolTreeItem* child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    ProtocolTreeItem* parentItem();
    // Метод для обновления иконки (вызывается при CloseStage):
    void setIconData(const QVariant& icon) { qvl_data_[0] = icon; }
    LogMessageTypes   messageType() const;

    int errors()   const { return m_errors; }
    int warnings() const { return m_warnings; }
    int messages() const { return m_messages; }
private:
    _vsptis vsptis_;
    QVariantList qvl_data_;
    ProtocolTreeItem* pti_parent_ = nullptr;
    LogMessageTypes   lmt_;

    int m_errors   = 0;
    int m_warnings = 0;
    int m_messages = 0;
};
