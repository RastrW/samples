#include "formsettings.h"

#include <QSplitter>
#include <QVBoxLayout>

#include <filesystem>
#include "qastra.h"
#include "params.h"
#include "common_qrastr.h"
#include "formsettingsdatas.h"
#include "formsettingsforms.h"
#include "formsettingsonloadfiles.h"
#include "formsettingsonloadtemplates.h"
#include "formsettingsonloadforms.h"

struct FormSettings::_tree_item{
    _tree_item( const std::string_view& sv_name_in, const std::string_view& sv_caption_in, QWidget* pw_show = nullptr )
        : str_name(sv_name_in)
        , str_caption(sv_caption_in)
        , pw(pw_show){
        if(nullptr == pw_show)
            pw_show = new QWidget();
    }
    _tree_item* getEq( const QTreeWidgetItem* ptwi){
        if(ptwi_my == ptwi)
            return this;
        for( auto& ti_child : v_childs ){
           _tree_item* ptwi_out = ti_child.getEq(ptwi);
           if(nullptr != ptwi_out)
               return ptwi_out;
        }
        return nullptr;
    }
    std::string      str_name;
    std::string      str_caption;
    QWidget*         pw = nullptr;
    QTreeWidgetItem* ptwi_my = nullptr;
    _v_tree_items    v_childs;
};
/*
class Widget : public QWidget
{
public:
    Widget(QWidget *parent = nullptr);
};
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
//! [0]
    QWidget *firstPageWidget = new QWidget{};
    //QWidget *secondPageWidget = new QWidget;
    QPlainTextEdit* secondPageWidget = new QPlainTextEdit();
    //QWidget *secondPageWidget = new QEditLin;
    QWidget *thirdPageWidget = new QWidget();

    QStackedWidget *stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(firstPageWidget);
    stackedWidget->addWidget(secondPageWidget);
    stackedWidget->addWidget(thirdPageWidget);

//! [0] //! [1]
    QComboBox *pageComboBox = new QComboBox;
    pageComboBox->addItem(tr("Page 1"));
    pageComboBox->addItem(tr("Page 2"));
    pageComboBox->addItem(tr("Page 3"));
    connect(pageComboBox, &QComboBox::activated,
            stackedWidget, &QStackedWidget::setCurrentIndex);
//! [1] //! [2]
    QVBoxLayout *layout = new QVBoxLayout;
//! [2]
    layout->addWidget(pageComboBox);
//! [3]
    layout->addWidget(stackedWidget);
    setLayout(layout);
//! [3]
    QTreeWidgetItemIterator it(ptwSections_);
    while (*it) {
      if ((*it)->text(0)=="searched")
        break;
      ++it;
    }
}
*/
void FormSettings::populateSettingsTree( _tree_item& ti_root, QTreeWidgetItem* ptwi_parent){
    for( auto& ti_child : ti_root.v_childs ){
        QTreeWidgetItem* ptwi_new = nullptr;
        if(nullptr == ptwi_parent) //root_items
            ptwi_new = new QTreeWidgetItem(ptw_sections_);
        else
            ptwi_new = new QTreeWidgetItem();
        ti_child.ptwi_my = ptwi_new;//for search by this pointer in click() handler
        static const int n_col = 0;
        ptwi_new->setText( n_col, ti_child.str_caption.c_str() );
        if(nullptr != ptwi_parent)
            ptwi_parent->addChild(ptwi_new);
        if(ti_child.v_childs.size() > 0)
            populateSettingsTree(ti_child, ptwi_new);
    }
}
FormSettings::FormSettings(QWidget *parent)
    : QWidget{parent} {
    setWindowTitle("Settings");
    resize(800,500);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    //setWindowModality(Qt::WindowModal);
    setWindowModality(Qt::ApplicationModal);
    QSplitter* splitter = new QSplitter(this);
    QVBoxLayout* layout = new QVBoxLayout();
    ptw_sections_ = new QTreeWidget{};
    splitter->addWidget(ptw_sections_);
    splitter->setOrientation(Qt::Orientation::Horizontal);
    psw_ = new QStackedWidget;
    splitter->addWidget(psw_);
    layout->addWidget(splitter);
    ppb_save_settings_ = new QPushButton();
    //setButtonSaveEnabled(false);
    ppb_save_settings_->setText(QString("%1 : %2").arg("Save: ").arg(Params::get_instance()->getFileAppsettings().string().c_str()));
    connect( ppb_save_settings_, &QPushButton::clicked, this, &FormSettings::onBtnSaveClick );
    layout->addWidget(ppb_save_settings_);
    setLayout(layout);
}
void FormSettings::closeEvent(QCloseEvent *event){
     if (event->spontaneous()) {
        spdlog::info("The close button was clicked");
        if(bl_app_settings_chnged_ == true){
            QMessageBox msgBox;
            msgBox.setText(tr("Appsettings changed, but not saved in the file. Ignore and close dialog?"));
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::Cancel );
            msgBox.setDefaultButton(QMessageBox::Yes);
            if(msgBox.exec() != QMessageBox::Yes){
                event->ignore();
                return;
            }
        }
     }else{
        QWidget::closeEvent(event);
     }
 }
