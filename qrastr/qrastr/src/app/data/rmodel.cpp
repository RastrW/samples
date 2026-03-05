#include "rmodel.h"
#include "CondFormat.h"
#include <QFont>
#include <QBrush>
#include <QTime>
#include <QDebug>
#include <QRegularExpression>
#include "QtitanGrid.h"
#include <string_bool.h>
#include <spdlog/spdlog.h>

RModel::RModel(QObject *parent, QAstra* pqastra, RTablesDataManager* pRTDM)
    : QAbstractTableModel(parent)
    , pqastra_(pqastra)
    , pRTDM_(pRTDM)
{
    setEmitSignals(true);
}

bool RModel::populateDataFromRastr(){

    try {
        rebuildStructure();
        reloadData();
        rebuildBackInfo();
    }
    catch (...) {
        qDebug() << "ERROR! populateDataFromRastr: "
                 << (up_rdata ? up_rdata->t_name_.c_str() : "<null>");
        return false;
    }
    return true;
};

void RModel::rebuildStructure()
{
    up_rdata = std::make_unique<RData>(pqastra_ ,*pUIForm_);
}

void RModel::reloadData()
{
    up_rdata->populate_qastra(this->pqastra_, pRTDM_);
}

void RModel::rebuildBackInfo()
{
    m_enum_.clear();
    mm_superenum_.clear();
    mm_nameref_.clear();

    for (RCol& rcol : *up_rdata)
    {
        rcol.setNameRef(rcol.getNameRef());

        if (rcol.getComPropTT() == enComPropTT::COM_PR_ENUM) // ex: Нет|Квадр.|Лин.|Комбинир.
        {
            QStringList list;
            int i = 0;
            for (const auto& val : split(rcol.getNameRef(), '|')) {
                list.append(QString::fromStdString(val));
            }
            m_enum_.emplace(rcol.getIndex(), std::move(list));
        }
        if (rcol.getComPropTT() == enComPropTT::COM_PR_SUPERENUM && !rcol.getNameRef().empty()) // ex: ti_prv.Name.Num
        {
            std::vector<std::string> parts {split(rcol.getNameRef(), '.')};
            if (parts.size() > 2)
            {
                QDataBlock QDB;
                long indx1 = pRTDM_->column_index(parts[0], parts[1]);
                long indx2 = pRTDM_->column_index(parts[0], parts[2]);
                if (indx1 > -1 && indx2 > -1)
                {
                    pRTDM_->getDataBlock(parts[0], parts[2] + "," + parts[1], QDB);
                    std::map<size_t, std::string> map_string;
                    map_string.emplace(0, "не задано");
                    for (int i = 0; i < QDB.RowsCount(); ++i) {
                        long        key = std::visit(ToLong(),   QDB.Get(i, 0));
                        std::string val = std::visit(ToString(), QDB.Get(i, 1));
                        map_string.emplace(key, val);
                    }
                    mm_superenum_.emplace(rcol.getIndex(), std::move(map_string));
                }
            }
        }
        if (rcol.getComPropTT() == enComPropTT::COM_PR_INT && !rcol.getNameRef().empty()) // ex: node[na]
        {
            size_t nopen  = rcol.getNameRef().find('[');
            size_t nclose = rcol.getNameRef().find(']');
            std::string table = rcol.getNameRef().substr(0, nopen);
            std::string col   = rcol.getNameRef().substr(nopen + 1, nclose - nopen - 1);

            long nameIndx = pRTDM_->column_index(table, "name");
            std::string cols = col + "," + (nameIndx > -1 ? std::string("name") : col);

            QDataBlock QDB;
            pRTDM_->getDataBlock(table, cols, QDB);

            std::map<size_t, std::string> map_string;
            map_string.emplace(0, "не задано");
            for (int i = 0; i < QDB.RowsCount(); ++i) {
                long        key = std::visit(ToLong(),   QDB.Get(i, 0));
                std::string val = std::visit(ToString(), QDB.Get(i, 1));
                map_string.emplace(key, val);
            }
            mm_nameref_.emplace(rcol.getIndex(), std::move(map_string));
        }
    }
}

