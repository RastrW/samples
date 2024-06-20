#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QtGui>
#include <QMdiArea>
#include <QFileDialog>
#include <QAction>
#include <QStatusBar>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QDebug>
#include "mdiChildTable.h"
#include "mdiChildGrid.h"
#include "mdiChildHeaderGrid.h"
#include <iostream>
#include "astra_exp.h"
#include "fmt/format.h"
#include <QSet>
#include <QListView>
#include <QTableView>
#include "mymodel.h"



MainWindow::MainWindow()
{
    m_workspace = new QMdiArea;
    setCentralWidget(m_workspace);
    connect(m_workspace, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(updateMenus()));
    m_windowMapper = new QSignalMapper(this);
    connect(m_windowMapper, SIGNAL(mappedWidget(QWidget *)), SLOT(setActiveSubWindow(QWidget *)));
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    updateMenus();
    readSettings();
    setWindowTitle(tr("~qrastr~"));
    //int nRes = test();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#if(!defined(QICSGRID_NO))
    m_workspace->closeAllSubWindows();
    if (activeMdiChild()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
#endif// #if(!defined(QICSGRID_NO))
}

void MainWindow::newFile()
{
#if(!defined(QICSGRID_NO))
    //MdiChild *child = createMdiChild(  j_forms_[0] );
    MdiChild *child = createMdiChild(  j_forms_[0] );
    child->newFile();
    child->show();
#endif
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
#if(!defined(QICSGRID_NO))
        QMdiSubWindow *existing = findMdiChild(fileName);
        if (existing) {
            m_workspace->setActiveSubWindow(existing);
            return;
        }
#endif//#if(!defined(QICSGRID_NO))
        int nRes = 0;
        nRes = Load( id_rastr_, fileName.toStdString().c_str(), "");
        if(nRes>0){
            std::string str_msg = fmt::format( "{}: {}", tr("File loaded").toStdString(), fileName.toStdString());
            statusBar()->showMessage( str_msg.c_str(), 2000 );
        } else {
            std::string str_msg = fmt::format( "{}: {}", tr("File not loaded").toStdString(), fileName.toStdString());
            QMessageBox msgBox;
            msgBox.critical( this, tr("File not loaded"), str_msg.c_str() );

        }

        /* // ustas
        MdiChild *child = createMdiChild();
        if (child->loadFile(fileName)) {
            statusBar()->showMessage(tr("File loaded"), 2000);
            child->show();
        } else {
            child->close();
        }
        */
    }
}

