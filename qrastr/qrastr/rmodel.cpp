#include "rmodel.h"
#include <QFont>
#include <QBrush>
#include <QTime>
#include <QDebug>

//#include "fmt/format.h"

RModel::RModel(QObject *parent, QAstra* pqastra, RTablesDataManager* pRTDM)
    : QAbstractTableModel(parent)
    , pqastra_(pqastra)
    , pRTDM_(pRTDM)
{
    setEmitSignals(true);
}

int RModel::populateDataFromRastr(){

    up_rdata = std::unique_ptr<RData>(new RData(pqastra_,pUIForm_->TableName()));
    up_rdata->Initialize(*pUIForm_,pqastra_);
    up_rdata->populate_qastra(this->pqastra_,pRTDM_);

    for (RCol &rcol : *up_rdata)
        vqcols_.push_back(rcol.title().c_str());

    return 1;
};
int RModel::rowCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->pnparray_->RowsCount());
    //return static_cast<int>(pRTDM_->Add("node","cols")->RowsCount());
}
int RModel::columnCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->pnparray_->ColumnsCount());
}

struct ToQVariant {
    QVariant operator()(std::monostate) { return { QVariant() }; }
    QVariant operator()(const long& value) { return (qlonglong)value; }
    QVariant operator()(const uint64_t& value) { return value; }
    QVariant operator()(const double& value) { return value; }
    QVariant operator()(const bool& value) { return value; }
    QVariant operator()(const std::string& value) { return std::string(value).c_str(); }

};

QVariant RModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    QVariant item;
    //RData::const_iterator iter_col = up_rdata->begin() + col;
    //_col_data::const_iterator iter_data = (*iter_col).begin() + row;
    // auto datablock_item = up_rdata->nparray_.Data()[row * up_rdata->nparray_.ColumnsCount() + col];
    //auto datablock_item = up_rdata->nparray_.Get(row,col);

    switch (role) {
        case Qt::BackgroundRole:
            if (row == 1 && col == 2)  //change background only for cell(1,2)
                return QBrush(Qt::red);
        /*case Qt::CheckStateRole:
            if (row == 1 && col == 0) //add a checkbox to cell(1,0)
                return Qt::Checked;*/
        //case Qt::CheckStateRole:
        case Qt::DisplayRole:
        case Qt::EditRole:

            // Fill from RData
            /*
             switch((*iter_data).index()){
            //case RCol::_en_data::DATA_BOOL: item =  std::get<bool>(*iter_data)?Qt::Checked: Qt::Unchecked; break;
                case RCol::_en_data::DATA_BOOL: item =  std::get<bool>(*iter_data); break;
                case RCol::_en_data::DATA_INT: item =  (qlonglong)std::get<long>(*iter_data) ;                 break;
                case RCol::_en_data::DATA_STR: item =  std::get<std::string>(*iter_data).c_str() ; break;
                case RCol::_en_data::DATA_DBL: item =  std::get<double>(*iter_data);               break;
            default :                      item =  ( "type_unknown" );                         break;

                (*iter_col).emplace<bool>(std::get<bool>(up_rdata->nparray_.Data()[row * up_rdata->nparray_.Columns() + col]));
              }
           */

            //Fill from QAstra->DataBlock
            /*switch( iter_col->en_data_){
                case RCol::_en_data::DATA_BOOL: item =  std::get<bool>(datablock_item); break;
                case RCol::_en_data::DATA_INT: item =  (qlonglong)std::get<long>(datablock_item) ;                 break;
                case RCol::_en_data::DATA_STR: item =  std::get<std::string>(datablock_item).c_str(); break;
                case RCol::_en_data::DATA_DBL: item =  std::get<double>(datablock_item);               break;
            default :                      item =  ( "type_unknown" );                         break;
            */

            //Fill from QAstra->DataBlock new
            item = std::visit(ToQVariant(),up_rdata->pnparray_->Get(row,col));

        return item;

        case Qt::ToolTipRole:
            return QString("Row %1, Column %2")
                .arg(index.row() + 1)
                .arg(index.column() +1);

        default:
            return QVariant();
    }

    // Some old examples , code not reach hear
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
    return Qt::ItemIsEditable | Qt::ItemIsEditable |  QAbstractTableModel::flags(index) ;
}


