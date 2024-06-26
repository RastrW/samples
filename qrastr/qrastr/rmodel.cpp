#include "rmodel.h"
#include <QFont>
#include <QBrush>
#include <QTime>
#include <QDebug>

#include "fmt/format.h"

RModel::RModel(QObject *parent, CRastrHlp& rastr)
    : QAbstractTableModel(parent)
        , rastr_(rastr)
    , timer_(new QTimer(this)){
    setEmitSignals(true);
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout , this, &RModel::timerHit);
    timer_->start();
}
int RModel::populateDataFromRastr(){

    rastr_.GetFormData(n_form_indx_);
    CUIForm form = rastr_.GetUIForm(n_form_indx_);

    up_rdata = std::unique_ptr<RData>(new RData( rastr_.GetRastrId(),form.TableName()));
    up_rdata->Initialize(form);
    up_rdata->populate();

    for (RCol &rcol : *up_rdata)
        vqcols_.push_back(rcol.title().c_str());

    return 1;
};
int RModel::rowCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->at(0).size());
    //return n_rows_;
    //return 2'0'0;
    //return 200'000;
}
int RModel::columnCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->size());
    //return n_cols_;
    //return 3'000;
    //return 3'0;
}
void RModel::timerHit(){
    QModelIndex topLeft = createIndex(1,1);
    //emit a signal to make the view reread identified data
    QVector<int>* pl = new QVector<int>{ Qt::DisplayRole};
    emit dataChanged(topLeft, topLeft, *pl);  //!!! emit dataChanged(topLeft, topLeft, {Qt::DisplayRole}); //ustas!!! not working!!
}
QVariant RModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    QVariant item;

    RData::const_iterator iter_col = up_rdata->begin() + col;
    _col_data::const_iterator iter_data = (*iter_col).begin() + row;
    switch (role) {
        /*case Qt::CheckStateRole:
            if (row == 1 && col == 0) //add a checkbox to cell(1,0)
                return Qt::Checked;*/
        case Qt::DisplayRole:
        case Qt::EditRole:

        switch((*iter_data).index()){
            case RCol::_en_data::DATA_INT: item =  std::get<int>(*iter_data) ;                 break;
            case RCol::_en_data::DATA_STR: item =  std::get<std::string>(*iter_data).c_str() ; break;
            case RCol::_en_data::DATA_DBL: item =  std::get<double>(*iter_data);               break;
            default :                      item =  ( "type_unknown" );                         break;
        }
        return item;

        case Qt::ToolTipRole:
            return QString("Row %1, Column %2")
                .arg(index.row() + 1)
                .arg(index.column() +1);

        default:
            return QVariant();
    }

    // generate a log message when this method gets called
    //qDebug() << QString("row %1, col%2, role %3").arg(row).arg(col).arg(role);

    //"2.5 The Minimal Editing Example"
   // if(role==Qt::DisplayRole) // else will see a checkboxes near string!!
    //    return QString("%1").arg(m_gridData[row][col]);


    //return QVariant();
    return QString("Row%1, Column%2")
        .arg(index.row() + 1)
        .arg(index.column() +1);

    //ELSE!

    switch (role) {
        case Qt::DisplayRole:
            if(row == 0 && col == 1) return QString("<--left");
            if(row == 1 && col == 1) return QString("right-->");
            if(row == 1 && col == 1 ) return QTime::currentTime().toString();
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

QVariant RModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //auto field = std::advance( this->rastr_.GetUIForm(section).Fields().begin(),section);
    //auto it_field =  this->rastr_.GetUIForm(section).Fields().begin();


    //this->rastr_.GetUIForm(section).Fields().begin()
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {

        if (section < vqcols_.size() )
            return vqcols_[section];
    }
    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {

        return section + 1;
    }
    return QVariant();
}

Qt::ItemFlags RModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index) ;
}


bool RModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int col = index.column();
    int row = index.row();

    long ret = -1;
    RData::iterator iter_col = up_rdata->begin() + col;
    _col_data::iterator iter_data = (*iter_col).begin() + row;
    switch((*iter_data).index()){
    case RCol::_en_data::DATA_INT:
        qDebug() << "int: " << value.toInt();
        (*iter_data).emplace<int> (value.toInt());
        ValSetInt(up_rdata->id_rastr_,up_rdata->t_name_.c_str(),(*iter_col).str_name_.c_str(),row,value.toInt());
        qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<value.toInt();
        break;
    case RCol::_en_data::DATA_STR:
        qDebug() << "str: " << value.toString();
        (*iter_data).emplace<std::string>(value.toString().toStdString().c_str());
        qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<value.toString().toStdString().c_str();
        break;
    case RCol::_en_data::DATA_DBL:
        //qDebug() << "double: " << value.toDouble();
        (*iter_data).emplace<double> (value.toDouble());
        ret = ValSetDbl(up_rdata->id_rastr_,up_rdata->t_name_.c_str(),(*iter_col).str_name_.c_str(),row,value.toDouble());
        qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<value.toDouble();
        break;
    default :                                               break;
    }

    if (emitSignals())
    {
        //emit modelChanged(QicsRegion(row, col, row, col)); // TO DO: handle signal ?
        emit editCompleted(value.toString());
    }
    return true;


    if (role == Qt::EditRole ) {
        if (!checkIndex(index))
            return false;
        //save value from editor to member m_gridData
        //m_gridData[index.row()][index.column()] = value.toString();
        //for presentation purposes only: build and emit a joined string
        QString result;
       // for (int row = 0; row < n_rows_; row++) {
        //    for (int col= 0; col < n_cols_; col++)
                //result += m_gridData[row][col] + ' ';
      //  }
        emit editCompleted(result);
        return true;
    }
    return false;
}
std::vector<std::tuple<int,int>>  RModel::ColumnsWidth()
{
    std::vector<std::tuple<int,int>> cw;

    int i = 0;
    for(RCol& col : *up_rdata)
        cw.emplace_back(i++,std::stoi(col.width()));
    return cw;
}
RCol* RModel::getRCol(int col)
{
    RData::iterator iter_col = up_rdata->begin() + col;
    return &(*iter_col);
}
RData* RModel::getRdata()
{
    return up_rdata.get();
}
bool RModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    getRdata()->AddRow(row);
    endInsertRows();
    return true;
}

bool RModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}

bool RModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    getRdata()->RemoveRDMRow(row);
    // FIXME: Implement me!
    endRemoveRows();
    return true;
}

bool RModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}
