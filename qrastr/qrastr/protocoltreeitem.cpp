#include "protocoltreeitem.h"


ProtocolTreeItem::ProtocolTreeItem(QVariantList data, ProtocolTreeItem* parent)
    //: m_itemData(std::move(data)), m_parentItem(parent){
    :qvl_data_(std::move(data)), pti_parent_(parent){
}
//void ProtocolTreeItem::appendChild(std::unique_ptr<ProtocolTreeItem>&& child){
void ProtocolTreeItem::appendChild( _vsptis::value_type& child){
    //m_childItems.emplace_back(std::move(child));
    //vsptis_.emplace_back(std::move(child));
    vsptis_.emplace_back(child);
}
ProtocolTreeItem* ProtocolTreeItem::child(int row){
    //return row >= 0 && row < childCount() ? m_childItems.at(row).get() : nullptr;
    return row >= 0 && row < childCount() ? vsptis_.at(row).get() : nullptr;
}
int ProtocolTreeItem::childCount() const {
    //return int(m_childItems.size());
    return int(vsptis_.size());
}
int ProtocolTreeItem::columnCount() const{
    //return int(m_itemData.count());
    return int(qvl_data_.count());
}
QVariant ProtocolTreeItem::data(int column) const{
    //return m_itemData.value(column);
    return qvl_data_.value(column);
}
ProtocolTreeItem* ProtocolTreeItem::parentItem(){
    //return m_parentItem;
    return pti_parent_;
}
int ProtocolTreeItem::row() const{ //REFACTOR THIS!
    if (pti_parent_ == nullptr)
        return 0;
    const auto it = std::find_if(pti_parent_->vsptis_.cbegin(), pti_parent_->vsptis_.cend(),
                                 [this](const _vsptis::value_type& item) {
                                     return item.get() == this;
                                 });

    if (it != pti_parent_->vsptis_.cend())
        return std::distance(pti_parent_->vsptis_.cbegin(), it);
    Q_ASSERT(false); // should not happen
    return -1;

    /*
    if (m_parentItem == nullptr)
        return 0;
    const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
                                 [this](const std::unique_ptr<ProtocolTreeItem> &treeItem) {
                                     return treeItem.get() == this;
                                 });

    if (it != m_parentItem->m_childItems.cend())
        return std::distance(m_parentItem->m_childItems.cbegin(), it);
    Q_ASSERT(false); // should not happen
    return -1;
    */
}
