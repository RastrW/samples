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
    Qt::ItemFlags f = QAbstractTableModel::flags(index); // уже содержит Enabled|Selectable
    const RCol* col = getRCol(index.column());
    if (col && col->getFF() == "1")
        return f & ~Qt::ItemIsEditable; // убираем только редактирование, Enabled оставляем
    return f | Qt::ItemIsEditable;
}

QVariant RModel::data(const QModelIndex& index, int role) const
{
    const int col = index.column();
    const int row = index.row();

    if (col < 0 || static_cast<size_t>(col) >= m_rdata->size()) {
        spdlog::error("Выход за границы модели");
        return {};
    }

    const RCol& rcol = *(m_rdata->begin() + col);
    const QVariant raw = std::visit(ToQVariant(), m_rdata->pnparray_->Get(row, col));

    switch (role) {
    case Qtitan::ComboBoxRole:
        return dataForComboBox(rcol);
    case Qt::DisplayRole:
    case Qt::EditRole:
        return dataForDisplayEdit(row, col, rcol, raw, role);
    case Qt::UserRole:
        // Сырое значение для сортировки числовых колонок.
        // QTitan получает double и сортирует числово, а не лексикографически.
        return raw;
    case Qt::BackgroundRole:
        return dataForBackground(row, col, rcol, raw);
    case Qt::DecorationRole:
        return dataForDecoration(row, col, rcol, raw);
    // ── ToolTipRole (Всплывающая подсказка)───────────────────────────────────
    case Qt::ToolTipRole:
        return QString("[%1, %2]").arg(row + 1).arg(col + 1);
    default:
        return {};
    }
}

QVariant RModel::dataForInvalidCellEditRole(int col, const RCol& rcol) const
{
    if (rcol.getComPropTT() == enComPropTT::COM_PR_REAL) {
        return QVariant(static_cast<double>(0.0));
    }
    if (rcol.getComPropTT() == enComPropTT::COM_PR_INT  ||
        rcol.getComPropTT() == enComPropTT::COM_PR_ENUM ||
        rcol.getComPropTT() == enComPropTT::COM_PR_SUPERENUM) {
        return QVariant(0);
    }
    if (rcol.getComPropTT() == enComPropTT::COM_PR_TIME)
        return QDateTime();
    return QVariant(QString());
}

QVariant RModel::dataForDisplayEdit(int row, int col, const RCol& rcol,
                                    const QVariant& raw, int role) const{
    if (!raw.isValid() && role == Qt::EditRole){
        return dataForInvalidCellEditRole(col, rcol);
    }
    // TIME: секунды от эпохи плагина → QDateTime
    if (rcol.getComPropTT() == enComPropTT::COM_PR_TIME) {
        QString qstr = QString::fromStdString(
            std::visit(ToString(), m_rdata->pnparray_->Get(row, col)));
        // Отрезаем суффикс "--7199", если он есть
        int dashPos = qstr.indexOf("--");
        if (dashPos >= 0) qstr = qstr.left(dashPos);
        // QTitan GridEditor::DateTime ждёт именно QDateTime
        return QDateTime::fromString(qstr.trimmed(), "dd.MM.yyyy HH:mm:ss");
    }
    // COLOR: упакованный RGB long → QColor
    if (rcol.getComPropTT() == enComPropTT::COM_PR_COLOR) {
        long packed = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
        return QColor(packed & 0xFF, (packed >> 8) & 0xFF, (packed >> 16) & 0xFF);
    }

    if (role == Qt::DisplayRole && !rcol.isDirectCode()) {
        if (auto resolved = resolveDisplayText(col, rcol, raw); !resolved.isNull())
            return resolved;
    }

    // ── Вещественные числа ───────────────────────────────────────────────────
    // DisplayRole И EditRole → одна и та же отформатированная строка.
    // Пользователь видит "3.14" в ячейке и то же самое видит в редакторе.
    // Сортировка идёт по Qt::UserRole (raw double)
    if (raw.type() == QVariant::Double) {
        const auto info = getColumnEditorInfo(col);
        if (info.editorType == ColumnEditorInfo::Type::Numeric)
            return QString::number(raw.toDouble(), 'f', info.decimals);
    }
    return raw;
}

