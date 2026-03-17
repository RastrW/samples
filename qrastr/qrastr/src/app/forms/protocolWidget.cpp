#include <QListView>
#include <QStringListModel>
#include <QAbstractItemModelTester>
#include <QFontDatabase>
#include <QGridLayout>
#include <QTreeView>
#include <QHeaderView>
#include <initializer_list>

#include "protocolWidget.h"
#include "protocoltreeitem.h"
#include "protocoltreemodel.h"
#include "protocolFilterProxyModel.h"
#include "qastra_events_data.h"
#include <QtitanGrid.h>

static const char* kIconPaths[] = {
    ":images/new_style/information.png",  // 0 — about/info
    ":images/new_style/verified.png",     // 1 — check
    ":images/new_style/cancellation.png", // 2 — error
    ":images/new_style/error.png",        // 3 — warning
    ":images/new_style/idea.png",         // 4 — message
};

static int iconIndexForMessage(LogMessageTypes lmt) {
    switch (lmt) {
    case LogMessageTypes::SystemError:
    case LogMessageTypes::Failed:
    case LogMessageTypes::Error:   return 2;
    case LogMessageTypes::Warning: return 3;
    case LogMessageTypes::Message: return 4;
    case LogMessageTypes::Info:    return 0;
    default:                       return -1;
    }
}

static int iconIndexForStage(const ProtocolTreeItem* item) {
    if (item->errors()   > 0) return 2;  // cancellation
    if (item->warnings() > 0) return 3;  // warning
    if (item->messages() > 0) return 4;  // idea
    return 1;                            // verified — всё хорошо
}

ProtocolWidget::ProtocolWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    setupFilterPanel(mainLayout);
    setupTreeView(mainLayout);
}

void ProtocolWidget::setupFilterPanel(QVBoxLayout* layout) {
    auto* panel  = new QWidget(this);
    auto* hbox   = new QHBoxLayout(panel);
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(6);

    auto makeBtn = [&](const QString& label,
                       const QString& color,
                       const QIcon& icon = {}) -> QPushButton* {

        auto* btn = new QPushButton(icon, label, panel);
        btn->setCheckable(true);

        btn->setMinimumWidth(90);
        btn->setMinimumHeight(28);

        btn->setStyleSheet(QString(
                               "QPushButton { padding: 4px 10px; }"
                               "QPushButton:checked { background-color:%1; color:white; border-radius:3px; }"
                               ).arg(color));

        hbox->addWidget(btn);
        return btn;
    };

    m_btnAll     = makeBtn(tr("Все"),          "#555555");
    m_btnError   = makeBtn(tr(""),       "#c0392b",
                            iconByIndex(iconIndexForMessage(LogMessageTypes::Error)));
    m_btnWarning = makeBtn(tr(""),     "#e67e22",
                            iconByIndex(iconIndexForMessage(LogMessageTypes::Warning)));
    m_btnMessage = makeBtn(tr(""),    "#2980b9",
                            iconByIndex(iconIndexForMessage(LogMessageTypes::Message)));
    m_btnInfo    = makeBtn(tr(""),   "#27ae60",
                            iconByIndex(iconIndexForMessage(LogMessageTypes::Info)));
    hbox->addStretch();

    // Начальное состояние:
    m_btnAll->setChecked(true);

    auto updateFilter = [this]() {
        struct BtnType {
            QPushButton* btn;
            QSet<LogMessageTypes> types;
        };

        const std::array<BtnType, 4> mapping = {{
                                                 {m_btnError,   {LogMessageTypes::SystemError,
                                                               LogMessageTypes::Failed,
                                                               LogMessageTypes::Error}},
                                                 {m_btnWarning, {LogMessageTypes::Warning}},
                                                 {m_btnMessage, {LogMessageTypes::Message}},
                                                 {m_btnInfo,    {LogMessageTypes::Info}},
                                                 }};

        QSet<LogMessageTypes> active;
        int checkedCount = 0;
        for (const auto& [btn, types] : mapping) {
            if (btn->isChecked()) {
                active.unite(types);
                ++checkedCount;
            }
        }

        // Правило: если нажаты все 4 — сбрасываем на "Все"
        const bool showAll = active.isEmpty() || checkedCount == 4;

        if (showAll) {
            for (const auto& [btn, _] : mapping)
                btn->setChecked(false);
            active.clear();
        }

        // Кнопка "Все" подсвечена только когда реально показываем всё
        m_btnAll->setChecked(showAll);
        applyFilter(active);
    };

    // Подключаем все типовые кнопки к одному обработчику:
    for (auto* btn : {m_btnError, m_btnWarning, m_btnMessage, m_btnInfo})
        connect(btn, &QPushButton::clicked, this, updateFilter);

    // Кнопка "Все" — принудительный сброс:
    connect(m_btnAll, &QPushButton::clicked, this, [this, updateFilter] {
        for (auto* b : {m_btnError, m_btnWarning, m_btnMessage, m_btnInfo})
            b->setChecked(false);
        updateFilter();
    });

    layout->addWidget(panel);
}

