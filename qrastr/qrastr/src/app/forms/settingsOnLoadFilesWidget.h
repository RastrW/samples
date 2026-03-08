#pragma once
#include "settingsStackedItemWidget.h"

class QTableWidgetItem;
class QTableWidget;
class QPushButton;

class SettingsOnLoadFilesWidget : public SettingsStackedItemWidget {
    Q_OBJECT

public:
    explicit SettingsOnLoadFilesWidget(QWidget *parent = nullptr);
    ~SettingsOnLoadFilesWidget() = default;

    void applyChanges() override;
private slots:
    /// Обработчик нажатия кнопки добавления файлов
    void onAddFilesClicked();
    /// Обработчик нажатия кнопки удаления выбранного файла
    void onRemoveFileClicked();
    /// Обработчик двойного клика на строку таблицы (редактирование шаблона)
    void onTableItemDoubleClicked(QTableWidgetItem* item);
private:
    void setupUI();
    void populateFiles();       // Только при инициализации
    void refreshTableDisplay(); // Только отрисовка

    QTableWidget* m_tableWidget {nullptr};
    QPushButton* m_pbAddFiles {nullptr};
    QPushButton* m_pbRemoveFile {nullptr};

    using FileTemplatePair = std::pair<std::string, std::string>;
    std::vector<FileTemplatePair> m_selectedFiles;

    bool m_isInitialized {false};

    static constexpr const int COLUMN_FILE = 0;
    static constexpr const int COLUMN_TEMPLATE = 1;
};
