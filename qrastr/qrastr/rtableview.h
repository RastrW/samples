#ifndef RTABLEVIEW_H
#define RTABLEVIEW_H

#include <QTableView>
#include <QtWidgets>

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

#endif // RTABLEVIEW_H
