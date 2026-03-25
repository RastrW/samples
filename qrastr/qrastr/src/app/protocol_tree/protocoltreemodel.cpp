#include <QStringList>

#include "protocoltreeitem.h"
#include "protocoltreemodel.h"
#include <astra/IPlainRastr.h>

ProtocolTreeModel::ProtocolTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    // Корень хранит заголовки; тип None — служебный
    rootItem = std::make_shared<ProtocolTreeItem>(
        QVariantList{ tr("Тип"), tr("Сообщение") },
        LogMessageTypes::None);
}

int ProtocolTreeModel::columnCount(const QModelIndex &parent) const{
    if (parent.isValid())
        return static_cast<ProtocolTreeItem*>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}

QVariant ProtocolTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};

    const auto* item =
        static_cast<const ProtocolTreeItem*>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
        // Колонка 0 — только иконка, текст в колонке 1
        return (index.column() == 0) ? QVariant{} : item->data(index.column());

    case Qt::DecorationRole:
        // Иконка только в колонке 0
        return (index.column() == 0) ? item->data(0) : QVariant{};

    case static_cast<int>(CustomRoles::MessageTypeRole):
        // Возвращаем int
        return static_cast<int>(item->messageType());

    default:
        return {};
    }
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
