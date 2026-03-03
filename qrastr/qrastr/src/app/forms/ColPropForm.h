#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class ColPropForm;
}
namespace Qtitan   { class GridTableView; }

class RData;
class RCol;

class ColPropForm : public QDialog
{
    Q_OBJECT

public:
    explicit ColPropForm(RData* prdata,
                         Qtitan::GridTableView* view,
                         RCol* prcol, QWidget *parent = nullptr);

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
    Ui::ColPropForm *ui;
    Qtitan::GridTableView* m_view;
    RData* m_prdata;
    RCol* m_prcol;
};
