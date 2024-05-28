#ifndef COLPROPFORM_H
#define COLPROPFORM_H

#include <QWidget>
#include <QString>
#include "rastrdatamodel.h"

namespace Ui {
class ColPropForm;
}

class ColPropForm : public QWidget
{
    Q_OBJECT

public:
#if(!defined(QICSGRID_NO))
    explicit ColPropForm(RData* prdata,RCol* prcol, QWidget *parent = nullptr);
#endif//#if(!defined(QICSGRID_NO))
    ~ColPropForm();

    void setName(const QString& name);
    void setTitle(const QString& title);
    void setDesc(const QString& desc);
    void setWidth(const QString& width);
    void setPrec(const QString& acc);
    void setExpr(const QString& expr);
    QString name() const;

private slots:

    void on_btn_cancel_clicked();
    void on_btn_ok_clicked();

private:
    Ui::ColPropForm *ui;
};

#endif // COLPROPFORM_H