void ProtocolWidget::setupTreeView(QVBoxLayout* layout) {
    m_model = new ProtocolTreeModel(this);
    m_proxy = new ProtocolFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_stages.emplace(m_model->getRootItemSp());

    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_proxy);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_treeView->setUniformRowHeights(true);
    m_treeView->setAnimated(true);

    //m_treeView->setIndentation(20);
    ///@note Стрелочка через setStyleSheet
    /// QTreeView рисует ветки (branch) сам — это маленькие области слева от каждого узла.
    /// CSS-псевдоселектор QTreeView::branch позволяет заменить стандартную системную стрелочку на любую картинку
    /// border-image: none сбрасывает системный фон, а image: подставляет указанную иконку.
    /// Сама область клика — это стандартный механизм Qt, setStyleSheet лишь меняет внешний вид, а не логику.
    m_treeView->setStyleSheet(R"(
        QTreeView::branch:has-children:!has-siblings:closed,
        QTreeView::branch:closed:has-children:has-siblings {
            border-image: none;
            image: url(:images/new_style/forward.png);
        }
        QTreeView::branch:open:has-children:!has-siblings,
        QTreeView::branch:open:has-children:has-siblings {
            border-image: none;
            image: url(:images/new_style/expand.png);
        }
    )");

    m_treeView->header()->setStretchLastSection(true);
    m_treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    //QFontDatabase::FixedFont` возвращает:
    //- **Windows** — `Consolas` (или `Courier New` на старых системах)
    //- **Linux** — `DejaVu Sans Mono`, `Liberation Mono`, `Monospace` — в зависимости от дистрибутива
    m_treeView->setFont(monoFont);

    layout->addWidget(m_treeView);
}

void ProtocolWidget::applyFilter(const QSet<LogMessageTypes>& filter) {
    m_proxy->setActiveFilter(filter);
    // После фильтрации раскрываем всё, чтобы пользователь видел результаты
    m_treeView->expandAll();
}

void ProtocolWidget::onRastrLog(const _log_data& log_data) {
    m_model->layoutAboutToBeChanged();

    if (log_data.lmt == LogMessageTypes::OpenStage) {
        auto sp = std::make_shared<ProtocolTreeItem>(
            QVariantList{ iconByIndex(1),  // временная иконка, заменится на CloseStage
                         QString::fromStdString(log_data.str_msg) },
            LogMessageTypes::OpenStage,
            m_stages.top().get());
        m_stages.top()->appendChild(sp);
        m_stages.emplace(sp);
    } else if (log_data.lmt == LogMessageTypes::CloseStage) {
        if (m_stages.size() > 1) {
            auto closingStage = m_stages.top();
            m_stages.pop();
            closingStage->setIconData(iconByIndex(iconIndexForStage(closingStage.get())));
        }
    } else {
        auto sp = std::make_shared<ProtocolTreeItem>(
            QVariantList{ iconByIndex(iconIndexForMessage(log_data.lmt)),
                         QString::fromStdString(log_data.str_msg) },
            log_data.lmt,
            m_stages.top().get());
        m_stages.top()->appendChild(sp);
    }

    m_model->layoutChanged();

    // Раскрываем только родителя нового элемента, не весь дерево
    const QModelIndex srcParent =
        m_model->index(m_stages.top()->row(), 0,
                       /* parent */ {});
    m_treeView->expand(m_proxy->mapFromSource(srcParent));
}

void ProtocolWidget::setIgnoreAppendProtocol(bool bl_ignore) {
    m_ignoreAppendProtocol = bl_ignore;
}

void ProtocolWidget::onAppendProtocol(const QString& qstr) {
    if (m_ignoreAppendProtocol) return;
    auto sp = std::make_shared<ProtocolTreeItem>(
        QVariantList{ iconByIndex(iconIndexForMessage(LogMessageTypes::Info)), qstr },
        LogMessageTypes::Info,
        m_stages.top().get());
    m_stages.top()->appendChild(sp);
    m_model->layoutChanged();
}

QPixmap ProtocolWidget::iconByIndex(int idx) const {
    if (idx < 0 || idx > 4) return {};
    return QPixmap(kIconPaths[idx])
        .scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}