#include "rtableview.h"
#include "filtertableheader.h"
#include <QScrollBar>


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

RTableView::RTableView(QWidget *parent) :
    QTableView(parent),
    cornerButton(this)
{
    // Set up filter row
    m_tableHeader = new FilterTableHeader(this);
    setHorizontalHeader(m_tableHeader);             // слетает сортировка по клику на заголовке столбца
    setContextMenuPolicy(Qt::CustomContextMenu);                   //https://forum.qt.io/topic/31233/how-to-create-a-custom-context-menu-for-qtableview/6
    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

    setSortingEnabled(true);
    //setSectionMoveable(true);
    horizontalHeader()->setSectionsMovable(true);
    setAlternatingRowColors(true);
    setAutoFillBackground(true);

    cornerButton.setText("*");
    disconnect(&cornerButton, &QPushButton::clicked, this, &QTableView::selectAll);
    connect(&cornerButton, &QPushButton::clicked, this, &RTableView::CornerButtonPressed);
}

void RTableView::generateFilters(int count)
{
    m_tableHeader->generateFilters(count);
}
void RTableView::CornerButtonPressed()
{
    emit onCornerButtonPressed();
}


