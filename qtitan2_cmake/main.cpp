#include "mainwindow.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QtitanGrid.h>

#include <QtitanDef.h>


#include <QtitanGrid.h>
//QTITAN_END_NAMESPACE

//namespace Qtitan{
inline qreal qtn_rand(qreal min, qreal max){
    qreal minVal = qMin(min, max);
    qreal maxVal = qMax(min, max);
    return (maxVal - minVal) * qreal(rand()) / RAND_MAX + minVal;
}
int main(int argc, char *argv[]){
    QApplication a(argc, argv);


    //snatched from C:\Qt\Developer_Machines\QtitanComponents2024.2.0_Demo\examples\grid\CustomEditor
    const int row_count = 100;
    const int col_count = 4;
    Qtitan::Grid* m_grid = new Qtitan::Grid();
    QStandardItemModel* model = new QStandardItemModel( row_count, col_count, m_grid);
    model->setHeaderData(0, Qt::Horizontal, QStringLiteral("Custom Value"));
    model->setHeaderData(1, Qt::Horizontal, QStringLiteral("Integer Value"));
    model->setHeaderData(2, Qt::Horizontal, QStringLiteral("String Value"));
    model->setHeaderData(3, Qt::Horizontal, QStringLiteral("Color Value"));
    for (int row = 0; row < model->rowCount(); ++row)    {
        model->setData(model->index(row, 0), row);
        model->setData(model->index(row, 1), row);
        model->setData(model->index(row, 2), QString("Value: %1").arg(row));
        QColor color(qtn_rand(0, 255), qtn_rand(0, 255), qtn_rand(0, 255));
        model->setData(model->index(row, 3), color);
    }
    // Configure grid view
    m_grid->setViewType(Qtitan::Grid::TableView);
    Qtitan::GridTableView* view = m_grid->view<Qtitan::GridTableView>();
    view->beginUpdate();
    view->options().setGridLineWidth(1);
//  Connect Grid's context menu handler.
//  connect(view, SIGNAL(contextMenu(ContextMenuEventArgs*)), this, SLOT(contextMenu(ContextMenuEventArgs* )));
//  connect(view, SIGNAL(editorModifying(GridEditor *)), this, SLOT(editorModifying(GridEditor *)));
//  connect(view, SIGNAL(editorValidating(EditorValidationEventArgs*)), this, SLOT(editorValidating(EditorValidationEventArgs*)));
    view->setModel(model);
    //Configure the grid columns.
    Qtitan::GridTableColumn* column = (Qtitan::GridTableColumn*)view->getColumn(0);
//    column->setEditorRepository(new CustomEditorRepository());
    column->editorRepository()->setEditorActivationPolicy(GridEditor::ActivationPolicy(GridEditor::ActivateByDblClick));
    column = (Qtitan::GridTableColumn*)view->getColumn(1);
    column->setEditorType(GridEditor::Numeric);
    column = (Qtitan::GridTableColumn*)view->getColumn(2);
    column->setEditorType(GridEditor::String);
    view->endUpdate();
    m_grid->show();

    //MainWindow w;
    //w.show();
    return a.exec();
}

