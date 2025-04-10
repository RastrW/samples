#ifndef RTABLEVIEW_H
#define RTABLEVIEW_H
#pragma once

#include <QTableView>
#include <QtWidgets>
#include <QtitanGrid.h>

class FilterTableHeader;
class RTabWidget;

class RTableCornerButton : public QAbstractButton
{
    Q_OBJECT
public:
    RTableCornerButton(QWidget *parent = 0);
    void paintEvent(QPaintEvent*);
    void setText(const QString &text) {m_text = text;}
    void setAlignment(Qt::Alignment align) {m_align = align;}

private:
    QString m_text;
    Qt::Alignment m_align;
};

class RTableView : public QTableView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTableView)
public:
    //RTableView();
    explicit RTableView(QWidget *parent = nullptr);
    // RTableView(QWidget *parent = nullptr);
    //~RTableView();
    //RTableView(RTabWidget* rtw);
    void generateFilters(int count);

signals:
    void onCornerButtonPressed();
protected slots:
    void CornerButtonPressed();

protected:
     RTableCornerButton cornerButton;
     FilterTableHeader* m_tableHeader;

     //PBSHeaderView pVertical, pHorizontal;
};

class QTRTableView : public Qtitan::GridTableView
{
    Q_OBJECT
};

#endif // RTABLEVIEW_H
