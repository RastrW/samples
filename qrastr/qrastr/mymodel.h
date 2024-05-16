#ifndef MYMODEL_H
#define MYMODEL_H

#include <QAbstractTableModel>
#include <QTimer>

class MyModel : public QAbstractTableModel
{
    Q_OBJECT
    QTimer* timer_;
public:
    MyModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    //"2.3 A Clock inside a Table Cell"
    inline static constexpr int n_timer_row_ = 3; // 3
    inline static constexpr int n_timer_col_ = 1; // 1
    void timerHit(); // from example of time change in cell

    //"2.4 Setting up Headers for Columns and Rows"
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    //"2.5 The Minimal Editing Example"
    inline static constexpr int n_rows_ = 300; // 3
    inline static constexpr int n_cols_ = 10; // 1
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
    QString m_gridData[n_rows_][n_cols_];  //holds text entered into QTableVi
signals:
    void editCompleted(const QString &);
};

#endif // MYMODEL_H