void MainWindow::save()
{
#if(!defined(QICSGRID_NO))
    if (activeMdiChild()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
#endif//#if(!defined(QICSGRID_NO))
}

void MainWindow::saveAs()
{
#if(!defined(QICSGRID_NO))
    if (activeMdiChild()->saveAs())
        statusBar()->showMessage(tr("File saved"), 2000);
#endif
}

void MainWindow::cut()
{
#if(!defined(QICSGRID_NO))
    activeMdiChild()->cut();
#endif//#if(!defined(QICSGRID_NO))
}

void MainWindow::copy()
{
#if(!defined(QICSGRID_NO))
    activeMdiChild()->copy();
#endif//#if(!defined(QICSGRID_NO))
}

void MainWindow::paste()
{
#if(!defined(QICSGRID_NO))
    activeMdiChild()->paste();
#endif// #if(!defined(QICSGRID_NO))
}
void MainWindow::insertRow()
{
#if(!defined(QICSGRID_NO))
    int rowIndex = activeMdiChild()->currentCell()->rowIndex();
    if (rowIndex < 0)
        return;
    activeMdiChild()->insertRow( rowIndex );
#endif//#if(!defined(QICSGRID_NO))
}

void MainWindow::deleteRow()
{
#if(!defined(QICSGRID_NO))
     // DO NOT WORK... WHY ?
    const QicsCell * cell = activeMdiChild()->currentCell();
    activeMdiChild()->deleteRow( cell->rowIndex() );
#endif
}

void MainWindow::insertCol()
{
#if(!defined(QICSGRID_NO))
    int colIndex = activeMdiChild()->currentCell() ->columnIndex();
    if (colIndex < 0)
        return;
    activeMdiChild()->insertColumn( colIndex );
#endif //#if(!defined(QICSGRID_NO))
}

void MainWindow::deleteCol()
{
#if(!defined(QICSGRID_NO))
    const QicsCell * cell = activeMdiChild()->currentCell();
    activeMdiChild()->deleteColumn( cell->columnIndex() );
#endif // #if(!defined(QICSGRID_NO))
}

void MainWindow::rgm_wrap(){
    long res = Rgm(id_rastr_,"");

    //qDebug() << "rgm return  " << res;
    std::string str_msg = "";
    if (res == 0)
        str_msg = "Расчет режима выполнен успешно";
    else
        str_msg = "Расчет режима завершился аварийно!";
    statusBar()->showMessage( str_msg.c_str(), 0 );

    emit rgm_signal();
}
void MainWindow::about(){
   QMessageBox::about( this, tr("About QRastr"), tr("About the <b>QRastr</b>.") );
}

void MainWindow::onOpenForm( QAction* p_actn ){
    int n_indx = p_actn->data().toInt();
    //typedef std::list<CUIForm> _lstUiForms;
    auto forms = up_rastr_->GetForms();
    auto it = forms.begin();
    std::advance(it,n_indx);
    auto form  =*it;
    qDebug() << "Open form:" + form.Name();

    QTableView* ptv = new QTableView();
    MyModel* pmm = new MyModel(nullptr, *up_rastr_.get() );
    pmm->setFormIndx(n_indx);
    int nRes = pmm->populateDataFromRastr();
    ptv->setSortingEnabled(true);
    ptv->setModel(pmm);
    ptv->show();

    // return;

    // const nlohmann::json j_form = j_forms_[n_indx];
    //for Dima
    //const nlohmann::json j_form = up_rastr_->GetJForms().operator[](n_indx);

#if(!defined(QICSGRID_NO))
    const nlohmann::json j_form = up_rastr_->GetJForms()[n_indx];
    MdiChild *child = createMdiChild( j_form );
    //child->newFile();
    child->show();
#endif // #if(!defined(QICSGRID_NO))

}

//void MainWindow::SetIdrastr(_idRastr id_rastr_in){    id_rastr_ = id_rastr_in;}

/*
void MainWindow::setForms(nlohmann::json& j_forms_in){ // https://stackoverflow.com/questions/14151443/how-to-pass-a-qstring-to-a-qt-slot-from-a-qmenu-via-qsignalmapper-or-otherwise
    j_forms_ = j_forms_in;
    int i = 0;
    QMap<QString,QMenu *> map_menu;
    for(const nlohmann::json& j_form : j_forms_){
        //std::string str_Collection = j_form["Collection"];
        std::string str_MenuPath = j_form["MenuPath"];
        if (!str_MenuPath.empty() && str_MenuPath.at(0) == '_')
            continue;
        QString qstr_MenuPath = str_MenuPath.c_str();
        if (!str_MenuPath.empty() && !map_menu.contains(qstr_MenuPath))
            map_menu.insert(qstr_MenuPath,m_openMenu->addMenu(str_MenuPath.c_str()));
    }
    for(const nlohmann::json& j_form : j_forms_){
        std::string str_Name = j_form["Name"];
        std::string str_TableName = j_form["TableName"];
        std::string str_MenuPath = j_form["MenuPath"];
        QString qstr_MenuPath = str_MenuPath.c_str();
        QMenu* cur_menu = m_openMenu;
        if (map_menu.contains(qstr_MenuPath))
            cur_menu = map_menu[qstr_MenuPath];
        if (!str_Name.empty() && str_Name.at(0) != '_')
        {
            QAction* p_actn = cur_menu->addAction(str_Name.c_str());
            p_actn->setData(i);
        }
        i++;
    }
    connect( m_openMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(onOpenForm(QAction *)), Qt::UniqueConnection);
}
*/

void MainWindow::updateMenus()
{
#if(!defined(QICSGRID_NO))
    bool hasMdiChild = (activeMdiChild() != 0);
    m_saveAct->setEnabled(hasMdiChild);
    m_saveAsAct->setEnabled(hasMdiChild);
    m_pasteAct->setEnabled(hasMdiChild);
    m_closeAct->setEnabled(hasMdiChild);
    m_closeAllAct->setEnabled(hasMdiChild);
    m_tileAct->setEnabled(hasMdiChild);
    m_cascadeAct->setEnabled(hasMdiChild);
    m_nextAct->setEnabled(hasMdiChild);
    m_previousAct->setEnabled(hasMdiChild);
    m_separatorAct->setVisible(hasMdiChild);
#endif //#if(!defined(QICSGRID_NO))
}

void MainWindow::updateWindowMenu()
{
    m_windowMenu->clear();
    m_windowMenu->addAction(m_closeAct);
    m_windowMenu->addAction(m_closeAllAct);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_tileAct);
    m_windowMenu->addAction(m_cascadeAct);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(m_nextAct);
    m_windowMenu->addAction(m_previousAct);
    m_windowMenu->addAction(m_separatorAct);
    QList<QMdiSubWindow *> windows = m_workspace->subWindowList();
    m_separatorAct->setVisible(!windows.isEmpty());

#if(!defined(QICSGRID_NO))
    for (int i = 0; i < windows.size(); ++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
        if ( !child ) printf("uh oh!\n");
        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child->userFriendlyCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(child->userFriendlyCurrentFile());
        }
        QAction *action  = m_windowMenu->addAction(text);
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), m_windowMapper, SLOT(map()));
        m_windowMapper->setMapping(action, windows.at(i));
    }
