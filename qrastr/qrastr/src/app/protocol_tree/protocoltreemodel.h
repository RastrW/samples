#pragma once
#include <memory>
#include <QAbstractItemModel>


class ProtocolTreeItem;

class ProtocolTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    enum class CustomRoles {
        MessageTypeRole = Qt::UserRole + 1
    };

    explicit ProtocolTreeModel(QObject* parent = nullptr);
    ~ProtocolTreeModel() override = default;
    Q_DISABLE_COPY_MOVE(ProtocolTreeModel)

    QVariant      data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant      headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex   index(int row, int column,
                      const QModelIndex& parent = {}) const override;
    QModelIndex   parent(const QModelIndex& index) const override;
    int           rowCount(const QModelIndex& parent = {}) const override;
    int           columnCount(const QModelIndex& parent = {}) const override;
    void clear();

    ProtocolTreeItem*                 getRootItem()   const { return rootItem.get(); }
    std::shared_ptr<ProtocolTreeItem> getRootItemSp() const { return rootItem; }

private:
    std::shared_ptr<ProtocolTreeItem> rootItem;
};
