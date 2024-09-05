#include "rmodel.h"
#include <QFont>
#include <QBrush>
#include <QTime>
#include <QDebug>

//#include "fmt/format.h"

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
}
int RModel::columnCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->size());
}
void RModel::timerHit(){
    QModelIndex topLeft = createIndex(1,1);
    //emit a signal to make the view reread identified data
    QVector<int>* pl = new QVector<int>{ Qt::DisplayRole};
    //emit dataChanged(topLeft, topLeft, *pl);  //!!! emit dataChanged(topLeft, topLeft, {Qt::DisplayRole}); //ustas!!! not working!!
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

    RData::iterator iter_col = up_rdata->begin() + col;
    _col_data::iterator iter_data = (*iter_col).begin() + row;

    if (data(index, role) != value)
    {
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
            ValSetStr(up_rdata->id_rastr_,up_rdata->t_name_.c_str(),(*iter_col).str_name_.c_str(),row,value.toString().toStdString().c_str());
            qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<value.toString().toStdString().c_str();
            break;
        case RCol::_en_data::DATA_DBL:
            //qDebug() << "double: " << value.toDouble();
            (*iter_data).emplace<double> (value.toDouble());
            ValSetDbl(up_rdata->id_rastr_,up_rdata->t_name_.c_str(),(*iter_col).str_name_.c_str(),row,value.toDouble());
            qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<value.toDouble();
            break;
        default :                                               break;
        }

        if (emitSignals())
        {
            emit dataChanged(getRdata()->t_name_,getRCol(col)->name(),row,value );
            return true;
        }
        return false;
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
int RModel::getIndexCol(std::string _col)
{
    for (int i = 0 ; i<this->columnCount(); i++)
    {
        if (this->getRCol(i)->name() == _col)
            return i;
    }
    return -1;
}
RData* RModel::getRdata()
{
    return up_rdata.get();
}
bool RModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    getRdata()->AddRow(row);
    if (emitSignals())
    {
        emit RowInserted(getRdata()->t_name_,row);
    }
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
    if (emitSignals())
    {
        emit RowDeleted(getRdata()->t_name_,row);
    }
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

void RModel::onRModelchange(std::string _t_name, std::string _col_name, int _row, QVariant _value)
{
    if (_t_name != this->getRdata()->t_name_)
        return;

    int col = this->getIndexCol(_col_name);
    if (col < 0)
        return;

    RData::iterator iter_col = up_rdata->begin() + col;
    _col_data::iterator iter_data = (*iter_col).begin() + _row;

    switch((*iter_data).index()){
    case RCol::_en_data::DATA_INT:
        (*iter_data).emplace<int> (_value.toInt());
        break;
    case RCol::_en_data::DATA_STR:
        (*iter_data).emplace<std::string>(_value.toString().toStdString().c_str());
        break;
    case RCol::_en_data::DATA_DBL:
        (*iter_data).emplace<double> (_value.toDouble());
        break;
    default :                                               break;
    }
}

void RModel::onrm_RowInserted(std::string _t_name, int _row)
{
     _vt val;
    if ( (_row < 0) || (_row > (this->getRdata()[0]).size() )) // add at end
    {
        for( RCol& col : *this->getRdata() )
        {
            col.push_back(val);
        }
    }
    else
    {
        for( RCol& col : *this->getRdata() )
        {
            col.insert(col.begin()+_row,val);
        }
    }
}
void RModel::onrm_RowDeleted(std::string _t_name, int _row)
{
    for( RCol& col : *this->getRdata() )
    {
        col.erase(col.begin()+_row);
    }
}

bool RModel::isBinary(const QModelIndex& index) const
{
   /* std::lock_guard<std::mutex> lock(m_mutexDataCache);

    const size_t row = static_cast<size_t>(index.row());
    if(!m_cache.count(row))
        return false;

    const auto & cached_row = m_cache.at(row);
    return isBinary(cached_row.at(static_cast<size_t>(index.column())));
    */
    switch (this->up_rdata->at(index.column()).en_data_)
   {
       case RCol::_en_data::DATA_INT:
        return true;
           break;
       case RCol::_en_data::DATA_DBL:
           return true;
           break;
       case RCol::_en_data::DATA_STR:
           return false;
           break;
       default:
           Q_ASSERT(!"unknown type");
           return true;
           break;
   }
}

bool RModel::isBinary(const QByteArray& data) const
{
    //return !isTextOnly(data, m_encoding, true);
    return true;
}