int RModel::rowCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->pnparray_->RowsCount());
}

int RModel::columnCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->pnparray_->ColumnsCount());
}

QVariant RModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();
    if (col < 0 || static_cast<size_t>(col) >= up_rdata->size()){
        //spdlog::error("Выход за границы модели");
        return QVariant();
    }

    int row = index.row();
    QVariant item;
    std::string item_str;
    QVariant condFormatColor;

    RData::iterator iter_col = up_rdata->begin() + col;
    RCol* prc =  &(*iter_col);

    if (role == Qt::BackgroundRole )
    {
        ///@todo временная заливка ячейки (1,2) красным — удалить перед релизом
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

        if (!prc->isDirectCode())
        {
            if (contains(m_enum_,col) )
                //if (contains(m_enum_.at(col),item.toInt()) )  // win not compile
                    return m_enum_.at(col).at(item.toInt());

            if (contains(mm_superenum_,col))
                if (contains(mm_superenum_.at(col),item.toInt()))
                    return mm_superenum_.at(col).at(item.toInt()).c_str();
        }

        return item;
    }
    else if  (role == Qtitan::ComboBoxRole)
    {
        QStringList list;
        if (contains(m_enum_,col) )
            list = m_enum_.at(col);

        if (contains(mm_nameref_,col) )
            for (const auto &val : mm_nameref_.at(col))
                list.append(val.second.c_str());

        if (contains(mm_superenum_,col) )
            for (const auto &val : mm_superenum_.at(col))
                list.append(val.second.c_str());

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

    return QVariant();
}

QVariant RModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        std::string title = up_rdata.get()->at(section).getTitle().c_str();
        return up_rdata.get()->at(section).getTitle().c_str();
    }
    if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        return section + 1;
    }
    return QVariant();
}

Qt::ItemFlags
RModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | Qt::ItemIsSelectable |  QAbstractTableModel::flags(index) ;
}

bool RModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int col = index.column();
    int row = index.row();

    RData::iterator iter_col = up_rdata->begin() + col;

    IRastrTablesPtr tablesx{this->pqastra_->getRastr()->Tables()};
    IRastrTablePtr table{ tablesx->Item(iter_col->getTableName()) };
    IRastrColumnsPtr columns{table->Columns()};
    IRastrColumnPtr col_ptr{ columns->Item(iter_col->getColName()) };

    if (value != data(index, role))
    {
        switch((*iter_col).getEnData()){
            case RCol::_en_data::DATA_BOOL:
            {
                bool val =  value.toBool();
                FieldVariantData vd(val);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."
                         << (*iter_col).getColName().c_str() << "(" << row << ")=" <<val;
                break;
             }
            case RCol::_en_data::DATA_INT:
            {
                long val = 0;
                if (!(*iter_col).isDirectCode())
                {
                    if (contains(m_enum_,col))       // ENUM
                    {
                        int i =0;
                        for (const auto &mval : m_enum_.at(col))
                        {
                            if (mval == value)
                                val = i;
                            i++;
                        }
                    }
                    else if (contains(mm_superenum_,col))     // SUPER_ENUM
                    {
                        for (const auto &[mkey,mval] : mm_superenum_.at(col) )
                            if (mval == value.toString().toStdString())
                                val = mkey;
                    }
                    else if (contains(mm_nameref_,col))     // RefCol: node[na]
                    {
                        for (const auto &[mkey,mval] : mm_nameref_.at(col) )
                            if (mval == value.toString().toStdString())
                                val = mkey;
                    }
                    else
                        val =  value.toInt();
                }
                else
                    val =  value.toInt();

                FieldVariantData vd(val);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).getColName().c_str() << "(" << row << ")=" <<val;

                break;
            }
            case RCol::_en_data::DATA_STR:
            {
                std::string val =  value.toString().toStdString().c_str();
                FieldVariantData vd(val);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).getColName().c_str() ;//<< "(" << row << ")=" <<  val;
                break;
            }
                break;
            case RCol::_en_data::DATA_DBL:
            {
                double val =  value.toDouble();
                FieldVariantData vd(val);
                IRastrResultVerify(col_ptr->SetValue(row,val));
                qDebug() << "set: "<<up_rdata->t_name_.c_str()<<"."<< (*iter_col).getColName().c_str() << "(" << row << ")=" <<val;
            }
                break;
        default :
                break;
        }

        if (emitSignals())
        {
            emit dataChanged(index,index );
            //сигнал используется при реструктуризации модели (удаление/вставка строк), не при изменении данных.
            //emit changePersistentIndex(index,index);
        }
        return true;
    }
    return false;
}

