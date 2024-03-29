#ifndef MDICHILD_H
#define MDICHILD_H

#include <QicsTable.h>
#include "License2/json.hpp"
#include "astra_exp.h"

class MdiChild : public QicsTable
{
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

protected:
    void closeEvent(QCloseEvent *event);
    QicsDataModel *dm;
private:
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    QString curFile;
    bool isUntitled;

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
