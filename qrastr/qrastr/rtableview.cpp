#include "rtableview.h"
#include "rtabwidget.h"
#include "FilterTableHeader.h"
#include <QScrollBar>
#include <private/qtableview_p.h>


RTableCornerButton::RTableCornerButton(QWidget *parent)
    : QAbstractButton(parent)
{
    m_align = Qt::AlignCenter;
    m_text = "*";
    setFocusPolicy(Qt::NoFocus);

    setObjectName("PBSTABLWCORNERWIDGET");
}

void RTableCornerButton::paintEvent(QPaintEvent*)
{
    QStyleOptionHeader opt;
    opt.initFrom(this);
    QStyle::State state = isDown() ? QStyle::State_Sunken : QStyle::State_Raised;

    if (isEnabled())
    {
        state |= QStyle::State_Enabled;
    }

    if (isActiveWindow())
    {
        state |= QStyle::State_Active;
    }

    opt.state = state;
    opt.text = m_text;
    opt.rect = rect();
    opt.position = QStyleOptionHeader::OnlyOneSection;
    opt.textAlignment = m_align;
    QPainter painter(this);
    style()->drawControl(QStyle::CE_Header, &opt, &painter, this);
}

//RTableView::RTableView()
RTableView::RTableView(QWidget *parent) :
    QTableView(parent),
    //pVertical(Qt::Vertical, this),
    //pHorizontal(Qt::Horizontal, this),
    cornerButton(this)
{
    //cornerButton = new RTableCornerButton(this);
    // Set up filter row
    m_tableHeader = new FilterTableHeader(this);
    //m_tableHeader->setFilter(4,tr("<100"));
    setHorizontalHeader(m_tableHeader);             // слетает сортировка по клику на заголовке столбца
    //connect(verticalScrollBar(), &QScrollBar::valueChanged,  dynamic_cast<RtabWidget*>(this->parentWidget()), &RtabWidget::vscrollbarChanged);
    setContextMenuPolicy(Qt::CustomContextMenu);                   //https://forum.qt.io/topic/31233/how-to-create-a-custom-context-menu-for-qtableview/6
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    //connect(ptv->horizontalHeader(), &FilterTableHeader::filterChanged, this, &RtabWidget::updateFilter);


    setSortingEnabled(true);
    //setSectionMoveable(true);
    horizontalHeader()->setSectionsMovable(true);
    setAlternatingRowColors(true);
    setAutoFillBackground(true);

    Q_D(QTableView);
    d->cornerWidget = (QTableCornerButton*)&cornerButton;

    cornerButton.setText("*");
    disconnect(&cornerButton, SIGNAL(clicked()), this, SLOT(selectAll()));
    connect(&cornerButton , SIGNAL(clicked()), this, SLOT(CornerButtonPressed()));
}
/*RTableView::RTableView(RTabWidget* rtw)
{
    m_tableHeader = new FilterTableHeader(this);
    setHorizontalHeader(m_tableHeader);             // слетает сортировка по клику на заголовке столбца

    connect(verticalScrollBar(), &QScrollBar::valueChanged,  dynamic_cast<RtabWidget*>(this->parentWidget()), &RtabWidget::vscrollbarChanged);
}*/
void RTableView::generateFilters(int count)
{
    m_tableHeader->generateFilters(count);
}
void RTableView::CornerButtonPressed()
{
    emit onCornerButtonPressed();
}


