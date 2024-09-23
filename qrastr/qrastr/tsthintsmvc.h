#ifndef TSTHINTSMVC_H
#define TSTHINTSMVC_H

#include <QObject>


#include <QAbstractTableModel>
#include <QString>

const int COLS= 3;
const int ROWS= 2;

class MyModel
    : public QAbstractTableModel
{
    Q_OBJECT
public:
    MyModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override{
        if (role == Qt::EditRole) {
             if (!checkIndex(index))
                 return false;
             //save value from editor to member m_gridData
             m_gridData[index.row()][index.column()] = value.toString();
             //for presentation purposes only: build and emit a joined string
             QString result;
             for (int row = 0; row < ROWS; row++) {
                 for (int col= 0; col < COLS; col++)
                     result += m_gridData[row][col] + ' ';
             }
             emit editCompleted(result);
             return true;
         }
         return false;
    };
    Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
    QString m_gridData[ROWS][COLS];  //holds text entered into QTableView
signals:
    void editCompleted(const QString &);
};

class TstHintsMVC
    :public QObject{
    Q_OBJECT
public:
    explicit TstHintsMVC(QObject *parent = nullptr);
signals:
};

#endif // TSTHINTSMVC_H
