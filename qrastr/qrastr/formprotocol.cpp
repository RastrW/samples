#include <QListView>
#include <QStringListModel>
#include <QAbstractItemModelTester>

#include "formprotocol.h"
#include "protocoltreeitem.h"
#include "protocoltreemodel.h"
#include "ui_formprotocol.h"
#include "qastra_events_data.h"

FormProtocol::FormProtocol(QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::FormProtocol){
    ui->setupUi(this);
    p_protocol_tree_model_ = new ProtocolTreeModel();
    ui->twProtocol->setModel(p_protocol_tree_model_);
    s_spti_stages_.emplace( p_protocol_tree_model_->getRootItemSp() );
}
FormProtocol::~FormProtocol(){
    delete ui;
}
//https://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
void FormProtocol::onAppendProtocol(const QString& qstr){
    auto sp_item = std::make_shared<ProtocolTreeItem>( QVariantList{
        QString("protocol"), qstr
        }, s_spti_stages_.top().get() );
    s_spti_stages_.top().get()->appendChild(sp_item);
    return;
}
void FormProtocol::onRastrLog(const _log_data& log_data){
    QString qstr2 ;
    p_protocol_tree_model_->layoutAboutToBeChanged();
    //https://forum.qt.io/topic/87721/index-for-begininsertrows-with-qtreeview/9
    //https://wiki.qt.io/Model_Test
    //beginInsertRows(parent, 2, 4);
    //endInsertRows();
    if( LogMessageTypes::OpenStage == log_data.lmt ){
        auto sp_item = std::make_shared<ProtocolTreeItem>( QVariantList{
            QString("STAGE!!%1").arg(log_data.n_stage_id), log_data.str_msg.c_str()
            },s_spti_stages_.top().get() );
        s_spti_stages_.top()->appendChild(sp_item);
        s_spti_stages_.emplace(sp_item);
    }else if( LogMessageTypes::CloseStage == log_data.lmt ){
        if(1 < s_spti_stages_.size()){
            s_spti_stages_.pop();
        }else{
            assert(!"trying to close Root stage!");
        }
    }else{
        QString qstr_type{};
        switch(log_data.lmt){
            case LogMessageTypes::SystemError:  qstr_type = "SYSTEM-ERROR"; break;
            case LogMessageTypes::Failed:       qstr_type = "FAILED"; break;
            case LogMessageTypes::Error:        qstr_type = "ERROR"; break;
            case LogMessageTypes::Warning:      qstr_type = "Warning"; break;
            case LogMessageTypes::Message:      qstr_type = "msg"; break;
            case LogMessageTypes::Info:         qstr_type = "inf"; break;
            case LogMessageTypes::EnterDefault: qstr_type = "EnterDefault"; break;
            case LogMessageTypes::Reset:        qstr_type = "RESET!"; break;
            case LogMessageTypes::None:         qstr_type = "None"; break;
            case LogMessageTypes::OpenStage:
            case LogMessageTypes::CloseStage:
            default:                            qstr_type = "InnerFail"; break;
        }
        auto sp_item = std::make_shared<ProtocolTreeItem>( QVariantList{
            qstr_type, log_data.str_msg.c_str()
            } , s_spti_stages_.top().get() );
        s_spti_stages_.top()->appendChild(sp_item);
    }
    p_protocol_tree_model_->layoutChanged();
    new QAbstractItemModelTester( p_protocol_tree_model_, QAbstractItemModelTester::FailureReportingMode::Fatal, this);
}
