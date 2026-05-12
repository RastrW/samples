#include "rmodel.h"
#include <QBrush>
#include <spdlog/spdlog.h>
#include "QtitanGrid.h"
#include <QElapsedTimer>
#include "rdata.h"
#include "UIForms.h"
#include "table/rTablesDataAdapter.h"
#include <string_bool.h>
#include "condFormat.h"
#include "сolumnEditorInfo.h"

RModel::RModel(QObject* parent, std::shared_ptr<ITableRepository> tables)
    : QAbstractTableModel(parent)
    , m_tables(tables)
{}

bool RModel::populateDataFromRastr()
{
    try {
        QElapsedTimer t; t.start();
        // Схема запрашивается у репозитория — единственный вызов
        // который обращается к плагину на этапе построения модели.
        // RData и RCol не знают про плагин вообще.
        auto schema = m_tables->getSchema(m_UIform->TableName());
        spdlog::debug("[PERF]   getSchema: {} ms", t.restart());

        // schema передаётся по const& в конструктор RData —
        // объект схемы не копируется, строки копируются по одному разу.
        m_rdata = std::make_unique<RData>(schema, *m_UIform);
        spdlog::debug("[PERF]   make RData: {} ms", t.restart());
        m_rdata->populateBlock(m_tables);
        spdlog::debug("[PERF]   populateBlock: {} ms", t.restart());

        m_cache.rebuild(*m_rdata, m_tables);
        spdlog::debug("[PERF]   cache.rebuild: {} ms", t.restart());
        // заполняем кеш после перестройки структуры
        m_bgCache.clear();
        buildEditorInfoCache();
        spdlog::debug("[PERF]   buildEditorInfoCache: {} ms", t.restart());
    } catch (...) {
        spdlog::critical("populateDataFromRastr failed: {}",
                         m_rdata ? m_rdata->t_name_ : "<null>");
        return false;
    }
    return true;
}

int RModel::rowCount(const QModelIndex&) const
{
    if (!m_rdata || !m_rdata->isReady()) return 0;
    return static_cast<int>(m_rdata->getRowsCount());
}

int RModel::columnCount(const QModelIndex&) const
{
    // Всегда по размеру RData, а не блока.
    return m_rdata ? static_cast<int>(m_rdata->size()) : 0;
}

QVariant RModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!isReady()) return {};
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Horizontal)
        return QString::fromStdString(m_rdata->at(section).getTitle());
    return section + 1;
}

Qt::ItemFlags RModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index); // уже содержит Enabled|Selectable
    const auto* col = getRCol(ModelIndex{index.column()});
    if (col && col->getFF() == "1")
        return f & ~Qt::ItemIsEditable; // убираем только редактирование, Enabled оставляем
    return f | Qt::ItemIsEditable;
}

QVariant RModel::data(const QModelIndex& index, int role) const
{
    if (!isReady()) return {};

    const ModelIndex col{ index.column() };
    const int      row     = index.row();

    if (row < 0 || row >= rowCount() || !col.valid_in(columnCount()) )
    {
        spdlog::error("RModel::data out of bounds [row={}, col={}]",
                      row, col.value);
        return {};
    }

    const RCol& rcol = *m_rdata->colAt(col);
    // Ранние ветки — до чтения raw, чтобы не читать блок вхолостую
    if (role == Qt::ToolTipRole)
        return QStringLiteral("[%1, %2]").arg(row + 1).arg(col.value + 1);
    if (role == Qtitan::ComboBoxRole) //Выпадающий popup ComboBox
        return dataForComboBox(rcol);

    // Lazy load если колонка ещё не в блоке
    LocalIndex li = m_rdata->localIndexOf(col);
    if (li.invalid()){
        // const, mutable внутри
        li = m_rdata->ensureLoaded(col, m_tables);
        spdlog::info("Добавление новой колонки: {}", col.value);

    }
    // колонка недоступна даже после попытки
    if (li.invalid()) return {};

    // Единственное чтение ячейки; getCell скрывает LocalIndex
    const FieldVariantData fvd = m_rdata->getCell(col, row);
    const QVariant         raw = std::visit(ToQVariant(), fvd);

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return dataForDisplayEdit(row, col, rcol, raw, fvd, role);
    case Qt::UserRole:
        if (rcol.getComPropTT() == enComPropTT::COM_PR_REAL) {
            const bool isEmpty =
                std::holds_alternative<std::monostate>(fvd);
            return isEmpty ? 0.0 : raw;
        }
        return raw;
    case Qt::BackgroundRole:
        return dataForBackground(row, col, rcol, fvd, raw);
    case Qt::DecorationRole:
        return dataForDecoration(row, col, fvd, rcol);
    default:
        return {};
    }
}

