#include "settingsOnLoadTemplatesWidget.h"

#include <spdlog/spdlog.h>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>

SettingsOnLoadTemplatesWidget::SettingsOnLoadTemplatesWidget(QWidget *parent)
    : SettingsStackedItemWidget(parent)
    , pTableWidget_(nullptr) {
    setupUI();
}

void SettingsOnLoadTemplatesWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Создаем таблицу для шаблонов
    pTableWidget_ = new QTableWidget(this);
    pTableWidget_->setColumnCount(2);
    pTableWidget_->setHorizontalHeaderLabels(QStringList() << tr("Выбрать") << tr("Шаблон"));
    pTableWidget_->setSelectionMode(QAbstractItemView::SingleSelection);
    pTableWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTableWidget_->horizontalHeader()->setStretchLastSection(true);
    pTableWidget_->verticalHeader()->setVisible(false);
    pTableWidget_->horizontalHeader()->setVisible(false);

    mainLayout->addWidget(pTableWidget_);
    setLayout(mainLayout);

    // Подключаем сигнал изменения элемента таблицы
    connect(pTableWidget_, &QTableWidget::itemChanged,
            this, &SettingsOnLoadTemplatesWidget::onItemChanged);

    populateTemplates();
}

void SettingsOnLoadTemplatesWidget::populateTemplates() {
    // Отключаем сигналы при заполнении таблицы, чтобы избежать
    // множественных сигналов settingsChanged()
    disconnect(pTableWidget_, &QTableWidget::itemChanged,
               this, &SettingsOnLoadTemplatesWidget::onItemChanged);

    pTableWidget_->setRowCount(0);

    // Получаем список доступных шаблонов
    const Params::_v_templates& v_templates =
        Params::get_instance()->getStartLoadTemplates();

    // Получаем все возможные расширения шаблонов
    const Params::_v_template_exts& v_template_ext =
        Params::get_instance()->getTemplateExts();

    // Заполняем таблицу
    int row_num = 0;
    for (const auto& template_ext : v_template_ext) {
        pTableWidget_->insertRow(row_num);

        // Создаем чекбокс для выбора
        QTableWidgetItem* pItemCheckbox = new QTableWidgetItem();
        pItemCheckbox->setCheckState(Qt::Unchecked);
        pItemCheckbox->setData(Qt::CheckStateRole, Qt::Unchecked);

        // Проверяем, включен ли этот шаблон в список загрузки
        const std::string str_template_full =
            template_ext.first + template_ext.second;

        for (const auto& templ : v_templates) {
            if (str_template_full == templ) {
                pItemCheckbox->setCheckState(Qt::Checked);
                spdlog::debug("Template {} is marked as selected", str_template_full);
                break;
            }
        }

        pTableWidget_->setItem(row_num, COLUMN_CHECKED, pItemCheckbox);

        // Создаем элемент с именем шаблона (не редактируемый)
        QString qstr_template_name = QString("%1%2")
                                         .arg(QString::fromStdString(template_ext.first))
                                         .arg(QString::fromStdString(template_ext.second));

        QTableWidgetItem* pItemName = new QTableWidgetItem(qstr_template_name);
        pItemName->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        pTableWidget_->setItem(row_num, COLUMN_NAME, pItemName);

        row_num++;
    }

    pTableWidget_->resizeColumnsToContents();

    // Переподключаем сигнал после заполнения
    connect(pTableWidget_, &QTableWidget::itemChanged,
            this, &SettingsOnLoadTemplatesWidget::onItemChanged);
}

void SettingsOnLoadTemplatesWidget::onItemChanged(QTableWidgetItem* item) {
    if (item->column() == COLUMN_CHECKED) {
        m_pendingTemplates.clear();  // Собираем в m_pendingTemplates

        for (int row = 0; row < pTableWidget_->rowCount(); ++row) {
            QTableWidgetItem* pItemCheckbox = pTableWidget_->item(row, COLUMN_CHECKED);
            if (pItemCheckbox && pItemCheckbox->checkState() == Qt::Checked) {
                QTableWidgetItem* pItemName = pTableWidget_->item(row, COLUMN_NAME);
                if (pItemName) {
                    m_pendingTemplates.emplace_back(pItemName->text().toStdString());
                }
            }
        }

        m_hasChanges = true;
        emit settingsChanged();  // Сигнал для SettingsDialog
    }
}

void SettingsOnLoadTemplatesWidget::applyChanges() {
    if (m_hasChanges) {
        Params::get_instance()->setStartLoadTemplates(m_pendingTemplates);
        m_hasChanges = false;
    }
}
