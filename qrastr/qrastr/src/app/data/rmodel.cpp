#include "rmodel.h"
#include <QBrush>
#include <spdlog/spdlog.h>
#include "QtitanGrid.h"

#include "rdata.h"
#include "QDataBlocks.h"
#include "qastra.h"
#include "rtablesdatamanager.h"
#include <string_bool.h>
#include "condFormat.h"

RModel::RModel(QObject* parent, QAstra* pqastra, RTablesDataManager* pRTDM)
    : QAbstractTableModel(parent)
    , m_qastra(pqastra)
    , m_rtdm(pRTDM)
{}

bool RModel::populateDataFromRastr()
{
    try {
        m_rdata = std::make_unique<RData>(m_qastra, *m_UIform);
        m_rdata->populate_qastra(m_qastra, m_rtdm);
        m_cache.rebuild(*m_rdata, m_rtdm);
        //структура могла смениться
        m_bgCache.clear();
    } catch (...) {
       spdlog::critical("ERROR! populateDataFromRastr: {}",
                    (m_rdata ? m_rdata->t_name_.c_str() : "<null>"));
            return false;
    }
    return true;
}

int RModel::rowCount   (const QModelIndex&) const { return static_cast<int>(m_rdata->pnparray_->RowsCount()); }
int RModel::columnCount(const QModelIndex&) const { return static_cast<int>(m_rdata->pnparray_->ColumnsCount()); }

QVariant RModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal){
        return m_rdata->at(section).getTitle().c_str();
    }
    return section + 1;
}

Qt::ItemFlags RModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    const RCol* col = getRCol(index.column());
    if (col && col->getFF() == "1")    // формула активна → только чтение
        return f | Qt::ItemIsSelectable;
    return f | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

QVariant RModel::data(const QModelIndex& index, int role) const
{
    const int col = index.column();
    const int row = index.row();

    if (col < 0 || static_cast<size_t>(col) >= m_rdata->size()) {
        spdlog::error("Выход за границы модели");
        return {};
    }

    const RCol&       rcol      = *(m_rdata->begin() + col);
    const size_t      pluginIdx = static_cast<size_t>(rcol.getIndex());

    // ── BackgroundRole (Фон ячейки) ──────────────────────────────────────────
    if (role == Qt::BackgroundRole) {
        if (rcol.getComPropTT() != enComPropTT::COM_PR_TIME) {
            // форматы не заданы
            if (m_condFmt.formats().empty()) return {};
            // Проверяем кеш
            if (const QVariant* cached = m_bgCache.get(row, col)){
                // QVariant() → нет формата, тоже ок
                return *cached;
            }
            const QString val = QString::fromStdString(
                std::visit(ToString(), m_rdata->pnparray_->Get(row, col)));
            QVariant fmt = getMatchingCondFormat(row, col, val, role);
            m_bgCache.put(row, col, fmt);
            return fmt;
        }else{
            long packed = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
            return QColor::fromRgb(static_cast<QRgb>(packed)); // QVariant с QColor
        }
    }

    // ── DisplayRole / EditRole (Текст для отображения/Значение для редактора)─
    if (role == Qt::DisplayRole || role == Qt::EditRole) {

        // TIME: секунды от эпохи плагина → QDateTime
        if (rcol.getComPropTT() == enComPropTT::COM_PR_TIME) {
            long secsLong = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
            std::string secsStr = std::visit(ToString(), m_rdata->pnparray_->Get(row, col));
            static const QDateTime epoch(QDate(1899, 12, 30), QTime(0, 0, 0));
            return epoch.addSecs(secsLong); // QVariant с QDateTime — QTitan сам форматирует
        }

        // COLOR: упакованный RGB long → QColor
        if (rcol.getComPropTT() == enComPropTT::COM_PR_COLOR) {
            //HTML строка со значением
            long packed = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
            std::string packedStr = std::visit(ToString(), m_rdata->pnparray_->Get(row, col));
            return QColor::fromRgb(static_cast<QRgb>(packed)); // QVariant с QColor
        }

        // Все остальные типы
        QVariant item = std::visit(ToQVariant(), m_rdata->pnparray_->Get(row, col));

        if (!rcol.isDirectCode()) {
            // Для ENPIC используем pluginIdx, для остальных — col
            if (const auto* pics = m_cache.pictureEnum(pluginIdx)) {
                int v = item.toInt();
                if (v >= 0 && v < pics->size()) return (*pics)[v].label;
            }
            if (const auto* enums = m_cache.enumItems(col))
                return enums->at(item.toInt());
            if (const auto* superenum = m_cache.superenumMap(col)) {
                auto it = superenum->find(static_cast<size_t>(item.toInt()));
                if (it != superenum->end()) return QString::fromStdString(it->second);
            }
        }

        if (item.type() == QVariant::Double) {
            const auto info = getColumnEditorInfo(col);
            if (info.editorType == ColumnEditorInfo::Type::Numeric)
                return QString::number(item.toDouble(), 'f', info.decimals);
        }
        return item;
    }

    // ── DecorationRole (Иконка слева от текста)───────────────────────────────
    if (role == Qt::DecorationRole) {
        // Для COLOR-колонок отображаем цветной прямоугольник
        if (rcol.getComPropTT() == enComPropTT::COM_PR_COLOR) {
            long packed = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
            QPixmap px(16, 16);
            px.fill(QColor::fromRgb(static_cast<QRgb>(packed)));
            return px;
        }
        if (const auto* pics = m_cache.pictureEnum(pluginIdx)) {
            int v = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
            if (v >= 0 && v < pics->size()) return (*pics)[v].image;
        }
        return {};
    }

    // ── ComboBoxRole ──────────────────────────────────────────────────────────
    if (role == Qtitan::ComboBoxRole) {
        if (const auto* pics = m_cache.pictureEnum(pluginIdx)) {
            QVariantList result;
            for (const auto& p : *pics) result << p.image;
            return result;
        }
        QStringList list;
        if (const auto* enums  = m_cache.enumItems(col))
            list = *enums;
        else if (const auto* nref = m_cache.namerefMap(col))
            for (const auto& [k, v] : *nref) list.append(QString::fromStdString(v));
        else if (const auto* senum = m_cache.superenumMap(col))
            for (const auto& [k, v] : *senum) list.append(QString::fromStdString(v));
        return list;
    }

    // ── ToolTipRole (Всплывающая подсказка)───────────────────────────────────
    if (role == Qt::ToolTipRole)
        return QString("[%1, %2]").arg(row + 1).arg(col + 1);

    return {};
}