bool RModel::AddRow(size_t count ,const QModelIndex &parent)
{
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    for (size_t i = 0 ; i < count ; i++ ){
        IRastrResultVerify{ table->AddRow()};
    }

    return true;
}

bool RModel::insertRows(int row, int count, const QModelIndex &parent)
{
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    for (size_t i = 0 ; i < count ; i++ ){
        IRastrResultVerify{table->InsertRow(row)};
    }

    return true;
}

bool RModel::DuplicateRow(int row, const QModelIndex &parent)
{
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };

    IRastrResultVerify{table->DuplicateRow(row)};

    //Дублируем данные в клиенте
    this->up_rdata->pnparray_->DuplicateRow(row);

    return true;
}

bool RModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    ///@todo FIXME: Implement me!
    endInsertColumns();
    return true;
}

bool RModel::removeRows(int row, int count, const QModelIndex &parent)
{
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };

    beginRemoveRows(parent,row,row + count -1);
    for (size_t i = 0 ; i < count ; i++ ){
       IRastrResultVerify{table->DeleteRow(row)};
    }
    endRemoveRows();

    return true;
}

bool RModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    ///@todo FIXME: Implement me!
    endRemoveColumns();
    return true;
}

void RModel::slot_DataChanged(std::string _t_name, int row_from,int col_from ,int row_to,int col_to)
{
    if (getRdata()->t_name_ != _t_name) return;
    QModelIndex top_left     = index(row_from, col_from);
    QModelIndex bottom_right = index(row_to,   col_to);
    emit dataChanged(top_left, bottom_right);
}

void RModel::slot_BeginResetModel(std::string _t_name)
{
    if (getRdata()->t_name_ == _t_name){
        beginResetModel();
    }
}

void RModel::slot_EndResetModel(std::string _t_name)
{
    if (this->getRdata()->t_name_ == _t_name){
        ///@note при сборсе обязательно целиком пересоздавать data
        /// populateDataFromRastr() вызывается МЕЖДУ beginResetModel и endResetModel.
        /// В этот момент View не обращается к модели — безопасно полностью
        ///пересоздать RData.
        populateDataFromRastr();
        endResetModel();
    }
}

void RModel::slot_BeginInsertRow(std::string _t_name,int first, int last){
    if (getRdata()->t_name_ == _t_name){
        beginInsertRows(QModelIndex(), first, last);
    }
}

void RModel::slot_EndInsertRow(std::string _t_name){

    if (this->getRdata()->t_name_ == _t_name){
        endInsertRows();
    }
}

void RModel::slot_BeginRemoveRows(std::string tname, int first, int last){
    if (getRdata()->t_name_ == tname){
        beginRemoveRows(QModelIndex(), first, last);
    }
}

void RModel::slot_EndRemoveRows(std::string tname){
    if (getRdata()->t_name_ == tname){
        endRemoveRows();
    }
}

