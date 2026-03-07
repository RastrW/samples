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
#include "qastra.h"
#include "rdata.h"
#include "rtablesdatamanager.h"
#include "QDataBlocks.h"

QMap<int,int> RModel::parseEnpicNameref(const std::string& nameref) const
{
    QMap<int,int> result;
    QString s = QString::fromStdString(nameref).trimmed();

    // Если нет групп (нет ';') — просто плоский список иконок
    // "2,0,1,4,5" -> value=0 -> иконка 2, value=1 -> иконка 0, ...
    if (!s.contains(';'))
    {
        int fieldVal = 0;
        for (const QString& t : s.split(',', Qt::SkipEmptyParts)) {
            bool ok = false;
            int iconIdx = t.trimmed().toInt(&ok);
            if (ok)
                result[fieldVal++] = iconIdx;
        }
        return result;
    }

    // Разбиваем на группы по ';', убираем скобки
    QStringList groups;
    for (const QString& part : s.split(';', Qt::SkipEmptyParts))
        groups << QString(part).remove('(').remove(')').trimmed();

    auto parseGroup = [](const QString& g) -> QList<int> {
        QList<int> res;
        if (g.isEmpty()) return res;
        for (const QString& t : g.split(',', Qt::SkipEmptyParts)) {
            bool ok = false;
            int v = t.trimmed().toInt(&ok);
            if (ok) res << v;
        }
        return res;
    };

    // Группа 0: РисункиИстинаЛожь — [0]=иконка для value=1, [1]=иконка для value=0
    if (!groups.isEmpty()) {
        QList<int> g = parseGroup(groups[0]);
        if (g.size() >= 1) result[1] = g[0];
        if (g.size() >= 2) result[0] = g[1];
    }

    // Группа 1: РисункиИстина — значения начинаются с 2
    int nextVal = 2;
    if (groups.size() >= 2) {
        for (int idx : parseGroup(groups[1]))
            result[nextVal++] = idx;
    }

    // Группа 2: РисункиЛожь — значения продолжаются
    if (groups.size() >= 3) {
        for (int idx : parseGroup(groups[2]))
            result[nextVal++] = idx;
    }

    return result;
}

QPixmap RModel::iconByQtitanIndex(int idx) const
{
    QString path;

    switch (idx) {
    case 0: path = ":/images/grid/checkMark.png"; break;
    case 1: path = ":/images/grid/exclamationMark.png";     break;
    case 2: path = ":/images/grid/cross.png";      break;
    case 3: path = ":/images/grid/blueFlag.png";     break;
    case 4: path = ":/images/grid/leftBreaker.png"; break;
    case 5: path = ":/images/grid/rightBreaker.png";     break;
    case 6: path = ":/images/grid/redArrowLeft.png";      break;
    case 7: path = ":/images/grid/redArrowRight.png";     break;
    case 8: path = ":/images/grid/greenArrowLeft.png";      break;
    case 9: path = ":/images/grid/greenArrowRight.png";     break;
    }

    QPixmap px(path);

    if (px.isNull())
        qWarning() << "iconByQtitanIndex: pixmap is NULL for idx=" << idx;

    px = px.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    return px;
}

RModel::RModel(QObject *parent, QAstra* pqastra, RTablesDataManager* pRTDM)
    : QAbstractTableModel(parent)
    , pqastra_(pqastra)
    , pRTDM_(pRTDM)
{}

