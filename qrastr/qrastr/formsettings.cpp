#include "formsettings.h"

#include <QSplitter>
#include <QVBoxLayout>

#include "formsettingsdatas.h"
#include "formsettingsforms.h"

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
    :QWidget{parent}{
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
    setLayout(layout);
}
int FormSettings::init(){
    pti_settings_root_ = new _tree_item{"root","Настройки"};
    _tree_item ti_datas     { "datas",     "Данные" , new FormSettingsDatas()  };
    _tree_item ti_protocol  { "protocol",  "Протокол" };
    _tree_item ti_forms     { "forms",     "Формы", new FormSettingsForms()    };
        ti_forms.v_childs.emplace_back(_tree_item{ "loaded", "Загруженные" });
    _tree_item ti_templates { "on_start", "Загружаемые при старте"  };
        ti_templates.v_childs.emplace_back(_tree_item{ "templates", "Загружаемые формы"   });
        ti_templates.v_childs.emplace_back(_tree_item{ "shablons",  "Загружаемые шаблоны" });
    _tree_item ti_modules   { "modules",   "Модули"   };
    pti_settings_root_->v_childs.emplace_back( ti_datas     );
    pti_settings_root_->v_childs.emplace_back( ti_protocol  );
    pti_settings_root_->v_childs.emplace_back( ti_forms     );
    pti_settings_root_->v_childs.emplace_back( ti_templates );
    pti_settings_root_->v_childs.emplace_back( ti_modules   );

    ptw_sections_->setHeaderLabels( QStringList() << tr(pti_settings_root_->str_caption.c_str()) );
    populateSettingsTree( *pti_settings_root_, nullptr );
    QWidget *firstPageWidget = new QWidget{};
    QPlainTextEdit* secondPageWidget = new QPlainTextEdit();
    secondPageWidget->setWindowTitle("TEster2");
    QWidget *thirdPageWidget = new QWidget();
    psw_->addWidget(firstPageWidget);
    psw_->addWidget(secondPageWidget);
    psw_->addWidget(thirdPageWidget);
    psw_->addWidget(ti_datas.pw);
    psw_->addWidget(ti_forms.pw);

    //connect( ptw_sections_, &QTreeView::clicked, psw_, [=]( const QModelIndex &index ){
    //connect( ptw_sections_, &QTreeView::clicked, psw_, [this]( const QModelIndex &index ){
    connect( ptw_sections_, &QTreeView::clicked, psw_, [this]( const QModelIndex &index ){
        qDebug()<<"pc."<< index.parent().column()<< " : pr."<< index.parent().row() <<" -- c."<< index.column()<< " : r."<< index.row()
            <<" == " << index.data().toString()<< " :: " << index.internalId()
        ;

        //const QStandardItemModel *model =            qobject_cast<QStandardItemModel *>( ptw_sections_->model() );
        //const QStandardItem *item = model->itemFromIndex( index );

        const QTreeWidgetItem* ptiw = this->ptw_sections_->currentItem();
        const _tree_item*      pti  = this->pti_settings_root_->getEq(ptiw);//strange to look by pointer, but it works
        this->psw_->setCurrentWidget(pti->pw);
    });
    return 1;
}