bool RModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const int col = index.column();
    const int row = index.row();
    const long         prow     = row;   // plugin-индекс строки
    // читаем сырое значение из кеша
    QVariant raw = std::visit(ToQVariant(), m_rdata->pnparray_->Get(row, col));
    if (raw == value) return false;

    RData::iterator iter_col = m_rdata->begin() + col;
    const std::string& tname   = iter_col->getTableName();
    const std::string& colName = iter_col->getColName();

    FieldVariantData vd;
    // TIME: QDateTime → секунды от эпохи плагина
    if (iter_col->getComPropTT() == enComPropTT::COM_PR_TIME) {
        static const QDateTime epoch(QDate(1899, 12, 30), QTime(0, 0, 0));
        vd = static_cast<long>(epoch.secsTo(value.toDateTime()));
    }
    // COLOR: QColor → упакованный RGB
    else if (iter_col->getComPropTT() == enComPropTT::COM_PR_COLOR) {
        vd = static_cast<long>(value.value<QColor>().rgb());
    }
    else {
        switch (iter_col->getEnData()) {
        case RCol::_en_data::DATA_BOOL:
            vd = value.toBool();
            break;

        case RCol::_en_data::DATA_INT: {
            long val = 0;
            const size_t idx = static_cast<size_t>(col);
            if (!iter_col->isDirectCode()) {
                if (const auto* enums = m_cache.enumItems(idx)) {
                    for (int i = 0; i < enums->size(); ++i)
                        if ((*enums)[i] == value) { val = i; break; }
                } else if (const auto* senum = m_cache.superenumMap(idx)) {
                    for (const auto& [k, v] : *senum)
                        if (v == value.toString().toStdString()) { val = static_cast<long>(k); break; }
                } else if (const auto* nref = m_cache.namerefMap(idx)) {
                    for (const auto& [k, v] : *nref)
                        if (v == value.toString().toStdString()) { val = static_cast<long>(k); break; }
                } else {
                    val = value.toInt();
                }
            } else {
                val = value.toInt();
            }
            vd = val;
            break;
        }

        case RCol::_en_data::DATA_STR:
            vd = value.toString().toStdString();
            break;

        case RCol::_en_data::DATA_DBL:
            vd = value.toDouble();
            break;

        default:
            return false;
        }
    }
    // Запись через RTDM — единственная точка доступа к плагину.
    // emit dataChanged НЕ вызывается: плагин сгенерирует ChangeData-хинт,
    // RTDM обновит QDataBlock и испустит sig_dataChanged → View обновится один раз.
    m_rtdm->setValue(tname, colName, prow, vd);
    return true;
}

