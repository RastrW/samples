#pragma once

#include <QAbstractTableModel>
#include "backInfoCache.h"
#include "condFormatStorage.h"
#include "rdata.h"
#include "table/backgroundCache.h"

class RTablesDataManager;
class CUIForm;
class RData;
class RCol;
class CondFormat;

struct ToQVariant {
    QVariant operator()(std::monostate) { return { QVariant() }; }
    QVariant operator()(const long& value) { return (qlonglong)value; }
    QVariant operator()(const uint64_t& value) { return QVariant::fromValue(value); }
    QVariant operator()(const double& value) { return value; }
    QVariant operator()(const bool& value) { return value; }
    QVariant operator()(const std::string& value) { return std::string(value).c_str(); }
};

///@brief Qt модель для связи QDataBlock с QTableView/QTitan Grid
class RModel : public QAbstractTableModel
{
    Q_OBJECT

signals:
    void editCompleted(const QString&);
    void sig_nameRefUpdated(std::vector<size_t> updatedCols);

public slots:
    ///@brief Уведомление от RTDM:
    /// плагин генерирует хинт → RTDM ловит → испускает сигнал → слот вызывает beginInsertRows / endInsertRows у Qt
    void slot_DataChanged(const std::string& tName, int rowFrom, int colFrom,
                          int rowTo, int colTo);
    void slot_BeginResetModel(const std::string& tName);
    void slot_EndResetModel(const std::string& tName);
    void slot_BeginInsertRow(const std::string& tName, int first, int last);
    void slot_EndInsertRow(const std::string& tName);
    void slot_BeginRemoveRows(const std::string& tName, int first, int last);
    void slot_EndRemoveRows(const std::string& tName);
    void slot_RefTableChanged(const std::string& tname);

public:
    struct ColumnEditorInfo {
        enum class Type {
            None, Numeric, CheckBox, ComboBox,
            ComboBoxPicture, DateTime, Color, NameRef
        };

        Type        editorType = Type::None;
        QStringList comboItems;
        int         decimals   = 2;
        double      minVal     = -1e6;
        double      maxVal     =  1e6;

        struct NameRefData {
            std::unordered_map<size_t, std::string> items;
        };
        NameRefData nameRefData;

        struct PicItem { QPixmap image; QString label; };
        QList<PicItem> picItems;
    };

    /**
     * @param repo  Невладеющий указатель на ITableRepository.
     *              Время жизни репозитория гарантируется RtabController.
     */
    explicit RModel(QObject* parent, ITableRepository* repo);

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
    void addCondFormat (size_t column, const CondFormat& condFormat);
    void setCondFormats(size_t column, const std::vector<CondFormat>& condFormats);
    // Читать текущие правила колонки для CondFormatController
    const std::vector<CondFormat>& getCondFormats(int column) const;

    // ── Утилиты ───────────────────────────────────────────────────────────
    std::vector<std::tuple<int,int>> columnsWidth() const;
    RCol*             getRCol    (int col) const;
    int               getIndexCol(const std::string& col) const;
    RData*            getRdata   ();
    std::vector<long> getRowsBySelection(const std::string& selection) const;
    ColumnEditorInfo  getColumnEditorInfo(int colIndex) const;

private:
    // ── Данные ───────────────────────────────────────────────────────────────
    ITableRepository*  m_repo;// m_repo — единственная точка входа к плагину.
    CUIForm*           m_UIform  {nullptr};

    std::unique_ptr<RData> m_rdata;

    BackInfoCache     m_cache;	// справочники ENUM / NAMEREF / SUPERENUM / ENPIC
    CondFormatStorage m_condFmt;// условные форматы

    mutable BackgroundCache              m_bgCache;
    // Кеш: индекс = логический номер колонки (позиция в RData).
    // Инвалидируется при полном перестроении модели.
    mutable std::vector<ColumnEditorInfo> m_editorInfoCache;

    // ── Условное форматирование ────────────────────────────
    QVariant getMatchingCondFormat(size_t row, size_t col,
                                   const QString& value, int role) const;
    // ── DisplayRole / EditRole (Текст для отображения/Значение для редактора)─
    QVariant dataForDisplayEdit(int row, int col, const RCol& rcol,
                                const QVariant& raw, int role) const;
    // ── BackgroundRole (Фон ячейки) ──────────────────────────────────────────
    QVariant dataForBackground (int row, int col,
                               const RCol& rcol, const QVariant& raw) const;
    // ── DecorationRole (Иконка слева от текста)───────────────────────────────
    QVariant dataForDecoration (int row, int col,
                               const RCol& rcol, const QVariant& raw) const;
    // ── ComboBoxRole (popup для ComboBox)─────────────────────────────────────
    ///@note Строит список элементов для ComboBox/ComboBoxPicture.
    /// Не зависит от raw-значения — возвращает одно и то же для валидных и невалидных ячеек.
    QVariant dataForComboBox   (const RCol& rcol) const;
    // Текст для DisplayRole ячеек Enpic/ Enum / NameRef / SuperEnum
    // EditRole возвращает сырой числовой код, чтобы QTitan
    // использовал его при сортировке (числовое сравнение, не строковое).
    QString  resolveDisplayText(int col, const RCol& rcol,
                               const QVariant& raw) const;
    // Данные отсутствуют (например, новая строка)
    QVariant dataForInvalidCellEditRole(int col, const RCol& rcol) const;
    // Функции получения редактора из кеша и вычисление редактора для колонки в кеш
    ColumnEditorInfo buildColumnEditorInfo(int colIndex) const;
    void             buildEditorInfoCache();
};