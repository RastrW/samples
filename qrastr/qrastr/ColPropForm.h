#ifndef COLPROPFORM_H
#define COLPROPFORM_H

#include <QWidget>
#include <QString>
//#include "rastrdatamodel.h"
#include "rmodel.h"
#include "rtableview.h"

namespace Ui {
class ColPropForm;
}

class ColPropForm : public QWidget
{
    Q_OBJECT

public:
//#if(!defined(QICSGRID_NO))
    explicit ColPropForm(RData* prdata,RTableView* ptv,Qtitan::GridTableView* view, RCol* prcol, QWidget *parent = nullptr);
    //explicit ColPropForm(RModel* prmodel,RCol* prcol, QWidget *parent = nullptr);
//#endif//#if(!defined(QICSGRID_NO))
    ~ColPropForm();

    void setName(const QString& name);
    void setTitle(const QString& title);
    void setDesc(const QString& desc);
    void setWidth(const QString& width);
    void setPrec(const QString& acc);
    void setExpr(const QString& expr);
    QString getName();
    QString getTitle();
    QString getDesc();
    QString getWidth();
    QString getPrec();
    QString getExpr();

private slots:

    void on_btn_cancel_clicked();
    void on_btn_ok_clicked();

private:
    RTableView* ptv;
    Qtitan::GridTableView* view;
    Ui::ColPropForm *ui;
    RData* prdata;
    RCol* prcol;
};

#endif // COLPROPFORM_H