bool RModel::addRow(size_t count, const QModelIndex&)
{
    IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
    IRastrTablePtr  table  { tablesx->Item(getRdata()->t_name_) };
    for (size_t i = 0; i < count; ++i)
        IRastrResultVerify{ table->AddRow() };
    return true;
}

bool RModel::duplicateRow(int row, const QModelIndex&)
{
    IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
    IRastrTablePtr  table  { tablesx->Item(getRdata()->t_name_) };
    IRastrResultVerify{ table->DuplicateRow(row)};
    // После DuplicateRow плагин сгенерирует InsertRow-хинт →
    // handleInsertRow вставит пустую строку в pnparray_.
    // DuplicateRow в QDenseDataBlock копирует данные из исходной строки в новую
    m_rdata->pnparray_->DuplicateRow(row);
    return true;
}
bool RModel::insertRows(int row, int count, const QModelIndex&)
{
    IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
    IRastrTablePtr  table  { tablesx->Item(getRdata()->t_name_) };
    for (int i = 0; i < count; ++i)
        IRastrResultVerify{ table->InsertRow(row) };
    return true;
}

bool RModel::insertColumns(int column, int count, const QModelIndex& parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    ///@todo FIXME: Implement me!
    endInsertColumns();
    return true;
}

bool RModel::removeRows(int row, int count, const QModelIndex&)
{
    IRastrTablesPtr tablesx{ m_qastra->getRastr()->Tables() };
    IRastrTablePtr  table  { tablesx->Item(getRdata()->t_name_) };
    for (int i = 0; i < count; ++i)
        IRastrResultVerify{ table->DeleteRow(row) };
    return true;
}

bool RModel::removeColumns(int column, int count, const QModelIndex& parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    ///@todo FIXME: Implement me!
    endRemoveColumns();
    return true;
}

void RModel::slot_DataChanged(std::string tName, int rowFrom, int colFrom,
                              int rowTo, int colTo){
    if (getRdata()->t_name_ != tName) return;
    // инвалидируем затронутые строки
    m_bgCache.invalidateRows(rowFrom, rowTo);
    emit dataChanged(index(rowFrom, colFrom), index(rowTo, colTo));
}

void RModel::slot_BeginResetModel(std::string tName)
{
    if (getRdata()->t_name_ == tName) beginResetModel();
}

void RModel::slot_EndResetModel(std::string tName)
{
    if (getRdata()->t_name_ != tName) return;
    // структура пересоздаётся полностью
    m_bgCache.clear();
    ///@note при сборсе обязательно целиком пересоздавать data
    /// populateDataFromRastr() вызывается МЕЖДУ beginResetModel и endResetModel.
    /// В этот момент View не обращается к модели — безопасно полностью
    ///пересоздать RData.
    populateDataFromRastr();
    endResetModel();
}

void RModel::slot_BeginInsertRow(std::string tName, int first, int last)
{
    if (getRdata()->t_name_ == tName) beginInsertRows({}, first, last);
}

void RModel::slot_EndInsertRow(std::string tName)
{
    if (getRdata()->t_name_ == tName) endInsertRows();
}

void RModel::slot_BeginRemoveRows(std::string tName, int first, int last)
{
    if (getRdata()->t_name_ == tName) beginRemoveRows({}, first, last);
}

void RModel::slot_EndRemoveRows(std::string tName)
{
    if (getRdata()->t_name_ == tName) endRemoveRows();
}

void RModel::addCondFormat(size_t column, const CondFormat& condFormat)
{
    m_condFmt.add(column, condFormat);
    m_bgCache.clear();
    emit layoutChanged();
}

void RModel::setCondFormats(size_t column, const std::vector<CondFormat>& condFormats)
{
    m_condFmt.set(column, condFormats);
    m_bgCache.clear();
    // layoutChanged вызывается из контроллера после
    // полной замены всех правил, не инкрементально.
}

const std::vector<CondFormat>& RModel::getCondFormats(int column) const
{
    // Если для колонки нет правил, возвращаем ссылку на статический пустой вектор.
    // Это безопаснее, чем бросать исключение или возвращать указатель.
    static const std::vector<CondFormat> empty;
    const auto* vec = m_condFmt.column(static_cast<size_t>(column));
    return vec ? *vec : empty;
}

