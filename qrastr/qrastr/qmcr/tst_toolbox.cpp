#include "tst_toolbox.h"
#include "ui_tst_toolbox.h"

Tst_ToolBox::Tst_ToolBox(QWidget *parent)
    //: QToolBox(parent)
    : QDialog(parent)
    , ui(new Ui::Tst_ToolBox){
    ui->setupUi(this);

    auto palette = ui->quickWidget->palette();
    palette.setColor(QPalette::Window, QColor(Qt::red));
    ui->quickWidget->setPalette(palette);
    ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
   // ui->quickWidget->setSource(QUrl("qrc:C:/projects/git_web/samples/qrastr/qrastr/qmcr/tst.qml"));
#if(defined(MSVC))
    ui->quickWidget->setSource(QUrl::fromLocalFile("C:/projects/git_web/samples/qrastr/qrastr/qmcr/tst.qml"));
#else
    ui->quickWidget->setSource(QUrl::fromLocalFile("/home/ustas/projects/git_web/samples/qrastr/qrastr/qmcr/tst.qml"));
#endif
    //ui->quickWidget->setSource(QUrl::fromLocalFile("tst.qml"));

    ui->quickWidget->show();
}

Tst_ToolBox::~Tst_ToolBox(){
    delete ui;
}