QVariant RModel::dataForBackground (int row, int col,
                                   const RCol& rcol, const QVariant& raw) const{
    // COLOR-ячейки: фон = сам цвет, условные форматы не нужны
    if (rcol.getComPropTT() == enComPropTT::COM_PR_COLOR) {
        long packed = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
        // COLORREF хранит каналы в порядке **BGR** (`0x00BBGGRR`), а `QColor::fromRgb` ожидает **RGB**.
        return QColor(packed & 0xFF, (packed >> 8) & 0xFF, (packed >> 16) & 0xFF);
    }
    // Для всех остальных — условный формат
    if (m_condFmt.formats().empty())
        return {};// форматы не заданы

    if (const QVariant* cached = m_bgCache.get(row, col))
        return *cached;// QVariant() → нет формата, тоже ок

    const QString val = QString::fromStdString(
        std::visit(ToString(), m_rdata->pnparray_->Get(row, col)));
    QVariant fmt = getMatchingCondFormat(row, col, val, Qt::BackgroundRole);
    m_bgCache.put(row, col, fmt);
    return fmt;
}

QVariant RModel::dataForDecoration (int row, int col,
                                   const RCol& rcol, const QVariant& raw) const{
    // COLOR: упакованный RGB long → QColor
    if (rcol.getComPropTT() == enComPropTT::COM_PR_COLOR) {
        long packed = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
        QPixmap px(16, 16);
        px.fill(QColor::fromRgb(static_cast<QRgb>(packed)));
        return px;
    }
    const size_t pluginIdx = static_cast<size_t>(rcol.getIndex());
    if (const auto* pics = m_cache.pictureEnum(pluginIdx)) {
        int v = std::visit(ToLong(), m_rdata->pnparray_->Get(row, col));
        if (v >= 0 && v < pics->size()) return (*pics)[v].image;
    }
    return {};
}

QVariant RModel::dataForComboBox(const RCol& rcol) const
{
    const size_t pluginIdx = static_cast<size_t>(rcol.getIndex());

    if (const auto* pics = m_cache.pictureEnum(pluginIdx)) {
        QVariantList result;
        result.reserve(pics->size());
        for (const auto& p : *pics) result << p.image;
        return result;
    }

    QStringList list;
    if (const auto* enums = m_cache.enumItems(pluginIdx))
        list = *enums;
    else if (const auto* nref = m_cache.namerefMap(pluginIdx))
        for (const auto& [k, v] : *nref)
            list.append(QString::fromStdString(v));
    else if (const auto* senum = m_cache.superenumMap(pluginIdx))
        for (const auto& [k, v] : *senum)
            list.append(QString::fromStdString(v));
    return list;
}

QString RModel::resolveDisplayText(int col, const RCol& rcol,
                                   const QVariant& raw) const
{
    const size_t pluginIdx = static_cast<size_t>(rcol.getIndex()); // единый ключ

    if (const auto* pics = m_cache.pictureEnum(pluginIdx)) {
        int v = raw.toInt();
        if (v >= 0 && v < pics->size()) return (*pics)[v].label;
        return {};
    }
    if (const auto* enums = m_cache.enumItems(pluginIdx)) {
        int v = raw.toInt();
        if (v >= 0 && v < enums->size()) return (*enums)[v];
        return {};
    }
    if (const auto* senum = m_cache.superenumMap(pluginIdx)) {
        auto it = senum->find(static_cast<size_t>(raw.toInt()));
        if (it != senum->end()) return QString::fromStdString(it->second);
        return {};
    }
    //nameref здесь не нужен
    return {};
}