QVariant RModel::getMatchingCondFormat(size_t row, size_t column,
                                       const QString& value, int role) const
{
    const auto* vec = m_condFmt.column(column);
    if (!vec || vec->empty()) return {};

    bool isNumber;
    value.toDouble(&isNumber);
    if (!isNumber) return {};

    for (const CondFormat& fmt : *vec) {
        STRING_BOOL sb(value.toStdString() + " " + fmt.sqlCondition());
        for (const std::string& op : sb.Check()) {
            if (contains(m_rdata->mCols_, op)) {
                int cind = m_rdata->mCols_.at(op);
                double v = std::visit(ToDouble(), m_rdata->pnparray_->Get(row, cind));
                sb.replace(op, std::to_string(v));
            }
        }

        if (fmt.filter().isEmpty() || sb.res()) {
            switch (role) {
            case Qt::ForegroundRole:    return fmt.foregroundColor();
            case Qt::BackgroundRole:    return fmt.backgroundColor();
            case Qt::FontRole:          return fmt.font();
            case Qt::TextAlignmentRole: return static_cast<int>(fmt.alignmentFlag() | Qt::AlignVCenter);
            }
        }
    }
    return {};
}

RModel::ColumnEditorInfo RModel::getColumnEditorInfo(int colIndex) const
{
    ColumnEditorInfo info;
    const RCol* col = getRCol(colIndex);
    if (!col) return info;

    const size_t idx      = static_cast<size_t>(colIndex);
    const size_t pluginIdx = static_cast<size_t>(col->getIndex());
    const auto   propTT   = col->getComPropTT();

    switch (propTT) {
    case enComPropTT::COM_PR_BOOL:
        info.editorType = ColumnEditorInfo::Type::CheckBox;
        break;

    case enComPropTT::COM_PR_REAL:
        info.editorType = ColumnEditorInfo::Type::Numeric;
        info.decimals   = std::atoi(col->getPrec().c_str());
        break;
    case enComPropTT::COM_PR_TIME:
        info.editorType = ColumnEditorInfo::Type::DateTime;
        info.decimals   = std::atoi(col->getPrec().c_str());
        break;
    case enComPropTT::COM_PR_ENUM:
        if (!col->isDirectCode()) {
            if (const auto* enums = m_cache.enumItems(idx)) {
                info.editorType = ColumnEditorInfo::Type::ComboBox;
                info.comboItems = *enums;
                break;
            }
        }
        info.editorType = ColumnEditorInfo::Type::Numeric;
        break;

    case enComPropTT::COM_PR_INT:
        if (!col->isDirectCode() && !col->getNameRef().empty()) {
            if (const auto* nref = m_cache.namerefMap(idx)) {
                info.editorType = ColumnEditorInfo::Type::ComboBox;
                for (const auto& [k, v] : *nref)
                    info.comboItems.append(QString::fromStdString(v));
                break;
            }
        }
        info.editorType = ColumnEditorInfo::Type::Numeric;
        break;

    case enComPropTT::COM_PR_SUPERENUM:
        if (!col->isDirectCode() && !col->getNameRef().empty()) {
            if (const auto* senum = m_cache.superenumMap(idx)) {
                info.editorType = ColumnEditorInfo::Type::ComboBox;
                for (const auto& [k, v] : *senum)
                    info.comboItems.append(QString::fromStdString(v));
                break;
            }
        }
        info.editorType = ColumnEditorInfo::Type::Numeric;
        break;

    case enComPropTT::COM_PR_ENPIC:
        if (!col->isDirectCode()) {
            // Ключ в m_pictureEnums_ — это plugin-индекс, не позиционный
            if (const auto* pics = m_cache.pictureEnum(pluginIdx)) {
                info.editorType = ColumnEditorInfo::Type::ComboBoxPicture;
                for (const auto& p : *pics)
                    info.picItems.append({ p.image, p.label });
                break;
            }
        }
        info.editorType = ColumnEditorInfo::Type::Numeric;
        break;

    default:
        break;
    }
    return info;
}

std::vector<std::tuple<int, int>> RModel::columnsWidth() const
{
    std::vector<std::tuple<int, int>> cw;
    if (!m_rdata) return cw;
    int i = 0;
    for (const RCol& col : *m_rdata)
        cw.emplace_back(i++, std::stoi(col.getWidth()));
    return cw;
}

RCol* RModel::getRCol(int col) const
{
    return &(*(m_rdata->begin() + col));
}

int RModel::getIndexCol(std::string col) const
{
    for (int i = 0; i < columnCount(); ++i)
        if (getRCol(i)->getColName() == col) return i;
    return -1;
}

RData* RModel::getRdata()
{
    return m_rdata.get();
}
