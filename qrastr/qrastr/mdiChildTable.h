#ifndef MDICHILD_H
#define MDICHILD_H

#if(!defined(QICSGRID_NO))

#include <QicsTable.h>
#include "License2/json.hpp"
#include "astra_exp.h"
#include <QicsDataModel.h>
#include "rastrdatamodel.h"
#include <QicsPushButtonCellDisplay.h>
#include <QicsCheckCellDisplay.h>
#include <QicsTextCellDisplay.h>
#include <QicsTextCellDisplayHelpers.h>

class MdiChild : public QicsTable
{
    typedef std::vector<std::string> _vstr;

    Q_OBJECT
public:
    MdiChild( const _idRastr id_rastr, const nlohmann::json& j_form_in );
    MdiChild( const _idRastr id_rastr, const nlohmann::json& j_form_in,  QicsTableGrid::Foundry tf,QicsHeaderGrid::Foundry hf,QWidget* parent = 0);

    void newFile();
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    void writeFile(QString fname);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }
    void mousePressEvent(QMouseEvent *event) override ;
    int ind_col_clicked = -1;                               // Индекс кликнутого столбца
    RCol* GetCol(int ColInd = -1);                          // Если -1 берем ind_col_clicked
protected:
    //virtual void handleMousePressEvent(const QicsICell &cell, QMouseEvent *m);



public slots:
    void update_data();
    void insertRows();
    void deleteRows();
    void hideColumns();
    //void AutoWidthColumns();

    void sort(int col_ind = -1,Qics::QicsSortOrder sort_type = Qics::Ascending);
    void sortAscending();
    void sortDescending();
    void sort_by_col(int col_ind);
    void OpenColPropForm();


    void clickColBtnHideCol();
    void clickColBtnFilterCol();
    void clickColBtnDeleteRow();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void closeEvent(QCloseEvent *event) override;
   // bool eventFilter(QObject *obj, QEvent *ev) override;
    QicsDataModel *dm;
private:
    void setCurrentFile(const QString &fileName);
    void MakeHeaders();                                     // Создать заголовки по колонкам и строкам
    QString strippedName(const QString &fullFileName);
    QString curFile;
    bool isUntitled;

    int ind_col_sortedby = -1;                              // Индекс стобца по которому была отсортирована таблица
    Qics::QicsSortOrder col_sort_type= Qics::Ascending;     // Тип сортировки столбца по которому была отсортирована таблица

    RData*  p_rdata;
    _vstr vstr_fields_form_;
    nlohmann::json j_Fields_;
    nlohmann::json j_metas_;
    nlohmann::json j_form_;
};

class CustomQicsPushButtonCellDisplay : public QicsPushButtonCellDisplay
{
    Q_OBJECT
public:
    CustomQicsPushButtonCellDisplay(QWidget *parent = 0)
        : QicsPushButtonCellDisplay(parent)	{}

    inline QicsCell *cell() const {return myCell;}
};

class CustomQicsTextCellDisplay : public QicsTextCellDisplay
{
    Q_OBJECT
public:
    CustomQicsTextCellDisplay(QWidget *parent = 0)
        : QicsTextCellDisplay(parent)	{}

    inline QicsCell *cell() const {return myEditCell;}
};

class CustomQicsTextCellDisplayHelpers : public QicsTextCellDisplay
{
    Q_OBJECT
public:
    CustomQicsTextCellDisplayHelpers(QWidget *parent = 0)
        : QicsTextCellDisplay(parent)	{}

    inline QicsCell *cell() const {return myCell;}
};


#endif// #if(!defined(QICSGRID_NO))

#endif // MDICHILD_H
