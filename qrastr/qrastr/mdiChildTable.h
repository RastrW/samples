#ifndef MDICHILD_H
#define MDICHILD_H

#include <QicsTable.h>
#include "License2/json.hpp"
#include "astra_exp.h"
#include "rastrdatamodel.h"

class MdiChild : public QicsTable
{
    typedef std::vector<std::string> _vstr;

    Q_OBJECT
public:
    MdiChild( const _idRastr id_rastr, const nlohmann::json& j_form_in );
    void newFile();
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    void writeFile(QString fname);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }
    void mousePressEvent(QMouseEvent *event) override ;


public slots:
    void update_data();
    void insertRows();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void closeEvent(QCloseEvent *event) override;
   // bool eventFilter(QObject *obj, QEvent *ev) override;
    QicsDataModel *dm;
private:
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    QString curFile;
    bool isUntitled;

    RData*  p_rdata;
    _vstr vstr_fields_form_;
    nlohmann::json j_Fields_;
    nlohmann::json j_metas_;


    nlohmann::json j_form_;
};

/*
class MdiChild
{
public:
    MdiChild();
};
*/

#endif // MDICHILD_H
