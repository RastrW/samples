#ifndef TSTHINTS_H
#define TSTHINTS_H
#pragma once

#include <QObject>
#include <QWidget>
#include <QTableWidget>

class QAstra;
struct _hint_data;
class TstHints
    //: public QWidget
    :public QTableWidget
{
    Q_OBJECT
public:
    typedef std::vector<std::string> _vstrs;

    explicit TstHints(QWidget *parent = nullptr);
    void setTableName(std::string_view sv_tname_in);
    void setColNames(const _vstrs& vstrs_col_names_in);
    void setQAstra(std::weak_ptr<QAstra> wp_qastra_in);

signals:

private slots:
    void onRastrHint(const _hint_data&);

private:
    std::string str_tname_;
    _vstrs vstrs_col_names_;
    std::weak_ptr<QAstra> wp_qastra_;
};

#endif // TSTHINTS_H
