#include "mainwindow.h"

#include <QObject>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include <QWindow>
#include <QMessageBox>
#include <QDir>
#include <iostream>
#include <fstream>
#include "astra_exp.h"
#include "License2/json.hpp"
#include "params.h"
#include "common.h"
#include "rastrhlp.h"

int main(int argc, char *argv[])
{
    long nRes = 0;
    QApplication a(argc, argv);
    /*
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qrastr_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
*/

/*
#if(defined(COMPILE_WIN))
    nRes = Load(id_rastr, pars.Get_on_start_load_file_rastr().c_str(), "");
    if(nRes<0){
        QMessageBox mb;
        QString qstr = QObject::tr("Can't load on_start_file: ");
        std::string qstr_fmt = qstr.toUtf8().constData(); //  qstr.toStdString(); !!not worked!!
        std::string ss = fmt::format( "{}{} ", qstr_fmt.c_str(), pars.Get_on_start_load_file_rastr());
        QString str = QString::fromUtf8(ss);
        mb.setText(str);
        mb.exec();
        return 13;
    }
    //nRes = Rgm(id_rastr,"");

    CRastrHlp rhlp;
    nRes = rhlp.CreateRastr();
    nRes = rhlp.Load(pars.Get_on_start_load_file_rastr());
    nRes = rhlp.ReadForms(pars.Get_on_start_load_file_forms());
    if(nRes<0){
        QMessageBox mb( QMessageBox::Icon::Critical,
                        QObject::tr("Error"),
                        QString("error: %1 wheh read file : %2").arg(nRes).arg(pars.Get_on_start_load_file_forms().c_str())
        );
        mb.exec();
        return 15;
    }

    MainWindow w;
    w.SetIdrastr(id_rastr);
//    w.setForms(j_forms);
    w.resize(800,500);
    w.show();
    //w.windowHandle()->setScreen(qApp->screens().last());
    //w.showNormal();
    return a.exec();

#else    //#if(defined(COMPILE_WIN))

    //nRes = test();
    _idRastr id_rastr = RastrCreate();
    nRes = Load(id_rastr, R"(/home/ustas/projects/test-rastr/cx195.rg2)", "");
    nRes = Rgm(id_rastr,"");

    const int SIZE_STR_BUF = 100000;
    std::string str_json;
    str_json.resize(SIZE_STR_BUF);
    nRes = GetForms( R"(/home/ustas/Документы/RastrWin3/form/Общие.fm)", "", const_cast<char*>(str_json.c_str()), str_json.size() );
    if(nRes<0){
        QMessageBox mb;
        mb.setText("forms not loaded!");
        mb.exec();
        return 13;
    }
    nlohmann::json j_forms = nlohmann::json::parse(str_json);
    //sqDebug() << str_json.c_str();
    typedef std::vector<std::string> _vstr;
    for(const nlohmann::json& j_form : j_forms ){

        std::string str_TableName  = j_form["TableName"];
        std::string str_Name       = j_form["Name"];
        std::string str_Collection = j_form["Collection"];
        std::string str_MenuPath   = j_form["MenuPath"];
        std::string str_Query      = j_form["Query"];

        nlohmann::json j_Fields = j_form["Fields"];
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
             continue;
        }
        //qDebug() << "Meta   : " << str_json.c_str();
        nlohmann::json j_metas = nlohmann::json::parse(str_json);
        _vstr vstr_fields_meta;
        str_tmp.clear();
        for(const nlohmann::json& j_meta : j_metas ){
            std::string str_Name       = j_meta["Name"];
            vstr_fields_meta.emplace_back(str_Name);
            str_tmp += str_Name;
            str_tmp += " # ";
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
    }
    MainWindow w;
    //w.SetIdrastr(id_rastr);
    //w.setForms(j_forms);
    w.resize(800,500);
    w.show();
    //w.windowHandle()->setScreen(qApp->screens().last());
    //w.showNormal();
    return a.exec();

#endif// #if(!defined(COMPILE_WIN))
*/
    MainWindow w;
    w.resize(800,500);
    w.show();
    //w.windowHandle()->setScreen(qApp->screens().last());
    //w.showNormal();
    return a.exec();
}
