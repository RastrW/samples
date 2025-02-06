#include "rmodel.h"
#include "CondFormat.h"
#include <QFont>
#include <QBrush>
#include <QTime>
#include <QDebug>
//#include <QJSEngine>
#include <QRegularExpression>
#include "QtitanGrid.h"
#include <string_bool.h>

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
    {
        vqcols_.push_back(rcol.title().c_str());

        if (rcol.com_prop_tt == enComPropTT::COM_PR_ENUM)
        {
            QStringList list;
            char delimiter = '|';
            //std::string delimiter = "|";
            std::string strItems =  rcol.NameRef();
            //if (strItems.find_first_of(',') != std::variant_npos)
            //   delimiter = ",";

            for (auto val : split(strItems,delimiter))
                list.append(QString(val.c_str()));
            mnamerefs_.insert(std::make_pair(rcol.index,list));
        }
    }
   // up_rdata->pnparray_->QDump(20,20);

    return 1;
};
int RModel::rowCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->pnparray_->RowsCount());
    //return static_cast<int>(pRTDM_->Add("node","cols")->RowsCount());
}
int RModel::columnCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->pnparray_->ColumnsCount());
}



QVariant RModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    QVariant item;
    std::string item_str;
    QVariant condFormatColor;

    if (role == Qt::BackgroundRole )
    {
            if (row == 1 && col == 2)  //change background only for cell(1,2)
                return QBrush(Qt::red);
            item_str = std::visit(ToString(),up_rdata->pnparray_->Get(row,col));
            condFormatColor = getMatchingCondFormat(row, col, item_str.c_str(), role);
            if (condFormatColor.isValid())
                return condFormatColor;
    }
    else if ( (role == Qt::DisplayRole ) ||
              (role == Qt::EditRole ))
    {
            item = std::visit(ToQVariant(),up_rdata->pnparray_->Get(row,col));
            if (contains(mnamerefs_,col))
                return mnamerefs_.at(col).at(item.toInt());
            return item;
    }
    else if  (role == Qtitan::ComboBoxRole)
    {
            QStringList list = mnamerefs_.at(col);
            return list;
    }
    else if (role ==  Qt::ToolTipRole)
    {
            return QString("Row %1, Column %2")
                .arg(index.row() + 1)
                .arg(index.column() +1);
    }
    else
            return QVariant();


    // Some old examples , code not reach hear
    /*switch (role) {
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
    */

    return QVariant();
}

QVariant RModel::headerData(int section, Qt::Orientation orientation, int role) const
{
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
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<val;
                break;
             }
            case RCol::_en_data::DATA_INT:
            {
                long val =  value.toInt();
                FieldVariantData vd(val);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<val;
                break;
            }
            case RCol::_en_data::DATA_STR:
            {
                std::string val =  value.toString().toStdString().c_str();
                FieldVariantData vd(val);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<val;
                break;
            }
                break;
            case RCol::_en_data::DATA_DBL:
            {
                double val =  value.toDouble();
                FieldVariantData vd(val);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).str_name_.c_str() << "(" << row << ")=" <<val;
            }
                break;
        default :
                break;
        }

        /*if (emitSignals())
        {
            emit dataChanged(getRdata()->t_name_,getRCol(col)->name(),row,value );
            return true;
        }*/
        return true;
    }
    return false;
}

static void addCondFormatToMap(std::map<size_t, std::vector<CondFormat>>& mCondFormats, size_t column, const CondFormat& condFormat)
{
    // If the condition is already present in the vector, update that entry and respect the order, since two entries with the same
    // condition do not make sense.
    auto it = std::find_if(mCondFormats[column].begin(), mCondFormats[column].end(), [condFormat](const CondFormat& format) {
        return format.sqlCondition() == condFormat.sqlCondition();
    });
    // Replace cond-format if present. push it back if it's a conditionless format (apply to every cell in column) or insert
    // as first element otherwise.
    if(it != mCondFormats[column].end()) {
        *it = condFormat;
    } else if (condFormat.filter().isEmpty())
        mCondFormats[column].push_back(condFormat);
    else
        mCondFormats[column].insert(mCondFormats[column].begin(), condFormat);
}

void RModel::addCondFormat(const bool isRowIdFormat, size_t column, const CondFormat& condFormat)
{
    if(isRowIdFormat)
        addCondFormatToMap(m_mRowIdFormats, column, condFormat);
    else
        addCondFormatToMap(m_mCondFormats, column, condFormat);
    emit layoutChanged();
}

void RModel::setCondFormats(const bool isRowIdFormat, size_t column, const std::vector<CondFormat>& condFormats)
{
    if(isRowIdFormat)
        m_mRowIdFormats[column] = condFormats;
    else if (!contains(m_mCondFormats, column))
    {
        m_mCondFormats.insert(make_pair(column,condFormats));
    }
    else
        m_mCondFormats[column] = condFormats;
   // emit layoutChanged();
}

