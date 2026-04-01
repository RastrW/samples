#include "settingsOnLoadFilesWidget.h"
#include "rastrParameters.h"
#include <QFileDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <spdlog/spdlog.h>

SettingsOnLoadFilesWidget::SettingsOnLoadFilesWidget(QWidget *parent)
    : SettingsStackedItemWidget(parent) {

    setupUI();
}

void SettingsOnLoadFilesWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Кнопки управления
    QHBoxLayout* hLayoutButtons = new QHBoxLayout();

    m_pbAddFiles = new QPushButton(tr("Добавить файлы..."), this);
    m_pbAddFiles->setMaximumWidth(150);
    connect(m_pbAddFiles, &QPushButton::clicked,
            this, &SettingsOnLoadFilesWidget::onAddFilesClicked);

    m_pbRemoveFile = new QPushButton(tr("Удалить выбранный"), this);
    m_pbRemoveFile->setMaximumWidth(150);
    connect(m_pbRemoveFile, &QPushButton::clicked,
            this, &SettingsOnLoadFilesWidget::onRemoveFileClicked);

    hLayoutButtons->addWidget(m_pbAddFiles);
    hLayoutButtons->addWidget(m_pbRemoveFile);
    hLayoutButtons->addStretch();

    mainLayout->addLayout(hLayoutButtons);

    // Таблица выбранных файлов
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(2);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Файл") << tr("Шаблон"));
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->verticalHeader()->setVisible(false);

    connect(m_tableWidget, &QTableWidget::itemDoubleClicked,
            this, &SettingsOnLoadFilesWidget::onTableItemDoubleClicked);

    mainLayout->addWidget(m_tableWidget);

    // Информационная зона
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->addStretch();
    mainLayout->addLayout(infoLayout);

    setLayout(mainLayout);
    populateFiles();
}

void SettingsOnLoadFilesWidget::populateFiles() {
    if (m_isInitialized) {
        return;  // Уже инициализировано, не перезагружаем
    }

    // Загрузить данные из Params только один раз
    const auto& v_files = RastrParameters::get_instance()->getStartLoadFileTemplates();
    m_selectedFiles.clear();
    m_selectedFiles.insert(m_selectedFiles.end(), v_files.begin(), v_files.end());

    refreshTableDisplay();
    m_isInitialized = true;
}

void SettingsOnLoadFilesWidget::refreshTableDisplay() {
    // Отрисовка таблицы без перезагрузки данных
    m_tableWidget->setRowCount(m_selectedFiles.size());

    for (size_t i = 0; i < m_selectedFiles.size(); ++i) {
        const auto& pair = m_selectedFiles[i];

        QTableWidgetItem* pItemFile = new QTableWidgetItem(
            QString::fromStdString(pair.first));
        pItemFile->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_tableWidget->setItem(i, COLUMN_FILE, pItemFile);

        QTableWidgetItem* pItemTemplate = new QTableWidgetItem(
            QString::fromStdString(pair.second));
        pItemTemplate->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                                Qt::ItemIsEditable);
        m_tableWidget->setItem(i, COLUMN_TEMPLATE, pItemTemplate);
    }

    m_tableWidget->resizeColumnsToContents();
}

void SettingsOnLoadFilesWidget::onAddFilesClicked() {
    QStringList selectedFiles = QFileDialog::getOpenFileNames(
        this, tr("Выберите файлы для загрузки"), QString(),
        tr("Все файлы (*);;RG файлы (*.rg2);;OS файлы (*.os);;GRF файлы (*.grf)"));

    if (selectedFiles.isEmpty()) {
        return;
    }

    const auto& v_template_exts = RastrParameters::get_instance()->getTemplateExts();

    // Добавить выбранные файлы
    for (const auto& filePath : selectedFiles) {
        QFileInfo fileInfo(filePath);
        QString fileExtension = fileInfo.suffix();

        std::string templateName;
        bool found = false;

        for (const auto& template_ext : v_template_exts) {
            const std::string ext_with_dot = template_ext.second;
            QString qstrExt = QString::fromStdString(ext_with_dot);

            if (qstrExt.remove(0, 1) == fileExtension) {
                templateName = template_ext.first + template_ext.second;
                found = true;
                break;
            }
        }

        if (!found) {
            templateName = "";
        }

        bool alreadyExists = false;
        for (const auto& existing : m_selectedFiles) {
            if (existing.first == filePath.toStdString()) {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists) {
            m_selectedFiles.emplace_back(filePath.toStdString(), templateName);
            spdlog::info("File added for loading: {}", filePath.toStdString());
        }
    }

    refreshTableDisplay();
    emit settingsChanged();
}

void SettingsOnLoadFilesWidget::onRemoveFileClicked() {
    int currentRow = m_tableWidget->currentRow();

    if (currentRow < 0 || currentRow >= static_cast<int>(m_selectedFiles.size())) {
        return;
    }

    m_selectedFiles.erase(m_selectedFiles.begin() + currentRow);
    spdlog::info("Файл удален из списка, строка: {}", currentRow);

    refreshTableDisplay();
    emit settingsChanged();
}

void SettingsOnLoadFilesWidget::onTableItemDoubleClicked(QTableWidgetItem* item) {
    if (item && item->column() == COLUMN_TEMPLATE) {
        int row = item->row();

        if (row >= 0 && row < static_cast<int>(m_selectedFiles.size())) {
            m_tableWidget->removeCellWidget(row, COLUMN_TEMPLATE);

            QComboBox* pComboBox = new QComboBox(this);

            const auto& v_template_exts = RastrParameters::get_instance()->getTemplateExts();
            pComboBox->addItem("");

            for (const auto& template_ext : v_template_exts) {
                QString qstrTemplate = QString("%1%2")
                .arg(template_ext.first.c_str())
                    .arg(template_ext.second.c_str());
                pComboBox->addItem(qstrTemplate);
            }

            pComboBox->setCurrentText(
                QString::fromStdString(m_selectedFiles[row].second));

            m_tableWidget->setCellWidget(row, COLUMN_TEMPLATE, pComboBox);

            connect(pComboBox, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
                    this, [this, row](const QString& text) {
                        if (row >= 0 && row < static_cast<int>(m_selectedFiles.size())) {
                            m_selectedFiles[row].second = text.toStdString();
                            emit settingsChanged();
                        }
                    });
        }
    }
}

void SettingsOnLoadFilesWidget::applyChanges() {
    RastrParameters::get_instance()->setStartLoadFileTemplates(m_selectedFiles);
}

