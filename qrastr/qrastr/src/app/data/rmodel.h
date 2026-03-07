#pragma once

#include <QAbstractTableModel>
#include <QIcon>

class QAstra;
class RTablesDataManager;
class CondFormat;
class RData;
class CUIForm;
class RCol;

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
    void editCompleted(const QString &);
public slots:
    ///@brief Уведомление:
    /// плагин генерирует хинт → RTDM ловит → испускает сигнал → слот вызывает beginInsertRows / endInsertRows у Qt
    void slot_DataChanged(std::string _t_name, int row_from,int col_from ,int row_to,int col_to);
    //сброс модели
    void slot_BeginResetModel(std::string _t_name);
    ///@todo Нужна ли загрузка справочных данных rebuildBackInfo() при сбросе модели?
    void slot_EndResetModel(std::string _t_name);
    //вставка строки
    void slot_BeginInsertRow(std::string _t_name,int first, int last);
    void slot_EndInsertRow(std::string _t_name);
    //удаление строки
    void slot_BeginRemoveRows(std::string tname, int first, int last);
    void slot_EndRemoveRows(std::string tname);
public:
    struct ColumnEditorInfo {
        enum class Type { None, Numeric, CheckBox, ComboBox, ComboBoxPicture };

        Type        editorType = Type::None;
        QStringList comboItems;
        int         decimals   = 2;
        double      minVal     = -1e6;
        double      maxVal     =  1e6;

        struct PicItem { QPixmap image; QString label; };
        QList<PicItem> picItems;
    };

    RModel(QObject *parent, QAstra* pqastra,RTablesDataManager* pRTDM);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void setForm( CUIForm* _pUIForm) { pUIForm_ = _pUIForm; };
    /** @brief
     * Перестроение структуры данных модели.
     * Вызывается:
     *   1. При первом открытии формы (из RtabWidget::CreateModel).
     *   2. При slot_EndResetModel — после полного сброса таблицы.
     *   обращается к плагину для каждой колонки, читает DataBlock для
     * SUPERENUM и NAMEREF-ссылок.
     */
    bool populateDataFromRastr();
    std::vector<std::tuple<int,int>>  ColumnsWidth ();
    RCol* getRCol(int n_col) const;
    int getIndexCol(std::string _col);
    RData* getRdata();

    //"2.4 Setting up Headers for Columns and Rows"
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    //"2.5 The Minimal Editing Example"
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    ///@brief Запись: пользователь → вызов API плагина
    bool AddRow(std::size_t count = 1,
                const QModelIndex &parent = QModelIndex());
    bool DuplicateRow(int row,
                      const QModelIndex &parent = QModelIndex());
    bool insertRows(int row, int count,
                    const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count,
                       const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count,
                       const QModelIndex &parent = QModelIndex()) override;

    // Conditional formats are of two kinds: regular conditional formats (including condition-free formats applying to any value in the
    // column) and formats applying to a particular row-id and which have always precedence over the first kind and whose filter apply
    // to the row-id column.
    void addCondFormat(const bool isRowIdFormat, std::size_t column, const CondFormat& condFormat);
    void setCondFormats(const bool isRowIdFormat, std::size_t column, const std::vector<CondFormat>& condFormats);

    ColumnEditorInfo getColumnEditorInfo(int colIndex) const;
private:
    /// @brief Пересоздаёт RData: читает структуру колонок из плагина.
    void rebuildStructure();
    /// @brief Перестраивает справочники.
    void rebuildBackInfo();
    /// @brief Обновляет только указатель на QDataBlock через RTDM.
    void reloadData();
    // Return matching conditional format color/font or invalid value, otherwise.
    // Only format roles are expected in role (Qt::ItemDataRole)
    QVariant getMatchingCondFormat(std::size_t row, std::size_t column, const QString& value, int role) const;
    QVariant getMatchingCondFormat(const std::map<std::size_t, std::vector<CondFormat>>& mCondFormats,
                                   std::size_t row, std::size_t column, const QString& value, int role) const;

    QAstra* pqastra_;
    RTablesDataManager* pRTDM_;
    std::unique_ptr<RData> up_rdata;
    CUIForm* pUIForm_;
    /// @note Условное форматирование:
    /// 1. Правила форматирования по значению ячейки
    std::map<std::size_t, std::vector<CondFormat>> m_mRowIdFormats;
    /// 2. Правила по идентификатору строки
    std::map<std::size_t, std::vector<CondFormat>> m_mCondFormats;

    //Справочные данные:
    //индекс -> список строк: ex. БАЗА|Ген|Нагр|Ген+
    std::map<std::size_t,QStringList> m_enum_;
    //колонки со ссылкой: код -> отображаемое имя: ex.RefCol -> node[na]
    std::map<std::size_t, std::map<std::size_t, std::string>> mm_nameref_;
    // код -> отображаемое имя: ex. ti_prv.Name.Num
    std::map<std::size_t, std::map<std::size_t, std::string>> mm_superenum_;
    struct PictureItem {
        QString label;
        int     qtitanIconIndex; // индекс из nameref
        QPixmap   image;
    };
    std::map<std::size_t, QList<PictureItem>> m_pictureEnums_;
};