QVariant RModel::dataForInvalidCellEditRole(const RCol& rcol) const
{
    if (rcol.getComPropTT() == enComPropTT::COM_PR_REAL)
        return QString();
    if (rcol.getComPropTT() == enComPropTT::COM_PR_INT  ||
        rcol.getComPropTT() == enComPropTT::COM_PR_ENUM ||
        rcol.getComPropTT() == enComPropTT::COM_PR_SUPERENUM) {
        return QVariant(0);
    }
    if (rcol.getComPropTT() == enComPropTT::COM_PR_TIME)
        return QDateTime();
    return QVariant(QString());
}

QVariant RModel::dataForDisplayEdit(int row, ModelIndex col, const RCol& rcol,
                                    const QVariant& raw,
                                    const FieldVariantData& fvd,
                                    int role) const{
    if (!raw.isValid() && role == Qt::EditRole){
        return dataForInvalidCellEditRole(rcol);
    }
    // TIME: секунды от эпохи плагина → QDateTime
    if (rcol.getComPropTT() == enComPropTT::COM_PR_TIME) {
        QString qstr = raw.toString();
        // Отрезаем суффикс "--7199", если он есть
        int dashPos = qstr.indexOf("--");
        if (dashPos >= 0) qstr = qstr.left(dashPos);
        // QTitan GridEditor::DateTime ждёт именно QDateTime
        return QDateTime::fromString(qstr.trimmed(), "dd.MM.yyyy HH:mm:ss");
    }
    // COLOR: упакованный RGB long → QColor
    if (rcol.getComPropTT() == enComPropTT::COM_PR_COLOR) {
        // Если raw действительно содержит число и не было изменено
        bool ok;
        long packed = raw.toLongLong(&ok);
        if (!ok) {
            spdlog::warn("Ошибка при преобразовании QVariant для "
                         "цвета через toLongLong [{},{}]", row, col.value);
            packed = std::visit(ToLong(), fvd);
        }
        return QColor(packed & 0xFF, (packed >> 8) & 0xFF, (packed >> 16) & 0xFF);
    }

    if (role == Qt::DisplayRole && !rcol.isDirectCode()) {
        if (auto resolved = resolveDisplayText(rcol, raw); !resolved.isNull())
            return resolved;
    }
    if (rcol.getComPropTT() == enComPropTT::COM_PR_REAL) {
        // Проверяем именно monostate, а не isValid() —
        // raw.isValid() == true даже для double(0.0)
        const bool isEmpty = std::holds_alternative<std::monostate>(fvd);
        if (isEmpty) return QVariant(); // невалидный QVariant — ячейка пуста

        // DisplayRole и EditRole — оба double
        // Qtitan вызовет convertToText() для отображения
        return raw.toDouble();
    }
    return raw;
}

QVariant RModel::dataForBackground (int row, ModelIndex col,
                                   const RCol& rcol,
                                   const FieldVariantData& fvd,
                                   const QVariant& raw) const{
    // COLOR-ячейки: фон = сам цвет, условные форматы не нужны
    if (rcol.getComPropTT() == enComPropTT::COM_PR_COLOR) {
        bool ok;
        const long packed = raw.toLongLong(&ok);
        if (!ok) return {};
        // COLORREF хранит каналы в порядке **BGR** (`0x00BBGGRR`), а `QColor::fromRgb` ожидает **RGB**.
        return QColor(packed & 0xFF, (packed >> 8) & 0xFF, (packed >> 16) & 0xFF);
    }
    // Для всех остальных — условный формат
    if (m_condFmt.formats().empty())
        return {};// форматы не заданы

    if (const QVariant* cached = m_bgCache.get(row, col))
        return *cached;// QVariant() → нет формата, тоже ок

    const QString val = QString::fromStdString(std::visit(ToString(), fvd));
    QVariant fmt = getMatchingCondFormat(row, col, val, Qt::BackgroundRole);
    m_bgCache.put(row, col, fmt);
    return fmt;
}