bool RModel::populateDataFromRastr(){

    try {
        rebuildStructure();
        reloadData();
        rebuildBackInfo();
    }
    catch (...) {
        qCritical() << "ERROR! populateDataFromRastr: "
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
    m_pictureEnums_.clear();

    for (RCol& rcol : *up_rdata)
    {
        rcol.setNameRef(rcol.getNameRef());
        const size_t idx = static_cast<size_t>(rcol.getIndex());
        // ── ENUM: "Нет|Квадр.|Лин.|Комбинир." ────────────────────────────
        if (rcol.getComPropTT() == enComPropTT::COM_PR_ENUM){
            QStringList list;
            for (const auto& val : split(rcol.getNameRef(), '|')) {
                list.append(QString::fromStdString(val));
            }
            m_enum_.emplace(idx, std::move(list));
        }
        // ── SUPERENUM: "ti_prv.Name.Num" ─────────────────────────────────
        if (rcol.getComPropTT() == enComPropTT::COM_PR_SUPERENUM && !rcol.getNameRef().empty()){
            std::vector<std::string> parts {split(rcol.getNameRef(), '.')};
            if (parts.size() > 2){
                long indx1 = pRTDM_->column_index(parts[0], parts[1]);
                long indx2 = pRTDM_->column_index(parts[0], parts[2]);
                if (indx1 > -1 && indx2 > -1)
                {
                    QDataBlock QDB;
                    pRTDM_->getDataBlock(parts[0], parts[2] + "," + parts[1], QDB);
                    std::map<size_t, std::string> map_string;
                    map_string.emplace(0, "не задано");
                    for (int i = 0; i < QDB.RowsCount(); ++i) {
                        long        key = std::visit(ToLong(),   QDB.Get(i, 0));
                        std::string val = std::visit(ToString(), QDB.Get(i, 1));
                        map_string.emplace(key, val);
                    }
                    mm_superenum_.emplace(idx, std::move(map_string));
                }
            }
        }
        // ── NAMEREF: "node[na]" ───────────────────────────────────────────
        if (rcol.getComPropTT() == enComPropTT::COM_PR_INT && !rcol.getNameRef().empty()){
                const auto& ref = rcol.getNameRef();
                size_t nopen  = ref.find('[');
                size_t nclose = ref.find(']');
                if (nopen == std::string::npos || nclose == std::string::npos)
                    continue;

                std::string table = ref.substr(0, nopen);
                std::string col   = ref.substr(nopen + 1, nclose - nopen - 1);

                long nameIndx = pRTDM_->column_index(table, "name");
                std::string cols = col + "," +
                                   (nameIndx > -1 ? std::string("name") : col);

                QDataBlock qdb;
                pRTDM_->getDataBlock(table, cols, qdb);

                std::map<size_t, std::string> map;
                map.emplace(0, "не задано");
                for (int i = 0; i < qdb.RowsCount(); ++i) {
                    size_t      key = static_cast<size_t>(
                        std::visit(ToLong(), qdb.Get(i, 0)));
                    std::string val = std::visit(ToString(), qdb.Get(i, 1));
                    map.emplace(key, std::move(val));
                }
                mm_nameref_.emplace(idx, std::move(map));
        }
        if (rcol.getComPropTT() == enComPropTT::COM_PR_ENPIC){
            QMap<int,int> iconMap = parseEnpicNameref(rcol.getNameRef());

            QList<PictureItem> items;
            if (!iconMap.isEmpty()) {
                int maxVal = iconMap.lastKey();
                for (int v = 0; v <= maxVal; ++v) {
                    QPixmap px = iconMap.contains(v) ? iconByQtitanIndex(iconMap[v]) : QPixmap();
                    items.append({ "", iconMap.value(v, -1), px });
                }
            }
            m_pictureEnums_.emplace(rcol.getIndex(), std::move(items));
        }
    }
}

QVariant RModel::data(const QModelIndex &index, int role) const
{
    int col = index.column();
    if (col < 0 || static_cast<size_t>(col) >= up_rdata->size()){
        spdlog::error("Выход за границы модели");
        return QVariant();
    }

    int row = index.row();
    QVariant item;
    std::string item_str;
    QVariant condFormatColor;

    RCol* prc = &(*( up_rdata->begin() + col ));
    const std::size_t pluginIdx = static_cast<std::size_t>(prc->getIndex());

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
            // Для ENPIC используем pluginIdx, для остальных — col
            if (contains(m_pictureEnums_, pluginIdx))
            {
                int value = item.toInt();
                const auto& list = m_pictureEnums_.at(pluginIdx);
                if (value >= 0 && value < list.size())
                    return list[value].label;
            }

            if (contains(m_enum_,col) )
                return m_enum_.at(col).at(item.toInt());

            if (contains(mm_superenum_,col))
                if (contains(mm_superenum_.at(col),item.toInt()))
                    return mm_superenum_.at(col).at(item.toInt()).c_str();
        }

        return item;
    }
    else if (role == Qt::DecorationRole)
    {
        if (contains(m_pictureEnums_, pluginIdx))
        {
            int value = std::visit(ToLong(), up_rdata->pnparray_->Get(row, col));
            const auto& list = m_pictureEnums_.at(pluginIdx);
            if (value >= 0 && value < list.size())
                return list[value].image;
        }
    }
    else if (role == Qtitan::ComboBoxRole){
        if (contains(m_pictureEnums_, pluginIdx))
        {
            QVariantList result;
            for (const auto& p : m_pictureEnums_.at(pluginIdx))
            {
                // qtn_qvariant_to_pixmap ожидает QPixmap
                result << p.image;
            }
            return result;
        }
        QStringList list;
        if (auto it = m_enum_.find(col); it != m_enum_.end())
            list = it->second;
        if (auto it = mm_nameref_.find(col); it != mm_nameref_.end())
            for (const auto& [k, v] : it->second)
                list.append(QString::fromStdString(v));
        if (auto it = mm_superenum_.find(col); it != mm_superenum_.end())
            for (const auto& [k, v] : it->second)
                list.append(QString::fromStdString(v));
        return list;
    }
    else if (role ==  Qt::ToolTipRole)
    {
        return QString("Row %1, Column %2")
        .arg(index.row() + 1)
            .arg(index.column() +1);
    }

    return QVariant();
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
    case enComPropTT::COM_PR_ENPIC:
    {
        // Ключ в m_pictureEnums_ — это plugin-индекс, не позиционный
        const std::size_t pluginIdx = static_cast<std::size_t>(col->getIndex());
        if (!col->isDirectCode() && m_pictureEnums_.count(pluginIdx)) {
            info.editorType = ColumnEditorInfo::Type::ComboBoxPicture;
            for (const auto& p : m_pictureEnums_.at(pluginIdx))
                info.picItems.append({ p.image, p.label });
        } else {
            info.editorType = ColumnEditorInfo::Type::Numeric;
        }
        break;
    }
    default:
        break;
    }
    return info;
}

