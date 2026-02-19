#ifndef TESTMODEL_H
#define TESTMODEL_H
#pragma once

#include <QAbstractTableModel>

#include <QTableView>
#include <QStandardItemModel>
#include <QLinearGradient>
#include <QList>
#include <QStandardItem>

class TestModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TestModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section,
                       Qt::Orientation orientation,
                       const QVariant &value,
                       int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

private:
};

class ChartModel : public QStandardItemModel
{
public:
    ChartModel():QStandardItemModel()
    {
        for (int i = 0; i < 10; ++i)
        {
            QList<QStandardItem*> row;
            for (int j = 0; j < 2; ++j)
            {
                QStandardItem* item = new QStandardItem;
                item->setData("50 %", Qt::EditRole);
                row.push_back(item);
            }
            appendRow(row);
        }
    }

    QVariant data(const QModelIndex& index, int role) const
    {
        if (role == Qt::BackgroundRole && index.isValid())
        {
            QLinearGradient grad(QPointF(0, 0), QPointF(1, 0));
            grad.setCoordinateMode(QGradient::ObjectBoundingMode);
            grad.setColorAt(0, Qt::blue);
            grad.setColorAt(0.5, Qt::blue);
            grad.setColorAt(0.50001, Qt::white);
            grad.setColorAt(1, Qt::white);
            grad.setSpread(QGradient::RepeatSpread);
            return QBrush(grad);
        }
        return QStandardItemModel::data(index, role);
    }
};

#endif // TESTMODEL_H
