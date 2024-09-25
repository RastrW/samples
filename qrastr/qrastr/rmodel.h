#ifndef RMODEL_H
#define RMODEL_H

#include <QAbstractTableModel>
#include <QTimer>
#include "rastrhlp.h"
#include "astra_exp.h"
//#include "astra_shared.h"
#include "rdata.h"
#include "qastra.h"



class RModel : public QAbstractTableModel
{
    Q_OBJECT
    //CRastrHlp& rastr_;
    QAstra* pqastra_;
    std::unique_ptr<RData> up_rdata;
    std::vector<QString> vqcols_;                                 // Заголовки столбцов
    CUIForm* pUIForm_;
public:
    RModel(QObject *parent, QAstra* pqastra);
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

    void timerHit(); // from example of time change in cell

    //"2.4 Setting up Headers for Columns and Rows"
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    //"2.5 The Minimal Editing Example"
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool isBinary(const QModelIndex& index) const;
private:
    bool isBinary(const QByteArray& index) const;
    bool m_emitSignals;
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
