#include "mymodel.h"
#include <QFont>
#include <QBrush>
#include <QTime>


MyModel::MyModel(QObject *parent)
    : QAbstractTableModel(parent)
    , timer_(new QTimer(this)){
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout , this, &MyModel::timerHit);
    timer_->start();
}
int MyModel::rowCount(const QModelIndex & /*parent*/) const{
    return n_rows_;
    //return 2'0'0;
    //return 200'000;
}
int MyModel::columnCount(const QModelIndex & /*parent*/) const{
    return n_cols_;
    //return 3'000;
    //return 3'0;
}
void MyModel::timerHit(){
    QModelIndex topLeft = createIndex(n_timer_row_,n_timer_col_);
    //emit a signal to make the view reread identified data
    QList<int>* pl = new QList<int>{ Qt::DisplayRole};
    emit dataChanged(topLeft, topLeft, *pl);  //!!! emit dataChanged(topLeft, topLeft, {Qt::DisplayRole}); //ustas!!! not working!!
}
QVariant MyModel::data(const QModelIndex &index, int role) const
{
    /*
    int roww = index.row();
    int colw = index.column();

    if (role == Qt::DisplayRole && roww == 0 && colw == 0)
        return QTime::currentTime().toString();
    return QVariant();
*/

    int row = index.row();
    int col = index.column();
    // generate a log message when this method gets called
    qDebug() << QString("row %1, col%2, role %3").arg(row).arg(col).arg(role);

    //"2.5 The Minimal Editing Example"
    if(role==Qt::DisplayRole) // else will see a checkboxes near string!!
        return QString("%1").arg(m_gridData[row][col]);
    return QVariant();

    //ELSE!

    switch (role) {
        case Qt::DisplayRole:
            if(row == 0 && col == 1) return QString("<--left");
            if(row == 1 && col == 1) return QString("right-->");
            if(row == n_timer_row_ && col == n_timer_col_ ) return QTime::currentTime().toString();
        return QString("Row%1, Column%2")
            .arg(row + 1)
            .arg(col +1);
        case Qt::FontRole:
            if (row == 0 && col == 0) { //change font only for cell(0,0)
                QFont boldFont;
                boldFont.setBold(true);
        return boldFont;
        }
        break;
        case Qt::BackgroundRole:
            if (row == 1 && col == 2)  //change background only for cell(1,2)
                return QBrush(Qt::red);
        break;
        case Qt::TextAlignmentRole:
            if (row == 1 && col == 1) //change text alignment only for cell(1,1)
                return int(Qt::AlignRight | Qt::AlignVCenter);
        break;
        case Qt::CheckStateRole:
            if (row == 1 && col == 0) //add a checkbox to cell(1,0)
                return Qt::Checked;
        break;
    }
    return QVariant();
    /*
    if (role == Qt::DisplayRole)
        return QString("Row%1, Column%2")
            .arg(index.row() + 1)
            .arg(index.column() +1);
*/
}

QVariant MyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QString("col_first");
        case 1:
            return QString("col_second");
        case 2:
            return QString("col_third");
        }
    }
    return QVariant();
}

Qt::ItemFlags MyModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index) ;
}


bool MyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole ) {
        if (!checkIndex(index))
            return false;
        //save value from editor to member m_gridData
        m_gridData[index.row()][index.column()] = value.toString();
        //for presentation purposes only: build and emit a joined string
        QString result;
        for (int row = 0; row < n_rows_; row++) {
            for (int col= 0; col < n_cols_; col++)
                result += m_gridData[row][col] + ' ';
        }
        emit editCompleted(result);
        return true;
    }
    return false;
}
