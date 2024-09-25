#include "rtabwidget.h"
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QLineEdit>
#include <QScrollBar>
#include "FilterTableHeader.h"
#include <QMimeData>
#include <QApplication>
#include <QClipboard>
#include <QBuffer>
#include <QDateTime>
#include <QProgressDialog>
#include "CondFormat.h"
#include "qastra.h"
using WrapperExceptionType = std::runtime_error;
#include "IPlainRastrWrappers.h"
#include "comboboxdelegate.h"
#include "doubleitemdelegate.h"
#include "checkboxdelegate.h"



//#include "tableview.h"


RtabWidget::RtabWidget(QWidget *parent)
    : QWidget{parent}
{

    ptv = new RTableView();

    ptv->setContextMenuPolicy(Qt::CustomContextMenu);                   //https://forum.qt.io/topic/31233/how-to-create-a-custom-context-menu-for-qtableview/6
    ptv->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ptv, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(customMenuRequested(QPoint)));
    connect(ptv, SIGNAL(pressed(const QModelIndex &)),
            SLOT(onItemPressed(const QModelIndex &)));
    connect(ptv->horizontalHeader(),
            SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(customHeaderMenuRequested(QPoint)));

    //connect(ptv->horizontalHeader(), &FilterTableHeader::filterChanged, this, &RtabWidget::updateFilter);
    connect(ptv->horizontalHeader(), SIGNAL(filterChanged(size_t , QString )), this, SLOT(updateFilter(size_t , QString) ));

    ptv->setSortingEnabled(true);
}

//RtabWidget::RtabWidget(CRastrHlp& rh,QAstra* pqastra,int n_indx, QWidget *parent)
RtabWidget::RtabWidget(QAstra* pqastra,CUIForm UIForm, QWidget *parent)
    : RtabWidget{parent}
{
    m_UIForm = UIForm;
    CreateModel(pqastra,&m_UIForm);

    //SetTableView(*ptv,*prm);                // ширина по шаблону
    //ptv->resizeColumnsToContents();         // ширина по контенту
    ptv->setParent(this);

    int ncols = prm->columnCount();
    ptv->generateFilters(ncols);
    ptv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(ptv);
    setLayout(layout);
}

void RtabWidget::CreateModel(QAstra* pqastra, CUIForm* pUIForm)
{
    prm = new RModel(nullptr, pqastra );
    proxyModel = new QSortFilterProxyModel(prm); // used for sorting: create proxy //https://doc.qt.io/qt-5/qsortfilterproxymodel.html#details
    proxyModel->setSourceModel(prm);
    prm->setForm(pUIForm);
    prm->populateDataFromRastr();


    for (RCol& rcol : *prm->getRdata())
    {
        if (rcol.com_prop_tt == enComPropTT::COM_PR_ENUM)
        {
            ComboBoxDelegate* delegate = new ComboBoxDelegate(this,rcol.NameRef());
            ptv->setItemDelegateForColumn(rcol.index, delegate);
            // Make the combo boxes always displayed.
            /*for ( int i = 0; i < prm->rowCount(); ++i )
            {
                ptv->openPersistentEditor( prm->index(i, rcol.index) );
            }*/
        }

        if (rcol.com_prop_tt == enComPropTT::COM_PR_REAL)
        {
            int prec = std::atoi(rcol.prec().c_str());
            DoubleItemDelegate* delegate = new DoubleItemDelegate(prec,this);
            ptv->setItemDelegateForColumn(rcol.index, delegate);
        }

        if (rcol.com_prop_tt == enComPropTT::COM_PR_BOOL)
        {
            checkboxDelegate* delegate = new checkboxDelegate(this);
            ptv->setItemDelegateForColumn(rcol.index, delegate);
        }
    }

    ptv->setModel(proxyModel);

    this->update();
    this->repaint();
}

void RtabWidget::SetTableView(QTableView& tv, RModel& mm, int myltiplier  )
{
    // Ширина колонок
    //int myltiplier = 15;
    for (auto cw : mm.ColumnsWidth())
        tv.setColumnWidth(std::get<0>(cw),std::get<1>(cw)*myltiplier);
}

