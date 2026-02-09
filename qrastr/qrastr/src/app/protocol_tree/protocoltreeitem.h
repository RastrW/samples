#ifndef PROTOCOLTREEITEM_H
#define PROTOCOLTREEITEM_H
#pragma once

#include <memory>
#include <QVariant>
#include <QList>

class ProtocolTreeItem{
public:
    using _vsptis = std::vector<std::shared_ptr<ProtocolTreeItem>>;
    explicit ProtocolTreeItem(QVariantList data, ProtocolTreeItem* parentItem = nullptr);
    //void appendChild(std::unique_ptr<ProtocolTreeItem>&& child );
    void appendChild( _vsptis::value_type& child);
    ProtocolTreeItem* child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    ProtocolTreeItem* parentItem();
private:
    //std::vector<std::unique_ptr<ProtocolTreeItem>> m_childItems;
    _vsptis vsptis_;
    QVariantList qvl_data_;
    ProtocolTreeItem* pti_parent_ = nullptr;
};

#endif // PROTOCOLTREEITEM_H
