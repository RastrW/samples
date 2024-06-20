#ifndef MYMODEL_H
#define MYMODEL_H

#include <QAbstractTableModel>
#include <QTimer>
#include "rastrhlp.h"
#include "astra_exp.h"
//#include "astra_shared.h"
#include "rdata.h"



class MyModel : public QAbstractTableModel
{
    Q_OBJECT
    QTimer* timer_;
    CRastrHlp& rastr_;
    std::unique_ptr<RData> up_rdata;
    //RData rdata_;
    int n_form_indx_ = -1;
    std::vector<QString> vqcols_;                                 // Заголовки столбцов
public:
    MyModel(QObject *parent, CRastrHlp& rastr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void setFormIndx(int n_form_indx) { n_form_indx_ = n_form_indx; };
    int populateDataFromRastr();

    inline bool emitSignals() const { return m_emitSignals; }
    inline void setEmitSignals(bool b)  { m_emitSignals = b; }

    //"2.3 A Clock inside a Table Cell"
    static constexpr int n_timer_row_ = 3; // 3
    static constexpr int n_timer_col_ = 1; // 1
    void timerHit(); // from example of time change in cell

    //"2.4 Setting up Headers for Columns and Rows"
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    //"2.5 The Minimal Editing Example"
    static constexpr int n_rows_ = 300; // 3
    static constexpr int n_cols_ = 10; // 1
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
private:
    //QString m_gridData[n_rows_][n_cols_];  //holds text entered into QTableVi
    bool m_emitSignals;
signals:
    void editCompleted(const QString &);
};

#endif // MYMODEL_H
