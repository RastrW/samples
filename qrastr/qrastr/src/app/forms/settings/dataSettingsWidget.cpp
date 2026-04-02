#include "dataSettingsWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QSettings>
#include <spdlog/spdlog.h>
#include "settingsKeys.h"

DataSettingsWidget::DataSettingsWidget(QWidget *parent)
    : SettingsStackedItemWidget(parent) {
    setupUI();
}

void DataSettingsWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // ========== Группа "Обслуживание пользовательского интерфейса" ==========
    pGroupBoxMaintenance_ = new QGroupBox(tr("Обслуживание пользовательского интерфейса"), this);
    QVBoxLayout* maintenanceLayout = new QVBoxLayout(pGroupBoxMaintenance_);
    maintenanceLayout->setContentsMargins(10, 15, 10, 10);
    maintenanceLayout->setSpacing(10);

    // Количество элементов в "Последние"
    QHBoxLayout* hLayoutNumItems = new QHBoxLayout();
    QLabel* labelNumItems = new QLabel(tr("Количество элементов в меню 'Последние':"), this);
    QSettings s;
    int maxAllowed = s.value(SK::Files::maxRecentFiles,
                             SK::Files::defMaxRecent).toInt();

    pSpinBoxNumItems_ = new QSpinBox(this);
    pSpinBoxNumItems_->setMinimum(1);
    pSpinBoxNumItems_->setMaximum(100);
    pSpinBoxNumItems_->setValue(maxAllowed);
    pSpinBoxNumItems_->setMinimumWidth(80);

    hLayoutNumItems->addWidget(labelNumItems);
    hLayoutNumItems->addWidget(pSpinBoxNumItems_);
    hLayoutNumItems->addStretch();

    maintenanceLayout->addLayout(hLayoutNumItems);

    // Кнопка восстановления директории пользователя
    QHBoxLayout* hLayoutRestore = new QHBoxLayout();
    pPushButtonRestore_ = new QPushButton(tr("Восстановить"), this);
    pPushButtonRestore_->setMaximumWidth(120);
    QLabel* labelRestore = new QLabel(tr("Восстановить директорию пользователя"), this);

    hLayoutRestore->addWidget(pPushButtonRestore_);
    hLayoutRestore->addWidget(labelRestore);
    hLayoutRestore->addStretch();

    maintenanceLayout->addLayout(hLayoutRestore);

    mainLayout->addWidget(pGroupBoxMaintenance_);

    // ========== Группа "Настройка истории" ==========
    pGroupBoxHistory_ = new QGroupBox(tr("Настройка истории"), this);
    QVBoxLayout* historyLayout = new QVBoxLayout(pGroupBoxHistory_);
    historyLayout->setContentsMargins(10, 15, 10, 10);
    historyLayout->setSpacing(10);

    // Чекбокс "Запоминать режим Истории"
    pCheckBoxRememberHistory_ = new QCheckBox(tr("Запоминать режим Истории"), this);
    pCheckBoxRememberHistory_->setChecked(false);
    historyLayout->addWidget(pCheckBoxRememberHistory_);

    // Путь к XML протоколу
    QLabel* labelXmlPath = new QLabel(tr("Путь к XML протоколу:"), this);
    QHBoxLayout* hLayoutXmlPath = new QHBoxLayout();

    pLineEditXmlPath_ = new QLineEdit(this);
    pLineEditXmlPath_->setReadOnly(true);
    pLineEditXmlPath_->setPlaceholderText(tr("Выберите путь к файлу XML протокола..."));

    pPushButtonSelectXmlPath_ = new QPushButton(tr("Обзор..."), this);
    pPushButtonSelectXmlPath_->setMaximumWidth(100);

    hLayoutXmlPath->addWidget(pLineEditXmlPath_);
    hLayoutXmlPath->addWidget(pPushButtonSelectXmlPath_);

    historyLayout->addWidget(labelXmlPath);
    historyLayout->addLayout(hLayoutXmlPath);

    mainLayout->addWidget(pGroupBoxHistory_);

    // Растягивающий элемент в конец
    mainLayout->addStretch();

    setLayout(mainLayout);

    // ========== Подключение сигналов ==========
    connect(pPushButtonRestore_, &QPushButton::clicked,
            this, &DataSettingsWidget::onRestoreUserDirectoryClicked);

    connect(pPushButtonSelectXmlPath_, &QPushButton::clicked,
            this, &DataSettingsWidget::onSelectXmlProtocolPath);

    connect(pSpinBoxNumItems_, QOverload<int>::of(&QSpinBox::valueChanged),
            [=](int newValue){
                m_hasChanges = true;
                m_maxRecent = newValue;
                emit settingsChanged();
            });
}

void DataSettingsWidget::onRestoreUserDirectoryClicked() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Подтверждение"));
    msgBox.setText(tr("Вы запросили восстановление рабочей области \"RastrWin3\". "
                      "После выполнения этой операции все файлы шаблонов, форм, макросов "
                      "и других настроек будут восстановлены из резервной копии. "
                      "Любые изменения, которые Вы сделали в этих файла будут потеряны. "
                      "Пользовательские файлы, которые были созданы Вами (например файлы режима и графики) затронуты не будут. "
                      "Чтобы отказаться от восстановления рабочей области нажмите кнопку \"Отменить\". "
                      "Продолжить?"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        // Логика восстановления директории
        spdlog::info("Директория пользователя восстановлена");
        emit settingsChanged();
    }
}

void DataSettingsWidget::onSelectXmlProtocolPath() {
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Выберите XML файл протокола"),
                                                    QString(),
                                                    tr("XML файлы (*.xml);;Все файлы (*)"));

    if (!filePath.isEmpty()) {
        pLineEditXmlPath_->setText(filePath);
        m_hasChanges = true;
        spdlog::info("Выбран путь к XML-протоколу: {}", filePath.toStdString());
    }
}

void DataSettingsWidget::applyChanges() {
    if (m_hasChanges) {
        QSettings s;
        s.setValue(SK::Files::maxRecentFiles,
                   SK::Files::defMaxRecent);
        m_hasChanges = false;
    }
}