void RtabWidget::customMenuRequested(QPoint pos){
    index=ptv->indexAt(pos);

    QMenu *menu=new QMenu(this);
    QAction* copyAction = new QAction( tr("Copy"), menu);
    QAction* copyWithHeadersAction = new QAction( tr("Copy with Headers"), menu);
    menu->addAction(copyAction);
    menu->addAction(copyWithHeadersAction);

    //menu->addAction(QIcon(":/images/cut.png"),tr("Cut"), this, SLOT(cut()));
    //menu->addAction(QIcon(":/images/copy.png"),tr("Copy"), this, SLOT(copy()));
    //menu->addAction(QIcon(":/images/paste.png"),tr("Paste"), this, SLOT(paste()));
    //menu->addSeparator();
    //menu->addAction(tr("Clear Contents"),this,SLOT(clearContents()));
    //menu->addSeparator();
    menu->addAction(QIcon(":/images/Rastr3_grid_insrow_16x16.png"),tr("Insert Row"),this,SLOT(insertRow()));
    menu->addAction(QIcon(":/images/Rastr3_grid_delrow_16x16.png"),tr("Delete Row"),this,SLOT(deleteRow()));
    //menu->addAction(tr("Hide Rows"),this,SLOT(hideRows()));
    //menu->addAction(tr("Unhide Rows"),this,SLOT(unhideRows()));
    menu->addSeparator();
    //menu->addAction(tr("Insert Columns"),this,SLOT(insertColumns()));
    //menu->addAction(tr("Delete Columns"),this,SLOT(deleteColumns()));
    //menu->addAction(tr("Hide Columns"),this,SLOT(hideColumns()));
    //menu->addAction(tr("Unhide Columns"),this,SLOT(unhideColumns()));
    menu->addAction(tr("Выравнивание: по шаблону"),this,SLOT(widebyshabl()));
    menu->addAction(tr("Выравнивание: по данным"),this,SLOT(widebydata()));
    menu->addSeparator();
    menu->addAction(tr("Format"));
    menu->popup(ptv->viewport()->mapToGlobal(pos));

    connect(copyAction, &QAction::triggered, this, [&]() {
        copy(false, false);
    });

    //connect(copyAction, &QAction::triggered, this, &RtabWidget::copy);

    //connect(cutAction, &QAction::triggered, this, &ExtendedTableWidget::cut);
    connect(copyWithHeadersAction, &QAction::triggered, this, [&]() {
        copy(true, false);
    });
}

