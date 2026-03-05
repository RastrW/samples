#include <QListView>
#include <QStringListModel>
#include <QAbstractItemModelTester>
#include <QFontDatabase>
#include <QGridLayout>
#include <QTreeView>

#include "formprotocol.h"
#include "protocoltreeitem.h"
#include "protocoltreemodel.h"
#include "qastra_events_data.h"
#include <QtitanGrid.h>


FormProtocol::FormProtocol(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Form"));
    resize(863, 535);

    QGridLayout* gridLayout = new QGridLayout(this);

    twProtocol = new QTreeView(this);
    twProtocol->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    twProtocol->setEditTriggers(QAbstractItemView::NoEditTriggers);
    twProtocol->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    gridLayout->addWidget(twProtocol, 0, 0, 1, 1);

    m_protocolTreeModel = new ProtocolTreeModel();
    twProtocol->setModel(m_protocolTreeModel);
    m_sptiSstages.emplace(m_protocolTreeModel->getRootItemSp());
    twProtocol->hide();

    Grid::loadTranslation();
    m_ptg = new Qtitan::TreeGrid(this);
    gridLayout->addWidget(m_ptg, 1, 0, 1, 1);

    m_ptg->setViewType(Qtitan::TreeGrid::TreeView);
    Qtitan::GridTreeView* view = m_ptg->view<Qtitan::GridTreeView>();
    view->beginUpdate();
    view->options().setGestureEnabled(true);
    view->options().setShowFocusDecoration(true);
    view->options().setAlternatingRowColors(true);
    view->options().setGroupsHeader(false);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    view->options().setCellFont(fixedFont);
    view->setModel(m_protocolTreeModel);
    view->endUpdate();
    view->bestFit(Qtitan::FitToHeaderAndContent);
    view->getColumn(0)->setMinWidth(70);

    Qtitan::GridTableColumn* const pgc{ static_cast<Qtitan::GridTableColumn*>(view->getColumn(0)) };
    pgc->setEditorType(Qtitan::GridEditor::Type::Picture);

    view->getColumn(1)->setMinWidth(1000);
    view->expandToLevel(3);
}

void FormProtocol::setIgnoreAppendProtocol(bool bl_ignore)
{
    m_ignoreAppendProtocol = bl_ignore;
}

void FormProtocol::onAppendProtocol(const QString& qstr)
{
    if (m_ignoreAppendProtocol) return;
    auto sp_item = std::make_shared<ProtocolTreeItem>(
        QVariantList{ QPixmap(QStringLiteral(":images/about.png")), qstr },
        m_sptiSstages.top().get());
    m_sptiSstages.top().get()->appendChild(sp_item);
    m_protocolTreeModel->layoutChanged();
}

void FormProtocol::onRastrLog(const _log_data& log_data)
{
    Qtitan::GridTreeView* view = m_ptg->view<Qtitan::GridTreeView>();
    m_protocolTreeModel->layoutAboutToBeChanged();

    if (LogMessageTypes::OpenStage == log_data.lmt) {
        auto sp_item = std::make_shared<ProtocolTreeItem>(
            QVariantList{ QPixmap(QStringLiteral(":images/book_yellow.png")), log_data.str_msg.c_str() },
            m_sptiSstages.top().get());
        m_sptiSstages.top()->appendChild(sp_item);
        m_sptiSstages.emplace(sp_item);
    } else if (LogMessageTypes::CloseStage == log_data.lmt) {
        if (1 < m_sptiSstages.size())
            m_sptiSstages.pop();
        else
            assert(!"trying to close Root stage!");
    } else {
        std::shared_ptr<ProtocolTreeItem> sp_item;
        switch (log_data.lmt) {
        case LogMessageTypes::SystemError:
        case LogMessageTypes::Failed:
        case LogMessageTypes::Error:
            sp_item = std::make_shared<ProtocolTreeItem>(
                QVariantList{ QPixmap(QStringLiteral(":images/delete.png")), log_data.str_msg.c_str() },
                m_sptiSstages.top().get());
            break;
        case LogMessageTypes::Warning:
            sp_item = std::make_shared<ProtocolTreeItem>(
                QVariantList{ QPixmap(QStringLiteral(":images/warning.png")), log_data.str_msg.c_str() },
                m_sptiSstages.top().get());
            break;
        default:
            sp_item = std::make_shared<ProtocolTreeItem>(
                QVariantList{ QPixmap(QStringLiteral(":images/lightbulb_on.png")), log_data.str_msg.c_str() },
                m_sptiSstages.top().get());
            break;
        }
        m_sptiSstages.top()->appendChild(sp_item);
    }
    m_protocolTreeModel->layoutChanged();
}