QVariant RModel::dataForDecoration (int row, ModelIndex col,
                                    const FieldVariantData& fvd,
                                    const RCol& rcol) const{
    // COLOR: упакованный RGB long → QColor
    if (rcol.getComPropTT() == enComPropTT::COM_PR_COLOR) {
        const long packed = std::visit(ToLong(), fvd);
        QPixmap px(16, 16);
        px.fill(QColor::fromRgb(static_cast<QRgb>(packed)));
        return px;
    }
    const auto idx = rcol.astraIndex();
    if (const auto* pics = m_cache.pictureEnum(idx)) {
        const long v = std::visit(ToLong(), fvd);
        if (v >= 0 && v < static_cast<long>(pics->size()))
            return (*pics)[static_cast<int>(v)].image;
    }
    return {};
}

QVariant RModel::dataForComboBox(const RCol& rcol) const
{
    const auto idx = rcol.astraIndex();

    if (const auto* pics = m_cache.pictureEnum(idx)) {
        QVariantList result;
        result.reserve(pics->size());
        for (const auto& p : *pics) result << p.image;
        return result;
    }

    QStringList list;
    if (const auto* enums = m_cache.enumItems(idx)){
        list = *enums;
    }
    else if (const auto nrd = m_cache.namerefData(idx)){
        for (const auto& row : nrd->rows){
            if (!row.values.empty()){
                list.append(QString::fromStdString(row.values[0]));
            }
        }
    }
    else if (const auto* senum = m_cache.superenumMap(idx)){
        for (const auto& [k, v] : *senum){
            list.append(QString::fromStdString(v));
        }
    }
    return list;
}

QString RModel::resolveDisplayText(const RCol& rcol,
                                   const QVariant& raw) const
{
    const auto idx = rcol.astraIndex();

    if (const auto* pics = m_cache.pictureEnum(idx)) {
        const int v = raw.toInt();
        if (v >= 0 && v < static_cast<int>(pics->size()))  return (*pics)[v].label;
        return {};
    }
    if (const auto* enums = m_cache.enumItems(idx)) {
        const int v = raw.toInt();
        if (v >= 0 && v < static_cast<int>(enums->size())) return (*enums)[v];
        return {};
    }
    if (const auto* senum = m_cache.superenumMap(idx)) {
        auto it = senum->find(static_cast<AstraIndex>(raw.toInt()));
        if (it != senum->end()) return QString::fromStdString(it->second);
        return {};
    }
    //nameref здесь не нужен
    return {};
}

bool RModel::setData(const QModelIndex& index,
                     const QVariant& value, int role)
{
    if (role != Qt::EditRole) return false;

    const ModelIndex col{ index.column() };
    const int row = index.row();

    const FieldVariantData currentFvd = m_rdata->getCell(col, row);

    // Пропускаем запись если значение не изменилось.
    // Сравниваем через visit, чтобы обойти особенности QVariant::operator==
    // для невалидных/пустых вариантов (monostate → QVariant()).
    const bool isEmpty = std::holds_alternative<std::monostate>(currentFvd);
    if (!isEmpty) {
        const QVariant currentRaw = std::visit(ToQVariant(), currentFvd);
        if (currentRaw.isValid() && currentRaw == value) {
            spdlog::debug("setData: value unchanged");
            return true;
        }
    }

    RData::iterator iter_col = m_rdata->begin() + col.to_size();
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
            const auto idx = iter_col->astraIndex();
            if (!iter_col->isDirectCode()) {
                if (const auto* enums = m_cache.enumItems(idx)) {
                    for (int i = 0; i < enums->size(); ++i)
                        if ((*enums)[i] == value) { val = i; break; }
                } else if (const auto* senum = m_cache.superenumMap(idx)) {
                    for (const auto& [k, v] : *senum)
                        if (v == value.toString().toStdString())
                        { val = k.value; break; }
                } else if (const auto nrd = m_cache.namerefData(idx)) {
                    // value содержит числовой ключ (int), установленный getContextValue()
                    val = value.toLongLong();
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
        case RCol::_en_data::DATA_DBL: {
            const QString str = value.toString().trimmed();
            if (str.isEmpty()) {
                vd = 0.0;
            } else {
                QString normalized = str;
                normalized.replace(',', '.');
                vd = normalized.toDouble();
            }
            break;
        }
        default:
            return false;
        }
    }

    // Единственная точка записи — репозиторий.
    // emit dataChanged не вызываем: плагин сгенерирует хинт →
    // RTDA обновит блок → sig_dataChanged → slot_DataChanged → View.
    m_tables->setValue(tname, colName, row, vd);
    return true;
}

bool RModel::addRow(size_t count, const QModelIndex&)
{
    m_tables->addRows(m_rdata->t_name_, count);
    return true;
}

