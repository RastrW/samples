#include "formsettings.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QCloseEvent>

#include <filesystem>
#include "qastra.h"
#include "params.h"
#include "formsettingsdatas.h"
#include "formsettingsonloadfiles.h"
#include "formsettingsonloadtemplates.h"
#include "formsettingsonloadforms.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(tr("Настройки"));
    resize(900, 600);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                   Qt::WindowCloseButtonHint);
    setWindowModality(Qt::ApplicationModal);
    setupUI();
}

void SettingsDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    m_twSections = new QTreeWidget(this);
    m_twSections->setHeaderLabels(QStringList() << tr("Настройки"));
    splitter->addWidget(m_twSections);

    m_sw = new QStackedWidget(this);
    splitter->addWidget(m_sw);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    layout->addWidget(splitter);

    QHBoxLayout* hLayoutButtons = new QHBoxLayout();
    hLayoutButtons->addStretch();

    m_pbApplySettings = new QPushButton(tr("Применить"), this);
    m_pbApplySettings->setMinimumWidth(100);
    m_pbApplySettings->setEnabled(false);
    connect(m_pbApplySettings, &QPushButton::clicked,
            this, &SettingsDialog::onBtnApplyClick);
    hLayoutButtons->addWidget(m_pbApplySettings);

    layout->addLayout(hLayoutButtons);
    setLayout(layout);

    connect(m_twSections, &QTreeWidget::clicked,
            this, &SettingsDialog::onTreeItemClicked);
}

SettingsDialog::~SettingsDialog(){
    spdlog::info("Delete object FormSettings");
}

bool SettingsDialog::init(const std::shared_ptr<QAstra>& sp_qastra) {
    m_qAstra = sp_qastra;

    if (!Params::get_instance()->readTemplates()) {
        spdlog::error("Не удалось прочитать шаблоны");
        return false;
    }

    // Создать все виджеты
    auto* p_data_settings = new DataSettingsWidget(this);
    auto* p_templates = new SettingsOnLoadTemplatesWidget(this);
    auto* p_forms = new SettingsOnLoadFormsWidget(this);
    auto* p_files = new SettingsOnLoadFilesWidget(this);

    // Сохранить для позднейшего вызова applyChanges()
    m_settingWidgets.emplace_back(p_data_settings);
    m_settingWidgets.emplace_back(p_templates);
    m_settingWidgets.emplace_back(p_forms);
    m_settingWidgets.emplace_back(p_files);

    m_sw->addWidget(p_data_settings);
    m_sw->addWidget(p_templates);
    m_sw->addWidget(p_forms);
    m_sw->addWidget(p_files);

    // Подключить сигналы от всех виджетов
    for (auto* widget : m_settingWidgets) {
        connect(widget, &SettingsStackedItemWidget::settingsChanged,
                this, &SettingsDialog::setAppSettingsChanged);
    }
    // Заглушки для разделов без виджетов
    QWidget* w_protocol = new QWidget(this);
    QWidget* w_modules = new QWidget(this);
    m_sw->addWidget(w_protocol);
    m_sw->addWidget(w_modules);

    // Построить дерево и связать с виджетами

    // Получить корневой элемент дерева
    QTreeWidgetItem* root = m_twSections->invisibleRootItem();

    // Уровень 1: "Данные"
    auto* item_datas = addTreePage(root, tr("Данные"),
                                   p_data_settings);

    // Уровень 1: "Протокол"
    auto* item_protocol = addTreePage(root, tr("Протокол"),
                                      w_protocol);

    // Уровень 1: "Загружаемые при старте"
    auto* item_on_start = addTreePage(root, tr("Загружаемые при старте"), nullptr);

    // Уровень 2 ("Загружаемые при старте")
    addTreePage(item_on_start, tr("Шаблоны"),
                p_templates);
    addTreePage(item_on_start, tr("Формы"),
                p_forms);
    addTreePage(item_on_start, tr("Файлы"),
                p_files);

    // Уровень 1: "Модули"
    auto* item_modules = addTreePage(root, tr("Модули"),
                                     w_modules);

    // Установить первый виджет по умолчанию
    if (!m_itemToWidget.isEmpty()) {
        m_sw->setCurrentWidget(m_itemToWidget.first());
    }

    return true;
}

