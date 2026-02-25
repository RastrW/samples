#include "formsettingsonloadforms.h"
#include "params.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <spdlog/spdlog.h>

SettingsOnLoadFormsWidget::SettingsOnLoadFormsWidget(QWidget *parent)
    : SettingsStackedItemWidget(parent)
    , pTableWidget_(nullptr) {
    setupUI();
}

void SettingsOnLoadFormsWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Создаем таблицу для форм
    pTableWidget_ = new QTableWidget(this);
    pTableWidget_->setColumnCount(2);
    pTableWidget_->setHorizontalHeaderLabels(QStringList() << tr("Выбрать") << tr("Форма"));
    pTableWidget_->setSelectionMode(QAbstractItemView::SingleSelection);
    pTableWidget_->setSelectionBehavior(QAbstractItemView::SelectRows);
    pTableWidget_->horizontalHeader()->setStretchLastSection(true);
    pTableWidget_->verticalHeader()->setVisible(false);
    pTableWidget_->horizontalHeader()->setVisible(false);

    mainLayout->addWidget(pTableWidget_);
    setLayout(mainLayout);

    // Подключаем сигнал изменения элемента таблицы
    connect(pTableWidget_, &QTableWidget::itemChanged,
            this, &SettingsOnLoadFormsWidget::onItemChanged);

    populateForms();
}

void SettingsOnLoadFormsWidget::populateForms() {
    // Отключаем сигналы при заполнении таблицы, чтобы избежать
    // множественных сигналов settingsChanged()
    disconnect(pTableWidget_, &QTableWidget::itemChanged,
               this, &SettingsOnLoadFormsWidget::onItemChanged);

    pTableWidget_->setRowCount(0);

    // Получаем список всех доступных форм
    const Params::_v_forms& v_forms_exists =
        Params::get_instance()->getFormsExists();

    // Получаем список форм, загружаемых при старте
    const Params::_v_forms& v_forms_start_load =
        Params::get_instance()->getStartLoadForms();

    // Заполняем таблицу
    int row_num = 0;
    for (const auto& form_name : v_forms_exists) {
        pTableWidget_->insertRow(row_num);

        // Создаем чекбокс для выбора
        QTableWidgetItem* pItemCheckbox = new QTableWidgetItem();
        pItemCheckbox->setCheckState(Qt::Unchecked);
        pItemCheckbox->setData(Qt::CheckStateRole, Qt::Unchecked);

        // Проверяем, включена ли эта форма в список загрузки при старте
        for (const auto& form_load : v_forms_start_load) {
            if (form_name == form_load) {
                pItemCheckbox->setCheckState(Qt::Checked);
                break;
            }
        }

        pTableWidget_->setItem(row_num, COLUMN_CHECKED, pItemCheckbox);

        // Создаем элемент с именем формы (не редактируемый)
        QTableWidgetItem* pItemName =
            new QTableWidgetItem(QString::fromStdString(form_name));
        pItemName->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        pTableWidget_->setItem(row_num, COLUMN_NAME, pItemName);

        row_num++;
    }

    pTableWidget_->resizeColumnsToContents();

    // Переподключаем сигнал после заполнения
    connect(pTableWidget_, &QTableWidget::itemChanged,
            this, &SettingsOnLoadFormsWidget::onItemChanged);
}

void SettingsOnLoadFormsWidget::onItemChanged(QTableWidgetItem* item) {
    if (item->column() == COLUMN_CHECKED) {
        m_pendingForms.clear();

        for (int row = 0; row < pTableWidget_->rowCount(); ++row) {
            QTableWidgetItem* pItemCheckbox = pTableWidget_->item(row, COLUMN_CHECKED);
            // Если чекбокс отмечен, добавляем форму в список
            if (pItemCheckbox && pItemCheckbox->checkState() == Qt::Checked) {
                QTableWidgetItem* pItemName = pTableWidget_->item(row, COLUMN_NAME);
                if (pItemName) {
                    m_pendingForms.emplace_back(pItemName->text().toStdString());
                }
            }
        }

        m_hasChanges = true;
        emit settingsChanged();
    }
}

void SettingsOnLoadFormsWidget::applyChanges() {
    if (m_hasChanges) {
        Params::get_instance()->setStartLoadForms(m_pendingForms);
        m_hasChanges = false;
    }
}
