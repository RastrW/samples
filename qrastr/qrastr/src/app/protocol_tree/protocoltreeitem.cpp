#include "protocoltreeitem.h"
#include <astra/IPlainRastr.h>

ProtocolTreeItem::ProtocolTreeItem(QVariantList data,
                                   LogMessageTypes lmt,
                                   ProtocolTreeItem* parent)
    : qvl_data_(std::move(data))
    , pti_parent_(parent)
    , lmt_(lmt){}

LogMessageTypes
ProtocolTreeItem::messageType() const {
    return lmt_;
}

void ProtocolTreeItem::appendChild(_vsptis::value_type& child) {
    switch (child->lmt_) {
    case LogMessageTypes::SystemError:
    case LogMessageTypes::Failed:
    case LogMessageTypes::Error:   m_errors++;   break;
    case LogMessageTypes::Warning: m_warnings++; break;
    case LogMessageTypes::Message: m_messages++; break;
    default: break;
    }

    vsptis_.emplace_back(child);
}

ProtocolTreeItem* ProtocolTreeItem::child(int row){
   return row >= 0 && row < childCount() ? vsptis_.at(row).get() : nullptr;
}

int ProtocolTreeItem::childCount() const {
    return int(vsptis_.size());
}

int ProtocolTreeItem::columnCount() const{
    return int(qvl_data_.count());
}

QVariant ProtocolTreeItem::data(int column) const{
    return qvl_data_.value(column);
}

ProtocolTreeItem* ProtocolTreeItem::parentItem(){
    return pti_parent_;
}

int ProtocolTreeItem::row() const{ ///@todo REFACTOR THIS!
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
}

void ProtocolTreeItem::propagateStageStats(const ProtocolTreeItem* child) {
    m_errors   += child->m_errors;
    m_warnings += child->m_warnings;
    m_messages += child->m_messages;
}

void ProtocolTreeItem::clearChildren() {
    vsptis_.clear();
    m_errors = m_warnings = m_messages = 0;
}