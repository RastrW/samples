#pragma once

#include <QDialog>
#include <QString>

namespace Qtitan   { class GridTableView; }

class RData;
class RCol;
class QPlainTextEdit;
class QLineEdit;

class ColPropForm : public QDialog
{
    Q_OBJECT

public:
    explicit ColPropForm(RData* prdata,
                         Qtitan::GridTableView* view,
                         RCol* prcol, QWidget *parent = nullptr);
    ~ColPropForm() = default;

private slots:
    void on_btn_ok_clicked();

private:
    Qtitan::GridTableView* m_view;
    RData*  m_prdata;
    RCol*   m_prcol;

    QLineEdit* m_leName;
    QLineEdit* m_leTitle;
    QPlainTextEdit* m_teDescr;
    QLineEdit* m_leWidth;
    QLineEdit* m_lePrecision;
    QLineEdit* m_leExpression;

    void setupUi();
};