#endif
}

    //QicsHeaderGrid::Foundry hf = mdiChildHeaderGrid::createHeaderGrid();


void MainWindow::setActiveSubWindow(QWidget *window)
{
    m_workspace->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::createActions()
{
    m_newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    m_newAct->setShortcut(tr("Ctrl+N"));
    m_newAct->setStatusTip(tr("Create a new file"));
    connect(m_newAct, SIGNAL(triggered()), this, SLOT(newFile()));
    m_openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    m_openAct->setShortcut(tr("Ctrl+O"));
    m_openAct->setStatusTip(tr("Open an existing file"));
    connect(m_openAct, SIGNAL(triggered()), this, SLOT(open()));
    m_saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    m_saveAct->setShortcut(tr("Ctrl+S"));
    m_saveAct->setStatusTip(tr("Save the document to disk"));
    connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));
    m_saveAsAct = new QAction(tr("Save &As..."), this);
    m_saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(m_saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    m_exitAct = new QAction(tr("E&xit"), this);
    m_exitAct->setShortcut(tr("Ctrl+Q"));
    m_exitAct->setStatusTip(tr("Exit the application"));
    connect(m_exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
    m_cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    m_cutAct->setShortcut(tr("Ctrl+X"));
    m_cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(m_cutAct, SIGNAL(triggered()), this, SLOT(cut()));
    m_copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    m_copyAct->setShortcut(tr("Ctrl+C"));
    m_copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(m_copyAct, SIGNAL(triggered()), this, SLOT(copy()));
    m_pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    m_pasteAct->setShortcut(tr("Ctrl+V"));
    m_pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(m_pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

    m_insertRowAct = new QAction( QIcon(":/images/Rastr3_grid_insrow_16x16.png"),tr( "&Insert Row" ), this );
    m_insertRowAct->setShortcut( tr( "Ctrl+I" ) );
    connect( m_insertRowAct, SIGNAL( triggered() ), this, SLOT( insertRow() ) );

    m_deleteRowAct = new QAction( QIcon(":/images/Rastr3_grid_delrow_16x16.png"), tr("&Delete Row" ), this );
    m_deleteRowAct->setShortcut( tr( "Ctrl+D" ) );
    connect( m_deleteRowAct, SIGNAL( triggered() ), this, SLOT( deleteRow() ) );

    m_insertColAct = new QAction( tr( "&Insert Column" ), this );
    m_insertColAct->setShortcut( tr( "Ctrl+K" ) );
    connect( m_insertColAct, SIGNAL( triggered() ), this, SLOT( insertCol() ) );

    m_deleteColAct = new QAction( tr( "&Delete Column" ), this );
    m_deleteColAct->setShortcut( tr( "Ctrl+L" ) );
    connect( m_deleteColAct, SIGNAL( triggered() ), this, SLOT( deleteCol() ) );

    m_closeAct = new QAction(tr("Cl&ose"), this);
    m_closeAct->setShortcut(tr("Ctrl+F4"));
    m_closeAct->setStatusTip(tr("Close the active window"));
    connect(m_closeAct, SIGNAL(triggered()),
            m_workspace, SLOT(closeActiveSubWindow()));
    m_closeAllAct = new QAction(tr("Close &All"), this);
    m_closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(m_closeAllAct, SIGNAL(triggered()),
            m_workspace, SLOT(closeAllSubWindows()));
    m_tileAct = new QAction(tr("&Tile"), this);
    m_tileAct->setStatusTip(tr("Tile the windows"));
    connect(m_tileAct, SIGNAL(triggered()), m_workspace, SLOT(tileSubWindows()));
    m_cascadeAct = new QAction(tr("&Cascade"), this);
    m_cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(m_cascadeAct, SIGNAL(triggered()), m_workspace, SLOT(cascadeSubWindows()));
    m_nextAct = new QAction(tr("Ne&xt"), this);
    m_nextAct->setShortcut(tr("Ctrl+F6"));
    m_nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(m_nextAct, SIGNAL(triggered()),
            m_workspace, SLOT(activateNextSubWindow()));
    m_previousAct = new QAction(tr("Pre&vious"), this);
    m_previousAct->setShortcut(tr("Ctrl+Shift+F6"));
    m_previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(m_previousAct, SIGNAL(triggered()),
            m_workspace, SLOT(activatePreviousSubWindow()));
    m_separatorAct = new QAction(this);
    m_separatorAct->setSeparator(true);
    m_aboutAct = new QAction(tr("&About"), this);
    m_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    m_SortAscAct = new QAction(QIcon(":/images/sortasc.png"),tr("&SortAsc"), this);
    m_SortAscAct->setShortcut(tr("Ctrl+Up"));
    m_SortAscAct->setStatusTip(tr("Sort Ascending (Ctrl+Up)"));
    connect(m_SortAscAct, SIGNAL(triggered()), this, SLOT(sortAscending()));

    m_SortDescAct = new QAction(QIcon(":/images/sortdesc.png"),tr("&SortDesc"), this);
    m_SortDescAct->setShortcut(tr("Ctrl+Down"));
    m_SortDescAct->setStatusTip(tr("Sort Descending (Ctrl+Down)"));
    connect(m_SortDescAct, SIGNAL(triggered()), this, SLOT(sortDescending()));


    m_RGMAct = new QAction(QIcon(":/images/Rastr3_rgm_16x16.png"),tr("&rgm"), this);
    m_RGMAct->setShortcut(tr("F5"));
    m_RGMAct->setStatusTip(tr("Calc rgm"));
    //connect(m_RGMAct, SIGNAL(triggered()), this, SLOT(Rgm(id_rastr_,"")));
    connect(m_RGMAct, SIGNAL(triggered()), this, SLOT(rgm_wrap()));

    ////////////////////////////// RASTR //////////////////////////
//    long nRes = 0;
//    id_rastr_ = RastrCreate();
//    nRes = Load(id_rastr_, R"(/home/ustas/projects/test-rastr/cx195.rg2)", "");
//    nRes = Rgm(id_rastr_,"");
}

void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newAct);
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_saveAsAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);
    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_cutAct);
    m_editMenu->addAction(m_copyAct);
    m_editMenu->addAction(m_pasteAct);
    m_editMenu->addAction( m_insertRowAct );
    m_editMenu->addAction( m_deleteRowAct );
    m_editMenu->addAction( m_insertColAct );
    m_editMenu->addAction( m_deleteColAct );
   // m_viewMenu = menuBar()->addMenu(tr("&View"));
    //m_editMenu->addAction(m_SortAscAct);

    m_CalcMenu = menuBar()->addMenu(tr("&Calc"));
    m_CalcMenu->addAction(m_RGMAct);

    m_openMenu = menuBar()->addMenu(tr("&Open") );
    m_windowMenu = menuBar()->addMenu(tr("&Window"));
    updateWindowMenu();
    connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));
    menuBar()->addSeparator();
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAct);
}