void RtabWidget::customHeaderMenuRequested(QPoint pos){
    column=ptv->horizontalHeader()->logicalIndexAt(pos);

    RCol* prcol = prm->getRCol(column);
    std::string str_col_prop = prcol->desc() + " |"+ prcol->title() + "| -(" + prcol->name() + "), [" +prcol->unit() + "]";
    QString qstr_col_props = str_col_prop.c_str();

    QMenu *menu=new QMenu(this);
    menu->addAction(qstr_col_props, this, SLOT(OpenColPropForm()));
    menu->addSeparator();
    menu->addAction(QIcon(":/images/sortasc.png"),tr("sortAscending"), this, SLOT(sortAscending()));
    menu->addAction(QIcon(":/images/sortdesc.png"),tr("sortDescending"), this, SLOT(sortDescending()));
    menu->addSeparator();
    menu->addAction(tr("Clear Contents"),this,SLOT(clearContents()));
    menu->addSeparator();
    menu->addAction(tr("Hide Columns"),this,SLOT(hideColumns()));
    menu->addAction(tr("Unhide Columns"),this,SLOT(unhideColumns()));
    //m_menu->addAction(tr("Подбор ширины все колонки"),this,SLOT(AutoWidthColumns())); // thats  not work
    //m_menu->addAction(Act_AutoWidthColumns);
    menu->addSeparator();
    menu->addAction(tr("Format"));
    menu->popup(ptv->horizontalHeader()->viewport()->mapToGlobal(pos));
}
void RtabWidget::onItemPressed(const QModelIndex &index)
{
    int row = index.row();
    int column = index.column();
    //QTableView* t = index.parent();
    qDebug()<<"Pressed:" <<row<< ","<<column;
}
void RtabWidget::insertRow()
{
#if(!defined(QICSGRID_NO))
    int rowIndex = activeMdiChild()->currentCell()->rowIndex();
    if (rowIndex < 0)
        return;
    activeMdiChild()->insertRow( rowIndex );
#endif//#if(!defined(QICSGRID_NO))
#if(defined(QICSGRID_NO))

    prm->insertRows(index.row(),1,index);

#endif//#if(defined(QICSGRID_NO))
}
void RtabWidget::deleteRow()
{
    prm->removeRows(index.row(),1,index);
}
void RtabWidget::widebyshabl()
{
    SetTableView(*ptv,*prm);                // ширина по шаблону
}
void RtabWidget::widebydata()
{
    ptv->resizeColumnsToContents();         // ширина по контенту
}
void RtabWidget::OpenColPropForm()
{
    //RCol* prcol = prm->getRCol(index.column());
    RCol* prcol = prm->getRCol(column);
    //ColPropForm* PropForm = new ColPropForm(prm->getRdata(),prcol);
    ColPropForm* PropForm = new ColPropForm(prm->getRdata(),ptv, prcol);
    PropForm->show();
}
void RtabWidget::sortAscending()
{
    proxyModel->sort(column,Qt::AscendingOrder);
}
void RtabWidget::sortDescending()
{
    proxyModel->sort(column,Qt::DescendingOrder);
}
void RtabWidget::onUpdate(std::string _t_name)
{
    if (prm->getRdata()->t_name_ == _t_name)
    {
        this->update();
        this->repaint();
    }
}
void RtabWidget::updateFilter(size_t column, QString value)
{
    proxyModel->setFilterRegularExpression(QRegularExpression(value));
    proxyModel->setFilterKeyColumn(column);                 //The default value is 0. If the value is -1, the keys will be read from all columns
}
void RtabWidget::onFileLoad()
{
   // CreateModel(_rh);
    prm->populateDataFromRastr();
}
void RtabWidget::update_data()
{
    prm->populateDataFromRastr();
    this->update();
    this->repaint();
}



