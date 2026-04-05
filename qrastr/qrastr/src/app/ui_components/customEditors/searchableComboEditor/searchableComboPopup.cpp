#include "searchableComboPopup.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QListView>
#include <QApplication>
#include <QKeyEvent>

SearchableComboPopup::SearchableComboPopup(QWidget* parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setFrameShape(QFrame::StyledPanel);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(tr("Поиск..."));
    m_search->setClearButtonEnabled(true);

    m_model = new QStringListModel(this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(0);

    m_list = new QListView(this);
    m_list->setModel(m_proxy);
    m_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layout->addWidget(m_search);
    layout->addWidget(m_list);

    connect(m_search, &QLineEdit::textChanged,
            this, &SearchableComboPopup::applyFilter);

    connect(m_list, &QListView::clicked,
            this, [this](const QModelIndex& idx) {
                emit itemSelected(m_proxy->data(idx).toString());
                hide();
            });

    // Enter в списке подтверждает выбор
    connect(m_list, &QListView::activated,
            this, [this](const QModelIndex& idx) {
                emit itemSelected(m_proxy->data(idx).toString());
                hide();
            });

    m_search->installEventFilter(this);
    m_list->installEventFilter(this);
}

void SearchableComboPopup::setItems(const QStringList& items)
{
    m_model->setStringList(items);
    // Высота: не больше 10 строк
    const int rowH    = m_list->sizeHintForRow(0);
    const int visible = qMin(items.size(), 10);
    m_list->setFixedHeight(rowH * visible + 4);
}

void SearchableComboPopup::setCurrentText(const QString& text)
{
    m_search->clear();
    // Выделяем текущее значение в списке
    const auto matches = m_proxy->match(
        m_proxy->index(0, 0), Qt::DisplayRole, text, 1, Qt::MatchExactly);
    if (!matches.isEmpty())
        m_list->setCurrentIndex(matches.first());
}

QString SearchableComboPopup::currentText() const
{
    return m_list->currentIndex().isValid()
               ? m_proxy->data(m_list->currentIndex()).toString()
               : QString{};
}

void SearchableComboPopup::applyFilter(const QString& text)
{
    m_proxy->setFilterFixedString(text);
    if (m_proxy->rowCount() > 0)
        m_list->setCurrentIndex(m_proxy->index(0, 0));
}

bool SearchableComboPopup::eventFilter(QObject* obj, QEvent* e)
{
    if (e->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Escape) {
            emit editingCancelled();
            hide();
            return true;
        }
        // Стрелки из строки поиска переходят в список
        if (obj == m_search &&
            (ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Up)) {
            QApplication::sendEvent(m_list, e);
            return true;
        }
        // Enter в строке поиска — подтверждает первый элемент
        if (obj == m_search && ke->key() == Qt::Key_Return) {
            if (m_proxy->rowCount() > 0) {
                emit itemSelected(
                    m_proxy->data(m_proxy->index(0, 0)).toString());
                hide();
            }
            return true;
        }
    }
    return QFrame::eventFilter(obj, e);
}