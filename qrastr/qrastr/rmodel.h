#ifndef RMODEL_H
#define RMODEL_H

#include <QAbstractTableModel>
#include <QTimer>
#include "astra_exp.h"
#include "rdata.h"
#include "qastra.h"
#include "rtablesdatamanager.h"

class CondFormat;

struct ToQVariant {
    QVariant operator()(std::monostate) { return { QVariant() }; }
    QVariant operator()(const long& value) { return (qlonglong)value; }
    QVariant operator()(const uint64_t& value) { return value; }
    QVariant operator()(const double& value) { return value; }
    QVariant operator()(const bool& value) { return value; }
    QVariant operator()(const std::string& value) { return std::string(value).c_str(); }
};

class RModel : public QAbstractTableModel
{
    Q_OBJECT
    QAstra* pqastra_;
    RTablesDataManager* pRTDM_;
    std::unique_ptr<RData> up_rdata;
    std::vector<QString> vqcols_;                                 // Заголовки столбцов
    CUIForm* pUIForm_;
public:
    std::map<size_t,QStringList> mnamerefs_;                      // Хранилище енумов колонок для ComboBox
public:
    RModel(QObject *parent, QAstra* pqastra,RTablesDataManager* pRTDM);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    //void setFormIndx(int n_form_indx) { n_form_indx_ = n_form_indx; };
    void setForm( CUIForm* _pUIForm) { pUIForm_ = _pUIForm; };
    int populateDataFromRastr();
    std::vector<std::tuple<int,int>>  ColumnsWidth ();
    RCol* getRCol(int n_col);
    int getIndexCol(std::string _col);
    RData* getRdata();
    inline bool emitSignals() const { return m_emitSignals; }
    inline void setEmitSignals(bool b)  { m_emitSignals = b; }

    //"2.4 Setting up Headers for Columns and Rows"
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    //"2.5 The Minimal Editing Example"
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Add data:
    bool AddRow(size_t count = 1,const QModelIndex &parent = QModelIndex());
    bool DuplicateRow(int row, const QModelIndex &parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool isBinary(const QModelIndex& index) const;

    void SetSelection(std::string Selection);

    // Conditional formats are of two kinds: regular conditional formats (including condition-free formats applying to any value in the
    // column) and formats applying to a particular row-id and which have always precedence over the first kind and whose filter apply
    // to the row-id column.
    void addCondFormat(const bool isRowIdFormat, size_t column, const CondFormat& condFormat);
    void setCondFormats(const bool isRowIdFormat, size_t column, const std::vector<CondFormat>& condFormats);

private:
    bool isBinary(const QByteArray& index) const;
    bool m_emitSignals;
    // Return matching conditional format color/font or invalid value, otherwise.
    // Only format roles are expected in role (Qt::ItemDataRole)
    QVariant getMatchingCondFormat(size_t row, size_t column, const QString& value, int role) const;
    QVariant getMatchingCondFormat(const std::map<size_t, std::vector<CondFormat>>& mCondFormats, size_t row, size_t column, const QString& value, int role) const;
    std::map<size_t, std::vector<CondFormat>> m_mRowIdFormats;
    std::map<size_t, std::vector<CondFormat>> m_mCondFormats;
signals:

    void editCompleted(const QString &);
     //QModelIndex &index, const QVariant &value

    //void dataChanged2(std::string _t_name);
    void dataChanged(std::string _t_name, QModelIndex index, QVariant value);
    void dataChanged(std::string _t_name, std::string _col_name, int _row, QVariant _value);
    void RowInserted(std::string _t_name, int _row);
    void RowDeleted(std::string _t_name, int _row);
//private slots:
    public slots:
    //void onRModelchange(std::string _t_name, QModelIndex index, QVariant value);
    void onRModelchange(std::string _t_name, std::string _col_name, int _row, QVariant _value);
    void onrm_RowInserted(std::string _t_name, int _row);
    void onrm_RowDeleted(std::string _t_name, int _row);
};

#endif // RMODEL_H