bool RModel::duplicateRow(int row, const QModelIndex&)
{
    m_tables->duplicateRow(m_rdata->t_name_, row);
    // После DuplicateRow плагин сгенерирует InsertRow-хинт →
    // handleInsertRow вставит пустую строку в datablock.
    // DuplicateRow в QDenseDataBlock копирует данные из исходной строки в новую
    m_rdata->duplicateRow(row);
    return true;
}

bool RModel::insertRows(int row, int count, const QModelIndex&)
{
    m_tables->insertRows(m_rdata->t_name_, row, count);
    return true;
}

bool RModel::removeRows(int row, int count, const QModelIndex&)
{
    const long sz = m_tables->tableSize(m_rdata->t_name_);
    // НЕ вызываем beginRemoveRows здесь —
    // это сделает slot_BeginRemoveRows через цепочку хинтов
    if (static_cast<long>(count) == sz)
        m_tables->setTableSize(m_rdata->t_name_, 0);
    else
        m_tables->deleteRows(m_rdata->t_name_, row, count);
    return true;
}

bool RModel::removeColumns(int column, int count, const QModelIndex& parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    ///@todo FIXME: Implement me!
    endRemoveColumns();
    return true;
}

bool RModel::insertColumns(int column, int count, const QModelIndex& parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    ///@todo FIXME: Implement me!
    endInsertColumns();
    return true;
}

void RModel::slot_DataChanged(const std::string& tName,
                              int rowFrom, const std::string& colNameFrom,
                              int rowTo,   const std::string& colNameTo)
{
    if (!isMyTable(tName)) return;
    // инвалидируем затронутые строки
    m_bgCache.invalidateRows(rowFrom, rowTo);

    // Конвертируем имена → ModelIndex. Пустое имя = весь диапазон.
    const ModelIndex from = colNameFrom.empty()
                              ? ModelIndex{0}
                              : m_rdata->modelIndexOf(colNameFrom);

    const ModelIndex to = colNameTo.empty()
                            ? ModelIndex{columnCount() - 1}
                            : m_rdata->modelIndexOf(colNameTo);

    // Колонка не в нашей RData (не загружена в форме) — пропускаем
    if (from.invalid() || to.invalid()) return;

    emit dataChanged(index(rowFrom, from.value), index(rowTo, to.value));
}

void RModel::slot_BeginResetModel(const std::string& tName)
{
    if (!isMyTable(tName)){ return;}
    beginResetModel();
}

void RModel::slot_EndResetModel(const std::string& tName)
{
    if (!isMyTable(tName)){ return;}
    ///@note при сборсе обязательно целиком пересоздавать data
    /// populateDataFromRastr() вызывается МЕЖДУ beginResetModel и endResetModel.
    /// В этот момент View не обращается к модели — безопасно полностью
    ///пересоздать RData.
    populateDataFromRastr();
    endResetModel();
}

void RModel::slot_BeginInsertRow(const std::string& tName,
                                 int first, int last)
{
    if (!isMyTable(tName)){ return;}
    // сдвиг форматирования перед вставкой
    m_bgCache.shiftRowsDown(first, last - first + 1);
    beginInsertRows({}, first, last);
}

void RModel::slot_EndInsertRow(const std::string& tName)
{
    if (!isMyTable(tName)){ return;}
    endInsertRows();
}

void RModel::slot_BeginRemoveRows(const std::string& tName,
                                  int first, int last)
{
    if (!isMyTable(tName)){ return;}
    beginRemoveRows({}, first, last);
}

void RModel::slot_EndRemoveRows(const std::string& tName)
{
    if (!isMyTable(tName)){ return;}
    // Определяем диапазон через beginRemoveRows/endRemoveRows не передаёт параметры,
    // поэтому проще инвалидировать весь кеш при удалении (операция редкая).
    m_bgCache.clear();
    endRemoveRows();
}

void RModel::slot_RefTableChanged(const std::string& tName) {
    // Если изменилась наша же таблица — RTDA уже послал ChangeTable/ChangeAll,
    // не нужно дублировать.
    if (isMyTable(tName)) return;
    // Перестраиваем только затронутые записи кеша
    std::vector<ModelIndex> updated =
        m_cache.rebuildRefsFrom(tName, *m_rdata, m_tables);
    if (updated.empty()) return;
    // Обновляем m_editorInfoCache для затронутых колонок
    for (ModelIndex col : updated) {
        if (col.valid_in(m_editorInfoCache.size())){
            m_editorInfoCache[col.to_size()] = buildColumnEditorInfo(col);
        }
    }
    // Просим View обновить ячейки во всех строках затронутых колонок
    const int nRows = rowCount();
    if (nRows > 0) {
        for (ModelIndex col : updated) {
            emit dataChanged(index(0, col.value),
                             index(nRows - 1, col.value));
        }
    }
    // Уведомляем контроллер — пусть обновит репозитории Qtitan
    emit sig_nameRefUpdated(updated);
}