QTreeWidgetItem* SettingsDialog::addTreePage(
    QTreeWidgetItem* parent,
    const QString& caption,
    QWidget* widget) {

    QTreeWidgetItem* item = nullptr;

    if (parent == nullptr) {
        // Если родителя нет, добавить как корневой элемент
        item = new QTreeWidgetItem(m_twSections);
    } else {
        // Иначе добавить как дочерний элемент
        item = new QTreeWidgetItem(parent);
    }

    item->setText(0, caption);

    // Если виджет указан, добавить его в map
    if (widget != nullptr) {
        m_itemToWidget[item] = widget;

        // проверить, что виджет действительно в стеке
        if (m_sw->indexOf(widget) == -1) {
            spdlog::error("Warning: Widget не добавлен в QStackedWidget: {}", caption.toStdString());
        }
    }

    return item;
}

void SettingsDialog::onTreeItemClicked(const QModelIndex &index) {
    QTreeWidgetItem* item = m_twSections->itemFromIndex(index);

    if (item == nullptr) {
        return;
    }

    // Получить виджет из map
    if (m_itemToWidget.contains(item)) {
        QWidget* widget = m_itemToWidget[item];

        // Проверить, что виджет действительно в стеке
        int widgetIndex = m_sw->indexOf(widget);
        if (widgetIndex != -1) {
            m_sw->setCurrentWidget(widget);
        } else {
            spdlog::error("Error: Widget отсутствует в QStackedWidget: {}",
                          item->text(0).toStdString());
        }
    }
}

void SettingsDialog::setAppSettingsChanged() {
    m_settingsChanged = true;
    m_pbApplySettings->setEnabled(true);
}

void SettingsDialog::onBtnApplyClick() {
    // Применить изменения со всех виджетов
    for (auto* widget : m_settingWidgets) {
        if (widget != nullptr) {
            widget->applyChanges();
        }
    }

    Params* p_params = Params::get_instance();
    const std::filesystem::path path_appsettings = p_params->getFileAppsettings();

    if (std::filesystem::exists(path_appsettings)) {
        QMessageBox msgBox(this);
        msgBox.setText(tr("Файл существует. Перезаписать?"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::No);

        if (QMessageBox::Yes == msgBox.exec()) {
            // Создать резервную копию
            std::filesystem::path path_appsettings_backup = path_appsettings;
            path_appsettings_backup.replace_filename(
                path_appsettings.stem().string() + "_backup" +
                path_appsettings.extension().string());

            try {
                std::filesystem::copy(path_appsettings, path_appsettings_backup,
                                      std::filesystem::copy_options::overwrite_existing);
                spdlog::info("Создана резервная копия: {}", path_appsettings_backup.string());
            } catch (const std::exception& e) {
                spdlog::error("Не удалось создать резервную копию: {}", e.what());
                QMessageBox::warning(this, tr("Ошибка"),
                                     tr("Не удалось создать резервную копию"));
                return;
            }

            if (p_params->writeJsonFile(path_appsettings.string())) {
                m_settingsChanged = false;
                m_pbApplySettings->setEnabled(false);
                spdlog::info("Настройки успешно сохранены");
            } else {
                QMessageBox::warning(this, tr("Ошибка"),
                                     tr("Не удалось сохранить настройки"));
                spdlog::error("Не удалось сохранить настройки");
            }
        }
    } else {
        spdlog::error("Файл настроек не найден: {}", path_appsettings.string());
        QMessageBox::warning(this, tr("Ошибка"),
                             tr("Файл настроек не найден"));
    }
}

void SettingsDialog::closeEvent(QCloseEvent *event) {
    if (event->spontaneous()) {
        if (m_settingsChanged) {
            QMessageBox msgBox(this);
            msgBox.setText(tr("Настройки были изменены, но не сохранены. "
                              "Закрыть окно без сохранения?"));
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);

            if (msgBox.exec() != QMessageBox::Yes) {
                event->ignore();
                return;
            }
        }
        event->accept();
    } else {
        QWidget::closeEvent(event);
    }
}
