#include "searchableComboPopupTwo.h"
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QHeaderView>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include "twoColumnFilterProxy.h"

SearchableComboPopupTwo::SearchableComboPopupTwo(QWidget* parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setFrameShape(QFrame::StyledPanel);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(3);

    // ── Модель + прокси ──────────────────────────────────────────────────────
    m_model = new QStandardItemModel(0, 2, this);
    m_model->setHorizontalHeaderLabels({ tr("Номер"), tr("Название") });

    m_proxy = new TwoColumnFilterProxy(this);
    m_proxy->setSourceModel(m_model);

    // ── Строки фильтров ──────────────────────────────────────────────────────
    auto* filterRow = new QHBoxLayout();
    filterRow->setSpacing(2);

    m_searchIndex = new QLineEdit(this);
    m_searchIndex->setPlaceholderText(tr("Фильтр по номеру..."));
    m_searchIndex->setClearButtonEnabled(true);

    m_searchName = new QLineEdit(this);
    m_searchName->setPlaceholderText(tr("Фильтр по названию..."));
    m_searchName->setClearButtonEnabled(true);

    filterRow->addWidget(m_searchIndex, 1);
    filterRow->addWidget(m_searchName,  3);

    // ── Таблица ──────────────────────────────────────────────────────────────
    m_table = new QTableView(this);
    m_table->setModel(m_proxy);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_table->setSortingEnabled(false);
    m_table->setMinimumHeight(150);

    // ── Кнопки внизу ─────────────────────────────────────────────────────────
    auto* btnRow = new QHBoxLayout();
    m_btnClose  = new QPushButton(tr("Закрыть"),  this);
    m_btnNotSet = new QPushButton(tr("Не задано"), this);
    btnRow->addWidget(m_btnClose);
    btnRow->addStretch();
    btnRow->addWidget(m_btnNotSet);

    mainLayout->addLayout(filterRow);
    mainLayout->addWidget(m_table);
    mainLayout->addLayout(btnRow);

    // ── Соединения ───────────────────────────────────────────────────────────
    connect(m_searchIndex, &QLineEdit::textChanged,
            m_proxy, &TwoColumnFilterProxy::setIndexFilter);
    connect(m_searchName,  &QLineEdit::textChanged,
            m_proxy, &TwoColumnFilterProxy::setNameFilter);

    connect(m_table, &QTableView::clicked,
            this, &SearchableComboPopupTwo::onRowActivated);
    connect(m_table, &QTableView::activated,
            this, &SearchableComboPopupTwo::onRowActivated);

    connect(m_btnNotSet, &QPushButton::clicked, this, [this]() {
        emit clearSelected();
        hide();
    });
    connect(m_btnClose, &QPushButton::clicked, this, [this]() {
        emit editingCancelled();
        hide();
    });

    m_searchIndex->installEventFilter(this);
    m_searchName->installEventFilter(this);
    m_table->installEventFilter(this);
}

void SearchableComboPopupTwo::setItems(const std::unordered_map<size_t, std::string>& items)
{
    m_model->removeRows(0, m_model->rowCount());

    for (const auto& [key, name] : items) {
        auto* itemKey  = new QStandardItem(QString::number(key));
        auto* itemName = new QStandardItem(QString::fromStdString(name));
        // Храним числовой ключ в UserRole — используем при itemSelected
        itemKey ->setData(static_cast<int>(key), Qt::UserRole);
        itemKey ->setEditable(false);
        itemName->setEditable(false);
        m_model->appendRow({ itemKey, itemName });
    }
    m_table->resizeColumnToContents(0);  // подогнать ширину столбца "Индекс"
}

void SearchableComboPopupTwo::setCurrentKey(int key)
{
    // Сбрасываем фильтры, чтобы текущая запись точно была видна
    m_searchIndex->clear();
    m_searchName->clear();

    for (int r = 0; r < m_proxy->rowCount(); ++r) {
        const QModelIndex idx = m_proxy->index(r, 0);
        if (m_proxy->data(idx, Qt::UserRole).toInt() == key) {
            m_table->setCurrentIndex(idx);
            m_table->scrollTo(idx, QAbstractItemView::PositionAtCenter);
            return;
        }
    }
}

void SearchableComboPopupTwo::onRowActivated(const QModelIndex& proxyIdx)
{
    if (!proxyIdx.isValid()) return;
    // Берём ключ из первого столбца через UserRole
    const QModelIndex srcIdx = m_proxy->mapToSource(
        m_proxy->index(proxyIdx.row(), 0));
    const int key = m_model->data(srcIdx, Qt::UserRole).toInt();
    emit itemSelected(key);
    hide();
}

bool SearchableComboPopupTwo::eventFilter(QObject* obj, QEvent* e)
{
    if (e->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(e);

        // Escape — отмена
        if (ke->key() == Qt::Key_Escape) {
            emit editingCancelled();
            hide();
            return true;
        }

        // Стрелки вверх/вниз из любого фильтра → передаём в таблицу
        if ((obj == m_searchIndex || obj == m_searchName) &&
            (ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Up)) {
            QApplication::sendEvent(m_table, e);
            return true;
        }

        // Enter в фильтре → выбираем первую видимую строку
        if ((obj == m_searchIndex || obj == m_searchName) &&
            (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)) {
            if (m_proxy->rowCount() > 0)
                onRowActivated(m_proxy->index(0, 0));
            return true;
        }

        // Enter в таблице → подтверждаем выбранную строку
        if (obj == m_table &&
            (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)) {
            onRowActivated(m_table->currentIndex());
            return true;
        }
    }
    return QFrame::eventFilter(obj, e);
}