QVariant RModel::getMatchingCondFormat(const std::map<size_t, std::vector<CondFormat>>& mCondFormats, size_t row,size_t column, const QString& value, int role) const
{
    if (!mCondFormats.count(column))
        return QVariant();

    bool isNumber;
    value.toDouble(&isNumber);
    std::string sql;

    if (!isNumber)
        return QVariant();

    // For each conditional format for this column,
    // if the condition matches the current data, return the associated format.
    for (const CondFormat& eachCondFormat : mCondFormats.at(column)) {
        if (isNumber && !contains(eachCondFormat.sqlCondition(), '\''))
            //sql = "SELECT " + value.toStdString() + " " + eachCondFormat.sqlCondition();
            sql = value.toStdString() + " " + eachCondFormat.sqlCondition();
        else
            //sql = "SELECT " + sqlb::escapeString(value.toStdString()) + " " + eachCondFormat.sqlCondition();
            //sql = "SELECT " + value.toStdString() + " " + eachCondFormat.sqlCondition();
            sql = value.toStdString() + " " + eachCondFormat.sqlCondition();

        // Empty filter means: apply format to any row.
        // Query the DB for the condition, waiting in case there is a loading in progress.
        //if (eachCondFormat.filter().isEmpty() || m_db.querySingleValueFromDb(sql, false, DBBrowserDB::Wait) == "1")
        /*
         * Похоже тут стоит попробовать написать парсер string to bool
        */
        STRING_BOOL SB(sql);
        std::vector<std::string> replace_vals = SB.Check();
        for (std::string &op : replace_vals )
        {
            if (contains(up_rdata->mCols_,op))
            {
                int cind = this->up_rdata->mCols_.at(op);
                //up_rdata->pnparray_->Get(row,cind);
                double ditem = std::visit(ToDouble(),up_rdata->pnparray_->Get(row,cind));
                std::string sitem = std::to_string(ditem);
                SB.replace(op,sitem);
            }
        }
        if (eachCondFormat.filter().isEmpty() || SB.res())
            switch (role) {
            case Qt::ForegroundRole:
                return eachCondFormat.foregroundColor();
            case Qt::BackgroundRole:
                return eachCondFormat.backgroundColor();
            case Qt::FontRole:
                return eachCondFormat.font();
            case Qt::TextAlignmentRole:
                return static_cast<int>(eachCondFormat.alignmentFlag() | Qt::AlignVCenter);
            }
    }
    return QVariant();
}

QVariant RModel::getMatchingCondFormat(size_t row, size_t column, const QString& value, int role) const
{
    QVariant format;
    // Check first for a row-id format and when there is none, for a conditional format.
    /*if (m_mRowIdFormats.count(column))
    {
        std::unique_lock<std::mutex> lock(m_mutexDataCache);
        const bool row_available = m_cache.count(row);
        const QByteArray blank_data("");
        const QByteArray& row_id_data = row_available ? m_cache.at(row).at(0) : blank_data;
        lock.unlock();

        format = getMatchingCondFormat(m_mRowIdFormats, column, row_id_data, role);
        if (format.isValid())
            return format;
    }*/
    if (m_mCondFormats.count(column))
        return getMatchingCondFormat(m_mCondFormats, row, column,value, role);
    else
        return QVariant();
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

bool RModel::AddRow(size_t count ,const QModelIndex &parent)
{
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    IRastrPayload sz{table->Size()};

    beginInsertRows(parent,sz.Value(),sz.Value() + count -1);
    for (size_t i = 0 ; i < count ; i++ )
        IPlainRastrResult* pres = table->AddRow();
    endInsertRows();

    return true;
}
bool RModel::insertRows(int row, int count, const QModelIndex &parent)
{
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    IRastrPayload sz{table->Size()};

    beginInsertRows(parent,sz.Value(),sz.Value() + count -1);
    for (size_t i = 0 ; i < count ; i++ )
        IPlainRastrResult* pres = table->InsertRow(row);
    endInsertRows();

    return true;
}
bool RModel::DuplicateRow(int row, const QModelIndex &parent)
{
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrPayload tablecount{ tablesx->Count() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    IRastrPayload sz{table->Size()};

    beginInsertRows(parent,sz.Value(),sz.Value());
    IPlainRastrResult* pres = table->DuplicateRow(row); // send EventHints::InsertRow
    endInsertRows();

    //Дублируем данные в клиенте
    this->up_rdata->pnparray_->DuplicateRow(row);


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
    IRastrPayload sz{table->Size()};

    //beginRemoveRows(parent,sz.Value(),sz.Value() + count -1);
    beginRemoveRows(parent,row,row + count -1);
    for (size_t i = 0 ; i < count ; i++ )
        IPlainRastrResult* pres = table->DeleteRow(row);
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

/*void RModel::onRModelchange(std::string _t_name, std::string _col_name, int row, QVariant value)
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
*/

void RModel::onrm_BeginResetModel(std::string _t_name)
{
    if (this->getRdata()->t_name_ == _t_name)
        beginResetModel();
}
void RModel::onrm_EndResetModel(std::string _t_name)
{
    if (this->getRdata()->t_name_ == _t_name)
        endResetModel();
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