int RModel::rowCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->pnparray_->RowsCount());
}

int RModel::columnCount(const QModelIndex & /*parent*/) const{
    return static_cast<int>(up_rdata->pnparray_->ColumnsCount());
}

QVariant RModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
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

bool RModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const int col = index.column();
    const int row = index.row();

    if (value == data(index, role))
        return false;

    RData::iterator iter_col = up_rdata->begin() + col;

    const std::string& tname   = iter_col->getTableName();
    const std::string& colName = iter_col->getColName();

    FieldVariantData vd;   // разрешённое значение для передачи в плагин

    switch (iter_col->getEnData()){
    case RCol::_en_data::DATA_BOOL:{
        vd = value.toBool();
        break;
    }
    case RCol::DATA_INT:{
        long val = 0;
        const size_t idx = static_cast<size_t>(col);

        if (!iter_col->isDirectCode())
        {
            auto itEnum = m_enum_.find(idx);
            if (itEnum != m_enum_.end()) {
                // ENUM: ищем позицию строки в списке
                int i = 0;
                for (const auto& s : itEnum->second) {
                    if (s == value) { val = i; break; }
                    ++i;
                }
            }
            else if (auto itSuper = mm_superenum_.find(idx);
                     itSuper != mm_superenum_.end())
            {
                // SUPERENUM: ищем ключ по строковому значению
                for (const auto& [k, v] : itSuper->second)
                    if (v == value.toString().toStdString()) { val = static_cast<long>(k); break; }
            }
            else if (auto itRef = mm_nameref_.find(idx);
                     itRef != mm_nameref_.end())
            {
                // NAMEREF: аналогично
                for (const auto& [k, v] : itRef->second)
                    if (v == value.toString().toStdString()) { val = static_cast<long>(k); break; }
            }
            else {
                val = value.toInt();
            }
        }
        else {
            val = value.toInt();
        }
        vd = val;
        break;
    }
    case RCol::_en_data::DATA_STR:{
        vd = value.toString().toStdString();
        break;
    }
    case RCol::_en_data::DATA_DBL:{
        vd = value.toDouble();
        break;
    }
    default:
        return false;
    }

    // Запись через RTDM — единственная точка доступа к плагину.
    // emit dataChanged НЕ вызывается: плагин сгенерирует ChangeData-хинт,
    // RTDM обновит QDataBlock и испустит sig_dataChanged → View обновится один раз.
    pRTDM_->setValue(tname, colName, row, vd);
    return true;
}

bool RModel::addRow(size_t count ,const QModelIndex &parent)
{
    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    for (size_t i = 0 ; i < count ; i++ ){
        IRastrResultVerify{ table->AddRow()};
    }

    return true;
}

bool RModel::insertRows(int row, int count, const QModelIndex &parent){

    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };
    for (size_t i = 0 ; i < count ; i++ ){
        IRastrResultVerify{table->InsertRow(row)};
    }

    return true;
}

bool RModel::duplicateRow(int row, const QModelIndex &parent){

    IRastrTablesPtr tablesx{ this->pqastra_->getRastr()->Tables() };
    IRastrTablePtr table{ tablesx->Item(getRdata()->t_name_) };

    IRastrResultVerify{table->DuplicateRow(row)};

    //Дублируем данные в клиенте (иначе дублированная строка будет пустой)
    up_rdata->pnparray_->DuplicateRow(row);

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

    for (size_t i = 0 ; i < count ; i++ ){
       IRastrResultVerify{table->DeleteRow(row)};
    }

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

static void addCondFormatToMap(std::map<size_t, std::vector<CondFormat>>& mCondFormats,
                               size_t column, const CondFormat& condFormat)
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

void RModel::addCondFormat(const bool isRowIdFormat, size_t column,
                           const CondFormat& condFormat)
{
    if(isRowIdFormat)
        addCondFormatToMap(m_mRowIdFormats, column, condFormat);
    else
        addCondFormatToMap(m_mCondFormats, column, condFormat);
    emit layoutChanged();
}

void RModel::setCondFormats(const bool isRowIdFormat, size_t column,
                            const std::vector<CondFormat>& condFormats)
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

QVariant RModel::getMatchingCondFormat
    (const std::map<size_t, std::vector<CondFormat>>& mCondFormats,
     size_t row,size_t column, const QString& value, int role) const
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
RModel::columnsWidth(){
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