void RModel::buildEditorInfoCache() {
    const int n = columnCount();
    m_editorInfoCache.resize(n);
    for (int i = 0; i < n; ++i)
        m_editorInfoCache[i] = buildColumnEditorInfo(ModelIndex{i});
}

const ColumnEditorInfo&
RModel::getColumnEditorInfo(ModelIndex colIndex) const
{
    static const ColumnEditorInfo s_empty;

    if (colIndex.valid_in(m_editorInfoCache.size()))
        return m_editorInfoCache[colIndex.to_size()];
    return s_empty;
}

ColumnEditorInfo
RModel::buildColumnEditorInfo(ModelIndex colIndex) const
{
    ColumnEditorInfo info;
    const RCol* col = getRCol(colIndex);
    if (!col) return info;
    // astra — индекс колонки в astra (стабилен, не зависит от визуального порядка).
    // Именно по нему BackInfoCache хранит все справочники.
    const auto idx = col->astraIndex();
    const auto propTT  = col->getComPropTT();

    switch (propTT) {
    case enComPropTT::COM_PR_BOOL:
        info.editorType = ColumnEditorInfo::Type::CheckBox;
        break;

    case enComPropTT::COM_PR_REAL:
        info.editorType = ColumnEditorInfo::Type::Numeric;
        // пустая строка и мусор дают 2
        try { info.decimals = std::stoi(col->getPrec()); }
        catch (...) { info.decimals = 2; }
        break;

    case enComPropTT::COM_PR_TIME:
        info.editorType = ColumnEditorInfo::Type::DateTime;
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
            if (const auto nrd = m_cache.namerefData(idx)) {
                info.editorType  = ColumnEditorInfo::Type::NameRef;
                info.nameRefData = nrd;
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
            if (const auto* pics = m_cache.pictureEnum(idx)) {
                info.editorType = ColumnEditorInfo::Type::ComboBoxPicture;
                for (const auto& p : *pics)
                    info.picItems.append({p.image, p.label});
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

void RModel::setCondFormats(ModelIndex column,
                            const std::vector<CondFormat>& condFormats)
{
    m_condFmt.set(column, condFormats);
    m_bgCache.invalidateColumn(column);
    // layoutChanged вызывается из контроллера после полной замены
}

const std::vector<CondFormat>&
RModel::getCondFormats(ModelIndex column) const
{
    // Если для колонки нет правил, возвращаем ссылку на статический пустой вектор.
    static const std::vector<CondFormat> empty;
    const auto* vec = m_condFmt.column(column);
    return vec ? *vec : empty;
}

QVariant RModel::getMatchingCondFormat(size_t row, ModelIndex column,
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
                double v = std::visit(ToDouble(), m_rdata->getCell(it->second, row));
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

std::vector<std::tuple<int,int>>
RModel::columnsWidth() const{
    std::vector<std::tuple<int,int>> result;
    if (!m_rdata) return result;

    int modelPos = 0;
    for (const RCol& rcol : *m_rdata) {
        int width = 0;
        try { width = std::stoi(rcol.getWidth()); }
        catch (...) { width = 10; }           // fallback

        result.emplace_back(modelPos, width); //model position
        ++modelPos;
    }
    return result;
}

void RModel::invertDirectCode(ModelIndex col){

    if (!m_rdata || col.valid_in(m_rdata->size())) return;
    (m_rdata->begin() + col.to_size())->invertDirectCodeStatus();
}

const RCol*
RModel::getRCol(ModelIndex col) const{

    if (!m_rdata || !col.valid_in(m_rdata->size()))
        return nullptr;
    return &(*(m_rdata->begin() + col.to_size()));
}

const RData& RModel::getRdata() const{
    return *m_rdata;
}

bool RModel::isReady() const noexcept{
    return m_rdata != nullptr && m_rdata->isReady();
}

bool RModel::isMyTable(const std::string& tName) const noexcept{
    return m_rdata && m_rdata->t_name_ == tName;
}