void FormSettings::setButtonSaveEnabled(bool bl_new_val){
    ppb_save_settings_->setEnabled(bl_new_val);
}
void FormSettings::setAppSettingsChanged(){
    bl_app_settings_chnged_ = true;
}
void FormSettings::onBtnSaveClick(){
    //Params::GetInstance()->Get_on_start_load_file_forms();
    Params* p_params = Params::get_instance();
    const std::filesystem::path path_appsettings = p_params->getFileAppsettings();
    if(std::filesystem::exists(path_appsettings)){
        QMessageBox msgBox;
        msgBox.setText(tr("File existed.Overwrite?"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
        msgBox.setDefaultButton(QMessageBox::No);
        if(QMessageBox::Yes == msgBox.exec()){
            std::filesystem::path path_appsettings_cpy = path_appsettings;
            path_appsettings_cpy.replace_filename(path_appsettings.stem().string()+"_previos"+path_appsettings.extension().string());
            std::filesystem::copy(path_appsettings, path_appsettings_cpy, std::filesystem::copy_options::overwrite_existing);
            Params::get_instance()->writeJsonFile(path_appsettings.string());
            bl_app_settings_chnged_ = false;
        }
    }
}
int FormSettings::init(const std::shared_ptr<QAstra>& sp_qastra){
    int n_res = 0;
    sp_qastra_ = sp_qastra;
    n_res = Params::get_instance()->readTemplates
            ( Params::get_instance()->getDirSHABLON().absolutePath().toStdString() );
    assert(n_res>0);

    pti_settings_root_ = new _tree_item{"root","Настройки"};
    _tree_item ti_datas     { "datas",     "Данные" , new FormSettingsDatas()  };
    _tree_item ti_protocol  { "protocol",  "Протокол" };
    _tree_item ti_on_start { "on_start", "Загружаемые при старте"  };
        _tree_item  ti_onload_templates{ "templates",  "Шаблоны", new FormSettingsOnLoadTemplates() };
        ti_on_start.v_childs.emplace_back(ti_onload_templates);
        _tree_item  ti_onload_forms{ "forms",  "Формы", new FormSettingsOnLoadForms() };
        ti_on_start.v_childs.emplace_back(ti_onload_forms);
        _tree_item ti_on_load_files{ "templates", "Файлы", new FormSettingsOnLoadFiles(this) };
        ti_on_start.v_childs.emplace_back(ti_on_load_files);
    _tree_item ti_modules   { "modules",   "Модули"   };
    pti_settings_root_->v_childs.emplace_back( ti_datas     );
    pti_settings_root_->v_childs.emplace_back( ti_protocol  );
    pti_settings_root_->v_childs.emplace_back( ti_on_start );
    pti_settings_root_->v_childs.emplace_back( ti_modules   );

    ptw_sections_->setHeaderLabels( QStringList() << tr(pti_settings_root_->str_caption.c_str()) );
    populateSettingsTree( *pti_settings_root_, nullptr );
    QWidget *firstPageWidget = new QWidget();
    QPlainTextEdit* secondPageWidget = new QPlainTextEdit();
    secondPageWidget->setWindowTitle("Tester2");
    QWidget *thirdPageWidget = new QWidget();
    psw_->addWidget(firstPageWidget);
    psw_->addWidget(secondPageWidget);
    psw_->addWidget(thirdPageWidget);
    psw_->addWidget(ti_datas.pw);
    psw_->addWidget(ti_on_load_files.pw);
    psw_->addWidget(ti_onload_templates.pw);
    psw_->addWidget(ti_onload_forms.pw);

    connect( ptw_sections_, &QTreeView::clicked, psw_, [this]( const QModelIndex &index ){
        qDebug()<<"pc."<< index.parent().column()<< " : pr."<< index.parent().row() <<" -- c."<< index.column()<< " : r."<< index.row()
            <<" == " << index.data().toString()<< " :: " << index.internalId();

        const QTreeWidgetItem* ptiw = this->ptw_sections_->currentItem();
        const _tree_item*      pti  = this->pti_settings_root_->getEq(ptiw);//strange to look by pointer, but it works
        this->psw_->setCurrentWidget(pti->pw);
    });
    class Header //from https://stackoverflow.com/questions/10082028/how-to-add-a-button-or-other-widget-in-qtreewidget-header
        : public QHeaderView
    {
        public:
        Header(QWidget* parent)
            : QHeaderView(Qt::Horizontal, parent)
            , pb_save_(new QPushButton("Save", this)){
        }
        private:
        QPushButton* pb_save_ = nullptr;
    };

    return 1;
}
