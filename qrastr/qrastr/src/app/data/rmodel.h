#pragma once

#include <QAbstractTableModel>
#include "backInfoCache.h"
#include "condFormatStorage.h"
#include "rdata.h"
#include "table/backgroundCache.h"

class QAstra;
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
public slots:
    ///@brief Уведомление от RTDM:
    /// плагин генерирует хинт → RTDM ловит → испускает сигнал → слот вызывает beginInsertRows / endInsertRows у Qt
    void slot_DataChanged(std::string tName, int rowFrom, int colFrom, int rowTo, int colTo);
    void slot_BeginResetModel(std::string tName);
    void slot_EndResetModel(std::string tName);
    void slot_BeginInsertRow(std::string tName, int first, int last);
    void slot_EndInsertRow(std::string tName);
    void slot_BeginRemoveRows(std::string tName, int first, int last);
    void slot_EndRemoveRows(std::string tName);
    void slot_RefTableChanged(std::string tname);

public:
    // Описание редактора для конкретной колонки — используется делегатом.
    struct ColumnEditorInfo {
        enum class Type { None, Numeric, CheckBox, ComboBox, ComboBoxPicture, DateTime, Color, NameRef};

        Type        editorType = Type::None;
        QStringList comboItems;
        int         decimals   = 2;
        double      minVal     = -1e6;
        double      maxVal     =  1e6;

        struct NameRefData {
            std::map<size_t, std::string> items;   // key → имя
        };
        NameRefData nameRefData;

        struct PicItem { QPixmap image; QString label; };
        QList<PicItem> picItems;
    };

    explicit RModel(QObject* parent, QAstra* pqastra, RTablesDataManager* pRTDM);

    void setForm(CUIForm* pUIForm) { m_UIform = pUIForm; }

    /** @brief
     * Перестроение структуры данных модели.
     * Вызывается:
     *   1. При первом открытии формы (из RtabWidget::CreateModel).
     *   2. При slot_EndResetModel — после полного сброса таблицы.
     *   обращается к плагину для каждой колонки, читает DataBlock для BackInfoCache
    */
    bool populateDataFromRastr();

    // ── QAbstractTableModel interface ────────────────────────────────────────
    int      rowCount   (const QModelIndex& parent = {}) const override;
    int      columnCount(const QModelIndex& parent = {}) const override;
    QVariant data       (const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData (int section, Qt::Orientation orientation, int role)   const override;
    bool     setData    (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags (const QModelIndex& index) const override;
    ///@brief Запись: пользователь → вызов API плагина
    // ── Строки / колонки ─────────────────────────────────────────────────────
    bool addRow      (size_t count = 1,  const QModelIndex& parent = {});
    bool duplicateRow(int row,           const QModelIndex& parent = {});
    bool insertRows  (int row,   int count, const QModelIndex& parent = {}) override;
    bool insertColumns(int col,  int count, const QModelIndex& parent = {}) override;
    bool removeRows  (int row,   int count, const QModelIndex& parent = {}) override;
    bool removeColumns(int col,  int count, const QModelIndex& parent = {}) override;

    // ── Условное форматирование ──────────────────────────────────────────────
    void addCondFormat (size_t column, const CondFormat& condFormat);
    void setCondFormats(size_t column, const std::vector<CondFormat>& condFormats);
    // Читать текущие правила колонки для CondFormatController
    const std::vector<CondFormat>& getCondFormats(int column) const;

    // ── Утилиты ──────────────────────────────────────────────────────────────
    ColumnEditorInfo            getColumnEditorInfo(int colIndex) const;
    std::vector<std::tuple<int,int>> columnsWidth() const;
    RCol*  getRCol    (int col) const;
    int    getIndexCol(std::string col) const;
    RData* getRdata();

private:
    // ── Данные ───────────────────────────────────────────────────────────────
    QAstra*             m_qastra;
    RTablesDataManager* m_rtdm;
    CUIForm*            m_UIform {nullptr};

    std::unique_ptr<RData> m_rdata;

    BackInfoCache    m_cache;      // справочники ENUM / NAMEREF / SUPERENUM / ENPIC
    CondFormatStorage m_condFmt;   // условные форматы

    mutable BackgroundCache m_bgCache;

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
    QVariant dataForComboBox   (int col, const RCol& rcol, const QVariant& raw) const;
    // Текст для DisplayRole ячеек Enpic/ Enum / NameRef / SuperEnum
    // EditRole возвращает сырой числовой код, чтобы QTitan
    // использовал его при сортировке (числовое сравнение, не строковое).
    QString resolveDisplayText(int col, const RCol& rcol,
                               const QVariant& raw) const;
    // Данные отсутствуют (например, новая строка)
    QVariant dataForInvalidCellEditRole(int col, const RCol& rcol) const;
    QVariant dataForInvalidCellComboBoxRole(int col, const RCol& rcol) const;
};

