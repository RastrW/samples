#pragma once

#include <QDialog>
#include <QString>
#include "table/ITableRepository.h"

namespace Qtitan   { class GridTableView; }

class RData;
class RCol;
class QPlainTextEdit;
class QLineEdit;

class ColPropDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ColPropDialog(ITableRepository*                   repo,
                           const ITableRepository::ColumnSchema& schema,
                           Qtitan::GridTableView*              view,
                           QWidget*                            parent = nullptr);
    ~ColPropDialog() = default;

private slots:
    void on_btn_ok_clicked();

private:
    ITableRepository*              m_repo;
    ITableRepository::ColumnSchema m_schema;   // копия — имя, таблица, индекс
    Qtitan::GridTableView*         m_view;

    QLineEdit*      m_leName;
    QLineEdit*      m_leTitle;
    QPlainTextEdit* m_teDescr;
    QLineEdit*      m_leWidth;
    QLineEdit*      m_lePrecision;
    QLineEdit*      m_leExpression;

    void setupUi();
};