void MainWindow::createToolBars()
{
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->addAction(m_newAct);
    m_fileToolBar->addAction(m_openAct);
    m_fileToolBar->addAction(m_saveAct);
    m_editToolBar = addToolBar(tr("Edit"));
    m_editToolBar->addAction(m_cutAct);
    m_editToolBar->addAction(m_copyAct);
    m_editToolBar->addAction(m_pasteAct);
    m_viewToolBar = addToolBar(tr("View"));
    m_viewToolBar->addAction(m_SortAscAct);
    m_viewToolBar->addAction(m_SortDescAct);
    m_calcToolBar = addToolBar(tr("Calc"));
    m_calcToolBar->addAction(m_RGMAct);

    createCalcLayout();
}

void Btn1_onClick()
{
    QTableView* ptv = new QTableView();
    CRastrHlp rhlp;
    MyModel* pmm = new MyModel(nullptr,rhlp);
    ptv->setSortingEnabled(true);
    ptv->setModel(pmm);
    ptv->show();
}

void MainWindow::createCalcLayout()
{
    // набор вложенных виджетов - кнопок
    QPushButton *btn1 = new QPushButton("Button 1");
    QPushButton *btn2 = new QPushButton("Button 2");
    //QPushButton *btn3 = new QPushButton("Button 3");
    //QPushButton *btn4 = new QPushButton("Button 4");

    connect(btn1,&QPushButton::clicked,Btn1_onClick);

    QWidget* widget = new QWidget;
       widget -> setWindowTitle("Functions");
    m_ActionsLayout = new QHBoxLayout(widget);
    m_ActionsLayout->addWidget(btn1);
    m_ActionsLayout->addWidget(btn2);
    //m_ActionsLayout->addWidget(btn3);
   // m_ActionsLayout->addWidget(btn4);
    m_calcToolBar->addWidget(widget);
    //view->setSortingEnabled(true);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings("MDI Example");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(600, 800)).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSettings()
{
    QSettings settings("MDI Example");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

#if(!defined(QICSGRID_NO))

MdiChild *MainWindow::createMdiChild( nlohmann::json j_form )
{
    //MdiChild* child = new MdiChild(id_rastr_, j_form ,mdiChildGrid::createGrid,mdiChildHeaderGrid::createHeaderGrid,this);
    MdiChild* child = new MdiChild( up_rastr_->GetRastrId(), j_form, mdiChildGrid::createGrid,mdiChildHeaderGrid::createHeaderGrid,this);

    QObject::connect(this, SIGNAL(rgm_signal()), child, SLOT(update_data()));

    //QObject::connect(child, SIGNAL(rowsDeleted()), child, SLOT(update_data()));


    m_workspace->addSubWindow(child);
    return child;
}

MdiChild *MainWindow::activeMdiChild()
{
    if (m_workspace->activeSubWindow())
        return qobject_cast<MdiChild *>(m_workspace->activeSubWindow()->widget());
    return 0;
}

QicsTable* MainWindow::activeTable()
{
    QMdiSubWindow* activeWindow = m_workspace->activeSubWindow();
    if(!activeWindow)
        return 0;

    QicsTable *table = static_cast<QicsTable*>(activeMdiChild());
    return table;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    foreach (QMdiSubWindow *subWindow, m_workspace->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(subWindow->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
            return subWindow;
    }
    return 0;
}

void MainWindow::sortAscending()
{
    QicsTable *table = activeTable();
    if (table) {
        QicsSelectionList *list = table->selectionList(true);
        if (!list)
            return;

        QVector<int> selectedCols = list->columns().toVector();
        if (selectedCols.size() <= 0) selectedCols << 0;

        //QicsRegion reg = list->region();
        //table->sortRows(selectedCols, Qics::Ascending, reg.startRow(), reg.endRow());

        table->sortRows(selectedCols, Qics::Ascending);
    }
}

void MainWindow::sortDescending()
{
    QicsTable *table = activeTable();
    if (table) {
        QicsSelectionList *list = table->selectionList(true);
        if (!list)
            return;

        QVector<int> selectedCols = list->columns().toVector();
        if (selectedCols.size() <= 0) selectedCols << 0;

        //QicsRegion reg = list->region();
        //table->sortRows(selectedCols, Qics::Ascending, reg.startRow(), reg.endRow());

        table->sortRows(selectedCols, Qics::Descending);
    }
}

#endif //#if(!defined(QICSGRID_NO))

/*
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
*/

