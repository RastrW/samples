#pragma once

#include <QDialog>

class QTreeWidgetItem;
class QAstra;
class QTreeWidget;
class QStackedWidget;
class SettingsStackedItemWidget;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    bool init();
    void closeEvent(QCloseEvent *event) override;

public slots:
    void onBtnApplyClick();

private slots:
    void onTreeItemClicked(QTreeWidgetItem *item, int column);

private:
    // Добавить элемент в дерево и связать с виджетом
    QTreeWidgetItem* addTreePage(
        QTreeWidgetItem* parent,
        const QString& caption,
        QWidget* widget);

    void setAppSettingsChanged();
    void setupUI();

    QPushButton* m_pbApplySettings {nullptr};
    QTreeWidget* m_twSections {nullptr};
    QStackedWidget* m_sw {nullptr};
    bool m_settingsChanged {false};

    std::unordered_map<QTreeWidgetItem*, QWidget*>
        m_itemToWidget;

    // Кэш всех создаваемых виджетов
    std::vector<SettingsStackedItemWidget*>
        m_settingWidgets;
};
