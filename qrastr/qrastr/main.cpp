#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include "mainwindow.h"
#include "app.h"

#include <QtWidgets>
#include <QSplitter>
#include <QVBoxLayout>

#include "formsettingsdatas.h"
#include "formsettingsforms.h"


struct _tree_item;
using _v_tree_items = std::vector<_tree_item>;
struct _tree_item{
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
    }
    std::string      str_name;
    std::string      str_caption;
    QWidget*         pw = nullptr;
    QTreeWidgetItem* ptwi_my = nullptr;
    _v_tree_items    v_childs;
};

class Widget2 : public QWidget
{
    _tree_item ti_settings_root_{ "root", "Настройки" };
public:
    QTreeWidget* ptwSections_ = nullptr;
    void populateSettingsTree( _tree_item& ti_root, QTreeWidgetItem* ptwi_parent){
        for( auto& ti_child : ti_root.v_childs ){
            QTreeWidgetItem* ptwi_new = nullptr;
            if(nullptr == ptwi_parent) //root_items
                ptwi_new = new QTreeWidgetItem(ptwSections_);
            else
                ptwi_new = new QTreeWidgetItem();
            ti_child.ptwi_my = ptwi_new;
            static const int n_col = 0;
            ptwi_new->setText( n_col, ti_child.str_caption.c_str() );
            //ti_child.qmi = ptwi_new->treeWidget()->indexFromItem(ptwi_new,n_col);
            if(nullptr != ptwi_parent)
                ptwi_parent->addChild(ptwi_new);
            if(ti_child.v_childs.size() > 0)
                populateSettingsTree(ti_child, ptwi_new);
        }
    }
    Widget2(QWidget *parent = nullptr)
        :QWidget(parent){
        setWindowTitle("WIDGET2");
        resize(800,500);

        _tree_item ti_datas     { "datas",     "Данные" , new FormSettingsDatas()  };
        _tree_item ti_protocol  { "protocol",  "Протокол" };
        _tree_item ti_forms     { "forms",     "Формы", new FormSettingsForms()    };
            ti_forms.v_childs.emplace_back(_tree_item{ "loaded", "Загруженные" });
        _tree_item ti_templates { "on_start", "Загружаемые при старте"  };
            ti_templates.v_childs.emplace_back(_tree_item{ "templates", "Загружаемые формы"   });
            ti_templates.v_childs.emplace_back(_tree_item{ "shablons",  "Загружаемые шаблоны" });
        _tree_item ti_modules   { "modules",   "Модули"   };
        ti_settings_root_.v_childs.emplace_back( ti_datas     );
        ti_settings_root_.v_childs.emplace_back( ti_protocol  );
        ti_settings_root_.v_childs.emplace_back( ti_forms     );
        ti_settings_root_.v_childs.emplace_back( ti_templates );
        ti_settings_root_.v_childs.emplace_back( ti_modules   );

        QSplitter* splitter = new QSplitter(this);
        QVBoxLayout* layout = new QVBoxLayout();
        ptwSections_ = new QTreeWidget{};
        ptwSections_->setHeaderLabels(QStringList() << tr(ti_settings_root_.str_caption.c_str()) );
        populateSettingsTree(ti_settings_root_, nullptr);

        splitter->addWidget(ptwSections_);
        splitter->setOrientation(Qt::Orientation::Horizontal);

        //! [0]
        QWidget *firstPageWidget = new QWidget{};
        QPlainTextEdit* secondPageWidget = new QPlainTextEdit();
        secondPageWidget->setWindowTitle("TEster2");
        QWidget *thirdPageWidget = new QWidget();

        QStackedWidget *stackedWidget = new QStackedWidget;
        stackedWidget->addWidget(firstPageWidget);
        stackedWidget->addWidget(secondPageWidget);
        stackedWidget->addWidget(thirdPageWidget);
        stackedWidget->addWidget(ti_datas.pw);
        stackedWidget->addWidget(ti_forms.pw);

        splitter->addWidget(stackedWidget);
        connect( ptwSections_, &QTreeView::clicked, stackedWidget, [=]( const QModelIndex &index ){
            qDebug()<<"pc."<< index.parent().column()<< " : pr."<< index.parent().row() <<" -- c."<< index.column()<< " : r."<< index.row()
                <<" == " << index.data().toString()<< " :: " << index.internalId()
            ;
            const QTreeWidgetItem* ptiw = ptwSections_->currentItem();
            const _tree_item*      pti  = ti_settings_root_.getEq(ptiw);
            stackedWidget->setCurrentWidget(pti->pw);
        });

        layout->addWidget(splitter);
        setLayout(layout);

        QTreeWidgetItemIterator it(ptwSections_);
        while (*it) {
          if ((*it)->text(0)=="searched")
            break;
          ++it;
        }



    }// Widget2()
    void addTreeRoot(QString name, QString description){
        // QTreeWidgetItem(QTreeWidget * parent, int type = Type)
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(ptwSections_);

        // QTreeWidgetItem::setText(int column, const QString & text)
        treeItem->setText(0, name);
//        treeItem->setText(1, description);
        addTreeChild(treeItem, name + "A", "Child_first");
        addTreeChild(treeItem, name + "B", "Child_second");
    }
    void addTreeChild(QTreeWidgetItem *parent, QString name, QString description){
        // QTreeWidgetItem(QTreeWidget * parent, int type = Type)
        QTreeWidgetItem *treeItem = new QTreeWidgetItem();

        // QTreeWidgetItem::setText(int column, const QString & text)
        treeItem->setText(0, name);
//        treeItem->setText(1, description);

        // QTreeWidgetItem::addChild(QTreeWidgetItem * child)
        //parent->addChild(treeItem);
        parent->insertChild(0,treeItem);

    }
};


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
}

int main2(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/images/new.png"));

    Widget widget;
    widget.show();
    Widget2  widget2;
    widget2.show();
    return app.exec();
}



int main(int argc, char *argv[]){
    long n_res = 0;
    App app(argc, argv);

    n_res = app.init();
    if(n_res<0){
        return 100;
    }
    app.setWindowIcon(QIcon(":/images/new.png"));
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
    MainWindow w;
    n_res = app.start();
    if(n_res<0){
        return 200;
    }
    w.setForms(app.GetForms());
    w.setQAstra(app.getQAstraPtr());
    w.resize(800,500);
    w.show();
    //w.windowHandle()->setScreen(qApp->screens().last());
    //w.showNormal();
    return app.exec();
}
