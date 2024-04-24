
#include <QtGui>
#include <QFileDialog>

#include "mdiChildTable.h"
#include <QicsDataModelDefault.h>
#include "rastrdatamodel.h"
#include <QToolButton>
#include <QicsNavigator.h>
#include <QicsRegionalAttributeController.h>


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
    const int SIZE_STR_BUF = 500'000'000;
    std::string str_json;
    str_json.resize(SIZE_STR_BUF);
    //nRes = GetForms( R"(/home/ustas/Документы/RastrWin3/form/poisk.fm)", "", const_cast<char*>(str_json.c_str()), str_json.size() );
    nRes = GetForms( R"(/home/ustas/Документы/RastrWin3/form/Общие.fm)", "", const_cast<char*>(str_json.c_str()), str_json.size() );

    size_t sz = str_json.size();
    size_t ln = str_json.length();
    size_t ln1 = std::strlen(str_json.c_str());
    str_json.resize(std::strlen(str_json.c_str())+1);
    nlohmann::json j_forms = nlohmann::json::parse(str_json);
    //sqDebug() << str_json.c_str();
    //typedef std::vector<std::string> _vstr;
    std::string str_TableName  = j_form_in["TableName"];
    std::string str_Name       = j_form_in["Name"];
    std::string str_Collection = j_form_in["Collection"];
    std::string str_MenuPath   = j_form_in["MenuPath"];
    std::string str_Query      = j_form_in["Query"];

    //nlohmann::json j_Fields = j_form_in["Fields"];
    j_Fields_ = j_form_in["Fields"];
    //_vstr vstr_fields_form;
    std::string str_tmp;
    for(const nlohmann::json& j_field : j_Fields_){
        vstr_fields_form_.emplace_back(j_field);
        str_tmp += j_field;
        str_tmp += " # ";
    }
    qDebug() << "Table  : " << str_TableName.c_str() << "|" << QString::fromUtf8( str_Name.c_str()) << "|" <<  str_Collection.c_str() << "|" << str_MenuPath.c_str() << "|" << str_Query.c_str();
    qDebug() << "Fields : " << str_tmp.c_str();
    str_json.resize(SIZE_STR_BUF);
    nRes = GetMeta( id_rastr, str_TableName.c_str(), "", const_cast<char*>(str_json.c_str()), str_json.size() );
    if(nRes<0){
         qDebug() << "GetMeta(...)  return error" << nRes;
    }
    qDebug() << "Meta   : " << str_json.c_str();
    str_json.resize(std::strlen(str_json.c_str())+1);
    //nlohmann::json j_metas = nlohmann::json::parse(str_json);
    j_metas_ = nlohmann::json::parse(str_json);

    //RData*  p_rdata = new RData(id_rastr,str_TableName);
    p_rdata = new RData(id_rastr,str_TableName);

    p_rdata->Initialize(j_Fields_,j_metas_,vstr_fields_form_);
    p_rdata->populate(j_Fields_,j_metas_,vstr_fields_form_);

    //mainGridRef().setDisplayer(new SpreadsheetCellDisplay(this));

    setSelectionStyle(Qics::Exclusive);
    setCellWidthMode(Qics::ChangeHeightWidth);

    //Навигатор данных Grid -> кнопка справа снизу
    setNavigatorAllowed(true);
    navigator()->setText("#");
    navigator()->setToolTip("Press and hold to navigate throungh the grid");

    QicsRegionalAttributeController controller;
    setExternalAttributeController(controller);

    setWideKeyActions();

    //Выбрать все ячейки таблицы -> кнопка слева сверху
    QToolButton *bSelectAll = new QToolButton(this);
    bSelectAll->setText("*");
    bSelectAll->setToolTip("Select all cells");
    bSelectAll->setFixedSize(18,18);
    connect(bSelectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
    setTopLeftCornerWidget(bSelectAll);



    dm = new RastrDataModel(*p_rdata);
    this->setDataModel(dm);

    // Некоторые настройки отображения
    columnHeaderRef().setFrameStyle(QFrame::Panel);
    rowHeaderRef().setFrameStyle(QFrame::Panel);
    mainGridRef().setSelectedBackgroundColor(QColor(0x92a2b9));
    rowHeaderRef().setSelectedBackgroundColor(QColor(0x92a2d9));
    rowHeaderRef().setSelectedForegroundColor(Qt::white);
    columnHeaderRef().setSelectedBackgroundColor(QColor(0x92a2d9));
    columnHeaderRef().setSelectedForegroundColor(Qt::white);
    setExclusiveSelectionDragBackColor(QColor(Qt::transparent));

    setCellOverflowBehavior(Qics::ToolTip);
      //mainGridRef().setCursor(QCursor(QPixmap(":/images/cursor.png")));
      setCurrentCellStyle(Qics::Spreadsheet);


    this->installEventFilter(this);

    /* // from consultant
    //Format the billable rate column with a dollar sign, decimal point,
    //and the appropriate cents digits.
    QicsDataItemSprintfFormatter *brFormatter = new QicsDataItemSprintfFormatter();
    brFormatter->addFormatString(QicsDataItem_Float, "$%.2f");
    brFormatter->addFormatString(QicsDataItem_Int, "$%d.00");
    m_table->columnRef(6).setFormatter(brFormatter);
    m_table->columnRef(6).setValidator(new QDoubleValidator(m_table));

*/

    columnHeaderRef().setAlignment(Qt::AlignCenter);
    int i = 0;
    for(RCol& col: *p_rdata){
        //columnHeaderRef().cellRef(0,i).setLabel(col.str_name_.c_str());
        columnHeaderRef().cellRef(0,i).setLabel(col.title().c_str());
        //columnRef(i).setWidthInChars(10);
        i++;
    }

    //this->sortRows(3);  // work ok!

    //columnHeaderRef().cellRef(0,0).event(

    // from void MdiChild::newFile()
    setWindowTitle(str_Name.c_str());
    setWindowIcon(QIcon(":/images/new.png"));
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

bool MdiChild::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == dm->children().value(0)) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            qDebug() << "Ate key press" << keyEvent->key();
            return true;
        } else {
            return false;
        }
    }
    else {
        // pass the event on to the parent class
        //return MdiChild::eventFilter(obj, event);
        return false;
    }
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

void MdiChild::mousePressEvent(QMouseEvent *event)
{
    //QMessageBox msgBox;
    //msgBox.setText("Btn1 Clicked !");
    int x = event->x();
    int y = event->y();


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


void MdiChild::update_data()
{
    p_rdata->populate(j_Fields_,j_metas_,vstr_fields_form_);
    this->update();
    this->repaint();
}

void MdiChild::insertRows()
{
    QicsCell* cur_cell = currentCell();
    QicsICell cell(cur_cell->rowIndex(),cur_cell->columnIndex());

    if(!cell.isValid())
        return;
    else {
        setRepaintBehavior(Qics::RepaintOff);

        QList<int> v = selectionList(true)->rows();
        if (v.isEmpty())
            insertRow(cell.row());
        else
            for (int i = 0; i < v.count(); ++i)
                insertRow(cell.row());

        setRepaintBehavior(Qics::RepaintOn);
    }
}





/*#include "mdiChildTable.h"

MdiChild::MdiChild()
{

}*/
