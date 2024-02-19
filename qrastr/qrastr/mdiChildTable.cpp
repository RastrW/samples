
#include <QtGui>
#include <QFileDialog>

#include "mdiChildTable.h"
#include <QicsDataModelDefault.h>
#include "rastrdatamodel.h"

MdiChild::MdiChild( const _idRastr id_rastr, const nlohmann::json& j_form_in )
{
    setAttribute(Qt::WA_DeleteOnClose);
    /*
    dm = new QicsDataModelDefault(10,10);

    dm = new RastrDataModel();
    this->setDataModel(dm);


    ///////////////////////////////example.begin
    // create the data model
    StockDataModel *dm = new StockDataModel();
    // fill the data model with some values
    dm->insertStock(-1);
    dm->setSymbol(0, "ATT");
    dm->setClose(0, 37.73);
    dm->setHigh(0, 38.0);
    dm->setLow(0, 37.55);
    dm->setVolume(0, 503333);

    dm->insertStock(-1);
    dm->setSymbol(1, "RJR");
    dm->setClose(1, 67.05);
    dm->setHigh(1, 67.05);
    dm->setLow(1, 64.89);
    dm->setVolume(1, 997323);
    ///////////////////////////////example.end.

    //this->setDataModel(dm);
*/

    //this->setDataModel(new QicsDataModelDefault);

    int nRes = 0;
    const int SIZE_STR_BUF = 100'000;
    std::string str_json;
    str_json.resize(SIZE_STR_BUF);
    //nRes = GetForms( R"(/home/ustas/Документы/RastrWin3/form/poisk.fm)", "", const_cast<char*>(str_json.c_str()), str_json.size() );
    nRes = GetForms( R"(/home/ustas/Документы/RastrWin3/form/Общие.fm)", "", const_cast<char*>(str_json.c_str()), str_json.size() );
    nlohmann::json j_forms = nlohmann::json::parse(str_json);
    //sqDebug() << str_json.c_str();
    typedef std::vector<std::string> _vstr;
    std::string str_TableName  = j_form_in["TableName"];
    std::string str_Name       = j_form_in["Name"];
    std::string str_Collection = j_form_in["Collection"];
    std::string str_MenuPath   = j_form_in["MenuPath"];
    std::string str_Query      = j_form_in["Query"];

    nlohmann::json j_Fields = j_form_in["Fields"];
    _vstr vstr_fields_form;
    std::string str_tmp;
    for(const nlohmann::json& j_field : j_Fields){
        vstr_fields_form.emplace_back(j_field);
        str_tmp += j_field;
        str_tmp += " # ";
    }
    qDebug() << "Table  : " << str_TableName.c_str() << "|" << QString::fromUtf8( str_Name.c_str()) << "|" <<  str_Collection.c_str() << "|" << str_MenuPath.c_str() << "|" << str_Query.c_str();
    qDebug() << "Fields : " << str_tmp.c_str();
    nRes = GetMeta( id_rastr, str_TableName.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
    if(nRes<0){
         qDebug() << "GetMeta(...)  return error" << nRes;

    }
    RData rdata;
    //qDebug() << "Meta   : " << str_json.c_str();
    nlohmann::json j_metas = nlohmann::json::parse(str_json);
    _vstr vstr_fields_meta;
    str_tmp.clear();
    for(const nlohmann::json& j_meta : j_metas ){
        std::string str_Name       = j_meta["Name"];
        vstr_fields_meta.emplace_back(str_Name);
        str_tmp += str_Name;
        str_tmp += " # ";

        RCol rc;
        rc.str_name_ = str_Name;
        nRes = rdata.AddCol(rc); Q_ASSERT(nRes>=0);
    }
    qDebug() << "FieldsBd: " << str_tmp.c_str();
    _vstr vstr_fields_distilled;
    str_tmp.clear();
    for(std::string& str_field_form : vstr_fields_form){
        _vstr::const_iterator iter_vstr_fields_meta =
            std::find(vstr_fields_meta.begin(), vstr_fields_meta.end(), str_field_form );
        if(iter_vstr_fields_meta != vstr_fields_meta.end()){
            vstr_fields_distilled.emplace_back(str_field_form);
            str_tmp += str_field_form;
            str_tmp += ",";
        }
    }
    if(str_tmp.length()>0){
        str_tmp.erase(str_tmp.length()-1);
        nRes = GetJSON( id_rastr, str_TableName.c_str(), str_tmp.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.length() );
        qDebug() << "Data: " << str_json.c_str();
    }

    nlohmann::json j_data_arr = nlohmann::json::parse(str_json);

    size_t sz_num_cols = vstr_fields_distilled.size();
    size_t sz_num_rows = j_data_arr.size();
    int n_col_num = 0;
    for(const nlohmann::json& j_meta : j_metas ){
        std::string str_Name       = j_meta["Name"];

        RCol rc;
        rc.str_name_ = str_Name;
        nRes = rdata.AddCol(rc); Q_ASSERT(nRes>=0);
        n_col_num++;
    }
    /*
    for(const nlohmann::json& j_row_arr : j_data_arr){
        n_col_num = 0;
        for(const nlohmann::json& j_data : j_row_arr){
            n_col_num++;
        }

    }
*/
    //rdata_.SetNumRows(rc_int.size());
    //nRes = rdata_.AddCol(rc_int); Q_ASSERT(nRes>=0);


    dm = new RastrDataModel();
    this->setDataModel(dm);

    isUntitled = true;
}

void MdiChild::newFile()
{
    static int sequenceNumber = 1;
    isUntitled = true;
    curFile = tr("qicstable%1.dat").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");
    setWindowIcon(QIcon(":/images/new.png"));
}

bool MdiChild::loadFile(const QString &fileName)
{
    // Create the stream from the file, read into data model
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);
        dm->readASCII(stream, ',');
    }
    curFile = fileName;
    isUntitled = false;
    return true;
}

bool MdiChild::save()
{
    if (isUntitled == true)
    {
        return saveAs();
    }
    else
    {
        return saveFile(curFile);
    }
}

bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), curFile);
    if (fileName.isEmpty())
    {
        return false;
    }
    return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
    if (fileName != QString())
    {
        writeFile(fileName);
        isUntitled = false;
        return true;
    }
    return false;
}

void MdiChild::writeFile(QString fname)
{
    // Create the stream from the file, read into data model
    QFile file(fname);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        dm->writeASCII(stream, ',', 0, 0, 10, 10);
    }
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void MdiChild::closeEvent(QCloseEvent *event)
{
    event = event;
    /*if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }*/
}

void MdiChild::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

QString MdiChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}



/*#include "mdiChildTable.h"

MdiChild::MdiChild()
{

}*/