bool RModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int col = index.column();
    int row = index.row();

    RData::iterator iter_col = up_rdata->begin() + col;

    IRastrTablesPtr tablesx{this->pqastra_->getRastr()->Tables()};
    IRastrTablePtr table{ tablesx->Item(iter_col->table_name_) };
    IRastrColumnsPtr columns{table->Columns()};
    IRastrColumnPtr col_ptr{ columns->Item(iter_col->str_name_) };

    if (data(index, role) != value)
    {
        switch((*iter_col).en_data_){
            case RCol::_en_data::DATA_BOOL:
            {
                bool val =  value.toBool();
                FieldVariantData vd(val);
                //up_rdata->nparray_.EmplaceSaveIndChange(row,col,vd);
                //up_rdata->nparray_.Set(row,col,vd);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<val;
                break;
             }
            case RCol::_en_data::DATA_INT:
            {
                long val =  value.toInt();
                FieldVariantData vd(val);
                //up_rdata->nparray_.EmplaceSaveIndChange(row,col,vd);
                //up_rdata->nparray_.Set(row,col,vd);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<val;
                break;
            }
            case RCol::_en_data::DATA_STR:
            {
                std::string val =  value.toString().toStdString().c_str();
                FieldVariantData vd(val);
                //up_rdata->nparray_.EmplaceSaveIndChange(row,col,vd);
                //up_rdata->nparray_.Set(row,col,vd);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<val;
                break;
            }
                break;
            case RCol::_en_data::DATA_DBL:
            {
                double val =  value.toDouble();
                FieldVariantData vd(val);
                //up_rdata->nparray_.EmplaceSaveIndChange(row,col,vd);
                //up_rdata->nparray_.Set(row,col,vd);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<val;
            }
                break;
        default :
                break;
        }

        if (emitSignals())
        {
            emit dataChanged(getRdata()->t_name_,getRCol(col)->name(),row,value );
            return true;
        }
        return false;
    }

    // Old version
    /*
    _col_data::iterator iter_data = (*iter_col).begin() + row;
    if (data(index, role) != value)
    {
        switch((*iter_data).index()){
        case RCol::_en_data::DATA_INT:
            qDebug() << "int: " << value.toInt();
            (*iter_data).emplace<long> (value.toInt());
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
    */

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
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    IPlainRastrResult* pres = table->InsertRow(row);

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
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    IPlainRastrResult* pres = table->DeleteRow(row);

    return true;
}

bool RModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}

void RModel::onRModelchange(std::string _t_name, std::string _col_name, int row, QVariant value)
{
    if (_t_name != this->getRdata()->t_name_)
        return;

    int col = this->getIndexCol(_col_name);
    if (col < 0)
        return;

    RData::iterator iter_col = up_rdata->begin() + col;
    switch((*iter_col).en_data_){
        case RCol::_en_data::DATA_BOOL:
        {
            bool val =  value.toBool();
            FieldVariantData vd(val);
            //up_rdata->nparray_.EmplaceSaveIndChange(row,col,vd);
            up_rdata->pnparray_->Set(row,col,vd);
            break;
        }
        case RCol::_en_data::DATA_INT:
        {
            long val =  value.toInt();
            FieldVariantData vd(val);
            up_rdata->pnparray_->Set(row,col,vd);
            break;
        }
        case RCol::_en_data::DATA_STR:
        {
            std::string val =  value.toString().toStdString().c_str();
            FieldVariantData vd(val);
            up_rdata->pnparray_->Set(row,col,vd);
            break;
        }
        break;
        case RCol::_en_data::DATA_DBL:
        {
            double val =  value.toDouble();
            FieldVariantData vd(val);
            up_rdata->pnparray_->Set(row,col,vd);
        }
        break;
        default :
            break;
    }
}

void RModel::onrm_RowInserted(std::string _t_name, int _row)
{
    // видимо проще всего получить новый DataBlock

    /*
     * Плоская dll
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
    */
}
void RModel::onrm_RowDeleted(std::string _t_name, int _row)
{
    /*for( RCol& col : *this->getRdata() )
    {
        col.erase(col.begin()+_row);
    }
    */
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
       case RCol::_en_data::DATA_BOOL:
           return false;
           break;
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

