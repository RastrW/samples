#pragma once

#include <QAbstractTableModel>
#include "backInfoCache.h"
#include "condFormatStorage.h"
#include "rdata.h"
#include "table/backgroundCache.h"
#include "table/tableIndexHash.h"

class CUIForm;
class RData;
class RCol;
class CondFormat;
struct ColumnEditorInfo;

struct ToQVariant {
    QVariant operator()(std::monostate) { return { QVariant() }; }
    QVariant operator()(const long& value) { return (qlonglong)value; }
    QVariant operator()(const uint64_t& value) { return QVariant::fromValue(value); }
    QVariant operator()(const double& value) { return value; }
    QVariant operator()(const bool& value) { return value; }
    QVariant operator()(const std::string& value) { return std::string(value).c_str(); }
};

///@brief Qt модель для связи плагина с QTableView/QTitan Grid
class RModel : public QAbstractTableModel
{
    Q_OBJECT

signals:
    void editCompleted(const QString&);
    void sig_nameRefUpdated(const std::vector<ModelColumn>& updatedCols);

public slots:
    ///@brief Уведомление от RTDA:
    /// плагин генерирует хинт → RTDA ловит → испускает сигнал → слот вызывает beginInsertRows / endInsertRows у Qt
    void slot_DataChanged(const std::string& tName,
                          int rowFrom, const std::string& colNameFrom,
                          int rowTo,   const std::string& colNameTo);
    void slot_BeginResetModel(const std::string& tName);
    void slot_EndResetModel(const std::string& tName);
    void slot_BeginInsertRow(const std::string& tName, int first, int last);
    void slot_EndInsertRow(const std::string& tName);
    void slot_BeginRemoveRows(const std::string& tName, int first, int last);
    void slot_EndRemoveRows(const std::string& tName);
    void slot_RefTableChanged(const std::string& tName);

public:
    /**
     * @param repo  Невладеющий указатель на ITableRepository.
     *              Время жизни репозитория гарантируется RtabController.
     */
    explicit RModel(QObject* parent, std::shared_ptr<ITableRepository>        tables);

    void setForm(CUIForm* pUIForm) { m_UIform = pUIForm; }

    /**
     * Перестроение структуры данных модели.
     * Вызывается при первом открытии и при slot_EndResetModel.
     * Запрашивает схему у репозитория
     */
    bool populateDataFromRastr();

    // ── QAbstractTableModel ───────────────────────────────────────────────
    int           rowCount   (const QModelIndex& parent = {}) const override;
    int           columnCount(const QModelIndex& parent = {}) const override;
    QVariant      data       (const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant      headerData (int section, Qt::Orientation orientation, int role) const override;
    bool          setData    (const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags      (const QModelIndex& index) const override;

    // ── Строки ───────────────────────────────────────────────────────────
    bool addRow      (size_t count = 1,  const QModelIndex& parent = {});
    bool duplicateRow(int row,           const QModelIndex& parent = {});
    bool insertRows  (int row,   int count, const QModelIndex& parent = {}) override;
    bool insertColumns(int col,  int count, const QModelIndex& parent = {}) override;
    bool removeRows  (int row,   int count, const QModelIndex& parent = {}) override;
    bool removeColumns(int col,  int count, const QModelIndex& parent = {}) override;

    // ── Условное форматирование ───────────────────────────────────────────
    void setCondFormats(ModelColumn column, const std::vector<CondFormat>& condFormats);
    // Читать текущие правила колонки для CondFormatController
    const std::vector<CondFormat>& getCondFormats(ModelColumn column) const;

    // ── Утилиты ───────────────────────────────────────────────────────────
    std::vector<std::tuple<int,int>>
        columnsWidth() const;
    const RCol* getRCol   (ModelColumn col) const;
    const RData& getRdata () const;
    const ColumnEditorInfo&
        getColumnEditorInfo(ModelColumn colIndex) const;
    void invertDirectCode(ModelColumn col);
private:
    // ── Данные ───────────────────────────────────────────────────────────────
    std::shared_ptr<ITableRepository> m_tables;
    CUIForm*           m_UIform  {nullptr};

    std::unique_ptr<RData> m_rdata;

    BackInfoCache     m_cache;	// справочники ENUM / NAMEREF / SUPERENUM / ENPIC
    CondFormatStorage m_condFmt;// условные форматы

    mutable BackgroundCache              m_bgCache;
    // Кеш: индекс = логический номер колонки (позиция в RData).
    // Инвалидируется при полном перестроении модели.
    mutable std::vector<ColumnEditorInfo> m_editorInfoCache;

    // ── Условное форматирование ────────────────────────────
    QVariant getMatchingCondFormat(size_t row, ModelColumn col,
                                   const QString& value, int role) const;
    // ── DisplayRole / EditRole (Текст для отображения/Значение для редактора)─
    QVariant dataForDisplayEdit(int row, ModelColumn col, const RCol& rcol,
                                const QVariant& raw,
                                const FieldVariantData& fvd,
                                int role) const;
    // ── BackgroundRole (Фон ячейки) ──────────────────────────────────────────
    QVariant dataForBackground (int row, ModelColumn col,
                                const RCol& rcol,
                                const FieldVariantData& fvd,
                                const QVariant& raw) const;
    // ── DecorationRole (Иконка слева от текста)───────────────────────────────
    QVariant dataForDecoration (int row, ModelColumn col,
                                const FieldVariantData& fvd,
                                const RCol& rcol) const;
    // ── ComboBoxRole (popup для ComboBox)─────────────────────────────────────
    ///@note Строит список элементов для ComboBox/ComboBoxPicture.
    /// Не зависит от raw-значения — возвращает одно и то же для валидных и невалидных ячеек.
    QVariant dataForComboBox   (const RCol& rcol) const;
    // Текст для DisplayRole ячеек Enpic/ Enum / NameRef / SuperEnum
    // EditRole возвращает сырой числовой код, чтобы QTitan
    // использовал его при сортировке (числовое сравнение, не строковое).
    QString  resolveDisplayText(const RCol& rcol,
                               const QVariant& raw) const;
    // Данные отсутствуют (например, новая строка)
    QVariant dataForInvalidCellEditRole(const RCol& rcol) const;
    // Функции получения редактора из кеша и вычисление редактора для колонки в кеш
    ColumnEditorInfo buildColumnEditorInfo(ModelColumn colIndex) const;
    void             buildEditorInfoCache();
    //Guard для неинициализированной модели
    bool isReady() const noexcept;
    bool isMyTable(const std::string& tName) const noexcept;
};