RModel::ColumnEditorInfo
RModel::getColumnEditorInfo(int colIndex) const
{
    ColumnEditorInfo info;
    const RCol* col = getRCol(colIndex);
    if (!col) return info;

    const auto propTT = col->getComPropTT();

    switch (propTT)
    {
    case enComPropTT::COM_PR_BOOL:
        info.editorType = ColumnEditorInfo::Type::CheckBox;
        break;

    case enComPropTT::COM_PR_REAL:
        info.editorType = ColumnEditorInfo::Type::Numeric;
        info.decimals   = std::atoi(col->getPrec().c_str());
        break;

    case enComPropTT::COM_PR_ENUM:
        if (!col->isDirectCode() && m_enum_.count(colIndex)) {
            info.editorType = ColumnEditorInfo::Type::ComboBox;
            info.comboItems = m_enum_.at(colIndex);
        } else {
            info.editorType = ColumnEditorInfo::Type::Numeric;
        }
        break;

    case enComPropTT::COM_PR_INT:
        if (!col->isDirectCode()
            && !col->getNameRef().empty()
            && mm_nameref_.count(colIndex))
        {
            info.editorType = ColumnEditorInfo::Type::ComboBox;
            for (const auto& [k, v] : mm_nameref_.at(colIndex))
                info.comboItems.append(QString::fromStdString(v));
        } else {
            info.editorType = ColumnEditorInfo::Type::Numeric;
        }
        break;

    case enComPropTT::COM_PR_SUPERENUM:
        if (!col->isDirectCode()
            && !col->getNameRef().empty()
            && mm_superenum_.count(colIndex))
        {
            info.editorType = ColumnEditorInfo::Type::ComboBox;
            for (const auto& [k, v] : mm_superenum_.at(colIndex))
                info.comboItems.append(QString::fromStdString(v));
        } else {
            info.editorType = ColumnEditorInfo::Type::Numeric;
        }
        break;

    default:
        break;
    }
    return info;
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
    if(isRowIdFormat){
        m_mRowIdFormats[column] = condFormats;
    }
    else if (!contains(m_mCondFormats, column)){
        m_mCondFormats.insert(make_pair(column,condFormats));
    }
    else{
        m_mCondFormats[column] = condFormats;
    }
}

QVariant RModel::getMatchingCondFormat(const std::map<size_t, std::vector<CondFormat>>& mCondFormats, size_t row,size_t column, const QString& value, int role) const
{
    if (!mCondFormats.count(column))
        return QVariant();

    bool isNumber;
    value.toDouble(&isNumber);

    if (!isNumber)
        return QVariant();

    // For each conditional format for this column,
    // if the condition matches the current data, return the associated format.
    for (const CondFormat& eachCondFormat : mCondFormats.at(column)) {
        std::string sql = value.toStdString() + " " + eachCondFormat.sqlCondition();

        // Empty filter means: apply format to any row.
        // Query the DB for the condition, waiting in case there is a loading in progress.
        /*
         * Похоже тут стоит попробовать написать парсер string to bool
        */
        STRING_BOOL SB(sql);
        std::vector<std::string> replace_vals = SB.Check();
        for (std::string &op : replace_vals ){
            if (contains(up_rdata->mCols_,op))
            {
                int cind = this->up_rdata->mCols_.at(op);
                //up_rdata->pnparray_->Get(row,cind);
                double ditem = std::visit(ToDouble(),up_rdata->pnparray_->Get(row,cind));
                SB.replace(op,std::to_string(ditem));
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
    if (!m_mCondFormats.count(column))
        return QVariant();
    return getMatchingCondFormat(m_mCondFormats, row, column, value, role);
}

std::vector<std::tuple<int,int>>
RModel::ColumnsWidth(){
    std::vector<std::tuple<int,int>> cw;
    if (!up_rdata) return cw;
    int i = 0;
    for (RCol& col : *up_rdata)
        cw.emplace_back(i++, std::stoi(col.getWidth()));
    return cw;
}

RCol* RModel::getRCol(int col) const{
    return &(*(up_rdata->begin() + col));
}

int RModel::getIndexCol(std::string _col)
{
    for (int i = 0 ; i< columnCount(); i++){
        if (getRCol(i)->getColName() == _col){
            return i;
        }
    }
    return -1;
}

RData* RModel::getRdata()
{
    return up_rdata.get();
}