void RtabWidget::test(const QModelIndexList& fromIndices)
{
    size_t sz = fromIndices.count();
    return;
}
void RtabWidget::copyMimeData(const QModelIndexList& fromIndices, QMimeData* mimeData, const bool withHeaders, const bool inSQL)
{
    mimeData->setText("test res");
    //return;
    QModelIndexList indices = fromIndices;
//return;
    // Remove all indices from hidden columns, because if we don't, we might copy data from hidden columns as well which is very
    // unintuitive; especially copying the rowid column when selecting all columns of a table is a problem because pasting the data
    // won't work as expected.
    QMutableListIterator<QModelIndex> it(indices);
    while (it.hasNext()) {
        if ( this->ptv->isColumnHidden(it.next().column()))
            it.remove();
    }

    // Abort if there's nothing to copy
    if (indices.isEmpty())
        return;

    //RModel* m = qobject_cast<RModel*>( this->ptv->model());
    RModel* m =  this->prm;


    // Clear internal copy-paste buffer
    m_buffer.clear();

    // If a single cell is selected which contains an image, copy it to the clipboard
    if (!inSQL && !withHeaders && indices.size() == 1) {
        QImage img;
        QVariant varData = m->data(indices.first(), Qt::EditRole);

        if (img.loadFromData(varData.toByteArray()))
        {
            // If it's an image, copy the image data to the clipboard
            mimeData->setImageData(img);
            return;
        }
    }

    // If we got here, a non-image cell was or multiple cells were selected, or copy with headers was requested.
    // In this case, we copy selected data into internal copy-paste buffer and then
    // we write a table both in HTML and text formats to the system clipboard.

    // Copy selected data into internal copy-paste buffer
    int last_row = indices.first().row();
    BufferRow lst;
    for(int i=0;i<indices.size();i++)
    {
        if(indices.at(i).row() != last_row)
        {
            m_buffer.push_back(lst);
            lst.clear();
        }
        lst.push_back(indices.at(i).data(Qt::EditRole).toByteArray());
        last_row = indices.at(i).row();
    }
    m_buffer.push_back(lst);

    // TSV text or SQL
    QString result;

    // HTML text
    QString htmlResult = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
                         "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">"
                         "<title></title>";

    // The generator-stamp is later used to know whether the data in the system clipboard is still ours.
    // In that case we will give precedence to our internal copy buffer.
    QString now = QDateTime::currentDateTime().toString("YYYY-MM-DDTHH:mm:ss.zzz");
    m_generatorStamp = QString("<meta name=\"generator\" content=\"%1\"><meta name=\"date\" content=\"%2\">").arg(QApplication::applicationName().toHtmlEscaped(), now);
    htmlResult.append(m_generatorStamp);
    // TODO: is this really needed by Excel, since we use <pre> for multi-line cells?
    htmlResult.append("<style type=\"text/css\">br{mso-data-placement:same-cell;}</style></head><body>"
                      "<table border=1 cellspacing=0 cellpadding=2>");

    // Insert the columns in a set, since they could be non-contiguous.
    std::set<int> colsInIndexes, rowsInIndexes;
    for(const QModelIndex & idx : qAsConst(indices)) {
        colsInIndexes.insert(idx.column());
        rowsInIndexes.insert(idx.row());
    }

    const QString fieldSepText = "\t";
#ifdef Q_OS_WIN
    const QString rowSepText = "\r\n";
#else
    const QString rowSepText = "\n";
#endif


    int firstColumn = *colsInIndexes.begin();

    QString sqlInsertStatement;
    // Table headers
    if (withHeaders || inSQL) {
        if (inSQL)
            sqlInsertStatement = QString("INSERT INTO %1 (").arg(QString::fromStdString(m->getRdata()->t_name_));
        else
            htmlResult.append("<tr><th>");

        for(int col : colsInIndexes) {
            QByteArray headerText_ = ptv->model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toByteArray();
            std::string headerText = ptv->model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString().toUtf8().constData();
            if (col != firstColumn) {
                if (inSQL)
                    sqlInsertStatement.append(", ");
                else {
                    result.append(fieldSepText);
                    htmlResult.append("</th><th>");
                }
            }
            if (inSQL)
            {
                //sqlInsertStatement.append(sqlb::escapeIdentifier(headerText));
            }
            else {
                result.append(headerText);
                htmlResult.append(headerText);
            }
        }
        if (inSQL)
            sqlInsertStatement.append(") VALUES (");
        else {
            result.append(rowSepText);
            htmlResult.append("</th></tr>");
        }
    }


 /*   QProgressDialog progress(this);
    progress.setWindowModality(Qt::ApplicationModal);
    // Disable context help button on Windows
    progress.setWindowFlags(progress.windowFlags()
                            & ~Qt::WindowContextHelpButtonHint);
    progress.setRange(*rowsInIndexes.begin(), *rowsInIndexes.end());
    progress.setMinimumDuration(2000);
*/

    // Iterate over rows x cols checking if the index actually exists when needed, in order
    // to support non-rectangular selections.



    for(const int row : rowsInIndexes) {

        // Beginning of row
        if (inSQL)
        {
            //result.append(sqlInsertStatement);
        }
        else
            htmlResult.append("<tr>");

        for(const int column : colsInIndexes) {

            const QModelIndex index = indices.first().sibling(row, column);
            const bool isContained = indices.contains(index);

            if (column != firstColumn) {
                // Add text separators
                if (inSQL)
                    result.append(", ");
                else
                    result.append(fieldSepText);
            }

            if(isContained) {
                /*
                QFont font;
                font.fromString(index.data(Qt::FontRole).toString());

                const Qt::Alignment align(index.data(Qt::TextAlignmentRole).toInt());
                const QString textAlign(CondFormat::alignmentTexts().at(CondFormat::fromCombinedAlignment(align)).toLower());
                htmlResult.append(QString("<td style=\"font-family:'%1';font-size:%2pt;font-style:%3;font-weight: %4;%5 "
                                          "background-color:%6;color:%7;text-align:%8\">").arg(
                                          font.family().toHtmlEscaped(), // font-family
                                          QString::number(font.pointSize()), // font-size
                                          font.italic() ? "italic" : "normal", // font-style,
                                          font.bold() ? "bold" : "normal", // font-weigth,
                                          font.underline() ? " text-decoration: underline;" : "", // text-decoration,
                                          index.data(Qt::BackgroundRole).toString(), // background-color
                                          index.data(Qt::ForegroundRole).toString(), // color
                                          textAlign));
                    */
            } else {
                htmlResult.append("<td>");
            }
            QImage img;
            const QVariant bArrdata = isContained ? index.data(Qt::EditRole) : QVariant();

            if (bArrdata.isNull()) {
                // NULL data: NULL in SQL, empty in HTML or text.
                if (inSQL) result.append("NULL");
            } else if(!m->isBinary(index)) {
                // Text data
                QByteArray text = bArrdata.toByteArray();

                if (inSQL)
                {
                    //result.append(sqlb::escapeString(text));
                }
                else {
                    result.append(text);
                    // Table cell data: text
                    if (text.contains('\n') || text.contains('\t'))
                        htmlResult.append(QString("<pre>%1</pre>").arg(QString(text).toHtmlEscaped()));
                    else
                        htmlResult.append(QString(text).toHtmlEscaped());
                }
            } else if (inSQL) {
                // Table cell data: binary in SQL. Save as BLOB literal.
                result.append(QString("X'%1'").arg(QString(bArrdata.toByteArray().toHex())));
            } else if (img.loadFromData(bArrdata.toByteArray())) {
                // Table cell data: image. Store it as an embedded image in HTML
                QByteArray ba;
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                img.save(&buffer, "PNG");
                buffer.close();

                htmlResult.append(QString("<img src=\"data:image/png;base64,%1\" alt=\"Image\">")
                                      .arg(QString(ba.toBase64())));
                result.append(index.data(Qt::DisplayRole).toByteArray());
            }
            else {
                //result.append(QString("%1").arg(QString(bArrdata.toByteArray().toHex())));
                //result.append(QString("%1").arg(QString(bArrdata.toString())));

                QByteArray text = bArrdata.toByteArray();
                result.append(text);
                // Table cell data: text
                if (text.contains('\n') || text.contains('\t'))
                    htmlResult.append(QString("<pre>%1</pre>").arg(QString(text).toHtmlEscaped()));
                else
                    htmlResult.append(QString(text).toHtmlEscaped());
            }


            // End of column
            // Add HTML cell terminator
            htmlResult.append("</td>");
        }

        // End of row
        if (inSQL)
            result.append(");");
        else
            htmlResult.append("</tr>");
        result.append(rowSepText);

        /*
        progress.setValue(row);
        // Abort the operation if the user pressed ESC key or Cancel button
        if (progress.wasCanceled()) {
            return;
        }
        */
    }

    if (!inSQL) {
        htmlResult.append("</table></body></html>");
        mimeData->setHtml(htmlResult);
    }
    result.removeLast();
    QString test = "test str\r\n";
    QString test2 = "test str\r\n";
    test.removeLast();
    test2.remove(test2.length()-2,2);
    //result.remove(result.length()-2,2);

    int pos = result.lastIndexOf(rowSepText);
    result = result.left(pos);
    mimeData->setText(result.toUtf8().data());

}

void RtabWidget::copy(const bool withHeaders, const bool inSQL )
{
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(tr("test"));

    copyMimeData( this->ptv->selectionModel()->selectedIndexes(), mimeData, withHeaders, inSQL);  // так падает https://stackoverflow.com/questions/15123109/crash-with-qitemselectionmodelselectedindexes
    // TO DO : выяснить почему падает !
    //test(this->ptv->selectionModel()->selectedIndexes().toList());
    // Падает только в DEBUG так как QT  использует библиотеки от Release и в этом случае деструктор отрабатывает некорректно.
    // В  Release не падает все ОК.

    qApp->clipboard()->setMimeData(mimeData);
}
void RtabWidget::copy()
{
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(tr("test"));
    //copyMimeData( this->ptv->selectionModel()->selectedIndexes(), mimeData, withHeaders, inSQL);
    //copyMimeData( this->ptv->selectionModel()->selectedIndexes(), mimeData, withHeaders, inSQL);
    test(this->ptv->selectionModel()->selectedIndexes());
    qApp->clipboard()->setMimeData(mimeData);

}




