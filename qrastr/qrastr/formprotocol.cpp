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
    // определяем данные для модели
    QStringList list = { "Tom", "Bob", "Sam" };
    // определяем модель
    pmodel_ = new QStringListModel(list);
    //ui->twProtocol->setModel(pmodel_);
    p_protocol_tree_model_ = new ProtocolTreeModel();
    ui->twProtocol->setModel(p_protocol_tree_model_);
    //new ProtocolTreeModel();
    spti_stage_ = p_protocol_tree_model_->getRootItemSp();
    spti_stage_previos_ = spti_stage_;
    s_spti_stages_.emplace( p_protocol_tree_model_->getRootItemSp() );
}
FormProtocol::~FormProtocol(){
    delete ui;
}
//https://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
void FormProtocol::onAppendProtocol(const QString& qstr){
    auto sp_item = std::make_shared<ProtocolTreeItem>( QVariantList{
        QString("row%1").arg(n_row_counter_), qstr
        },s_spti_stages_.top().get() );
        //}, spti_stage_previos_.get() );
    //spti_stage_->appendChild(sp_item);
    s_spti_stages_.top().get()->appendChild(sp_item);
    return;

    //std::string str{qstr.toStdString()};
    std::string str{""};
    str += qstr.toStdString();
    //encode(str);
    str += "\n";
    //shProt_->my_appendTect(str);
    if(pmodel_->insertRow(pmodel_->rowCount())) {
        QModelIndex index = pmodel_->index(pmodel_->rowCount() - 1, 0);
        pmodel_->setData(index, qstr);
    }else{
        assert(!"cant insert row in Protocol");
    }
    //QVariant qv = qstr;
    QVariantList qvl;
    qvl<< qstr;
    ProtocolTreeItem* pti_new = new ProtocolTreeItem(qvl);
    //p_protocol_tree_model_->in

    //if( p_protocol_tree_model_->insertRow( p_protocol_tree_model_->rowCount() ) ){
    //    QModelIndex qindx = p_protocol_tree_model_->index( p_protocol_tree_model_->rowCount()-1,0);
    //    p_protocol_tree_model_->setData(qindx,qstr);
    //}
    //p_protocol_tree_model_->getRootItem()->appendChild(std::make_unique<ProtocolTreeItem>(qvl));

    for(int i = 0 ; i < 1 ; i++, n_row_counter_++){
        //auto item = std::make_unique<ProtocolTreeItem>( QVariantList{tr(QString("row%1").arg(n_row_counter_).toUtf8().constData()), qstr.toUtf8().constData()}, p_protocol_tree_model_->getRootItem() );
        auto item = std::make_shared<ProtocolTreeItem>( QVariantList{QString("row%1").arg(n_row_counter_), qstr}, p_protocol_tree_model_->getRootItem() );

        ProtocolTreeItem* p_item_prev = item.get();
        for(int j = 0 ; j < 10 ; j++){
            QString qstr2 = qstr;
            qstr2.insert(0,"itemm-2-");
            //auto item2 = std::make_unique<ProtocolTreeItem>( QVariantList{tr(QString("item-2-row%1").arg(n_row_counter_).toUtf8().constData()), qstr2.toUtf8().constData()}, item.get() );
            auto item2 = std::make_shared<ProtocolTreeItem>( QVariantList{QString("item-2-row%1").arg(n_row_counter_), qstr2}, item.get() );
            p_item_prev = item2.get();
            qstr2.insert(0,"itemm-3-");
            //auto item3 = std::make_unique<ProtocolTreeItem>( QVariantList{tr(QString("item-3-row%1").arg(n_row_counter_).toUtf8().constData()), qstr2.toUtf8().constData()}, item2.get() );
            auto item3 = std::make_shared<ProtocolTreeItem>( QVariantList{QString("item-3-row%1").arg(n_row_counter_), qstr2}, item2.get() );
            item2->appendChild(item3);
            item->appendChild((item2));
        }
        //item->appendChild();
        p_protocol_tree_model_->getRootItem()->appendChild( item );
        //p_protocol_tree_model_->getRootItem()->appendChild(std::make_unique<ProtocolTreeItem>( QVariantList{tr(QString("row%1").arg(n_row_counter_).toUtf8().constData()), qstr.toUtf8().constData()}, p_protocol_tree_model_->getRootItem() ));
    }
    //p_protocol_tree_model_->setData()
    //p_protocol_tree_model_->se
}
void FormProtocol::onRastrLog(const _log_data& log_data){
    QString qstr2 ;
    p_protocol_tree_model_->layoutAboutToBeChanged();
//https://forum.qt.io/topic/87721/index-for-begininsertrows-with-qtreeview/9
//https://wiki.qt.io/Model_Test
    //beginInsertRows(parent, 2, 4);
    //endInsertRows();
 //   p_protocol_tree_model_->beginInsertRows(QModelIndex(),spti_stage_->childCount(), spti_stage_->childCount()+1);
    //auto item = std::make_shared<ProtocolTreeItem>( QVariantList{tr(QString("row%1").arg(n_row_counter_).toUtf8().constData()), qstr.toUtf8().constData()}, p_protocol_tree_model_->getRootItem() );
    std::string str = "";
    if( LogMessageTypes::OpenStage == log_data.lmt ){
        str += "<STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\t";
        str += log_data.str_msg;
        str += "\n";

        auto sp_item = std::make_shared<ProtocolTreeItem>( QVariantList{
            QString("STAGE!!%1").arg(n_row_counter_), log_data.str_msg.c_str()
            //}, spti_stage_.get() );
            },s_spti_stages_.top().get() );
        //spti_stage_->appendChild(sp_item);

    //    spti_stage_previos_ = spti_stage_;
     //   spti_stage_ = sp_item;

        //spti_stage_previos_->appendChild(sp_item);
        s_spti_stages_.top()->appendChild(sp_item);
        s_spti_stages_.emplace(sp_item);
        //spti_stage_ = std::make_shared<ProtocolTreeItem>( QVariantList{tr(QString("item-3-row%1").arg(n_row_counter_).toUtf8().constData()), qstr2.toUtf8().constData()}, item2.get() );
        //shProt_->my_appendTect(str);
        //n_stage_max_id_ = log_data.n_stage_id;
    }else if( LogMessageTypes::CloseStage == log_data.lmt ){
        str += "</STAGE";
        str += std::to_string(log_data.n_stage_id);
        str += ">\n";


   //     spti_stage_ = spti_stage_previos_;
        s_spti_stages_.pop();

        //shProt_->my_appendTect(str);
        //assert(n_stage_max_id_ == log_data.n_stage_id);
        //if(n_stage_max_id_ == log_data.n_stage_id){
        //    n_stage_max_id_--;
        //}
    }else {

        auto sp_item = std::make_shared<ProtocolTreeItem>( QVariantList{
            QString("msg!!%1").arg(n_row_counter_), log_data.str_msg.c_str()
           // }, spti_stage_.get() );
             },s_spti_stages_.top().get() );
        //spti_stage_->appendChild(sp_item);

        //spti_stage_previos_ = spti_stage_;
        //spti_stage_ = sp_item;

        //spti_stage_->appendChild(sp_item);
        s_spti_stages_.top()->appendChild(sp_item);

    }
//    p_protocol_tree_model_->endInsertRows();
//    p_protocol_tree_model_->rowsInserted(QModelIndex{}, 0,0);
     p_protocol_tree_model_->layoutChanged();
    new QAbstractItemModelTester( p_protocol_tree_model_, QAbstractItemModelTester::FailureReportingMode::Fatal, this);
    //emit dataChanged();
}