bool RModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const int col = index.column();
    const int row = index.row();

    // --- ДИАГНОСТИКА ---
    spdlog::debug("setData [{},{}] role={} value='{}' type={}",
                  row, col, role,
                  value.toString().toStdString(),
                  value.typeName());  // покажет "QString", "int", "double"...
    // -------------------
    // читаем сырое значение из кеша
    QVariant raw = std::visit(ToQVariant(), m_rdata->pnparray_->Get(row, col));
    if (raw == value) return false;

    RData::iterator iter_col = m_rdata->begin() + col;
    const std::string& tname   = iter_col->getTableName();
    const std::string& colName = iter_col->getColName();

    FieldVariantData vd;
    // TIME: QDateTime → секунды от эпохи плагина
    if (iter_col->getComPropTT() == enComPropTT::COM_PR_TIME) {
        QDateTime dt = value.toDateTime();
        if (!dt.isValid()) return false;
        // Пишем в том же формате, что плагин ожидает на входе
        vd = dt.toString("dd.MM.yyyy HH:mm:ss--7199").toStdString();
    }
    // COLOR: QColor → упакованный RGB
    else if (iter_col->getComPropTT() == enComPropTT::COM_PR_COLOR) {
        QColor c = value.value<QColor>();
        // Собираем обратно в COLORREF (BGR)
        long colorref = static_cast<long>(c.red())
                        | (static_cast<long>(c.green()) << 8)
                        | (static_cast<long>(c.blue())  << 16);
        vd = colorref;
    }
    else {
        switch (iter_col->getEnData()) {
        case RCol::_en_data::DATA_BOOL:
            vd = value.toBool();
            break;

        case RCol::_en_data::DATA_INT: {
            long val = 0;
            const size_t pluginIdx = static_cast<size_t>(iter_col->getIndex());
            if (!iter_col->isDirectCode()) {
                if (const auto* enums = m_cache.enumItems(pluginIdx)) {
                    for (int i = 0; i < enums->size(); ++i)
                        if ((*enums)[i] == value) { val = i; break; }
                } else if (const auto* senum = m_cache.superenumMap(pluginIdx)) {
                    for (const auto& [k, v] : *senum)
                        if (v == value.toString().toStdString()) { val = static_cast<long>(k); break; }
                } else if (const auto* nref = m_cache.namerefMap(pluginIdx)) {
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
    m_rtdm->setValue(tname, colName, row, vd);
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
    size_t t_sz = IRastrPayload(table->Size()).Value();

    // НЕ вызываем beginRemoveRows здесь —
    // это сделает slot_BeginRemoveRows через цепочку хинтов
    if (static_cast<size_t>(count) == t_sz) {
        IRastrResultVerify{ table->SetSize(0) };
    } else {
        for (int i = 0; i < count; ++i)
            IRastrResultVerify{ table->DeleteRow(row) };
    }
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
    if (getRdata()->t_name_ != tName) return;
    const int count = last - first + 1;
    // ← сдвиг форматирования перед вставкой
    m_bgCache.shiftRowsDown(first, count);
    beginInsertRows({}, first, last);
}

void RModel::slot_EndInsertRow(std::string tName)
{
    if (getRdata()->t_name_ == tName) endInsertRows();
}

void RModel::slot_BeginRemoveRows(std::string tName, int first, int last)
{
    if (getRdata()->t_name_ != tName) return;
    beginRemoveRows({}, first, last);
}

void RModel::slot_EndRemoveRows(std::string tName)
{
    if (getRdata()->t_name_ != tName) return;
    // Определяем диапазон через beginRemoveRows/endRemoveRows не передаёт параметры,
    // поэтому проще инвалидировать весь кеш при удалении (операция редкая).
    m_bgCache.clear();
    endRemoveRows();
}

void RModel::slot_RefTableChanged(std::string tname)
{
    // Если изменилась наша же таблица — RTDM уже послал ChangeTable/ChangeAll,
    // не нужно дублировать.
    if (getRdata()->t_name_ == tname) return;
    spdlog::debug("Обновление ссылочных данных в {} для {}", tname, getRdata()->t_name_);

    // Перестраиваем только затронутые записи кеша
    std::vector<size_t> updated = m_cache.rebuildRefsFrom(tname, *m_rdata, m_rtdm);
    if (updated.empty()) return;

    // Преобразуем size_t → int для сигналов Qt
    std::vector<int> updatedInt(updated.begin(), updated.end());

    // Просим View обновить ячейки во всех строках затронутых колонок
    const int nRows = rowCount();
    if (nRows > 0) {
        for (int col : updatedInt)
            emit dataChanged(index(0, col), index(nRows - 1, col));
    }
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
    m_bgCache.invalidateColumn(static_cast<int>(column));
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

    for (const CondFormat& fmt : *vec) {
        // Пустой фильтр = правило по умолчанию, всегда срабатывает
        if (fmt.filter().isEmpty()) {
            switch (role) {
            case Qt::ForegroundRole:    return fmt.foregroundColor();
            case Qt::BackgroundRole:    return fmt.backgroundColor();
            case Qt::FontRole:          return fmt.font();
            case Qt::TextAlignmentRole: return static_cast<int>(fmt.alignmentFlag() | Qt::AlignVCenter);
            }
        }

        STRING_BOOL sb(value.toStdString() + " " + fmt.sqlCondition());

        // Подставляем значения колонок той же строки
        bool allResolved = true;
        for (const std::string& colName : sb.Check()) {
            auto it = m_rdata->mCols_.find(colName);
            if (it != m_rdata->mCols_.end()) {
                double v = std::visit(ToDouble(), m_rdata->pnparray_->Get(row, it->second));
                sb.replace(colName, std::to_string(v));
            } else {
                // Имя не найдено — условие не может быть вычислено
                spdlog::warn("getMatchingCondFormat: unknown column '{}' in condition '{}'",
                             colName, fmt.filter().toStdString());
                allResolved = false;
                break;
            }
        }
        if (!allResolved) continue;

        bool matches = false;
        try {
            matches = (sb.res() != 0.0);
        } catch (const std::exception& e) {
            spdlog::error("getMatchingCondFormat: eval error: {}", e.what());
            continue;
        }

        if (matches) {
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

    // pluginIdx — индекс колонки в плагине (стабилен, не зависит от визуального порядка).
    // Именно по нему BackInfoCache хранит все справочники.
    const size_t pluginIdx = static_cast<size_t>(col->getIndex());
    const auto   propTT    = col->getComPropTT();

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
        break;

    case enComPropTT::COM_PR_ENUM:
        if (!col->isDirectCode()) {
            if (const auto* enums = m_cache.enumItems(pluginIdx)) {
                info.editorType = ColumnEditorInfo::Type::ComboBox;
                info.comboItems = *enums;
                break;
            }
        }
        info.editorType = ColumnEditorInfo::Type::Numeric;
        break;

    case enComPropTT::COM_PR_INT:
        if (!col->isDirectCode() && !col->getNameRef().empty()) {
            if (const auto* nref = m_cache.namerefMap(pluginIdx)) {
                if (static_cast<int>(nref->size()) > 20) {
                    info.editorType        = ColumnEditorInfo::Type::NameRef;
                    info.nameRefData.items = *nref;
                    break;
                }
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
            if (const auto* senum = m_cache.superenumMap(pluginIdx)) {
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
    if (!m_rdata || col < 0 || col >= (int)m_rdata->size()){
        return nullptr;
    }
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

std::vector<long> RModel::getRowsBySelection(const std::string& selection) const
{
    return m_rtdm->getRowsBySelection(m_rdata->t_name_, selection);
}
