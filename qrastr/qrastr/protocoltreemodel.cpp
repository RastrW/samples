#include <QStringList>

#include "protocoltreeitem.h"
#include "protocoltreemodel.h"
//using namespace Qt::StringLiterals;

ProtocolTreeModel::ProtocolTreeModel(QObject *parent)
    : QAbstractItemModel{parent}{
    rootItem = std::make_unique<ProtocolTreeItem>( QVariantList{tr("Title"), tr("Summary")} );
    //setupModelData(QStringView{data}.split(u'\n'), rootItem.get());
}
int ProtocolTreeModel::columnCount(const QModelIndex &parent) const{
    if (parent.isValid())
        return static_cast<ProtocolTreeItem*>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}
QVariant ProtocolTreeModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};
    const auto *item = static_cast<const ProtocolTreeItem*>(index.internalPointer());
    return item->data(index.column());
}
Qt::ItemFlags ProtocolTreeModel::flags(const QModelIndex &index) const{
    return index.isValid()
        ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}
QVariant ProtocolTreeModel::headerData(int section, Qt::Orientation orientation, int role) const{
    return orientation == Qt::Horizontal && role == Qt::DisplayRole
        ? rootItem->data(section) : QVariant{};
}
QModelIndex ProtocolTreeModel::index(int row, int column, const QModelIndex &parent) const{
    if (!hasIndex(row, column, parent))
        return {};
    ProtocolTreeItem *parentItem = parent.isValid()
        ? static_cast<ProtocolTreeItem*>(parent.internalPointer())
        : rootItem.get();
    if (auto *childItem = parentItem->child(row))
        return createIndex(row, column, childItem);
    return {};
}
QModelIndex ProtocolTreeModel::parent(const QModelIndex &index) const{
    if (!index.isValid())
        return {};
    auto *childItem = static_cast<ProtocolTreeItem*>(index.internalPointer());
    ProtocolTreeItem *parentItem = childItem->parentItem();
    return parentItem != rootItem.get()
        ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}
int ProtocolTreeModel::rowCount(const QModelIndex &parent) const{
    if (parent.column() > 0)
        return 0;
    const ProtocolTreeItem *parentItem = parent.isValid()
        ? static_cast<const ProtocolTreeItem*>(parent.internalPointer())
        : rootItem.get();
    return parentItem->childCount();
}
void ProtocolTreeModel::setupModelData(const QList<QStringView> &lines, ProtocolTreeItem *parent){
    struct ParentIndentation{
        ProtocolTreeItem *parent;
        qsizetype indentation;
    };
    QList<ParentIndentation> state{{parent, 0}};
    for (const auto &line : lines){
        qsizetype position = 0;
        for ( ; position < line.length() && line.at(position).isSpace(); ++position){
        }
        const QStringView lineData = line.sliced(position).trimmed();
        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            const auto columnStrings = lineData.split(u'\t', Qt::SkipEmptyParts);
            QVariantList columnData;
            columnData.reserve(columnStrings.count());
            for (const auto &columnString : columnStrings)
                columnData << columnString.toString();

            if(position > state.constLast().indentation){
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.
                auto *lastParent = state.constLast().parent;
                if (lastParent->childCount() > 0)
                    state.append({lastParent->child(lastParent->childCount() - 1), position});
            }else{
                while (position < state.constLast().indentation && !state.isEmpty())
                    state.removeLast();
            }
            // Append a new item to the current parent's list of children.
            auto *lastParent = state.constLast().parent;
            //lastParent->appendChild(std::make_unique<ProtocolTreeItem>(columnData, lastParent));
            auto item = std::make_shared<ProtocolTreeItem>(columnData, lastParent);
            lastParent->appendChild(item);
        }
    }


}
